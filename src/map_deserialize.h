#ifndef _MAP_DESERIALIZE_H
#define _MAP_DESERIALIZE_H
#include "map_internal.h"
#include "cconfigspace_json.h"

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

static inline ccs_result_t
_ccs_deserialize_bin_ccs_map_data(
	_ccs_map_data_mock_t *data,
	size_t               *buffer_size,
	const char          **buffer)
{
	CCS_VALIDATE(_ccs_deserialize_bin_size(
		&data->num_pairs, buffer_size, buffer));
	{
		size_t _sz;
		CCS_REFUTE(
			CCS_ALLOC_SIZE(
				&_sz,
				CCS_ALLOC_SIZE_ARRAY(
					data->num_pairs, _ccs_map_pair_t)),
			CCS_RESULT_ERROR_OUT_OF_MEMORY);
		data->pairs = (_ccs_map_pair_t *)calloc(1, _sz);
		CCS_REFUTE(!data->pairs, CCS_RESULT_ERROR_OUT_OF_MEMORY);
	}
	for (size_t i = 0; i < data->num_pairs; i++) {
		CCS_VALIDATE(_ccs_deserialize_bin_ccs_datum(
			&data->pairs[i].key, buffer_size, buffer));
		CCS_VALIDATE(_ccs_deserialize_bin_ccs_datum(
			&data->pairs[i].value, buffer_size, buffer));
	}
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_deserialize_bin_map(
	ccs_map_t                         *map_ret,
	uint32_t                           version,
	size_t                            *buffer_size,
	const char                       **buffer,
	_ccs_object_deserialize_options_t *opts)
{
	(void)version;
	(void)opts;
	ccs_result_t         res  = CCS_RESULT_SUCCESS;
	_ccs_map_data_mock_t data = {0, NULL};

	CCS_VALIDATE_ERR_GOTO(
		res,
		_ccs_deserialize_bin_ccs_map_data(&data, buffer_size, buffer),
		end);
	CCS_VALIDATE_ERR_GOTO(res, ccs_create_map(map_ret), end);
	for (size_t i = 0; i < data.num_pairs; i++)
		CCS_VALIDATE_ERR_GOTO(
			res,
			ccs_map_set(
				*map_ret, data.pairs[i].key,
				data.pairs[i].value),
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

static inline ccs_result_t
_ccs_deserialize_json_map(
	ccs_map_t                         *map_ret,
	uint32_t                           version,
	size_t                            *buffer_size,
	const char                       **buffer,
	_ccs_object_deserialize_options_t *opts)
{
	(void)version;
	(void)buffer_size;
	(void)opts;
	ccs_result_t res = CCS_RESULT_SUCCESS;
	cJSON       *json;
	cJSON       *j_pairs;
	size_t       num;

	json = *(cJSON **)buffer;
	CCS_VALIDATE(_ccs_json_extract_array(json, "pairs", &j_pairs, &num));

	CCS_VALIDATE(ccs_create_map(map_ret));
	for (size_t i = 0; i < num; i++) {
		cJSON      *pair = cJSON_GetArrayItem(j_pairs, (int)i);
		ccs_datum_t key, value;

		CCS_REFUTE_ERR_GOTO(
			res, !pair || !cJSON_IsObject(pair),
			CCS_RESULT_ERROR_INVALID_VALUE, err_map);
		CCS_VALIDATE_ERR_GOTO(
			res, _ccs_json_extract_datum(pair, "key", &key),
			err_map);
		CCS_VALIDATE_ERR_GOTO(
			res, _ccs_json_extract_datum(pair, "value", &value),
			err_map);
		/* Strings from cJSON point into the cJSON tree which will
		 * be freed after deserialization.  Mark them transient so
		 * ccs_map_set copies the data. */
		if (key.type == CCS_DATA_TYPE_STRING)
			key.flags |= CCS_DATUM_FLAG_TRANSIENT;
		if (value.type == CCS_DATA_TYPE_STRING)
			value.flags |= CCS_DATUM_FLAG_TRANSIENT;
		CCS_VALIDATE_ERR_GOTO(
			res, ccs_map_set(*map_ret, key, value), err_map);
	}
	return CCS_RESULT_SUCCESS;
err_map:
	ccs_release_object(*map_ret);
	*map_ret = NULL;
	return res;
}

static ccs_result_t
_ccs_map_deserialize(
	ccs_map_t                         *map_ret,
	ccs_serialize_format_t             format,
	uint32_t                           version,
	size_t                            *buffer_size,
	const char                       **buffer,
	_ccs_object_deserialize_options_t *opts)
{
	switch (format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE(_ccs_deserialize_bin_map(
			map_ret, version, buffer_size, buffer, opts));
		break;
	case CCS_SERIALIZE_FORMAT_JSON:
		CCS_VALIDATE(_ccs_deserialize_json_map(
			map_ret, version, buffer_size, buffer, opts));
		break;
	default:
		CCS_RAISE(
			CCS_RESULT_ERROR_INVALID_VALUE,
			"Unsupported serialization format: %d", format);
	}
	return CCS_RESULT_SUCCESS;
}

#endif //_MAP_DESERIALIZE_H
