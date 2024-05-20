#ifndef _DISTRIBUTION_SPACE_DESERIALIZE_H
#define _DISTRIBUTION_SPACE_DESERIALIZE_H

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
	mem = (uintptr_t)calloc(
		data->num_distributions *
				(sizeof(ccs_distribution_t) + sizeof(size_t)) +
			data->num_parameters * sizeof(size_t),
		1);
	CCS_REFUTE(!mem, CCS_RESULT_ERROR_OUT_OF_MEMORY);

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
	_ccs_object_internal_t    obj;
	ccs_datum_t               d;
	ccs_configuration_space_t cs;
	ccs_object_t              handle;
	ccs_distribution_space_t  distrib_space;
	ccs_result_t              res = CCS_RESULT_SUCCESS;
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_object_internal(
		&obj, buffer_size, buffer, &handle));
	CCS_REFUTE(
		obj.type != CCS_OBJECT_TYPE_DISTRIBUTION_SPACE,
		CCS_RESULT_ERROR_INVALID_TYPE);

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
	if (opts->map_values)
		CCS_VALIDATE_ERR_GOTO(
			res,
			_ccs_object_handle_check_add(
				opts->handle_map, handle,
				(ccs_object_t)distrib_space),
			err_distribution_space);
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
	default:
		CCS_RAISE(
			CCS_RESULT_ERROR_INVALID_VALUE,
			"Unsupported serialization format: %d", format);
	}
	return CCS_RESULT_SUCCESS;
}

#endif //_DISTRIBUTION_SPACE_DESERIALIZE_H
