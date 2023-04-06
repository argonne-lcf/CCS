#ifndef _MAP_DESERIALIZE_H
#define _MAP_DESERIALIZE_H
#include "cconfigspace_internal.h"
#include "map_internal.h"

struct _ccs_map_pair_s {
	ccs_datum_t key;
	ccs_datum_t value;
};
typedef struct _ccs_map_pair_s _ccs_map_pair_t;

struct _ccs_map_data_mock_s {
	size_t           num_pairs;
	_ccs_map_pair_t *pairs;
};
typedef struct _ccs_map_data_mock_s _ccs_map_data_mock_t;

static inline ccs_error_t
_ccs_deserialize_bin_ccs_map_data(
		_ccs_map_data_mock_t  *data,
		size_t                *buffer_size,
		const char           **buffer) {
	CCS_VALIDATE(_ccs_deserialize_bin_size(
		&data->num_pairs, buffer_size, buffer));
	data->pairs = (_ccs_map_pair_t *)malloc(data->num_pairs*sizeof(_ccs_map_pair_t));
	CCS_REFUTE(!data->pairs, CCS_OUT_OF_MEMORY);
	for (size_t i = 0; i < data->num_pairs; i++) {
		CCS_VALIDATE(_ccs_deserialize_bin_ccs_datum(
			&data->pairs[i].key, buffer_size, buffer));
		CCS_VALIDATE(_ccs_deserialize_bin_ccs_datum(
			&data->pairs[i].value, buffer_size, buffer));
	}
	return CCS_SUCCESS; 
}

static inline ccs_error_t
_ccs_deserialize_bin_map(
		ccs_map_t                          *map_ret,
		uint32_t                            version,
		size_t                             *buffer_size,
		const char                        **buffer,
		_ccs_object_deserialize_options_t  *opts) {
	(void)version;
	ccs_error_t res = CCS_SUCCESS;
	_ccs_object_internal_t obj;
	_ccs_map_data_mock_t data = { 0, NULL };
	ccs_object_t handle;
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_object_internal(
		&obj, buffer_size, buffer, &handle));
	CCS_REFUTE(obj.type != CCS_MAP, CCS_INVALID_TYPE);

	CCS_VALIDATE_ERR_GOTO(res, _ccs_deserialize_bin_ccs_map_data(
		&data, buffer_size, buffer), end);
	CCS_VALIDATE_ERR_GOTO(res, ccs_create_map(map_ret), end);
	for (size_t i = 0; i < data.num_pairs; i++)
		CCS_VALIDATE_ERR_GOTO(res, ccs_map_set(
			*map_ret, data.pairs[i].key, data.pairs[i].value),
			err_map);
	if (opts->handle_map)
		CCS_VALIDATE_ERR_GOTO(res,
			_ccs_object_handle_check_add(
				opts->handle_map, handle,
				(ccs_object_t)*map_ret),
			err_map);
	goto end;
err_map:
	ccs_release_object(*map_ret);
	*map_ret = NULL;
end:
	if (data.pairs)
		free(data.pairs);
	return res;
}

static ccs_error_t
_ccs_map_deserialize(
		ccs_map_t                          *map_ret,
		ccs_serialize_format_t              format,
		uint32_t                            version,
		size_t                             *buffer_size,
		const char                        **buffer,
		_ccs_object_deserialize_options_t  *opts) {
	switch(format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE(_ccs_deserialize_bin_map(
			map_ret, version, buffer_size, buffer, opts));
		break;
	default:
		CCS_RAISE(CCS_INVALID_VALUE, "Unsupported serialization format: %d", format);
	}
	CCS_VALIDATE(_ccs_object_deserialize_user_data(
		(ccs_object_t)*map_ret, format, version, buffer_size, buffer, opts));
	return CCS_SUCCESS;
}

#endif //_MAP_DESERIALIZE_H
