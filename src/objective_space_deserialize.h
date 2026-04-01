#ifndef _OBJECTIVE_SPACE_DESERIALIZE_H
#define _OBJECTIVE_SPACE_DESERIALIZE_H
#include "objective_space_internal.h"
#include "cconfigspace_json.h"

struct _ccs_objective_space_data_mock_s {
	const char           *name;
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
	{
		size_t _sz;
		CCS_REFUTE(
			CCS_ALLOC_SIZE(
				&_sz,
				CCS_ALLOC_SIZE_ARRAY(
					data->num_parameters, ccs_parameter_t),
				CCS_ALLOC_SIZE_ARRAY(
					data->num_objectives, ccs_expression_t),
				CCS_ALLOC_SIZE_ARRAY(
					data->num_objectives,
					ccs_objective_type_t)),
			CCS_RESULT_ERROR_OUT_OF_MEMORY);
		mem = (uintptr_t)calloc(1, _sz);
		CCS_REFUTE(!mem, CCS_RESULT_ERROR_OUT_OF_MEMORY);
	}
	data->parameters = CCS_ALLOC_CARVE_ARRAY(
		mem, data->num_parameters, ccs_parameter_t);
	data->objectives = CCS_ALLOC_CARVE_ARRAY(
		mem, data->num_objectives, ccs_expression_t);
	data->objective_types = CCS_ALLOC_CARVE_ARRAY(
		mem, data->num_objectives, ccs_objective_type_t);

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

	_ccs_objective_space_data_mock_t data = {NULL, NULL, 0,   0,
						 NULL, NULL, NULL};
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

static inline ccs_result_t
_ccs_deserialize_json_ccs_objective_space_data(
	_ccs_objective_space_data_mock_t  *data,
	uint32_t                           version,
	cJSON                             *json,
	_ccs_object_deserialize_options_t *opts)
{
	cJSON      *j_name;
	cJSON      *j_ss;
	cJSON      *j_params;
	cJSON      *j_objs;
	size_t      num;
	size_t      num_objs;
	uintptr_t   mem;
	const char *cbuf;
	size_t      dummy;

	j_name = cJSON_GetObjectItemCaseSensitive(json, "name");
	CCS_REFUTE(
		!j_name || !cJSON_IsString(j_name),
		CCS_RESULT_ERROR_INVALID_VALUE);
	data->name = j_name->valuestring;

	/* search_space */
	j_ss       = cJSON_GetObjectItemCaseSensitive(json, "search_space");
	CCS_REFUTE(
		!j_ss || !cJSON_IsObject(j_ss), CCS_RESULT_ERROR_INVALID_VALUE);
	cbuf  = (const char *)j_ss;
	dummy = 0;
	CCS_VALIDATE(_ccs_object_deserialize_with_opts(
		(ccs_object_t *)&data->search_space, CCS_SERIALIZE_FORMAT_JSON,
		version, &dummy, &cbuf, opts));

	/* parameters */
	j_params = cJSON_GetObjectItemCaseSensitive(json, "parameters");
	CCS_REFUTE(
		!j_params || !cJSON_IsArray(j_params),
		CCS_RESULT_ERROR_INVALID_VALUE);
	num                  = (size_t)cJSON_GetArraySize(j_params);
	data->num_parameters = num;

	/* objectives */
	j_objs = cJSON_GetObjectItemCaseSensitive(json, "objectives");
	CCS_REFUTE(
		!j_objs || !cJSON_IsArray(j_objs),
		CCS_RESULT_ERROR_INVALID_VALUE);
	num_objs             = (size_t)cJSON_GetArraySize(j_objs);
	data->num_objectives = num_objs;

	if (!(num + num_objs))
		return CCS_RESULT_SUCCESS;

	{
		size_t _sz;
		CCS_REFUTE(
			CCS_ALLOC_SIZE(
				&_sz,
				CCS_ALLOC_SIZE_ARRAY(num, ccs_parameter_t),
				CCS_ALLOC_SIZE_ARRAY(num_objs, ccs_expression_t),
				CCS_ALLOC_SIZE_ARRAY(
					num_objs, ccs_objective_type_t)),
			CCS_RESULT_ERROR_OUT_OF_MEMORY);
		mem = (uintptr_t)calloc(1, _sz);
		CCS_REFUTE(!mem, CCS_RESULT_ERROR_OUT_OF_MEMORY);
	}

	data->parameters = CCS_ALLOC_CARVE_ARRAY(mem, num, ccs_parameter_t);
	data->objectives =
		CCS_ALLOC_CARVE_ARRAY(mem, num_objs, ccs_expression_t);
	data->objective_types =
		CCS_ALLOC_CARVE_ARRAY(mem, num_objs, ccs_objective_type_t);

	for (size_t i = 0; i < num; i++) {
		cJSON *child = cJSON_GetArrayItem(j_params, (int)i);
		cbuf         = (const char *)child;
		dummy        = 0;
		CCS_VALIDATE(_ccs_object_deserialize_with_opts_check(
			(ccs_object_t *)data->parameters + i,
			CCS_OBJECT_TYPE_PARAMETER, CCS_SERIALIZE_FORMAT_JSON,
			version, &dummy, &cbuf, opts));
	}

	for (size_t i = 0; i < num_objs; i++) {
		cJSON *obj_item = cJSON_GetArrayItem(j_objs, (int)i);
		cJSON *j_expr;
		cJSON *j_type;
		j_expr = cJSON_GetObjectItemCaseSensitive(
			obj_item, "expression");
		CCS_REFUTE(
			!j_expr || !cJSON_IsObject(j_expr),
			CCS_RESULT_ERROR_INVALID_VALUE);
		cbuf  = (const char *)j_expr;
		dummy = 0;
		CCS_VALIDATE(_ccs_object_deserialize_with_opts_check(
			(ccs_object_t *)data->objectives + i,
			CCS_OBJECT_TYPE_EXPRESSION, CCS_SERIALIZE_FORMAT_JSON,
			version, &dummy, &cbuf, opts));
		j_type = cJSON_GetObjectItemCaseSensitive(obj_item, "type");
		CCS_REFUTE(
			!j_type || !cJSON_IsString(j_type),
			CCS_RESULT_ERROR_INVALID_VALUE);
		CCS_VALIDATE(_ccs_json_objective_type_from_string(
			j_type->valuestring, data->objective_types + i));
	}

	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_deserialize_json_objective_space(
	ccs_objective_space_t             *objective_space_ret,
	uint32_t                           version,
	size_t                            *buffer_size,
	const char                       **buffer,
	_ccs_object_deserialize_options_t *opts)
{
	_ccs_object_deserialize_options_t new_opts = *opts;
	ccs_result_t                      res      = CCS_RESULT_SUCCESS;
	_ccs_objective_space_data_mock_t  data     = {NULL, NULL, 0,   0,
						      NULL, NULL, NULL};

	(void)buffer_size;

	if (!opts->map_values) {
		new_opts.map_values = CCS_TRUE;
		CCS_VALIDATE(ccs_create_map(&new_opts.handle_map));
	}

	CCS_VALIDATE_ERR_GOTO(
		res,
		_ccs_deserialize_json_ccs_objective_space_data(
			&data, version, *(cJSON **)buffer, &new_opts),
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
	case CCS_SERIALIZE_FORMAT_JSON:
		CCS_VALIDATE(_ccs_deserialize_json_objective_space(
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
