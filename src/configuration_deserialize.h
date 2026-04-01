#ifndef _CONFIGURATION_DESERIALIZE_H
#define _CONFIGURATION_DESERIALIZE_H
#include "configuration_internal.h"
#include "cconfigspace_json.h"

struct _ccs_configuration_data_mock_s {
	_ccs_binding_data_t base;
	ccs_bool_t          features_present;
	ccs_features_t      features;
};
typedef struct _ccs_configuration_data_mock_s _ccs_configuration_data_mock_t;

static inline ccs_result_t
_ccs_deserialize_bin_ccs_configuration_data(
	_ccs_configuration_data_mock_t    *data,
	uint32_t                           version,
	size_t                            *buffer_size,
	const char                       **buffer,
	_ccs_object_deserialize_options_t *opts)
{
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_binding_data(
		&data->base, version, buffer_size, buffer));
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_bool(
		&data->features_present, buffer_size, buffer));
	if (data->features_present)
		CCS_VALIDATE(_ccs_object_deserialize_with_opts_check(
			(ccs_object_t *)&data->features,
			CCS_OBJECT_TYPE_FEATURES, CCS_SERIALIZE_FORMAT_BINARY,
			version, buffer_size, buffer, opts));
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_deserialize_bin_configuration(
	ccs_configuration_t               *configuration_ret,
	uint32_t                           version,
	size_t                            *buffer_size,
	const char                       **buffer,
	_ccs_object_deserialize_options_t *opts)
{
	CCS_CHECK_OBJ(opts->handle_map, CCS_OBJECT_TYPE_MAP);
	_ccs_object_deserialize_options_t new_opts = *opts;
	ccs_datum_t                       d;
	ccs_configuration_space_t         cs;
	ccs_configuration_t               configuration;
	ccs_result_t                      res = CCS_RESULT_SUCCESS;

	new_opts.map_values                   = CCS_FALSE;
	_ccs_configuration_data_mock_t data   = {
                {NULL, 0, NULL}, CCS_FALSE, NULL};
	CCS_VALIDATE_ERR_GOTO(
		res,
		_ccs_deserialize_bin_ccs_configuration_data(
			&data, version, buffer_size, buffer, &new_opts),
		end);

	CCS_VALIDATE_ERR_GOTO(
		res,
		ccs_map_get(opts->handle_map, ccs_object(data.base.context), &d),
		end);
	CCS_REFUTE_ERR_GOTO(
		res, d.type != CCS_DATA_TYPE_OBJECT,
		CCS_RESULT_ERROR_INVALID_HANDLE, end);
	cs = (ccs_configuration_space_t)(d.value.o);

	CCS_VALIDATE_ERR_GOTO(
		res,
		ccs_create_configuration(
			cs, data.features, data.base.num_values,
			data.base.values, &configuration),
		end);

	*configuration_ret = configuration;
	goto end;
end:
	if (data.features)
		ccs_release_object(data.features);
	if (data.base.values)
		free(data.base.values);
	return res;
}

static inline ccs_result_t
_ccs_deserialize_json_configuration(
	ccs_configuration_t               *configuration_ret,
	uint32_t                           version,
	size_t                            *buffer_size,
	const char                       **buffer,
	_ccs_object_deserialize_options_t *opts)
{
	ccs_datum_t               d;
	ccs_configuration_space_t cs;
	ccs_configuration_t       configuration;
	ccs_result_t              res      = CCS_RESULT_SUCCESS;
	_ccs_binding_data_t       data     = {NULL, 0, NULL};
	ccs_features_t            features = NULL;
	cJSON                    *json;
	cJSON                    *j_features;
	const char               *cbuf;
	size_t                    dummy;

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
	cs         = (ccs_configuration_space_t)(d.value.o);

	/* optional features */
	j_features = cJSON_GetObjectItemCaseSensitive(json, "features");
	if (j_features && cJSON_IsObject(j_features)) {
		_ccs_object_deserialize_options_t feat_opts = *opts;
		feat_opts.map_values                        = CCS_FALSE;
		cbuf  = (const char *)j_features;
		dummy = 0;
		CCS_VALIDATE_ERR_GOTO(
			res,
			_ccs_object_deserialize_with_opts_check(
				(ccs_object_t *)&features,
				CCS_OBJECT_TYPE_FEATURES,
				CCS_SERIALIZE_FORMAT_JSON, version, &dummy,
				&cbuf, &feat_opts),
			end);
	}

	CCS_VALIDATE_ERR_GOTO(
		res,
		ccs_create_configuration(
			cs, features, data.num_values, data.values,
			&configuration),
		end);

	*configuration_ret = configuration;
end:
	if (features)
		ccs_release_object(features);
	if (data.values)
		free(data.values);
	return res;
}

static ccs_result_t
_ccs_configuration_deserialize(
	ccs_configuration_t               *configuration_ret,
	ccs_serialize_format_t             format,
	uint32_t                           version,
	size_t                            *buffer_size,
	const char                       **buffer,
	_ccs_object_deserialize_options_t *opts)
{
	switch (format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE(_ccs_deserialize_bin_configuration(
			configuration_ret, version, buffer_size, buffer, opts));
		break;
	case CCS_SERIALIZE_FORMAT_JSON:
		CCS_VALIDATE(_ccs_deserialize_json_configuration(
			configuration_ret, version, buffer_size, buffer, opts));
		break;
	default:
		CCS_RAISE(
			CCS_RESULT_ERROR_INVALID_VALUE,
			"Unsupported serialization format: %d", format);
	}
	return CCS_RESULT_SUCCESS;
}

#endif //_CONFIGURATION_DESERIALIZE_H
