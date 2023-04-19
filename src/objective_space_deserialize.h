#ifndef _OBJECTIVE_SPACE_DESERIALIZE_H
#define _OBJECTIVE_SPACE_DESERIALIZE_H
#include "context_deserialize.h"
#include "expression_deserialize.h"

struct _ccs_objective_space_data_mock_s {
	const char           *name;
	size_t                num_parameters;
	size_t                num_objectives;
	ccs_parameter_t      *parameters;
	ccs_expression_t     *objectives;
	ccs_objective_type_t *objective_types;
};
typedef struct _ccs_objective_space_data_mock_s _ccs_objective_space_data_mock_t;

static inline ccs_error_t
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
	CCS_VALIDATE(_ccs_deserialize_bin_size(
		&data->num_parameters, buffer_size, buffer));
	CCS_VALIDATE(_ccs_deserialize_bin_size(
		&data->num_objectives, buffer_size, buffer));

	if (!(data->num_parameters + data->num_objectives))
		return CCS_SUCCESS;
	mem = (uintptr_t)calloc(
		data->num_parameters * sizeof(ccs_parameter_t) +
			data->num_objectives * (sizeof(ccs_expression_t) +
						sizeof(ccs_objective_type_t)),
		1);
	CCS_REFUTE(!mem, CCS_OUT_OF_MEMORY);
	data->parameters = (ccs_parameter_t *)mem;
	mem += data->num_parameters * sizeof(ccs_parameter_t);
	data->objectives = (ccs_expression_t *)mem;
	mem += data->num_objectives * sizeof(ccs_expression_t);
	data->objective_types = (ccs_objective_type_t *)mem;

	for (size_t i = 0; i < data->num_parameters; i++)
		CCS_VALIDATE(_ccs_parameter_deserialize(
			data->parameters + i, CCS_SERIALIZE_FORMAT_BINARY,
			version, buffer_size, buffer, opts));

	for (size_t i = 0; i < data->num_objectives; i++) {
		CCS_VALIDATE(_ccs_expression_deserialize(
			data->objectives + i, CCS_SERIALIZE_FORMAT_BINARY,
			version, buffer_size, buffer, opts));
		CCS_VALIDATE(_ccs_deserialize_bin_ccs_objective_type(
			data->objective_types + i, buffer_size, buffer));
	}

	return CCS_SUCCESS;
}

static inline ccs_error_t
_ccs_deserialize_bin_objective_space(
	ccs_objective_space_t             *objective_space_ret,
	uint32_t                           version,
	size_t                            *buffer_size,
	const char                       **buffer,
	_ccs_object_deserialize_options_t *opts)
{
	_ccs_object_deserialize_options_t new_opts = *opts;
	_ccs_object_internal_t            obj;
	ccs_object_t                      handle;
	ccs_error_t                       res = CCS_SUCCESS;
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_object_internal(
		&obj, buffer_size, buffer, &handle));
	CCS_REFUTE(
		obj.type != CCS_OBJECT_TYPE_OBJECTIVE_SPACE, CCS_INVALID_TYPE);

	new_opts.map_values = CCS_TRUE;
	CCS_VALIDATE(ccs_create_map(&new_opts.handle_map));

	_ccs_objective_space_data_mock_t data = {NULL, 0, 0, NULL, NULL, NULL};
	CCS_VALIDATE_ERR_GOTO(
		res,
		_ccs_deserialize_bin_ccs_objective_space_data(
			&data, version, buffer_size, buffer, &new_opts),
		end);
	CCS_VALIDATE_ERR_GOTO(
		res, ccs_create_objective_space(data.name, objective_space_ret),
		end);
	CCS_VALIDATE_ERR_GOTO(
		res,
		ccs_objective_space_add_parameters(
			*objective_space_ret, data.num_parameters,
			data.parameters),
		err_objective_space);
	CCS_VALIDATE_ERR_GOTO(
		res,
		ccs_objective_space_add_objectives(
			*objective_space_ret, data.num_objectives,
			data.objectives, data.objective_types),
		err_objective_space);
	if (opts && opts->map_values && opts->handle_map)
		CCS_VALIDATE_ERR_GOTO(
			res,
			_ccs_object_handle_check_add(
				opts->handle_map, handle,
				(ccs_object_t)*objective_space_ret),
			err_objective_space);
	goto end;

err_objective_space:
	ccs_release_object(*objective_space_ret);
	*objective_space_ret = NULL;
end:
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
	ccs_release_object(new_opts.handle_map);
	return res;
}

static ccs_error_t
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
			CCS_INVALID_VALUE,
			"Unsupported serialization format: %d", format);
	}
	CCS_VALIDATE(_ccs_object_deserialize_user_data(
		(ccs_object_t)*objective_space_ret, format, version,
		buffer_size, buffer, opts));
	return CCS_SUCCESS;
}

#endif //_OBJECTIVE_SPACE_DESERIALIZE_H
