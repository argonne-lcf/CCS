#ifndef _EVALUATION_DESERIALIZE_H
#define _EVALUATION_DESERIALIZE_H
#include "cconfigspace_internal.h"
#include "cconfigspace_json.h"
#include "evaluation_internal.h"
#include "configuration_deserialize.h"

struct _ccs_evaluation_data_mock_s {
	_ccs_binding_data_t        base;
	ccs_search_configuration_t configuration;
	ccs_evaluation_result_t    result;
};
typedef struct _ccs_evaluation_data_mock_s _ccs_evaluation_data_mock_t;

static inline ccs_result_t
_ccs_deserialize_bin_ccs_evaluation_data(
	_ccs_evaluation_data_mock_t       *data,
	uint32_t                           version,
	size_t                            *buffer_size,
	const char                       **buffer,
	_ccs_object_deserialize_options_t *opts)
{
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_binding_data(
		&data->base, version, buffer_size, buffer));
	CCS_VALIDATE(_ccs_object_deserialize_with_opts(
		(ccs_object_t *)&data->configuration,
		CCS_SERIALIZE_FORMAT_BINARY, version, buffer_size, buffer,
		opts));
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_evaluation_result(
		&data->result, buffer_size, buffer));
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_deserialize_bin_ccs_evaluation(
	ccs_evaluation_t                  *evaluation_ret,
	uint32_t                           version,
	size_t                            *buffer_size,
	const char                       **buffer,
	_ccs_object_deserialize_options_t *opts)
{
	CCS_CHECK_OBJ(opts->handle_map, CCS_OBJECT_TYPE_MAP);
	_ccs_object_deserialize_options_t new_opts = *opts;
	ccs_datum_t                       d;
	ccs_objective_space_t             os;
	ccs_evaluation_t                  evaluation;
	ccs_result_t                      res = CCS_RESULT_SUCCESS;

	new_opts.map_values                   = CCS_FALSE;
	_ccs_evaluation_data_mock_t data      = {
                {NULL, 0, NULL}, NULL, CCS_RESULT_SUCCESS};
	CCS_VALIDATE_ERR_GOTO(
		res,
		_ccs_deserialize_bin_ccs_evaluation_data(
			&data, version, buffer_size, buffer, &new_opts),
		end);

	CCS_VALIDATE_ERR_GOTO(
		res,
		ccs_map_get(opts->handle_map, ccs_object(data.base.context), &d),
		end);
	CCS_REFUTE_ERR_GOTO(
		res, d.type != CCS_DATA_TYPE_OBJECT,
		CCS_RESULT_ERROR_INVALID_HANDLE, end);
	os = (ccs_objective_space_t)(d.value.o);

	CCS_VALIDATE_ERR_GOTO(
		res,
		ccs_create_evaluation(
			os, data.configuration, data.result,
			data.base.num_values, data.base.values, &evaluation),
		end);

	*evaluation_ret = evaluation;
end:
	if (data.configuration)
		ccs_release_object(data.configuration);
	if (data.base.values)
		free(data.base.values);
	return res;
}

static inline ccs_result_t
_ccs_deserialize_json_ccs_evaluation(
	ccs_evaluation_t                  *evaluation_ret,
	uint32_t                           version,
	size_t                            *buffer_size,
	const char                       **buffer,
	_ccs_object_deserialize_options_t *opts)
{
	ccs_datum_t                d;
	ccs_objective_space_t      os;
	ccs_evaluation_t           evaluation;
	ccs_result_t               res           = CCS_RESULT_SUCCESS;
	_ccs_binding_data_t        data          = {NULL, 0, NULL};
	ccs_search_configuration_t configuration = NULL;
	ccs_evaluation_result_t    result        = CCS_RESULT_SUCCESS;
	cJSON                     *json;
	cJSON                     *j_conf;
	cJSON                     *j_result;
	const char                *cbuf;
	size_t                     dummy;
	ccs_int_t                  result_val;

	(void)version;
	(void)buffer_size;
	CCS_CHECK_OBJ(opts->handle_map, CCS_OBJECT_TYPE_MAP);

	json = *(cJSON **)buffer;
	CCS_VALIDATE_ERR_GOTO(
		res, _ccs_deserialize_json_ccs_binding_data(&data, json), end);

	CCS_VALIDATE_ERR_GOTO(
		res,
		ccs_map_get(opts->handle_map, ccs_object(data.context), &d),
		end);
	CCS_REFUTE_ERR_GOTO(
		res, d.type != CCS_DATA_TYPE_OBJECT,
		CCS_RESULT_ERROR_INVALID_HANDLE, end);
	os     = (ccs_objective_space_t)(d.value.o);

	/* configuration */
	j_conf = cJSON_GetObjectItemCaseSensitive(json, "configuration");
	CCS_REFUTE_ERR_GOTO(
		res, !j_conf || !cJSON_IsObject(j_conf),
		CCS_RESULT_ERROR_INVALID_VALUE, end);
	{
		_ccs_object_deserialize_options_t conf_opts = *opts;
		conf_opts.map_values                        = CCS_FALSE;
		cbuf  = (const char *)j_conf;
		dummy = 0;
		CCS_VALIDATE_ERR_GOTO(
			res,
			_ccs_object_deserialize_with_opts(
				(ccs_object_t *)&configuration,
				CCS_SERIALIZE_FORMAT_JSON, version, &dummy,
				&cbuf, &conf_opts),
			end);
	}

	/* result */
	j_result = cJSON_GetObjectItemCaseSensitive(json, "result");
	CCS_REFUTE_ERR_GOTO(
		res, !j_result, CCS_RESULT_ERROR_INVALID_VALUE, end);
	CCS_VALIDATE_ERR_GOTO(
		res, _ccs_json_get_int(j_result, &result_val), end);
	result = (ccs_evaluation_result_t)result_val;

	CCS_VALIDATE_ERR_GOTO(
		res,
		ccs_create_evaluation(
			os, configuration, result, data.num_values, data.values,
			&evaluation),
		end);

	*evaluation_ret = evaluation;
end:
	if (configuration)
		ccs_release_object(configuration);
	if (data.values)
		free(data.values);
	return res;
}

static ccs_result_t
_ccs_evaluation_deserialize(
	ccs_evaluation_t                  *evaluation_ret,
	ccs_serialize_format_t             format,
	uint32_t                           version,
	size_t                            *buffer_size,
	const char                       **buffer,
	_ccs_object_deserialize_options_t *opts)
{
	switch (format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE(_ccs_deserialize_bin_ccs_evaluation(
			evaluation_ret, version, buffer_size, buffer, opts));
		break;
	case CCS_SERIALIZE_FORMAT_JSON:
		CCS_VALIDATE(_ccs_deserialize_json_ccs_evaluation(
			evaluation_ret, version, buffer_size, buffer, opts));
		break;
	default:
		CCS_RAISE(
			CCS_RESULT_ERROR_INVALID_VALUE,
			"Unsupported serialization format: %d", format);
	}
	return CCS_RESULT_SUCCESS;
}

#endif //_EVALUATION_DESERIALIZE_H
