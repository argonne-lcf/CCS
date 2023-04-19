#ifndef _TREE_EVALUATION_DESERIALIZE_H
#define _TREE_EVALUATION_DESERIALIZE_H
#include "cconfigspace_internal.h"
#include "tree_evaluation_internal.h"
#include "tree_configuration_deserialize.h"

struct _ccs_tree_evaluation_data_mock_s {
	_ccs_binding_data_t      base;
	ccs_tree_configuration_t configuration;
	ccs_result_t             error;
};
typedef struct _ccs_tree_evaluation_data_mock_s _ccs_tree_evaluation_data_mock_t;

static inline ccs_error_t
_ccs_deserialize_bin_ccs_tree_evaluation_data(
	_ccs_tree_evaluation_data_mock_t  *data,
	uint32_t                           version,
	size_t                            *buffer_size,
	const char                       **buffer,
	_ccs_object_deserialize_options_t *opts)
{
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_binding_data(
		&data->base, version, buffer_size, buffer));
	CCS_VALIDATE(_ccs_tree_configuration_deserialize(
		&data->configuration, CCS_SERIALIZE_FORMAT_BINARY, version,
		buffer_size, buffer, opts));
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_result(
		&data->error, buffer_size, buffer));
	return CCS_SUCCESS;
}

static inline ccs_error_t
_ccs_deserialize_bin_ccs_tree_evaluation(
	ccs_tree_evaluation_t             *evaluation_ret,
	uint32_t                           version,
	size_t                            *buffer_size,
	const char                       **buffer,
	_ccs_object_deserialize_options_t *opts)
{
	CCS_CHECK_OBJ(opts->handle_map, CCS_MAP);
	_ccs_object_deserialize_options_t new_opts = *opts;
	_ccs_object_internal_t            obj;
	ccs_object_t                      handle;
	ccs_datum_t                       d;
	ccs_objective_space_t             os;
	ccs_error_t                       res = CCS_SUCCESS;
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_object_internal(
		&obj, buffer_size, buffer, &handle));
	CCS_REFUTE(obj.type != CCS_TREE_EVALUATION, CCS_INVALID_TYPE);

	new_opts.map_values                   = CCS_FALSE;
	_ccs_tree_evaluation_data_mock_t data = {
		{NULL, 0, NULL}, NULL, CCS_SUCCESS};
	CCS_VALIDATE_ERR_GOTO(
		res,
		_ccs_deserialize_bin_ccs_tree_evaluation_data(
			&data, version, buffer_size, buffer, &new_opts),
		end);

	CCS_VALIDATE_ERR_GOTO(
		res,
		ccs_map_get(opts->handle_map, ccs_object(data.base.context), &d),
		end);
	CCS_REFUTE_ERR_GOTO(res, d.type != CCS_OBJECT, CCS_INVALID_HANDLE, end);
	os = (ccs_objective_space_t)(d.value.o);

	CCS_VALIDATE_ERR_GOTO(
		res,
		ccs_create_tree_evaluation(
			os, data.configuration, data.error,
			data.base.num_values, data.base.values, evaluation_ret),
		end);

	if (opts->map_values)
		CCS_VALIDATE_ERR_GOTO(
			res,
			_ccs_object_handle_check_add(
				opts->handle_map, handle,
				(ccs_object_t)*evaluation_ret),
			err_evaluation);
	goto end;

err_evaluation:
	ccs_release_object(*evaluation_ret);
	*evaluation_ret = NULL;
end:
	if (data.configuration)
		ccs_release_object(data.configuration);
	if (data.base.values)
		free(data.base.values);
	return res;
}

static ccs_error_t
_ccs_tree_evaluation_deserialize(
	ccs_tree_evaluation_t             *evaluation_ret,
	ccs_serialize_format_t             format,
	uint32_t                           version,
	size_t                            *buffer_size,
	const char                       **buffer,
	_ccs_object_deserialize_options_t *opts)
{
	switch (format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE(_ccs_deserialize_bin_ccs_tree_evaluation(
			evaluation_ret, version, buffer_size, buffer, opts));
		break;
	default:
		CCS_RAISE(
			CCS_INVALID_VALUE,
			"Unsupported serialization format: %d", format);
	}
	CCS_VALIDATE(_ccs_object_deserialize_user_data(
		(ccs_object_t)*evaluation_ret, format, version, buffer_size,
		buffer, opts));
	return CCS_SUCCESS;
}

#endif //_TREE_EVALUATION_DESERIALIZE_H
