#ifndef _FEATURES_SPACE_DESERIALIZE_H
#define _FEATURES_SPACE_DESERIALIZE_H
#include "context_deserialize.h"

static inline ccs_result_t
_ccs_deserialize_bin_features_space(
		ccs_features_space_t               *features_space_ret,
		uint32_t                            version,
		size_t                             *buffer_size,
		const char                        **buffer,
		_ccs_object_deserialize_options_t  *opts) {
	_ccs_object_deserialize_options_t new_opts = *opts;
	new_opts.map_values = CCS_FALSE;
	new_opts.handle_map = NULL;
	_ccs_object_internal_t obj;
	ccs_object_t handle;
	ccs_result_t res = CCS_SUCCESS;
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_object_internal(
		&obj, buffer_size, buffer, &handle));
	if (CCS_UNLIKELY(obj.type != CCS_FEATURES_SPACE))
		return -CCS_INVALID_TYPE;

	_ccs_context_data_mock_t data;
	CCS_VALIDATE_ERR_GOTO(res, _ccs_deserialize_bin_ccs_context_data(
		&data, version, buffer_size, buffer, &new_opts), end);
	CCS_VALIDATE_ERR_GOTO(res, ccs_create_features_space(
		data.name, NULL, features_space_ret), end);
	CCS_VALIDATE_ERR_GOTO(res, ccs_features_space_add_hyperparameters(
		*features_space_ret, data.num_hyperparameters, data.hyperparameters),
		err_features_space);
	if (opts && opts->map_values && opts->handle_map)
		CCS_VALIDATE_ERR_GOTO(res,
			_ccs_object_handle_check_add(
				opts->handle_map, handle,
				(ccs_object_t)*features_space_ret),
			err_features_space);
	goto end;
err_features_space:
	ccs_release_object(*features_space_ret);
	*features_space_ret = NULL;
end:
	if (data.hyperparameters) {
		for (size_t i = 0; i < data.num_hyperparameters; i++)
			if(data.hyperparameters[i])
				ccs_release_object(data.hyperparameters[i]);
		free(data.hyperparameters);
	}
	return res;
}

static ccs_result_t
_ccs_features_space_deserialize(
		ccs_features_space_t              *features_space_ret,
		ccs_serialize_format_t             format,
		uint32_t                           version,
		size_t                            *buffer_size,
		const char                       **buffer,
		_ccs_object_deserialize_options_t *opts) {
	switch(format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE(_ccs_deserialize_bin_features_space(
			features_space_ret, version, buffer_size, buffer, opts));
		break;
	default:
		return -CCS_INVALID_VALUE;
	}
	return CCS_SUCCESS;
}

#endif //_FEATURES_SPACE_DESERIALIZE_H
