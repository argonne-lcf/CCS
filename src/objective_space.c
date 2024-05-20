#include "cconfigspace_internal.h"
#include "search_space_internal.h"
#include "objective_space_internal.h"
#include "evaluation_internal.h"
#include "expression_internal.h"

static ccs_result_t
_ccs_objective_space_del(ccs_object_t object)
{
	ccs_objective_space_t objective_space = (ccs_objective_space_t)object;
	_ccs_objective_space_data_t *data     = objective_space->data;
	ccs_search_space_t           search_space   = data->search_space;
	size_t                       num_parameters = data->num_parameters;
	ccs_parameter_t             *parameters     = data->parameters;
	size_t                       num_objectives = data->num_objectives;
	_ccs_objective_t            *objectives     = data->objectives;

	if (search_space)
		ccs_release_object(search_space);
	for (size_t i = 0; i < num_parameters; i++)
		if (parameters[i]) {
			_ccs_parameter_release_ownership(
				parameters[i], (ccs_context_t)objective_space);
			ccs_release_object(parameters[i]);
		}

	for (size_t i = 0; i < num_objectives; i++)
		if (objectives[i].expression)
			ccs_release_object(objectives[i].expression);

	HASH_CLEAR(hh_name, objective_space->data->name_hash);
	HASH_CLEAR(hh_handle, objective_space->data->handle_hash);
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_serialize_bin_size_ccs_objective_space_data(
	_ccs_objective_space_data_t     *data,
	size_t                          *cum_size,
	_ccs_object_serialize_options_t *opts)
{
	ccs_feature_space_t feature_space  = data->feature_space;
	ccs_search_space_t  search_space   = data->search_space;
	size_t              num_parameters = data->num_parameters;
	ccs_parameter_t    *parameters     = data->parameters;
	size_t              num_objectives = data->num_objectives;
	_ccs_objective_t   *objectives     = data->objectives;

	*cum_size += _ccs_serialize_bin_size_string(data->name);

	*cum_size += _ccs_serialize_bin_size_ccs_object(feature_space);
	*cum_size += _ccs_serialize_bin_size_ccs_object(search_space);
	CCS_VALIDATE(_ccs_object_serialize_size_with_opts(
		search_space, CCS_SERIALIZE_FORMAT_BINARY, cum_size, opts));

	*cum_size += _ccs_serialize_bin_size_size(num_parameters);
	*cum_size += _ccs_serialize_bin_size_size(num_objectives);

	/* parameters */
	for (size_t i = 0; i < num_parameters; i++)
		CCS_VALIDATE(_ccs_object_serialize_size_with_opts(
			parameters[i], CCS_SERIALIZE_FORMAT_BINARY, cum_size,
			opts));

	/* objectives */
	for (size_t i = 0; i < num_objectives; i++) {
		CCS_VALIDATE(_ccs_object_serialize_size_with_opts(
			objectives[i].expression, CCS_SERIALIZE_FORMAT_BINARY,
			cum_size, opts));
		*cum_size += _ccs_serialize_bin_size_ccs_objective_type(
			objectives[i].type);
	}

	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_serialize_bin_ccs_objective_space_data(
	_ccs_objective_space_data_t     *data,
	size_t                          *buffer_size,
	char                           **buffer,
	_ccs_object_serialize_options_t *opts)
{
	ccs_feature_space_t feature_space  = data->feature_space;
	ccs_search_space_t  search_space   = data->search_space;
	size_t              num_parameters = data->num_parameters;
	ccs_parameter_t    *parameters     = data->parameters;
	size_t              num_objectives = data->num_objectives;
	_ccs_objective_t   *objectives     = data->objectives;

	CCS_VALIDATE(
		_ccs_serialize_bin_string(data->name, buffer_size, buffer));

	CCS_VALIDATE(_ccs_serialize_bin_ccs_object(
		feature_space, buffer_size, buffer));
	CCS_VALIDATE(_ccs_serialize_bin_ccs_object(
		search_space, buffer_size, buffer));
	CCS_VALIDATE(_ccs_object_serialize_with_opts(
		search_space, CCS_SERIALIZE_FORMAT_BINARY, buffer_size, buffer,
		opts));

	CCS_VALIDATE(
		_ccs_serialize_bin_size(num_parameters, buffer_size, buffer));
	CCS_VALIDATE(
		_ccs_serialize_bin_size(num_objectives, buffer_size, buffer));

	/* parameters */
	for (size_t i = 0; i < num_parameters; i++)
		CCS_VALIDATE(_ccs_object_serialize_with_opts(
			parameters[i], CCS_SERIALIZE_FORMAT_BINARY, buffer_size,
			buffer, opts));

	/* objectives */
	for (size_t i = 0; i < num_objectives; i++) {
		CCS_VALIDATE(_ccs_object_serialize_with_opts(
			objectives[i].expression, CCS_SERIALIZE_FORMAT_BINARY,
			buffer_size, buffer, opts));
		CCS_VALIDATE(_ccs_serialize_bin_ccs_objective_type(
			objectives[i].type, buffer_size, buffer));
	}

	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_serialize_bin_size_ccs_objective_space(
	ccs_objective_space_t            objective_space,
	size_t                          *cum_size,
	_ccs_object_serialize_options_t *opts)
{
	_ccs_objective_space_data_t *data =
		(_ccs_objective_space_data_t *)(objective_space->data);
	CCS_VALIDATE(_ccs_serialize_bin_size_ccs_objective_space_data(
		data, cum_size, opts));
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_serialize_bin_ccs_objective_space(
	ccs_objective_space_t            objective_space,
	size_t                          *buffer_size,
	char                           **buffer,
	_ccs_object_serialize_options_t *opts)
{
	_ccs_objective_space_data_t *data =
		(_ccs_objective_space_data_t *)(objective_space->data);
	CCS_VALIDATE(_ccs_serialize_bin_ccs_objective_space_data(
		data, buffer_size, buffer, opts));
	return CCS_RESULT_SUCCESS;
}

static ccs_result_t
_ccs_objective_space_serialize_size(
	ccs_object_t                     object,
	ccs_serialize_format_t           format,
	size_t                          *cum_size,
	_ccs_object_serialize_options_t *opts)
{
	switch (format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE(_ccs_serialize_bin_size_ccs_objective_space(
			(ccs_objective_space_t)object, cum_size, opts));
		break;
	default:
		CCS_RAISE(
			CCS_RESULT_ERROR_INVALID_VALUE,
			"Unsupported serialization format: %d", format);
	}
	return CCS_RESULT_SUCCESS;
}

static ccs_result_t
_ccs_objective_space_serialize(
	ccs_object_t                     object,
	ccs_serialize_format_t           format,
	size_t                          *buffer_size,
	char                           **buffer,
	_ccs_object_serialize_options_t *opts)
{
	switch (format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE(_ccs_serialize_bin_ccs_objective_space(
			(ccs_objective_space_t)object, buffer_size, buffer,
			opts));
		break;
	default:
		CCS_RAISE(
			CCS_RESULT_ERROR_INVALID_VALUE,
			"Unsupported serialization format: %d", format);
	}
	return CCS_RESULT_SUCCESS;
}

static _ccs_objective_space_ops_t _objective_space_ops = {
	{{&_ccs_objective_space_del, &_ccs_objective_space_serialize_size,
	  &_ccs_objective_space_serialize}}};

static ccs_result_t
_ccs_objective_space_add_parameters(
	ccs_objective_space_t objective_space,
	size_t                num_parameters,
	ccs_parameter_t      *parameters)
{
	for (size_t i = 0; i < num_parameters; i++)
		CCS_VALIDATE(_ccs_context_add_parameter(
			(ccs_context_t)objective_space, parameters[i], i));
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
_ccs_objective_space_add_objectives(
	ccs_objective_space_t objective_space,
	size_t                num_objectives,
	ccs_expression_t     *expressions,
	ccs_objective_type_t *types)
{
	_ccs_objective_t *objectives = objective_space->data->objectives;
	for (size_t i = 0; i < num_objectives; i++) {
		CCS_VALIDATE(ccs_expression_check_contexts(
			expressions[i], 1, (ccs_context_t *)&objective_space));
		CCS_VALIDATE(ccs_retain_object(expressions[i]));
		objectives[i].expression = expressions[i];
		objectives[i].type       = types[i];
	}
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
ccs_create_objective_space(
	const char            *name,
	ccs_search_space_t     search_space,
	size_t                 num_parameters,
	ccs_parameter_t       *parameters,
	size_t                 num_objectives,
	ccs_expression_t      *objectives,
	ccs_objective_type_t  *types,
	ccs_objective_space_t *objective_space_ret)
{
	CCS_CHECK_PTR(name);
	CCS_CHECK_SEARCH_SPACE(search_space);
	CCS_CHECK_PTR(objective_space_ret);
	CCS_CHECK_ARY(num_parameters, parameters);
	CCS_CHECK_ARY(num_objectives, objectives);
	CCS_CHECK_ARY(num_objectives, types);

	uintptr_t mem = (uintptr_t)calloc(
		1,
		sizeof(struct _ccs_objective_space_s) +
			sizeof(struct _ccs_objective_space_data_s) +
			sizeof(ccs_parameter_t) * num_parameters +
			sizeof(_ccs_parameter_index_hash_t) * num_parameters +
			sizeof(_ccs_objective_t) * num_objectives +
			strlen(name) + 1);
	CCS_REFUTE(!mem, CCS_RESULT_ERROR_OUT_OF_MEMORY);
	uintptr_t             mem_orig = mem;
	ccs_result_t          err;
	ccs_objective_space_t obj_space = (ccs_objective_space_t)mem;
	mem += sizeof(struct _ccs_objective_space_s);
	_ccs_object_init(
		&(obj_space->obj), CCS_OBJECT_TYPE_OBJECTIVE_SPACE,
		(_ccs_object_ops_t *)&_objective_space_ops);
	obj_space->data = (struct _ccs_objective_space_data_s *)mem;
	mem += sizeof(struct _ccs_objective_space_data_s);
	obj_space->data->parameters = (ccs_parameter_t *)mem;
	mem += sizeof(ccs_parameter_t) * num_parameters;
	obj_space->data->hash_elems = (_ccs_parameter_index_hash_t *)mem;
	mem += sizeof(_ccs_parameter_index_hash_t) * num_parameters;
	obj_space->data->objectives = (_ccs_objective_t *)mem;
	mem += sizeof(_ccs_objective_t) * num_objectives;
	obj_space->data->name           = (const char *)mem;
	obj_space->data->num_parameters = num_parameters;
	obj_space->data->num_objectives = num_objectives;
	strcpy((char *)(obj_space->data->name), name);
	CCS_VALIDATE_ERR_GOTO(
		err,
		_ccs_objective_space_add_parameters(
			obj_space, num_parameters, parameters),
		errparams);
	CCS_VALIDATE_ERR_GOTO(
		err,
		_ccs_objective_space_add_objectives(
			obj_space, num_objectives, objectives, types),
		errparams);
	CCS_VALIDATE_ERR_GOTO(err, ccs_retain_object(search_space), errparams);
	obj_space->data->search_space = search_space;
	CCS_VALIDATE_ERR_GOTO(
		err,
		_ccs_search_space_get_feature_space(
			search_space, &obj_space->data->feature_space),
		errparams);
	*objective_space_ret = obj_space;
	return CCS_RESULT_SUCCESS;
errparams:
	_ccs_objective_space_del(obj_space);
	_ccs_object_deinit(&(obj_space->obj));
	free((void *)mem_orig);
	return err;
}

ccs_result_t
ccs_objective_space_get_search_space(
	ccs_objective_space_t objective_space,
	ccs_search_space_t   *search_space_ret)
{
	CCS_CHECK_OBJ(objective_space, CCS_OBJECT_TYPE_OBJECTIVE_SPACE);
	CCS_CHECK_PTR(search_space_ret);
	*search_space_ret = objective_space->data->search_space;
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_check_evaluation(
	ccs_objective_space_t objective_space,
	ccs_evaluation_t      evaluation,
	ccs_bool_t           *is_valid_ret)
{
	ccs_parameter_t *parameters     = objective_space->data->parameters;
	size_t           num_parameters = objective_space->data->num_parameters;
	ccs_datum_t     *values         = evaluation->data->values;

	*is_valid_ret                   = CCS_TRUE;
	for (size_t i = 0; i < num_parameters; i++) {
		CCS_VALIDATE(ccs_parameter_check_value(
			parameters[i], values[i], is_valid_ret));
		if (*is_valid_ret == CCS_FALSE)
			return CCS_RESULT_SUCCESS;
	}
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
ccs_objective_space_check_evaluation(
	ccs_objective_space_t objective_space,
	ccs_evaluation_t      evaluation,
	ccs_bool_t           *is_valid_ret)
{
	CCS_CHECK_OBJ(objective_space, CCS_OBJECT_TYPE_OBJECTIVE_SPACE);
	CCS_CHECK_OBJ(evaluation, CCS_OBJECT_TYPE_EVALUATION);
	CCS_REFUTE(
		evaluation->data->objective_space != objective_space,
		CCS_RESULT_ERROR_INVALID_EVALUATION);
	CCS_VALIDATE(
		_check_evaluation(objective_space, evaluation, is_valid_ret));
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
ccs_objective_space_get_objective(
	ccs_objective_space_t objective_space,
	size_t                index,
	ccs_expression_t     *expression_ret,
	ccs_objective_type_t *type_ret)
{
	CCS_CHECK_OBJ(objective_space, CCS_OBJECT_TYPE_OBJECTIVE_SPACE);
	CCS_CHECK_PTR(expression_ret);
	CCS_CHECK_PTR(type_ret);
	CCS_REFUTE(
		index >= objective_space->data->num_objectives,
		CCS_RESULT_ERROR_OUT_OF_BOUNDS);
	_ccs_objective_t *p_obj = objective_space->data->objectives + index;
	*expression_ret         = p_obj->expression;
	*type_ret               = p_obj->type;
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
ccs_objective_space_get_objectives(
	ccs_objective_space_t objective_space,
	size_t                num_objectives,
	ccs_expression_t     *expressions,
	ccs_objective_type_t *types,
	size_t               *num_objectives_ret)
{
	CCS_CHECK_OBJ(objective_space, CCS_OBJECT_TYPE_OBJECTIVE_SPACE);
	CCS_CHECK_ARY(num_objectives, expressions);
	CCS_CHECK_ARY(num_objectives, types);
	CCS_REFUTE(
		!expressions && !num_objectives_ret,
		CCS_RESULT_ERROR_INVALID_VALUE);
	_ccs_objective_t *objectives = objective_space->data->objectives;
	size_t            size       = objective_space->data->num_objectives;
	if (expressions) {
		CCS_REFUTE(
			num_objectives < size, CCS_RESULT_ERROR_INVALID_VALUE);
		for (size_t i = 0; i < size; i++) {
			expressions[i] = objectives[i].expression;
			types[i]       = objectives[i].type;
		}
		for (size_t i = size; i < num_objectives; i++) {
			expressions[i] = NULL;
			types[i]       = CCS_OBJECTIVE_TYPE_MINIMIZE;
		}
	}
	if (num_objectives_ret)
		*num_objectives_ret = size;
	return CCS_RESULT_SUCCESS;
}
