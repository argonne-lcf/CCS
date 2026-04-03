#ifndef _CONFIGURATION_SPACE_DESERIALIZE_H
#define _CONFIGURATION_SPACE_DESERIALIZE_H
#include "cconfigspace_json.h"

struct _ccs_configuration_space_data_mock_s {
	const char         *name;
	size_t              num_parameters;
	size_t              num_conditions;
	size_t              num_forbidden_clauses;
	ccs_object_t        feature_space_handle;
	ccs_feature_space_t feature_space;
	ccs_rng_t           rng;
	ccs_parameter_t    *parameters;
	ccs_expression_t   *conditions;
	ccs_expression_t   *forbidden_clauses;
};
typedef struct _ccs_configuration_space_data_mock_s
	_ccs_configuration_space_data_mock_t;

static inline ccs_result_t
_ccs_deserialize_bin_ccs_configuration_space_data(
	_ccs_configuration_space_data_mock_t *data,
	uint32_t                              version,
	size_t                               *buffer_size,
	const char                          **buffer,
	_ccs_object_deserialize_options_t    *opts)
{
	uintptr_t mem;

	CCS_VALIDATE(
		_ccs_deserialize_bin_string(&data->name, buffer_size, buffer));
	CCS_VALIDATE(_ccs_deserialize_bin_size(
		&data->num_parameters, buffer_size, buffer));
	CCS_VALIDATE(_ccs_deserialize_bin_size(
		&data->num_conditions, buffer_size, buffer));
	CCS_VALIDATE(_ccs_deserialize_bin_size(
		&data->num_forbidden_clauses, buffer_size, buffer));
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_object(
		&data->feature_space_handle, buffer_size, buffer));
	if (data->feature_space_handle) {
		CCS_VALIDATE(_ccs_object_deserialize_with_opts_check(
			(ccs_object_t *)&data->feature_space,
			CCS_OBJECT_TYPE_FEATURE_SPACE,
			CCS_SERIALIZE_FORMAT_BINARY, version, buffer_size,
			buffer, opts));
	}
	CCS_VALIDATE(_ccs_object_deserialize_with_opts_check(
		(ccs_object_t *)&data->rng, CCS_OBJECT_TYPE_RNG,
		CCS_SERIALIZE_FORMAT_BINARY, version, buffer_size, buffer,
		opts));

	if (!(data->num_parameters + data->num_conditions +
	      data->num_forbidden_clauses))
		return CCS_RESULT_SUCCESS;
	{
		size_t _sz;
		CCS_REFUTE(
			CCS_ALLOC_SIZE(
				&_sz,
				CCS_ALLOC_SIZE_ARRAY(
					data->num_parameters, ccs_parameter_t),
				CCS_ALLOC_SIZE_ARRAY(
					data->num_parameters, ccs_expression_t),
				CCS_ALLOC_SIZE_ARRAY(
					data->num_forbidden_clauses,
					ccs_expression_t)),
			CCS_RESULT_ERROR_OUT_OF_MEMORY);
		mem = (uintptr_t)calloc(1, _sz);
		CCS_REFUTE(!mem, CCS_RESULT_ERROR_OUT_OF_MEMORY);
	}

	data->parameters = CCS_ALLOC_CARVE_ARRAY(
		mem, data->num_parameters, ccs_parameter_t);
	data->conditions = CCS_ALLOC_CARVE_ARRAY(
		mem, data->num_parameters, ccs_expression_t);
	data->forbidden_clauses = CCS_ALLOC_CARVE_ARRAY(
		mem, data->num_forbidden_clauses, ccs_expression_t);

	for (size_t i = 0; i < data->num_parameters; i++)
		CCS_VALIDATE(_ccs_object_deserialize_with_opts_check(
			(ccs_object_t *)data->parameters + i,
			CCS_OBJECT_TYPE_PARAMETER, CCS_SERIALIZE_FORMAT_BINARY,
			version, buffer_size, buffer, opts));

	for (size_t i = 0; i < data->num_conditions; i++) {
		size_t index;
		CCS_VALIDATE(
			_ccs_deserialize_bin_size(&index, buffer_size, buffer));
		CCS_REFUTE(
			index >= data->num_parameters,
			CCS_RESULT_ERROR_INVALID_VALUE);
		CCS_VALIDATE(_ccs_object_deserialize_with_opts_check(
			(ccs_object_t *)data->conditions + index,
			CCS_OBJECT_TYPE_EXPRESSION, CCS_SERIALIZE_FORMAT_BINARY,
			version, buffer_size, buffer, opts));
	}

	for (size_t i = 0; i < data->num_forbidden_clauses; i++) {
		CCS_VALIDATE(_ccs_object_deserialize_with_opts_check(
			(ccs_object_t *)data->forbidden_clauses + i,
			CCS_OBJECT_TYPE_EXPRESSION, CCS_SERIALIZE_FORMAT_BINARY,
			version, buffer_size, buffer, opts));
	}

	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_deserialize_bin_configuration_space(
	ccs_configuration_space_t         *configuration_space_ret,
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

	_ccs_configuration_space_data_mock_t data = {
		NULL, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL};
	CCS_VALIDATE_ERR_GOTO(
		res,
		_ccs_deserialize_bin_ccs_configuration_space_data(
			&data, version, buffer_size, buffer, &new_opts),
		end);
	CCS_VALIDATE_ERR_GOTO(
		res,
		ccs_create_configuration_space(
			data.name, data.num_parameters, data.parameters,
			data.conditions, data.num_forbidden_clauses,
			data.forbidden_clauses, data.feature_space, data.rng,
			configuration_space_ret),
		end);

end:
	if (data.feature_space)
		ccs_release_object(data.feature_space);
	if (data.rng)
		ccs_release_object(data.rng);
	if (data.parameters)
		for (size_t i = 0; i < data.num_parameters; i++)
			if (data.parameters[i])
				ccs_release_object(data.parameters[i]);
	if (data.conditions)
		for (size_t i = 0; i < data.num_parameters; i++)
			if (data.conditions[i])
				ccs_release_object(data.conditions[i]);
	if (data.forbidden_clauses)
		for (size_t i = 0; i < data.num_forbidden_clauses; i++)
			if (data.forbidden_clauses[i])
				ccs_release_object(data.forbidden_clauses[i]);
	if (data.parameters)
		free(data.parameters);
	if (!opts->map_values)
		ccs_release_object(new_opts.handle_map);
	return res;
}

static inline ccs_result_t
_ccs_deserialize_json_ccs_configuration_space_data(
	_ccs_configuration_space_data_mock_t *data,
	uint32_t                              version,
	cJSON                                *json,
	_ccs_object_deserialize_options_t    *opts)
{
	cJSON      *j_name;
	cJSON      *j_fs;
	cJSON      *j_rng;
	cJSON      *j_params;
	cJSON      *j_conds;
	cJSON      *j_forbids;
	size_t      num;
	size_t      num_conds;
	size_t      num_forbids;
	uintptr_t   mem;
	const char *cbuf;
	size_t      dummy;

	j_name = cJSON_GetObjectItemCaseSensitive(json, "name");
	CCS_REFUTE(
		!j_name || !cJSON_IsString(j_name),
		CCS_RESULT_ERROR_INVALID_VALUE);
	data->name = j_name->valuestring;

	/* feature space (optional) */
	j_fs       = cJSON_GetObjectItemCaseSensitive(json, "feature_space");
	if (j_fs && cJSON_IsObject(j_fs)) {
		cbuf  = (const char *)j_fs;
		dummy = 0;
		CCS_VALIDATE(_ccs_object_deserialize_with_opts_check(
			(ccs_object_t *)&data->feature_space,
			CCS_OBJECT_TYPE_FEATURE_SPACE,
			CCS_SERIALIZE_FORMAT_JSON, version, &dummy, &cbuf,
			opts));
	}

	/* rng */
	j_rng = cJSON_GetObjectItemCaseSensitive(json, "rng");
	CCS_REFUTE(
		!j_rng || !cJSON_IsObject(j_rng),
		CCS_RESULT_ERROR_INVALID_VALUE);
	cbuf  = (const char *)j_rng;
	dummy = 0;
	CCS_VALIDATE(_ccs_object_deserialize_with_opts_check(
		(ccs_object_t *)&data->rng, CCS_OBJECT_TYPE_RNG,
		CCS_SERIALIZE_FORMAT_JSON, version, &dummy, &cbuf, opts));

	/* parameters */
	j_params = cJSON_GetObjectItemCaseSensitive(json, "parameters");
	CCS_REFUTE(
		!j_params || !cJSON_IsArray(j_params),
		CCS_RESULT_ERROR_INVALID_VALUE);
	num                  = (size_t)cJSON_GetArraySize(j_params);
	data->num_parameters = num;

	/* conditions */
	j_conds = cJSON_GetObjectItemCaseSensitive(json, "conditions");
	CCS_REFUTE(
		!j_conds || !cJSON_IsArray(j_conds),
		CCS_RESULT_ERROR_INVALID_VALUE);
	num_conds            = (size_t)cJSON_GetArraySize(j_conds);
	data->num_conditions = num_conds;

	/* forbidden clauses */
	j_forbids = cJSON_GetObjectItemCaseSensitive(json, "forbidden_clauses");
	CCS_REFUTE(
		!j_forbids || !cJSON_IsArray(j_forbids),
		CCS_RESULT_ERROR_INVALID_VALUE);
	num_forbids                 = (size_t)cJSON_GetArraySize(j_forbids);
	data->num_forbidden_clauses = num_forbids;

	if (!(num + num_conds + num_forbids))
		return CCS_RESULT_SUCCESS;

	{
		size_t _sz;
		CCS_REFUTE(
			CCS_ALLOC_SIZE(
				&_sz,
				CCS_ALLOC_SIZE_ARRAY(num, ccs_parameter_t),
				CCS_ALLOC_SIZE_ARRAY(num, ccs_expression_t),
				CCS_ALLOC_SIZE_ARRAY(
					num_forbids, ccs_expression_t)),
			CCS_RESULT_ERROR_OUT_OF_MEMORY);
		mem = (uintptr_t)calloc(1, _sz);
		CCS_REFUTE(!mem, CCS_RESULT_ERROR_OUT_OF_MEMORY);
	}

	data->parameters = CCS_ALLOC_CARVE_ARRAY(mem, num, ccs_parameter_t);
	data->conditions = CCS_ALLOC_CARVE_ARRAY(mem, num, ccs_expression_t);
	data->forbidden_clauses =
		CCS_ALLOC_CARVE_ARRAY(mem, num_forbids, ccs_expression_t);

	for (size_t i = 0; i < num; i++) {
		cJSON *child = cJSON_GetArrayItem(j_params, (int)i);
		cbuf         = (const char *)child;
		dummy        = 0;
		CCS_VALIDATE(_ccs_object_deserialize_with_opts_check(
			(ccs_object_t *)data->parameters + i,
			CCS_OBJECT_TYPE_PARAMETER, CCS_SERIALIZE_FORMAT_JSON,
			version, &dummy, &cbuf, opts));
	}

	for (size_t i = 0; i < num_conds; i++) {
		cJSON    *cond_obj = cJSON_GetArrayItem(j_conds, (int)i);
		cJSON    *j_index;
		cJSON    *j_expr;
		size_t    index;
		ccs_int_t index_val;
		j_index = cJSON_GetObjectItemCaseSensitive(cond_obj, "index");
		CCS_REFUTE(!j_index, CCS_RESULT_ERROR_INVALID_VALUE);
		CCS_VALIDATE(_ccs_json_get_int(j_index, &index_val));
		index = (size_t)index_val;
		CCS_REFUTE(index >= num, CCS_RESULT_ERROR_INVALID_VALUE);
		j_expr = cJSON_GetObjectItemCaseSensitive(
			cond_obj, "expression");
		CCS_REFUTE(
			!j_expr || !cJSON_IsObject(j_expr),
			CCS_RESULT_ERROR_INVALID_VALUE);
		cbuf  = (const char *)j_expr;
		dummy = 0;
		CCS_VALIDATE(_ccs_object_deserialize_with_opts_check(
			(ccs_object_t *)data->conditions + index,
			CCS_OBJECT_TYPE_EXPRESSION, CCS_SERIALIZE_FORMAT_JSON,
			version, &dummy, &cbuf, opts));
	}

	for (size_t i = 0; i < num_forbids; i++) {
		cJSON *child = cJSON_GetArrayItem(j_forbids, (int)i);
		cbuf         = (const char *)child;
		dummy        = 0;
		CCS_VALIDATE(_ccs_object_deserialize_with_opts_check(
			(ccs_object_t *)data->forbidden_clauses + i,
			CCS_OBJECT_TYPE_EXPRESSION, CCS_SERIALIZE_FORMAT_JSON,
			version, &dummy, &cbuf, opts));
	}

	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_deserialize_json_configuration_space(
	ccs_configuration_space_t         *configuration_space_ret,
	uint32_t                           version,
	size_t                            *buffer_size,
	const char                       **buffer,
	_ccs_object_deserialize_options_t *opts)
{
	_ccs_object_deserialize_options_t    new_opts = *opts;
	ccs_result_t                         res      = CCS_RESULT_SUCCESS;
	_ccs_configuration_space_data_mock_t data     = {
                NULL, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL};

	(void)buffer_size;

	if (!opts->map_values) {
		new_opts.map_values = CCS_TRUE;
		CCS_VALIDATE(ccs_create_map(&new_opts.handle_map));
	}

	CCS_VALIDATE_ERR_GOTO(
		res,
		_ccs_deserialize_json_ccs_configuration_space_data(
			&data, version, *(cJSON **)buffer, &new_opts),
		end);
	CCS_VALIDATE_ERR_GOTO(
		res,
		ccs_create_configuration_space(
			data.name, data.num_parameters, data.parameters,
			data.conditions, data.num_forbidden_clauses,
			data.forbidden_clauses, data.feature_space, data.rng,
			configuration_space_ret),
		end);

end:
	if (data.feature_space)
		ccs_release_object(data.feature_space);
	if (data.rng)
		ccs_release_object(data.rng);
	if (data.parameters)
		for (size_t i = 0; i < data.num_parameters; i++)
			if (data.parameters[i])
				ccs_release_object(data.parameters[i]);
	if (data.conditions)
		for (size_t i = 0; i < data.num_parameters; i++)
			if (data.conditions[i])
				ccs_release_object(data.conditions[i]);
	if (data.forbidden_clauses)
		for (size_t i = 0; i < data.num_forbidden_clauses; i++)
			if (data.forbidden_clauses[i])
				ccs_release_object(data.forbidden_clauses[i]);
	if (data.parameters)
		free(data.parameters);
	if (!opts->map_values)
		ccs_release_object(new_opts.handle_map);
	return res;
}

static ccs_result_t
_ccs_configuration_space_deserialize(
	ccs_configuration_space_t         *configuration_space_ret,
	ccs_serialize_format_t             format,
	uint32_t                           version,
	size_t                            *buffer_size,
	const char                       **buffer,
	_ccs_object_deserialize_options_t *opts)
{
	switch (format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE(_ccs_deserialize_bin_configuration_space(
			configuration_space_ret, version, buffer_size, buffer,
			opts));
		break;
	case CCS_SERIALIZE_FORMAT_JSON:
		CCS_VALIDATE(_ccs_deserialize_json_configuration_space(
			configuration_space_ret, version, buffer_size, buffer,
			opts));
		break;
	default:
		CCS_RAISE(
			CCS_RESULT_ERROR_INVALID_VALUE,
			"Unsupported serialization format: %d", format);
	}
	return CCS_RESULT_SUCCESS;
}

#endif //_CONFIGURATION_SPACE_DESERIALIZE_H
