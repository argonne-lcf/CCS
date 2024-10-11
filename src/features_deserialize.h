#ifndef _FEATURES_DESERIALIZE_H
#define _FEATURES_DESERIALIZE_H
#include "cconfigspace_internal.h"
#include "features_internal.h"

static inline ccs_result_t
_ccs_deserialize_bin_features(
	ccs_features_t                    *features_ret,
	uint32_t                           version,
	size_t                            *buffer_size,
	const char                       **buffer,
	_ccs_object_deserialize_options_t *opts)
{
	CCS_CHECK_OBJ(opts->handle_map, CCS_OBJECT_TYPE_MAP);
	ccs_datum_t         d;
	ccs_feature_space_t cs;
	ccs_result_t        res  = CCS_RESULT_SUCCESS;

	_ccs_binding_data_t data = {NULL, 0, NULL};
	CCS_VALIDATE_ERR_GOTO(
		res,
		_ccs_deserialize_bin_ccs_binding_data(
			&data, version, buffer_size, buffer),
		end);

	CCS_VALIDATE_ERR_GOTO(
		res,
		ccs_map_get(opts->handle_map, ccs_object(data.context), &d),
		end);
	CCS_REFUTE_ERR_GOTO(
		res, d.type != CCS_DATA_TYPE_OBJECT,
		CCS_RESULT_ERROR_INVALID_HANDLE, end);
	cs = (ccs_feature_space_t)(d.value.o);

	CCS_VALIDATE_ERR_GOTO(
		res,
		ccs_create_features(
			cs, data.num_values, data.values, features_ret),
		end);

end:
	if (data.values)
		free(data.values);
	return res;
}

static ccs_result_t
_ccs_features_deserialize(
	ccs_features_t                    *features_ret,
	ccs_serialize_format_t             format,
	uint32_t                           version,
	size_t                            *buffer_size,
	const char                       **buffer,
	_ccs_object_deserialize_options_t *opts)
{
	switch (format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE(_ccs_deserialize_bin_features(
			features_ret, version, buffer_size, buffer, opts));
		break;
	default:
		CCS_RAISE(
			CCS_RESULT_ERROR_INVALID_VALUE,
			"Unsupported serialization format: %d", format);
	}
	return CCS_RESULT_SUCCESS;
}

#endif //_FEATURES_DESERIALIZE_H
