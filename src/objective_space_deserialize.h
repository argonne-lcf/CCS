#ifndef _OBJECTIVE_SPACE_DESERIALIZE_H
#define _OBJECTIVE_SPACE_DESERIALIZE_H
#include "objective_space_internal.h"

struct _ccs_objective_space_data_mock_s {
	const char           *name;
	ccs_object_t          feature_space_handle;
	ccs_object_t          search_space_handle;
	ccs_search_space_t    search_space;
	size_t                num_parameters;
	size_t                num_objectives;
	ccs_parameter_t      *parameters;
	ccs_expression_t     *objectives;
	ccs_objective_type_t *objective_types;
};
typedef struct _ccs_objective_space_data_mock_s _ccs_objective_space_data_mock_t;

static inline ccs_result_t
_ccs_deserialize_bin_ccs_objective_space_data(
	_ccs_objective_space_data_mock_t  *data,
	uint32_t                           version,
	size_t                            *buffer_size,
	const char                       **buffer,
	_ccs_object_deserialize_options_t *opts)
{
	uintptr_t mem;

	CCS_VALIDATE(
		_ccs_deserialize_bin_string(&data->name, buffer_size, buffer));

	CCS_VALIDATE(_ccs_deserialize_bin_ccs_object(
		&data->feature_space_handle, buffer_size, buffer));
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_object(
		&data->search_space_handle, buffer_size, buffer));
	CCS_VALIDATE(_ccs_object_deserialize_with_opts(
		(ccs_object_t *)&data->search_space,
		CCS_SERIALIZE_FORMAT_BINARY, version, buffer_size, buffer,
		opts));
	CCS_VALIDATE(_ccs_deserialize_bin_size(
		&data->num_parameters, buffer_size, buffer));
	CCS_VALIDATE(_ccs_deserialize_bin_size(
		&data->num_objectives, buffer_size, buffer));

	if (!(data->num_parameters + data->num_objectives))
		return CCS_RESULT_SUCCESS;
	mem = (uintptr_t)calloc(
		data->num_parameters * sizeof(ccs_parameter_t) +
			data->num_objectives * (sizeof(ccs_expression_t) +
						sizeof(ccs_objective_type_t)),
		1);
	CCS_REFUTE(!mem, CCS_RESULT_ERROR_OUT_OF_MEMORY);
	data->parameters = (ccs_parameter_t *)mem;
	mem += data->num_parameters * sizeof(ccs_parameter_t);
	data->objectives = (ccs_expression_t *)mem;
	mem += data->num_objectives * sizeof(ccs_expression_t);
	data->objective_types = (ccs_objective_type_t *)mem;

	for (size_t i = 0; i < data->num_parameters; i++)
		CCS_VALIDATE(_ccs_object_deserialize_with_opts_check(
			(ccs_object_t *)data->parameters + i,
			CCS_OBJECT_TYPE_PARAMETER, CCS_SERIALIZE_FORMAT_BINARY,
			version, buffer_size, buffer, opts));

	for (size_t i = 0; i < data->num_objectives; i++) {
		CCS_VALIDATE(_ccs_object_deserialize_with_opts_check(
			(ccs_object_t *)data->objectives + i,
			CCS_OBJECT_TYPE_EXPRESSION, CCS_SERIALIZE_FORMAT_BINARY,
			version, buffer_size, buffer, opts));
		CCS_VALIDATE(_ccs_deserialize_bin_ccs_objective_type(
			data->objective_types + i, buffer_size, buffer));
	}

	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_deserialize_bin_objective_space(
	ccs_objective_space_t             *objective_space_ret,
	uint32_t                           version,
	size_t                            *buffer_size,
	const char                       **buffer,
	_ccs_object_deserialize_options_t *opts)
{
	_ccs_object_deserialize_options_t new_opts = *opts;
	ccs_result_t                      res      = CCS_RESULT_SUCCESS;

	if (!opts->map_values) {
		new_opts.map_values = CCS_TRUE;
		CCS_VALIDATE(ccs_create_map(&new_opts.handle_map));
	}

	_ccs_objective_space_data_mock_t data = {NULL, NULL, NULL, NULL, 0,
						 0,    NULL, NULL, NULL};
	CCS_VALIDATE_ERR_GOTO(
		res,
		_ccs_deserialize_bin_ccs_objective_space_data(
			&data, version, buffer_size, buffer, &new_opts),
		end);
	CCS_VALIDATE_ERR_GOTO(
		res,
		ccs_create_objective_space(
			data.name, data.search_space, data.num_parameters,
			data.parameters, data.num_objectives, data.objectives,
			data.objective_types, objective_space_ret),
		end);

end:
	if (data.search_space)
		ccs_release_object(data.search_space);
	if (data.parameters)
		for (size_t i = 0; i < data.num_parameters; i++)
			if (data.parameters[i])
				ccs_release_object(data.parameters[i]);
	if (data.objectives)
		for (size_t i = 0; i < data.num_objectives; i++)
			if (data.objectives[i])
				ccs_release_object(data.objectives[i]);
	if (data.parameters)
		free(data.parameters);
	if (!opts->map_values)
		ccs_release_object(new_opts.handle_map);
	return res;
}

static ccs_result_t
_ccs_objective_space_deserialize(
	ccs_objective_space_t             *objective_space_ret,
	ccs_serialize_format_t             format,
	uint32_t                           version,
	size_t                            *buffer_size,
	const char                       **buffer,
	_ccs_object_deserialize_options_t *opts)
{
	switch (format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE(_ccs_deserialize_bin_objective_space(
			objective_space_ret, version, buffer_size, buffer,
			opts));
		break;
	default:
		CCS_RAISE(
			CCS_RESULT_ERROR_INVALID_VALUE,
			"Unsupported serialization format: %d", format);
	}
	return CCS_RESULT_SUCCESS;
}

#endif //_OBJECTIVE_SPACE_DESERIALIZE_H
