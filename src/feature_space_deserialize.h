#ifndef _FEATURE_SPACE_DESERIALIZE_H
#define _FEATURE_SPACE_DESERIALIZE_H
#include "context_deserialize.h"

static inline ccs_result_t
_ccs_deserialize_bin_feature_space(
	ccs_feature_space_t               *feature_space_ret,
	uint32_t                           version,
	size_t                            *buffer_size,
	const char                       **buffer,
	_ccs_object_deserialize_options_t *opts)
{
	_ccs_object_deserialize_options_t new_opts = *opts;
	new_opts.map_values                        = CCS_FALSE;
	new_opts.handle_map                        = NULL;
	_ccs_object_internal_t obj;
	ccs_object_t           handle;
	ccs_result_t           res = CCS_RESULT_SUCCESS;
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_object_internal(
		&obj, buffer_size, buffer, &handle));
	CCS_REFUTE(
		obj.type != CCS_OBJECT_TYPE_FEATURE_SPACE,
		CCS_RESULT_ERROR_INVALID_TYPE);

	_ccs_context_data_mock_t data;
	CCS_VALIDATE_ERR_GOTO(
		res,
		_ccs_deserialize_bin_ccs_context_data(
			&data, version, buffer_size, buffer, &new_opts),
		end);
	CCS_VALIDATE_ERR_GOTO(
		res,
		ccs_create_feature_space(
			data.name, data.num_parameters, data.parameters,
			feature_space_ret),
		end);
	if (opts && opts->map_values && opts->handle_map)
		CCS_VALIDATE_ERR_GOTO(
			res,
			_ccs_object_handle_check_add(
				opts->handle_map, handle,
				(ccs_object_t)*feature_space_ret),
			err_feature_space);
	goto end;
err_feature_space:
	ccs_release_object(*feature_space_ret);
	*feature_space_ret = NULL;
end:
	if (data.parameters) {
		for (size_t i = 0; i < data.num_parameters; i++)
			if (data.parameters[i])
				ccs_release_object(data.parameters[i]);
		free(data.parameters);
	}
	return res;
}

static ccs_result_t
_ccs_feature_space_deserialize(
	ccs_feature_space_t               *feature_space_ret,
	ccs_serialize_format_t             format,
	uint32_t                           version,
	size_t                            *buffer_size,
	const char                       **buffer,
	_ccs_object_deserialize_options_t *opts)
{
	switch (format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE(_ccs_deserialize_bin_feature_space(
			feature_space_ret, version, buffer_size, buffer, opts));
		break;
	default:
		CCS_RAISE(
			CCS_RESULT_ERROR_INVALID_VALUE,
			"Unsupported serialization format: %d", format);
	}
	return CCS_RESULT_SUCCESS;
}

#endif //_FEATURE_SPACE_DESERIALIZE_H
