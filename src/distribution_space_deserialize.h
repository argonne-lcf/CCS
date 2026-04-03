#ifndef _DISTRIBUTION_SPACE_DESERIALIZE_H
#define _DISTRIBUTION_SPACE_DESERIALIZE_H
#include "cconfigspace_json.h"

struct _ccs_distribution_space_data_mock_s {
	ccs_configuration_space_t configuration_space;
	size_t                    num_parameters;
	size_t                    num_distributions;
	ccs_distribution_t       *distributions;
	size_t                   *dimensions;
	size_t                   *distrib_parameter_indices;
};
typedef struct _ccs_distribution_space_data_mock_s
	_ccs_distribution_space_data_mock_t;

static inline ccs_result_t
_ccs_deserialize_bin_ccs_distribution_space_data(
	_ccs_distribution_space_data_mock_t *data,
	uint32_t                             version,
	size_t                              *buffer_size,
	const char                         **buffer,
	_ccs_object_deserialize_options_t   *opts)
{
	uintptr_t mem;

	CCS_VALIDATE(_ccs_deserialize_bin_ccs_object(
		(ccs_object_t *)&data->configuration_space, buffer_size,
		buffer));

	CCS_VALIDATE(_ccs_deserialize_bin_size(
		&data->num_parameters, buffer_size, buffer));

	CCS_VALIDATE(_ccs_deserialize_bin_size(
		&data->num_distributions, buffer_size, buffer));

	if (!(data->num_distributions))
		return CCS_RESULT_SUCCESS;
	{
		size_t _sz;
		CCS_REFUTE(
			CCS_ALLOC_SIZE(
				&_sz,
				CCS_ALLOC_SIZE_ARRAY(
					data->num_distributions,
					ccs_distribution_t),
				CCS_ALLOC_SIZE_ARRAY(
					data->num_distributions, size_t),
				CCS_ALLOC_SIZE_ARRAY(
					data->num_parameters, size_t)),
			CCS_RESULT_ERROR_OUT_OF_MEMORY);
		mem = (uintptr_t)calloc(1, _sz);
		CCS_REFUTE(!mem, CCS_RESULT_ERROR_OUT_OF_MEMORY);
	}

	data->distributions = (ccs_distribution_t *)mem;
	mem += data->num_distributions * sizeof(ccs_distribution_t);
	data->dimensions = (size_t *)mem;
	mem += data->num_distributions * sizeof(size_t);
	data->distrib_parameter_indices = (size_t *)mem;
	mem += data->num_parameters * sizeof(size_t);

	size_t *indices;
	indices = data->distrib_parameter_indices;
	for (size_t i = 0; i < data->num_distributions; i++) {
		CCS_VALIDATE(_ccs_object_deserialize_with_opts_check(
			(ccs_object_t *)data->distributions + i,
			CCS_OBJECT_TYPE_DISTRIBUTION,
			CCS_SERIALIZE_FORMAT_BINARY, version, buffer_size,
			buffer, opts));
		CCS_VALIDATE(_ccs_deserialize_bin_size(
			data->dimensions + i, buffer_size, buffer));
		for (size_t j = 0; j < data->dimensions[i]; j++) {
			CCS_VALIDATE(_ccs_deserialize_bin_size(
				indices, buffer_size, buffer));
			indices++;
		}
	}

	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_deserialize_bin_distribution_space(
	ccs_distribution_space_t          *distribution_space_ret,
	uint32_t                           version,
	size_t                            *buffer_size,
	const char                       **buffer,
	_ccs_object_deserialize_options_t *opts)
{
	CCS_CHECK_OBJ(opts->handle_map, CCS_OBJECT_TYPE_MAP);
	_ccs_object_deserialize_options_t new_opts = *opts;
	new_opts.map_values                        = CCS_FALSE;
	new_opts.handle_map                        = NULL;
	ccs_datum_t                         d;
	ccs_configuration_space_t           cs;
	ccs_distribution_space_t            distrib_space;
	ccs_result_t                        res  = CCS_RESULT_SUCCESS;

	_ccs_distribution_space_data_mock_t data = {NULL, 0,    0,
						    NULL, NULL, NULL};
	CCS_VALIDATE_ERR_GOTO(
		res,
		_ccs_deserialize_bin_ccs_distribution_space_data(
			&data, version, buffer_size, buffer, &new_opts),
		end);

	CCS_VALIDATE_ERR_GOTO(
		res,
		ccs_map_get(
			opts->handle_map, ccs_object(data.configuration_space),
			&d),
		end);
	CCS_REFUTE_ERR_GOTO(
		res, d.type != CCS_DATA_TYPE_OBJECT,
		CCS_RESULT_ERROR_INVALID_HANDLE, end);
	cs = (ccs_configuration_space_t)(d.value.o);

	CCS_VALIDATE_ERR_GOTO(
		res, ccs_create_distribution_space(cs, &distrib_space), end);
	size_t *indices;
	indices = data.distrib_parameter_indices;
	for (size_t i = 0; i < data.num_distributions; i++) {
		CCS_VALIDATE_ERR_GOTO(
			res,
			ccs_distribution_space_set_distribution(
				distrib_space, data.distributions[i], indices),
			err_distribution_space);
		indices += data.dimensions[i];
	}
	*distribution_space_ret = distrib_space;
	goto end;

err_distribution_space:
	ccs_release_object(distrib_space);
end:
	if (data.distributions)
		for (size_t i = 0; i < data.num_distributions; i++)
			if (data.distributions[i])
				ccs_release_object(data.distributions[i]);
	if (data.distributions)
		free(data.distributions);
	return res;
}

static inline ccs_result_t
_ccs_deserialize_json_ccs_distribution_space_data(
	_ccs_distribution_space_data_mock_t *data,
	uint32_t                             version,
	cJSON                               *json,
	_ccs_object_deserialize_options_t   *opts)
{
	cJSON      *j_cs;
	cJSON      *j_distribs;
	size_t      num;
	size_t      total_indices = 0;
	uintptr_t   mem;
	const char *cbuf;
	size_t      dummy;

	/* configuration_space handle */
	j_cs = cJSON_GetObjectItemCaseSensitive(json, "configuration_space");
	CCS_REFUTE(
		!j_cs || !cJSON_IsString(j_cs), CCS_RESULT_ERROR_INVALID_VALUE);
	CCS_REFUTE(
		strlen(j_cs->valuestring) != sizeof(ccs_object_t) * 2,
		CCS_RESULT_ERROR_INVALID_VALUE);
	CCS_REFUTE(
		_ccs_json_hex_decode_buf(
			j_cs->valuestring, sizeof(ccs_object_t) * 2,
			&data->configuration_space),
		CCS_RESULT_ERROR_INVALID_VALUE);

	/* distributions array */
	j_distribs = cJSON_GetObjectItemCaseSensitive(json, "distributions");
	CCS_REFUTE(
		!j_distribs || !cJSON_IsArray(j_distribs),
		CCS_RESULT_ERROR_INVALID_VALUE);
	num                     = (size_t)cJSON_GetArraySize(j_distribs);
	data->num_distributions = num;

	if (!num)
		return CCS_RESULT_SUCCESS;

	/* First pass: count total indices */
	for (size_t i = 0; i < num; i++) {
		cJSON *entry     = cJSON_GetArrayItem(j_distribs, (int)i);
		cJSON *j_indices = cJSON_GetObjectItemCaseSensitive(
			entry, "parameter_indices");
		CCS_REFUTE(
			!j_indices || !cJSON_IsArray(j_indices),
			CCS_RESULT_ERROR_INVALID_VALUE);
		total_indices += (size_t)cJSON_GetArraySize(j_indices);
	}
	data->num_parameters = total_indices;

	{
		size_t _sz;
		CCS_REFUTE(
			CCS_ALLOC_SIZE(
				&_sz,
				CCS_ALLOC_SIZE_ARRAY(num, ccs_distribution_t),
				CCS_ALLOC_SIZE_ARRAY(num, size_t),
				CCS_ALLOC_SIZE_ARRAY(total_indices, size_t)),
			CCS_RESULT_ERROR_OUT_OF_MEMORY);
		mem = (uintptr_t)calloc(1, _sz);
		CCS_REFUTE(!mem, CCS_RESULT_ERROR_OUT_OF_MEMORY);
	}

	data->distributions =
		CCS_ALLOC_CARVE_ARRAY(mem, num, ccs_distribution_t);
	data->dimensions = CCS_ALLOC_CARVE_ARRAY(mem, num, size_t);
	data->distrib_parameter_indices =
		CCS_ALLOC_CARVE_ARRAY(mem, total_indices, size_t);

	size_t *indices = data->distrib_parameter_indices;
	for (size_t i = 0; i < num; i++) {
		cJSON *entry = cJSON_GetArrayItem(j_distribs, (int)i);
		cJSON *j_dist =
			cJSON_GetObjectItemCaseSensitive(entry, "distribution");
		cJSON *j_indices = cJSON_GetObjectItemCaseSensitive(
			entry, "parameter_indices");
		size_t dim;

		CCS_REFUTE(
			!j_dist || !cJSON_IsObject(j_dist),
			CCS_RESULT_ERROR_INVALID_VALUE);
		cbuf  = (const char *)j_dist;
		dummy = 0;
		CCS_VALIDATE(_ccs_object_deserialize_with_opts_check(
			(ccs_object_t *)data->distributions + i,
			CCS_OBJECT_TYPE_DISTRIBUTION, CCS_SERIALIZE_FORMAT_JSON,
			version, &dummy, &cbuf, opts));

		dim                 = (size_t)cJSON_GetArraySize(j_indices);
		data->dimensions[i] = dim;
		for (size_t j = 0; j < dim; j++) {
			ccs_int_t idx_val;
			cJSON    *idx = cJSON_GetArrayItem(j_indices, (int)j);
			CCS_REFUTE(!idx, CCS_RESULT_ERROR_INVALID_VALUE);
			CCS_VALIDATE(_ccs_json_get_int(idx, &idx_val));
			*indices = (size_t)idx_val;
			indices++;
		}
	}

	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_deserialize_json_distribution_space(
	ccs_distribution_space_t          *distribution_space_ret,
	uint32_t                           version,
	size_t                            *buffer_size,
	const char                       **buffer,
	_ccs_object_deserialize_options_t *opts)
{
	CCS_CHECK_OBJ(opts->handle_map, CCS_OBJECT_TYPE_MAP);
	_ccs_object_deserialize_options_t new_opts = *opts;
	new_opts.map_values                        = CCS_FALSE;
	new_opts.handle_map                        = NULL;
	ccs_datum_t               d;
	ccs_configuration_space_t cs;
	ccs_distribution_space_t  distrib_space;
	ccs_result_t              res = CCS_RESULT_SUCCESS;

	(void)buffer_size;

	_ccs_distribution_space_data_mock_t data = {NULL, 0,    0,
						    NULL, NULL, NULL};
	CCS_VALIDATE_ERR_GOTO(
		res,
		_ccs_deserialize_json_ccs_distribution_space_data(
			&data, version, *(cJSON **)buffer, &new_opts),
		end);

	CCS_VALIDATE_ERR_GOTO(
		res,
		ccs_map_get(
			opts->handle_map, ccs_object(data.configuration_space),
			&d),
		end);
	CCS_REFUTE_ERR_GOTO(
		res, d.type != CCS_DATA_TYPE_OBJECT,
		CCS_RESULT_ERROR_INVALID_HANDLE, end);
	cs = (ccs_configuration_space_t)(d.value.o);

	CCS_VALIDATE_ERR_GOTO(
		res, ccs_create_distribution_space(cs, &distrib_space), end);
	size_t *indices;
	indices = data.distrib_parameter_indices;
	for (size_t i = 0; i < data.num_distributions; i++) {
		CCS_VALIDATE_ERR_GOTO(
			res,
			ccs_distribution_space_set_distribution(
				distrib_space, data.distributions[i], indices),
			err_distribution_space);
		indices += data.dimensions[i];
	}
	*distribution_space_ret = distrib_space;
	goto end;

err_distribution_space:
	ccs_release_object(distrib_space);
end:
	if (data.distributions)
		for (size_t i = 0; i < data.num_distributions; i++)
			if (data.distributions[i])
				ccs_release_object(data.distributions[i]);
	if (data.distributions)
		free(data.distributions);
	return res;
}

static ccs_result_t
_ccs_distribution_space_deserialize(
	ccs_distribution_space_t          *distribution_space_ret,
	ccs_serialize_format_t             format,
	uint32_t                           version,
	size_t                            *buffer_size,
	const char                       **buffer,
	_ccs_object_deserialize_options_t *opts)
{
	switch (format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE(_ccs_deserialize_bin_distribution_space(
			distribution_space_ret, version, buffer_size, buffer,
			opts));
		break;
	case CCS_SERIALIZE_FORMAT_JSON:
		CCS_VALIDATE(_ccs_deserialize_json_distribution_space(
			distribution_space_ret, version, buffer_size, buffer,
			opts));
		break;
	default:
		CCS_RAISE(
			CCS_RESULT_ERROR_INVALID_VALUE,
			"Unsupported serialization format: %d", format);
	}
	return CCS_RESULT_SUCCESS;
}

#endif //_DISTRIBUTION_SPACE_DESERIALIZE_H
