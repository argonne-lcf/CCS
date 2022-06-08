#ifndef _CONFIGURATION_DESERIALIZE_H
#define _CONFIGURATION_DESERIALIZE_H
#include "cconfigspace_internal.h"
#include "configuration_internal.h"

static inline ccs_result_t
_ccs_deserialize_bin_configuration(
		ccs_configuration_t                *configuration_ret,
		uint32_t                            version,
		size_t                             *buffer_size,
		const char                        **buffer,
		_ccs_object_deserialize_options_t  *opts) {
	CCS_CHECK_PTR(opts);
	CCS_CHECK_OBJ(opts->handle_map, CCS_MAP);
	_ccs_object_internal_t obj;
	ccs_object_t handle;
	ccs_datum_t d;
	ccs_configuration_space_t cs;
	ccs_result_t res = CCS_SUCCESS;
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_object_internal(
		&obj, buffer_size, buffer, &handle));
	if (CCS_UNLIKELY(obj.type != CCS_CONFIGURATION))
		return -CCS_INVALID_TYPE;

	_ccs_binding_data_t data = {NULL, 0, NULL};
	CCS_VALIDATE_ERR_GOTO(res, _ccs_deserialize_bin_ccs_binding_data(
		&data, version, buffer_size, buffer), end);

	CCS_VALIDATE_ERR_GOTO(res, ccs_map_get(
		opts->handle_map, ccs_object(data.context), &d), end);
	if (CCS_UNLIKELY(d.type != CCS_OBJECT)) {
		res = -CCS_INVALID_HANDLE;
		goto end;
	}
	cs = (ccs_configuration_space_t)(d.value.o);

	CCS_VALIDATE_ERR_GOTO(res, ccs_create_configuration(
		cs, data.num_values, data.values, NULL, configuration_ret), end);

	if (opts->map_values)
		CCS_VALIDATE_ERR_GOTO(res,
			_ccs_object_handle_check_add(
				opts->handle_map, handle,
				(ccs_object_t)*configuration_ret),
			err_configuration);
	goto end;

err_configuration:
	ccs_release_object(*configuration_ret);
	*configuration_ret = NULL;
end:
	if (data.values)
		free(data.values);
	return res;
}

static ccs_result_t
_ccs_configuration_deserialize(
		ccs_configuration_t                *configuration_ret,
		ccs_serialize_format_t              format,
		uint32_t                            version,
		size_t                             *buffer_size,
		const char                        **buffer,
		_ccs_object_deserialize_options_t  *opts) {
	switch(format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE(_ccs_deserialize_bin_configuration(
			configuration_ret, version, buffer_size, buffer, opts));
		break;
	default:
		return -CCS_INVALID_VALUE;
	}
	return CCS_SUCCESS;
}

#endif //_CONFIGURATION_DESERIALIZE_H
