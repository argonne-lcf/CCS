#include "cconfigspace_internal.h"
#include "configuration_space_internal.h"
#include "distribution_space_internal.h"
#include "configuration_internal.h"
#include "distribution_internal.h"
#include "expression_internal.h"
#include "rng_internal.h"
#include "utlist.h"
#include "utarray.h"

static ccs_result_t
_generate_constraints(ccs_configuration_space_t configuration_space);

static ccs_result_t
_ccs_configuration_space_del(ccs_object_t object)
{
	ccs_configuration_space_t configuration_space =
		(ccs_configuration_space_t)object;
	_ccs_configuration_space_data_t *data = configuration_space->data;

	for (size_t i = 0; i < data->num_parameters; i++) {
		if (data->parameters[i])
			ccs_release_object(data->parameters[i]);
		if (data->conditions[i])
			ccs_release_object(data->conditions[i]);
		if (data->parents[i])
			utarray_free(data->parents[i]);
		if (data->children[i])
			utarray_free(data->children[i]);
	}
	for (size_t i = 0; i < data->num_forbidden_clauses; i++)
		if (data->forbidden_clauses[i])
			ccs_release_object(data->forbidden_clauses[i]);

	HASH_CLEAR(hh_name, configuration_space->data->name_hash);
	HASH_CLEAR(hh_handle, configuration_space->data->handle_hash);
	ccs_release_object(configuration_space->data->rng);
	if (configuration_space->data->default_distribution_space) {
		_ccs_distribution_space_del_no_release(
			configuration_space->data->default_distribution_space);
		free(configuration_space->data->default_distribution_space);
	}
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_serialize_bin_size_ccs_configuration_space_data(
	_ccs_configuration_space_data_t *data,
	size_t                          *cum_size,
	_ccs_object_serialize_options_t *opts)
{
	size_t condition_count;

	*cum_size += _ccs_serialize_bin_size_string(data->name);
	*cum_size += _ccs_serialize_bin_size_size(data->num_parameters);

	condition_count = 0;
	for (size_t i = 0; i < data->num_parameters; i++)
		if (data->conditions[i])
			condition_count++;

	*cum_size += _ccs_serialize_bin_size_size(condition_count);
	*cum_size += _ccs_serialize_bin_size_size(data->num_forbidden_clauses);

	/* rng */
	CCS_VALIDATE(data->rng->obj.ops->serialize_size(
		data->rng, CCS_SERIALIZE_FORMAT_BINARY, cum_size, opts));

	/* parameters */
	for (size_t i = 0; i < data->num_parameters; i++)
		CCS_VALIDATE(data->parameters[i]->obj.ops->serialize_size(
			data->parameters[i], CCS_SERIALIZE_FORMAT_BINARY,
			cum_size, opts));

	/* conditions */
	for (size_t i = 0; i < data->num_parameters; i++)
		if (data->conditions[i]) {
			/* parameter index and condition */
			*cum_size += _ccs_serialize_bin_size_size(i);
			CCS_VALIDATE(
				data->conditions[i]->obj.ops->serialize_size(
					data->conditions[i],
					CCS_SERIALIZE_FORMAT_BINARY, cum_size,
					opts));
		}

	/* forbidden clauses */
	for (size_t i = 0; i < data->num_forbidden_clauses; i++)
		CCS_VALIDATE(
			data->forbidden_clauses[i]->obj.ops->serialize_size(
				data->forbidden_clauses[i],
				CCS_SERIALIZE_FORMAT_BINARY, cum_size, opts));

	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_serialize_bin_ccs_configuration_space_data(
	_ccs_configuration_space_data_t *data,
	size_t                          *buffer_size,
	char                           **buffer,
	_ccs_object_serialize_options_t *opts)
{
	size_t condition_count;

	CCS_VALIDATE(
		_ccs_serialize_bin_string(data->name, buffer_size, buffer));
	CCS_VALIDATE(_ccs_serialize_bin_size(
		data->num_parameters, buffer_size, buffer));

	condition_count = 0;
	for (size_t i = 0; i < data->num_parameters; i++)
		if (data->conditions[i])
			condition_count++;
	CCS_VALIDATE(
		_ccs_serialize_bin_size(condition_count, buffer_size, buffer));

	CCS_VALIDATE(_ccs_serialize_bin_size(
		data->num_forbidden_clauses, buffer_size, buffer));

	/* rng */
	CCS_VALIDATE(data->rng->obj.ops->serialize(
		data->rng, CCS_SERIALIZE_FORMAT_BINARY, buffer_size, buffer,
		opts));

	/* parameters */
	for (size_t i = 0; i < data->num_parameters; i++)
		CCS_VALIDATE(data->parameters[i]->obj.ops->serialize(
			data->parameters[i], CCS_SERIALIZE_FORMAT_BINARY,
			buffer_size, buffer, opts));

	/* conditions */
	for (size_t i = 0; i < data->num_parameters; i++)
		if (data->conditions[i]) {
			CCS_VALIDATE(_ccs_serialize_bin_size(
				i, buffer_size, buffer));
			CCS_VALIDATE(data->conditions[i]->obj.ops->serialize(
				data->conditions[i],
				CCS_SERIALIZE_FORMAT_BINARY, buffer_size,
				buffer, opts));
		}

	/* forbidden clauses */
	for (size_t i = 0; i < data->num_forbidden_clauses; i++)
		CCS_VALIDATE(data->forbidden_clauses[i]->obj.ops->serialize(
			data->forbidden_clauses[i], CCS_SERIALIZE_FORMAT_BINARY,
			buffer_size, buffer, opts));

	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_serialize_bin_size_ccs_configuration_space(
	ccs_configuration_space_t        configuration_space,
	size_t                          *cum_size,
	_ccs_object_serialize_options_t *opts)
{
	_ccs_configuration_space_data_t *data =
		(_ccs_configuration_space_data_t *)(configuration_space->data);
	*cum_size += _ccs_serialize_bin_size_ccs_object_internal(
		(_ccs_object_internal_t *)configuration_space);
	CCS_VALIDATE(_ccs_serialize_bin_size_ccs_configuration_space_data(
		data, cum_size, opts));
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_serialize_bin_ccs_configuration_space(
	ccs_configuration_space_t        configuration_space,
	size_t                          *buffer_size,
	char                           **buffer,
	_ccs_object_serialize_options_t *opts)
{
	_ccs_configuration_space_data_t *data =
		(_ccs_configuration_space_data_t *)(configuration_space->data);
	CCS_VALIDATE(_ccs_serialize_bin_ccs_object_internal(
		(_ccs_object_internal_t *)configuration_space, buffer_size,
		buffer));
	CCS_VALIDATE(_ccs_serialize_bin_ccs_configuration_space_data(
		data, buffer_size, buffer, opts));
	return CCS_RESULT_SUCCESS;
}

static ccs_result_t
_ccs_configuration_space_serialize_size(
	ccs_object_t                     object,
	ccs_serialize_format_t           format,
	size_t                          *cum_size,
	_ccs_object_serialize_options_t *opts)
{
	switch (format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE(_ccs_serialize_bin_size_ccs_configuration_space(
			(ccs_configuration_space_t)object, cum_size, opts));
		break;
	default:
		CCS_RAISE(
			CCS_RESULT_ERROR_INVALID_VALUE,
			"Unsupported serialization format: %d", format);
	}
	CCS_VALIDATE(_ccs_object_serialize_user_data_size(
		object, format, cum_size, opts));
	return CCS_RESULT_SUCCESS;
}

static ccs_result_t
_ccs_configuration_space_serialize(
	ccs_object_t                     object,
	ccs_serialize_format_t           format,
	size_t                          *buffer_size,
	char                           **buffer,
	_ccs_object_serialize_options_t *opts)
{
	switch (format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE(_ccs_serialize_bin_ccs_configuration_space(
			(ccs_configuration_space_t)object, buffer_size, buffer,
			opts));
		break;
	default:
		CCS_RAISE(
			CCS_RESULT_ERROR_INVALID_VALUE,
			"Unsupported serialization format: %d", format);
	}
	CCS_VALIDATE(_ccs_object_serialize_user_data(
		object, format, buffer_size, buffer, opts));
	return CCS_RESULT_SUCCESS;
}

static _ccs_configuration_space_ops_t _configuration_space_ops = {
	{{&_ccs_configuration_space_del,
	  &_ccs_configuration_space_serialize_size,
	  &_ccs_configuration_space_serialize}}};

static UT_icd _size_t_icd = {sizeof(size_t), NULL, NULL, NULL};

static ccs_result_t
_ccs_configuration_space_add_parameters(
	ccs_configuration_space_t configuration_space,
	size_t                    num_parameters,
	ccs_parameter_t          *parameters)
{
	for (size_t i = 0; i < num_parameters; i++) {
		CCS_VALIDATE(_ccs_context_add_parameter(
			(ccs_context_t)configuration_space, parameters[i], i));
		configuration_space->data->sorted_indexes[i] = i;
	}
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_configuration_space_add_forbidden_clause(
	ccs_configuration_space_t configuration_space,
	ccs_expression_t          expression,
	size_t                    index,
	ccs_configuration_t       default_config)
{
	CCS_VALIDATE(ccs_expression_check_contexts(
		expression, 1, (ccs_context_t *)&configuration_space));
	ccs_datum_t d;
	CCS_VALIDATE(ccs_expression_eval(
		expression, 1, (ccs_binding_t *)&default_config, &d));
	CCS_REFUTE_MSG(
		d.type == CCS_DATA_TYPE_BOOL && d.value.i == CCS_TRUE,
		CCS_RESULT_ERROR_INVALID_CONFIGURATION,
		"Default configuration is invalid");
	CCS_VALIDATE(ccs_retain_object(expression));
	configuration_space->data->forbidden_clauses[index] = expression;
	return CCS_RESULT_SUCCESS;
}

static ccs_result_t
_ccs_configuration_space_add_forbidden_clauses(
	ccs_configuration_space_t configuration_space,
	size_t                    num_expressions,
	ccs_expression_t         *expressions)
{
	ccs_configuration_t default_config;
	ccs_result_t        err = CCS_RESULT_SUCCESS;
	CCS_VALIDATE(ccs_configuration_space_get_default_configuration(
		configuration_space, &default_config));
	for (size_t i = 0; i < num_expressions; i++)
		CCS_VALIDATE_ERR_GOTO(
			err,
			_ccs_configuration_space_add_forbidden_clause(
				configuration_space, expressions[i], i,
				default_config),
			end);
end:
	ccs_release_object(default_config);
	return err;
}

static int
_size_t_sort(const void *a, const void *b)
{
	const size_t sa = *(const size_t *)a;
	const size_t sb = *(const size_t *)b;
	return sa < sb ? -1 : sa > sb ? 1 : 0;
}

static void
_uniq_size_t_array(UT_array *array)
{
	size_t count = utarray_len(array);
	if (count == 0)
		return;
	utarray_sort(array, &_size_t_sort);
	size_t  real_count = 0;
	size_t *p          = (size_t *)utarray_front(array);
	size_t *p2         = p;
	real_count++;
	while ((p = (size_t *)utarray_next(array, p))) {
		if (*p != *p2) {
			p2  = (size_t *)utarray_next(array, p2);
			*p2 = *p;
			real_count++;
		}
	}
	utarray_resize(array, real_count);
}

struct _parameter_list_s;
struct _parameter_list_s {
	size_t                    in_edges;
	size_t                    index;
	struct _parameter_list_s *next;
	struct _parameter_list_s *prev;
};

static ccs_result_t
_topological_sort(ccs_configuration_space_t configuration_space)
{
	_ccs_configuration_space_data_t *data = configuration_space->data;
	size_t                           num_parameters = data->num_parameters;
	size_t                          *indexes        = data->sorted_indexes;
	UT_array                       **parents        = data->parents;
	UT_array                       **children       = data->children;

	for (size_t i = 0; i < num_parameters; i++)
		indexes[i] = 0;

	struct _parameter_list_s *list = (struct _parameter_list_s *)calloc(
		num_parameters, sizeof(struct _parameter_list_s));
	CCS_REFUTE(!list, CCS_RESULT_ERROR_OUT_OF_MEMORY);
	struct _parameter_list_s *queue = NULL;
	for (size_t index = 0; index < num_parameters; index++) {
		size_t in_edges      = utarray_len(parents[index]);
		list[index].in_edges = in_edges;
		list[index].index    = index;
		if (in_edges == 0)
			DL_APPEND(queue, list + index);
	}
	size_t processed = 0;
	while (queue) {
		struct _parameter_list_s *e = queue;
		DL_DELETE(queue, queue);
		size_t *child = NULL;
		while ((child = (size_t *)utarray_next(
				children[e->index], child))) {
			list[*child].in_edges--;
			if (list[*child].in_edges == 0) {
				DL_APPEND(queue, list + *child);
			}
		}
		indexes[processed++] = e->index;
	};
	free(list);
	CCS_REFUTE(processed < num_parameters, CCS_RESULT_ERROR_INVALID_GRAPH);
	return CCS_RESULT_SUCCESS;
}

#undef utarray_oom
#define utarray_oom()                                                          \
	{                                                                      \
		CCS_RAISE_ERR_GOTO(                                            \
			err, CCS_RESULT_ERROR_OUT_OF_MEMORY, errmem,           \
			"Not enough memory to allocate array");                \
	}
static ccs_result_t
_recompute_graph(ccs_configuration_space_t configuration_space)
{
	_ccs_configuration_space_data_t *data = configuration_space->data;
	size_t                           num_parameters = data->num_parameters;
	UT_array                       **pparents       = data->parents;
	UT_array                       **pchildren      = data->children;
	ccs_expression_t                *conditions     = data->conditions;

	for (size_t index = 0; index < num_parameters; index++) {
		utarray_clear(pparents[index]);
		utarray_clear(pchildren[index]);
	}
	intptr_t     mem = 0;
	ccs_result_t err = CCS_RESULT_SUCCESS;
	for (size_t index = 0; index < num_parameters; index++) {
		ccs_expression_t condition = conditions[index];
		if (!condition)
			continue;
		size_t count;
		CCS_VALIDATE_ERR_GOTO(
			err,
			ccs_expression_get_parameters(
				condition, 0, NULL, &count),
			errmem);
		if (count == 0)
			continue;
		ccs_parameter_t *parents       = NULL;
		size_t          *parents_index = NULL;
		intptr_t         oldmem        = mem;
		mem                            = (intptr_t)realloc(
                        (void *)oldmem,
                        count * (sizeof(ccs_parameter_t) + sizeof(size_t)));
		if (!mem) {
			mem = oldmem;
			CCS_RAISE_ERR_GOTO(
				err, CCS_RESULT_ERROR_OUT_OF_MEMORY, errmem,
				"Not enough memory to reallocate array");
		}
		parents = (ccs_parameter_t *)mem;
		parents_index =
			(size_t *)(mem + count * sizeof(ccs_parameter_t));
		CCS_VALIDATE_ERR_GOTO(
			err,
			ccs_expression_get_parameters(
				condition, count, parents, NULL),
			errmem);
		CCS_VALIDATE_ERR_GOTO(
			err,
			ccs_context_get_parameter_indexes(
				(ccs_context_t)configuration_space, count,
				parents, NULL, parents_index),
			errmem);
		for (size_t i = 0; i < count; i++) {
			utarray_push_back(pparents[index], parents_index + i);
			utarray_push_back(pchildren[parents_index[i]], &index);
		}
	}
	for (size_t index = 0; index < num_parameters; index++) {
		_uniq_size_t_array(pparents[index]);
		_uniq_size_t_array(pchildren[index]);
	}
errmem:
	if (mem)
		free((void *)mem);
	return err;
}
#undef utarray_oom
#define utarray_oom()                                                          \
	{                                                                      \
		exit(-1);                                                      \
	}

static ccs_result_t
_generate_constraints(ccs_configuration_space_t configuration_space)
{
	CCS_VALIDATE(_recompute_graph(configuration_space));
	CCS_VALIDATE(_topological_sort(configuration_space));
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_configuration_space_set_condition(
	ccs_configuration_space_t configuration_space,
	size_t                    parameter_index,
	ccs_expression_t          expression)
{
	CCS_VALIDATE(ccs_expression_check_contexts(
		expression, 1, (ccs_context_t *)&configuration_space));
	CCS_VALIDATE(ccs_retain_object(expression));
	configuration_space->data->conditions[parameter_index] = expression;
	return CCS_RESULT_SUCCESS;
}

static ccs_result_t
_ccs_configuration_space_set_conditions(
	ccs_configuration_space_t configuration_space,
	ccs_expression_t         *conditions)
{
	for (size_t i = 0; i < configuration_space->data->num_parameters; i++) {
		if (conditions[i])
			CCS_VALIDATE(_ccs_configuration_space_set_condition(
				configuration_space, i, conditions[i]));
	}
	CCS_VALIDATE(_generate_constraints(configuration_space));
	return CCS_RESULT_SUCCESS;
}

#undef utarray_oom
#define utarray_oom()                                                          \
	{                                                                      \
		CCS_RAISE_ERR_GOTO(                                            \
			err, CCS_RESULT_ERROR_OUT_OF_MEMORY, errparams,        \
			"Out of memory to allocate array");                    \
	}

ccs_result_t
ccs_create_configuration_space(
	const char                *name,
	size_t                     num_parameters,
	ccs_parameter_t           *parameters,
	ccs_expression_t          *conditions,
	size_t                     num_forbidden_clauses,
	ccs_expression_t          *forbidden_clauses,
	ccs_rng_t                  rng,
	ccs_configuration_space_t *configuration_space_ret)
{
	CCS_CHECK_PTR(name);
	CCS_CHECK_PTR(configuration_space_ret);
	CCS_CHECK_ARY(num_parameters, parameters);
	for (size_t i = 0; i < num_parameters; i++)
		CCS_CHECK_OBJ(parameters[i], CCS_OBJECT_TYPE_PARAMETER);
	if (conditions)
		for (size_t i = 0; i < num_parameters; i++)
			if (conditions[i])
				CCS_CHECK_OBJ(
					conditions[i],
					CCS_OBJECT_TYPE_EXPRESSION);
	CCS_CHECK_ARY(num_forbidden_clauses, forbidden_clauses);
	for (size_t i = 0; i < num_forbidden_clauses; i++)
		CCS_CHECK_OBJ(forbidden_clauses[i], CCS_OBJECT_TYPE_EXPRESSION);
	if (rng)
		CCS_CHECK_OBJ(rng, CCS_OBJECT_TYPE_RNG);

	ccs_result_t err;
	uintptr_t    mem = (uintptr_t)calloc(
                1,
                sizeof(struct _ccs_configuration_space_s) +
                        sizeof(struct _ccs_configuration_space_data_s) +
                        sizeof(ccs_parameter_t) * num_parameters +
                        sizeof(_ccs_parameter_index_hash_t) * num_parameters +
                        sizeof(ccs_expression_t) * num_parameters +
                        sizeof(UT_array *) * num_parameters * 2 +
                        sizeof(size_t) * num_parameters +
                        sizeof(ccs_expression_t) * num_forbidden_clauses +
                        strlen(name) + 1);
	CCS_REFUTE(!mem, CCS_RESULT_ERROR_OUT_OF_MEMORY);
	uintptr_t mem_orig = mem;
	if (rng)
		CCS_VALIDATE_ERR_GOTO(err, ccs_retain_object(rng), errmem);
	else
		CCS_VALIDATE_ERR_GOTO(err, ccs_create_rng(&rng), errmem);

	ccs_configuration_space_t config_space;
	config_space = (ccs_configuration_space_t)mem;
	mem += sizeof(struct _ccs_configuration_space_s);
	_ccs_object_init(
		&(config_space->obj), CCS_OBJECT_TYPE_CONFIGURATION_SPACE,
		(_ccs_object_ops_t *)&_configuration_space_ops);
	config_space->data = (struct _ccs_configuration_space_data_s *)mem;
	mem += sizeof(struct _ccs_configuration_space_data_s);
	config_space->data->parameters = (ccs_parameter_t *)mem;
	mem += sizeof(ccs_parameter_t) * num_parameters;
	config_space->data->hash_elems = (_ccs_parameter_index_hash_t *)mem;
	mem += sizeof(_ccs_parameter_index_hash_t) * num_parameters;
	config_space->data->conditions = (ccs_expression_t *)mem;
	mem += sizeof(ccs_expression_t) * num_parameters;
	config_space->data->parents = (UT_array **)mem;
	mem += sizeof(UT_array *) * num_parameters;
	config_space->data->children = (UT_array **)mem;
	mem += sizeof(UT_array *) * num_parameters;
	config_space->data->sorted_indexes = (size_t *)mem;
	mem += sizeof(size_t) * num_parameters;
	config_space->data->forbidden_clauses = (ccs_expression_t *)mem;
	mem += sizeof(ccs_expression_t) * num_forbidden_clauses;
	config_space->data->name                  = (const char *)mem;
	config_space->data->num_parameters        = num_parameters;
	config_space->data->num_forbidden_clauses = num_forbidden_clauses;
	config_space->data->rng                   = rng;
	strcpy((char *)(config_space->data->name), name);
	CCS_VALIDATE_ERR_GOTO(
		err,
		_ccs_configuration_space_add_parameters(
			config_space, num_parameters, parameters),
		errparams);
	CCS_VALIDATE_ERR_GOTO(
		err,
		_ccs_configuration_space_add_forbidden_clauses(
			config_space, num_forbidden_clauses, forbidden_clauses),
		errparams);
	CCS_VALIDATE_ERR_GOTO(
		err,
		_ccs_create_distribution_space_no_retain(
			config_space, NULL,
			&config_space->data->default_distribution_space),
		errparams);
	for (size_t i = 0; i < num_parameters; i++) {
		utarray_new(config_space->data->parents[i], &_size_t_icd);
		utarray_new(config_space->data->children[i], &_size_t_icd);
	}
	if (conditions)
		CCS_VALIDATE_ERR_GOTO(
			err,
			_ccs_configuration_space_set_conditions(
				config_space, conditions),
			errparams);
	*configuration_space_ret = config_space;
	return CCS_RESULT_SUCCESS;
errparams:
	_ccs_configuration_space_del(config_space);
	_ccs_object_deinit(&(config_space->obj));
errmem:
	free((void *)mem_orig);
	return err;
}
#undef utarray_oom
#define utarray_oom() exit(-1)

ccs_result_t
ccs_configuration_space_get_rng(
	ccs_configuration_space_t configuration_space,
	ccs_rng_t                *rng_ret)
{
	CCS_CHECK_OBJ(configuration_space, CCS_OBJECT_TYPE_CONFIGURATION_SPACE);
	CCS_CHECK_PTR(rng_ret);
	*rng_ret = configuration_space->data->rng;
	return CCS_RESULT_SUCCESS;
}

static ccs_result_t
_set_actives(
	ccs_configuration_space_t configuration_space,
	ccs_configuration_t       configuration)
{
	size_t           *indexes = configuration_space->data->sorted_indexes;
	ccs_expression_t *conditions = configuration_space->data->conditions;
	ccs_datum_t      *values     = configuration->data->values;

	for (size_t i = 0; i < configuration_space->data->num_parameters; i++) {
		size_t index = indexes[i];
		if (!conditions[index])
			continue;
		ccs_datum_t result;
		CCS_VALIDATE(ccs_expression_eval(
			conditions[index], 1, (ccs_binding_t *)&configuration,
			&result));
		if (!(result.type == CCS_DATA_TYPE_BOOL &&
		      result.value.i == CCS_TRUE))
			values[index] = ccs_inactive;
	}
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
ccs_configuration_space_get_default_configuration(
	ccs_configuration_space_t configuration_space,
	ccs_configuration_t      *configuration_ret)
{
	CCS_CHECK_OBJ(configuration_space, CCS_OBJECT_TYPE_CONFIGURATION_SPACE);
	CCS_CHECK_PTR(configuration_ret);
	ccs_result_t        err;
	ccs_configuration_t config;
	CCS_VALIDATE(_ccs_create_configuration(
		configuration_space, 0, NULL, &config));
	ccs_parameter_t *parameters = configuration_space->data->parameters;
	ccs_datum_t     *values     = config->data->values;
	for (size_t i = 0; i < configuration_space->data->num_parameters; i++)
		CCS_VALIDATE_ERR_GOTO(
			err,
			ccs_parameter_get_default_value(
				parameters[i], values + i),
			errc);
	CCS_VALIDATE_ERR_GOTO(
		err, _set_actives(configuration_space, config), errc);
	*configuration_ret = config;
	return CCS_RESULT_SUCCESS;
errc:
	ccs_release_object(config);
	return err;
}

static ccs_result_t
_test_forbidden(
	ccs_configuration_space_t configuration_space,
	ccs_configuration_t       configuration,
	ccs_bool_t               *is_valid)
{
	ccs_expression_t *forbidden_clauses =
		configuration_space->data->forbidden_clauses;
	*is_valid = CCS_FALSE;
	for (size_t i = 0; i < configuration_space->data->num_forbidden_clauses;
	     i++) {
		ccs_datum_t result;
		CCS_VALIDATE(ccs_expression_eval(
			forbidden_clauses[i], 1,
			(ccs_binding_t *)&configuration, &result));
		if (result.type == CCS_DATA_TYPE_INACTIVE)
			continue;
		if (result.type == CCS_DATA_TYPE_BOOL &&
		    result.value.i == CCS_TRUE)
			return CCS_RESULT_SUCCESS;
	}
	*is_valid = CCS_TRUE;
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_check_configuration(
	ccs_configuration_space_t configuration_space,
	ccs_configuration_t       configuration,
	ccs_bool_t               *is_valid_ret)
{
	size_t           *indexes = configuration_space->data->sorted_indexes;
	ccs_parameter_t  *parameters = configuration_space->data->parameters;
	ccs_expression_t *conditions = configuration_space->data->conditions;
	ccs_datum_t      *values     = configuration->data->values;

	for (size_t i = 0; i < configuration_space->data->num_parameters; i++) {
		ccs_bool_t active = CCS_TRUE;
		size_t     index  = indexes[i];
		if (conditions[index]) {
			ccs_datum_t result;
			CCS_VALIDATE(ccs_expression_eval(
				conditions[index], 1,
				(ccs_binding_t *)&configuration, &result));
			if (!(result.type == CCS_DATA_TYPE_BOOL &&
			      result.value.i == CCS_TRUE))
				active = CCS_FALSE;
		}
		if (active != (values[index].type == CCS_DATA_TYPE_INACTIVE ?
				       CCS_FALSE :
				       CCS_TRUE)) {
			*is_valid_ret = CCS_FALSE;
			return CCS_RESULT_SUCCESS;
		}
		if (active) {
			CCS_VALIDATE(ccs_parameter_check_value(
				parameters[index], values[index],
				is_valid_ret));
			if (*is_valid_ret == CCS_FALSE)
				return CCS_RESULT_SUCCESS;
		}
	}
	CCS_VALIDATE(_test_forbidden(
		configuration_space, configuration, is_valid_ret));
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
ccs_configuration_space_check_configuration(
	ccs_configuration_space_t configuration_space,
	ccs_configuration_t       configuration,
	ccs_bool_t               *is_valid_ret)
{
	CCS_CHECK_OBJ(configuration_space, CCS_OBJECT_TYPE_CONFIGURATION_SPACE);
	CCS_CHECK_OBJ(configuration, CCS_OBJECT_TYPE_CONFIGURATION);
	CCS_REFUTE(
		configuration->data->configuration_space != configuration_space,
		CCS_RESULT_ERROR_INVALID_CONFIGURATION);
	CCS_VALIDATE(_check_configuration(
		configuration_space, configuration, is_valid_ret));
	return CCS_RESULT_SUCCESS;
}

static ccs_result_t
_sample(ccs_configuration_space_t configuration_space,
	ccs_distribution_space_t  distribution_space,
	ccs_rng_t                 rng,
	ccs_configuration_t       config,
	ccs_bool_t               *found)
{
	ccs_result_t     err        = CCS_RESULT_SUCCESS;
	ccs_parameter_t *parameters = configuration_space->data->parameters;
	ccs_datum_t     *values     = config->data->values;
	size_t       num_parameters = configuration_space->data->num_parameters;
	ccs_datum_t *p_values;
	ccs_parameter_t *hps;
	uintptr_t        mem;
	mem = (uintptr_t)malloc(
		num_parameters *
		(sizeof(ccs_datum_t) + sizeof(ccs_parameter_t)));
	CCS_REFUTE(!mem, CCS_RESULT_ERROR_OUT_OF_MEMORY);

	p_values = (ccs_datum_t *)mem;
	hps = (ccs_parameter_t *)(mem + num_parameters * sizeof(ccs_datum_t));

	_ccs_distribution_wrapper_t *dwrapper = NULL;
	DL_FOREACH(distribution_space->data->distribution_list, dwrapper)
	{
		for (size_t i = 0; i < dwrapper->dimension; i++) {
			size_t hindex = dwrapper->parameter_indexes[i];
			hps[i]        = parameters[hindex];
		}
		CCS_VALIDATE_ERR_GOTO(
			err,
			ccs_distribution_parameters_sample(
				dwrapper->distribution, rng, hps, p_values),
			errmem);
		for (size_t i = 0; i < dwrapper->dimension; i++) {
			size_t hindex  = dwrapper->parameter_indexes[i];
			values[hindex] = p_values[i];
		}
	}
	CCS_VALIDATE_ERR_GOTO(
		err, _set_actives(configuration_space, config), errmem);
	CCS_VALIDATE_ERR_GOTO(
		err, _test_forbidden(configuration_space, config, found),
		errmem);
errmem:
	free((void *)mem);
	return err;
}

static inline ccs_result_t
_ccs_configuration_space_samples(
	ccs_configuration_space_t configuration_space,
	ccs_distribution_space_t  distrib_space,
	ccs_rng_t                 rng,
	size_t                    num_configurations,
	ccs_configuration_t      *configurations)
{
	ccs_result_t             err     = CCS_RESULT_SUCCESS;
	size_t                   counter = 0;
	size_t                   count   = 0;
	ccs_bool_t               found;
	ccs_configuration_t      config = NULL;
	ccs_distribution_space_t distribution_space;

	if (distrib_space) {
		distribution_space = distrib_space;
		CCS_OBJ_RDLOCK(distrib_space);
	} else
		distribution_space =
			configuration_space->data->default_distribution_space;

	if (!rng)
		rng = configuration_space->data->rng;

	for (size_t i = 0; i < num_configurations; i++)
		configurations[i] = NULL;
	while (count < num_configurations &&
	       counter < 100 * num_configurations) {
		if (!config)
			CCS_VALIDATE_ERR_GOTO(
				err,
				_ccs_create_configuration(
					configuration_space, 0, NULL, &config),
				end);
		CCS_VALIDATE_ERR_GOTO(
			err,
			_sample(configuration_space, distribution_space, rng,
				config, &found),
			errconf);
		if (found) {
			configurations[count++] = config;
			config                  = NULL;
		}
		counter++;
	}
	CCS_REFUTE_ERR_GOTO(
		err, count < num_configurations,
		CCS_RESULT_ERROR_SAMPLING_UNSUCCESSFUL, errconf);
	goto end;

errconf:
	if (config)
		ccs_release_object(config);
end:
	if (distrib_space)
		CCS_OBJ_UNLOCK(distrib_space);
	return err;
}

ccs_result_t
ccs_configuration_space_sample(
	ccs_configuration_space_t configuration_space,
	ccs_distribution_space_t  distribution_space,
	ccs_rng_t                 rng,
	ccs_configuration_t      *configuration_ret)
{
	CCS_CHECK_OBJ(configuration_space, CCS_OBJECT_TYPE_CONFIGURATION_SPACE);
	if (distribution_space) {
		CCS_CHECK_OBJ(
			distribution_space, CCS_OBJECT_TYPE_DISTRIBUTION_SPACE);
		CCS_REFUTE(
			distribution_space->data->configuration_space !=
				configuration_space,
			CCS_RESULT_ERROR_INVALID_DISTRIBUTION_SPACE);
	}
	if (rng)
		CCS_CHECK_OBJ(rng, CCS_OBJECT_TYPE_RNG);
	CCS_CHECK_PTR(configuration_ret);
	CCS_VALIDATE(_ccs_configuration_space_samples(
		configuration_space, distribution_space, rng, 1,
		configuration_ret));
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
ccs_configuration_space_samples(
	ccs_configuration_space_t configuration_space,
	ccs_distribution_space_t  distribution_space,
	ccs_rng_t                 rng,
	size_t                    num_configurations,
	ccs_configuration_t      *configurations)
{
	CCS_CHECK_OBJ(configuration_space, CCS_OBJECT_TYPE_CONFIGURATION_SPACE);
	if (distribution_space) {
		CCS_CHECK_OBJ(
			distribution_space, CCS_OBJECT_TYPE_DISTRIBUTION_SPACE);
		CCS_REFUTE(
			distribution_space->data->configuration_space !=
				configuration_space,
			CCS_RESULT_ERROR_INVALID_DISTRIBUTION_SPACE);
	}
	if (rng)
		CCS_CHECK_OBJ(rng, CCS_OBJECT_TYPE_RNG);
	CCS_CHECK_ARY(num_configurations, configurations);
	if (!num_configurations)
		return CCS_RESULT_SUCCESS;
	CCS_VALIDATE(_ccs_configuration_space_samples(
		configuration_space, distribution_space, rng,
		num_configurations, configurations));
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
ccs_configuration_space_get_condition(
	ccs_configuration_space_t configuration_space,
	size_t                    parameter_index,
	ccs_expression_t         *expression_ret)
{
	CCS_CHECK_OBJ(configuration_space, CCS_OBJECT_TYPE_CONFIGURATION_SPACE);
	CCS_CHECK_PTR(expression_ret);
	CCS_REFUTE(
		parameter_index >= configuration_space->data->num_parameters,
		CCS_RESULT_ERROR_OUT_OF_BOUNDS);
	*expression_ret =
		configuration_space->data->conditions[parameter_index];
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
ccs_configuration_space_get_conditions(
	ccs_configuration_space_t configuration_space,
	size_t                    num_expressions,
	ccs_expression_t         *expressions,
	size_t                   *num_expressions_ret)
{
	CCS_CHECK_OBJ(configuration_space, CCS_OBJECT_TYPE_CONFIGURATION_SPACE);
	CCS_CHECK_ARY(num_expressions, expressions);
	CCS_REFUTE(
		!expressions && !num_expressions_ret,
		CCS_RESULT_ERROR_INVALID_VALUE);
	ccs_expression_t *conditions = configuration_space->data->conditions;
	size_t num_parameters = configuration_space->data->num_parameters;
	if (expressions) {
		CCS_REFUTE(
			num_expressions < num_parameters,
			CCS_RESULT_ERROR_INVALID_VALUE);
		for (size_t i = 0; i < num_parameters; i++)
			expressions[i] = conditions[i];
		for (size_t i = num_parameters; i < num_expressions; i++)
			expressions[i] = NULL;
	}
	if (num_expressions_ret)
		*num_expressions_ret = num_parameters;
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
ccs_configuration_space_get_forbidden_clause(
	ccs_configuration_space_t configuration_space,
	size_t                    index,
	ccs_expression_t         *expression_ret)
{
	CCS_CHECK_OBJ(configuration_space, CCS_OBJECT_TYPE_CONFIGURATION_SPACE);
	CCS_CHECK_PTR(expression_ret);
	CCS_REFUTE(
		index >= configuration_space->data->num_forbidden_clauses,
		CCS_RESULT_ERROR_OUT_OF_BOUNDS);
	*expression_ret = configuration_space->data->forbidden_clauses[index];
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
ccs_configuration_space_get_forbidden_clauses(
	ccs_configuration_space_t configuration_space,
	size_t                    num_expressions,
	ccs_expression_t         *expressions,
	size_t                   *num_expressions_ret)
{
	CCS_CHECK_OBJ(configuration_space, CCS_OBJECT_TYPE_CONFIGURATION_SPACE);
	CCS_CHECK_ARY(num_expressions, expressions);
	CCS_REFUTE(
		!expressions && !num_expressions_ret,
		CCS_RESULT_ERROR_INVALID_VALUE);
	ccs_expression_t *forbidden_clauses =
		configuration_space->data->forbidden_clauses;
	size_t num_forbidden_clauses =
		configuration_space->data->num_forbidden_clauses;
	if (expressions) {
		CCS_REFUTE(
			num_expressions < num_forbidden_clauses,
			CCS_RESULT_ERROR_INVALID_VALUE);
		for (size_t i = 0; i < num_forbidden_clauses; i++)
			expressions[i] = forbidden_clauses[i];
		for (size_t i = num_forbidden_clauses; i < num_expressions; i++)
			expressions[i] = NULL;
	}
	if (num_expressions_ret)
		*num_expressions_ret = num_forbidden_clauses;
	return CCS_RESULT_SUCCESS;
}
