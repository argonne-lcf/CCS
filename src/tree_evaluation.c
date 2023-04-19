#include "cconfigspace_internal.h"
#include "tree_evaluation_internal.h"
#include "tree_configuration_internal.h"
#include <string.h>

static inline _ccs_tree_evaluation_ops_t *
ccs_tree_evaluation_get_ops(ccs_tree_evaluation_t evaluation)
{
	return (_ccs_tree_evaluation_ops_t *)evaluation->obj.ops;
}

static ccs_error_t
_ccs_tree_evaluation_del(ccs_object_t object)
{
	ccs_tree_evaluation_t evaluation = (ccs_tree_evaluation_t)object;
	ccs_release_object(evaluation->data->objective_space);
	ccs_release_object(evaluation->data->configuration);
	return CCS_SUCCESS;
}

static inline ccs_error_t
_ccs_serialize_bin_size_ccs_tree_evaluation_data(
	_ccs_tree_evaluation_data_t     *data,
	size_t                          *cum_size,
	_ccs_object_serialize_options_t *opts)
{
	*cum_size += _ccs_serialize_bin_size_ccs_binding_data(
		(_ccs_binding_data_t *)data);
	CCS_VALIDATE(data->configuration->obj.ops->serialize_size(
		data->configuration, CCS_SERIALIZE_FORMAT_BINARY, cum_size,
		opts));
	*cum_size +=
		_ccs_serialize_bin_size_ccs_evaluation_result(data->result);
	return CCS_SUCCESS;
}

static inline ccs_error_t
_ccs_serialize_bin_ccs_tree_evaluation_data(
	_ccs_tree_evaluation_data_t     *data,
	size_t                          *buffer_size,
	char                           **buffer,
	_ccs_object_serialize_options_t *opts)
{
	CCS_VALIDATE(_ccs_serialize_bin_ccs_binding_data(
		(_ccs_binding_data_t *)data, buffer_size, buffer));
	CCS_VALIDATE(data->configuration->obj.ops->serialize(
		data->configuration, CCS_SERIALIZE_FORMAT_BINARY, buffer_size,
		buffer, opts));
	CCS_VALIDATE(_ccs_serialize_bin_ccs_evaluation_result(
		data->result, buffer_size, buffer));
	return CCS_SUCCESS;
}

static inline ccs_error_t
_ccs_serialize_bin_size_ccs_tree_evaluation(
	ccs_tree_evaluation_t            evaluation,
	size_t                          *cum_size,
	_ccs_object_serialize_options_t *opts)
{
	*cum_size += _ccs_serialize_bin_size_ccs_object_internal(
		(_ccs_object_internal_t *)evaluation);
	CCS_VALIDATE(_ccs_serialize_bin_size_ccs_tree_evaluation_data(
		evaluation->data, cum_size, opts));
	return CCS_SUCCESS;
}

static inline ccs_error_t
_ccs_serialize_bin_ccs_tree_evaluation(
	ccs_tree_evaluation_t            evaluation,
	size_t                          *buffer_size,
	char                           **buffer,
	_ccs_object_serialize_options_t *opts)
{
	CCS_VALIDATE(_ccs_serialize_bin_ccs_object_internal(
		(_ccs_object_internal_t *)evaluation, buffer_size, buffer));
	CCS_VALIDATE(_ccs_serialize_bin_ccs_tree_evaluation_data(
		evaluation->data, buffer_size, buffer, opts));
	return CCS_SUCCESS;
}

static ccs_error_t
_ccs_tree_evaluation_serialize_size(
	ccs_object_t                     object,
	ccs_serialize_format_t           format,
	size_t                          *cum_size,
	_ccs_object_serialize_options_t *opts)
{
	switch (format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE(_ccs_serialize_bin_size_ccs_tree_evaluation(
			(ccs_tree_evaluation_t)object, cum_size, opts));
		break;
	default:
		CCS_RAISE(
			CCS_INVALID_VALUE,
			"Unsupported serialization format: %d", format);
	}
	CCS_VALIDATE(_ccs_object_serialize_user_data_size(
		object, format, cum_size, opts));
	return CCS_SUCCESS;
}

static ccs_error_t
_ccs_tree_evaluation_serialize(
	ccs_object_t                     object,
	ccs_serialize_format_t           format,
	size_t                          *buffer_size,
	char                           **buffer,
	_ccs_object_serialize_options_t *opts)
{
	switch (format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE(_ccs_serialize_bin_ccs_tree_evaluation(
			(ccs_tree_evaluation_t)object, buffer_size, buffer,
			opts));
		break;
	default:
		CCS_RAISE(
			CCS_INVALID_VALUE,
			"Unsupported serialization format: %d", format);
	}
	CCS_VALIDATE(_ccs_object_serialize_user_data(
		object, format, buffer_size, buffer, opts));
	return CCS_SUCCESS;
}

static ccs_error_t
_ccs_tree_evaluation_hash(
	_ccs_tree_evaluation_data_t *data,
	ccs_hash_t                  *hash_ret)
{
	ccs_hash_t h, ht;
	CCS_VALIDATE(_ccs_binding_hash((_ccs_binding_data_t *)data, &h));
	CCS_VALIDATE(ccs_tree_configuration_hash(data->configuration, &ht));
	h = _hash_combine(h, ht);
	HASH_JEN(&(data->result), sizeof(data->result), ht);
	h         = _hash_combine(h, ht);
	*hash_ret = h;
	return CCS_SUCCESS;
}

static ccs_error_t
_ccs_tree_evaluation_cmp(
	_ccs_tree_evaluation_data_t *data,
	ccs_tree_evaluation_t        other,
	int                         *cmp_ret)
{
	CCS_VALIDATE(_ccs_binding_cmp(
		(_ccs_binding_data_t *)data, (ccs_binding_t)other, cmp_ret));
	if (*cmp_ret)
		return CCS_SUCCESS;
	_ccs_tree_evaluation_data_t *other_data = other->data;
	*cmp_ret = data->result < other_data->result ? -1 :
		   data->result > other_data->result ? 1 :
						       0;
	if (*cmp_ret)
		return CCS_SUCCESS;
	CCS_VALIDATE(ccs_tree_configuration_cmp(
		data->configuration, other_data->configuration, cmp_ret));
	return CCS_SUCCESS;
}

static _ccs_tree_evaluation_ops_t _evaluation_ops = {
	{&_ccs_tree_evaluation_del, &_ccs_tree_evaluation_serialize_size,
	 &_ccs_tree_evaluation_serialize},
	&_ccs_tree_evaluation_hash,
	&_ccs_tree_evaluation_cmp};

ccs_error_t
ccs_create_tree_evaluation(
	ccs_objective_space_t    objective_space,
	ccs_tree_configuration_t configuration,
	ccs_evaluation_result_t  result,
	size_t                   num_values,
	ccs_datum_t             *values,
	ccs_tree_evaluation_t   *evaluation_ret)
{
	CCS_CHECK_OBJ(objective_space, CCS_OBJECT_TYPE_OBJECTIVE_SPACE);
	CCS_CHECK_OBJ(configuration, CCS_OBJECT_TYPE_TREE_CONFIGURATION);
	CCS_CHECK_PTR(evaluation_ret);
	CCS_CHECK_ARY(num_values, values);
	ccs_error_t err;
	size_t      num;
	CCS_VALIDATE(
		ccs_objective_space_get_num_parameters(objective_space, &num));
	CCS_REFUTE(values && num != num_values, CCS_INVALID_VALUE);
	uintptr_t mem = (uintptr_t)calloc(
		1, sizeof(struct _ccs_tree_evaluation_s) +
			   sizeof(struct _ccs_tree_evaluation_data_s) +
			   num * sizeof(ccs_datum_t));
	CCS_REFUTE(!mem, CCS_OUT_OF_MEMORY);
	CCS_VALIDATE_ERR_GOTO(err, ccs_retain_object(objective_space), errmem);
	CCS_VALIDATE_ERR_GOTO(err, ccs_retain_object(configuration), erros);
	ccs_tree_evaluation_t eval;
	eval = (ccs_tree_evaluation_t)mem;
	_ccs_object_init(
		&(eval->obj), CCS_OBJECT_TYPE_TREE_EVALUATION,
		(_ccs_object_ops_t *)&_evaluation_ops);
	eval->data                  = (struct _ccs_tree_evaluation_data_s
                              *)(mem + sizeof(struct _ccs_tree_evaluation_s));
	eval->data->num_values      = num;
	eval->data->objective_space = objective_space;
	eval->data->configuration   = configuration;
	eval->data->result          = result;
	eval->data->values =
		(ccs_datum_t
			 *)(mem + sizeof(struct _ccs_tree_evaluation_s) + sizeof(struct _ccs_tree_evaluation_data_s));
	if (values) {
		memcpy(eval->data->values, values, num * sizeof(ccs_datum_t));
		for (size_t i = 0; i < num_values; i++)
			if (values[i].flags & CCS_FLAG_TRANSIENT)
				CCS_VALIDATE_ERR_GOTO(
					err,
					ccs_objective_space_validate_value(
						objective_space, i, values[i],
						eval->data->values + i),
					errc);
	}
	*evaluation_ret = eval;
	return CCS_SUCCESS;
errc:
	ccs_release_object(configuration);
erros:
	ccs_release_object(objective_space);
errmem:
	free((void *)mem);
	return err;
}

ccs_error_t
ccs_tree_evaluation_get_objective_space(
	ccs_tree_evaluation_t  evaluation,
	ccs_objective_space_t *objective_space_ret)
{
	CCS_CHECK_OBJ(evaluation, CCS_OBJECT_TYPE_TREE_EVALUATION);
	CCS_VALIDATE(_ccs_binding_get_context(
		(ccs_binding_t)evaluation,
		(ccs_context_t *)objective_space_ret));
	return CCS_SUCCESS;
}

ccs_error_t
ccs_tree_evaluation_get_configuration(
	ccs_tree_evaluation_t     evaluation,
	ccs_tree_configuration_t *configuration_ret)
{
	CCS_CHECK_OBJ(evaluation, CCS_OBJECT_TYPE_TREE_EVALUATION);
	CCS_CHECK_PTR(configuration_ret);
	*configuration_ret = evaluation->data->configuration;
	return CCS_SUCCESS;
}

ccs_error_t
ccs_tree_evaluation_get_result(
	ccs_tree_evaluation_t    evaluation,
	ccs_evaluation_result_t *result_ret)
{
	CCS_CHECK_OBJ(evaluation, CCS_OBJECT_TYPE_TREE_EVALUATION);
	CCS_CHECK_PTR(result_ret);
	*result_ret = evaluation->data->result;
	return CCS_SUCCESS;
}

ccs_error_t
ccs_tree_evaluation_set_result(
	ccs_tree_evaluation_t   evaluation,
	ccs_evaluation_result_t result)
{
	CCS_CHECK_OBJ(evaluation, CCS_OBJECT_TYPE_TREE_EVALUATION);
	evaluation->data->result = result;
	return CCS_SUCCESS;
}

ccs_error_t
ccs_tree_evaluation_get_value(
	ccs_tree_evaluation_t evaluation,
	size_t                index,
	ccs_datum_t          *value_ret)
{
	CCS_CHECK_OBJ(evaluation, CCS_OBJECT_TYPE_TREE_EVALUATION);
	CCS_VALIDATE(_ccs_binding_get_value(
		(ccs_binding_t)evaluation, index, value_ret));
	return CCS_SUCCESS;
}

ccs_error_t
ccs_tree_evaluation_set_value(
	ccs_tree_evaluation_t evaluation,
	size_t                index,
	ccs_datum_t           value)
{
	CCS_CHECK_OBJ(evaluation, CCS_OBJECT_TYPE_TREE_EVALUATION);
	CCS_VALIDATE(_ccs_binding_set_value(
		(ccs_binding_t)evaluation, index, value));
	return CCS_SUCCESS;
}

ccs_error_t
ccs_tree_evaluation_get_values(
	ccs_tree_evaluation_t evaluation,
	size_t                num_values,
	ccs_datum_t          *values,
	size_t               *num_values_ret)
{
	CCS_CHECK_OBJ(evaluation, CCS_OBJECT_TYPE_TREE_EVALUATION);
	CCS_VALIDATE(_ccs_binding_get_values(
		(ccs_binding_t)evaluation, num_values, values, num_values_ret));
	return CCS_SUCCESS;
}

ccs_error_t
ccs_tree_evaluation_get_value_by_name(
	ccs_tree_evaluation_t evaluation,
	const char           *name,
	ccs_datum_t          *value_ret)
{
	CCS_CHECK_OBJ(evaluation, CCS_OBJECT_TYPE_TREE_EVALUATION);
	CCS_VALIDATE(_ccs_binding_get_value_by_name(
		(ccs_binding_t)evaluation, name, value_ret));
	return CCS_SUCCESS;
}

ccs_error_t
ccs_tree_evaluation_check(
	ccs_tree_evaluation_t evaluation,
	ccs_bool_t           *is_valid_ret)
{
	CCS_CHECK_OBJ(evaluation, CCS_OBJECT_TYPE_TREE_EVALUATION);
	CCS_VALIDATE(ccs_objective_space_check_evaluation_values(
		evaluation->data->objective_space, evaluation->data->num_values,
		evaluation->data->values, is_valid_ret));
	return CCS_SUCCESS;
}

ccs_error_t
ccs_tree_evaluation_get_objective_value(
	ccs_tree_evaluation_t evaluation,
	size_t                index,
	ccs_datum_t          *value_ret)
{
	CCS_CHECK_OBJ(evaluation, CCS_OBJECT_TYPE_TREE_EVALUATION);
	CCS_CHECK_PTR(value_ret);
	ccs_expression_t     expression;
	ccs_objective_type_t type;
	CCS_VALIDATE(ccs_objective_space_get_objective(
		evaluation->data->objective_space, index, &expression, &type));
	CCS_VALIDATE(ccs_expression_eval(
		expression, (ccs_context_t)evaluation->data->objective_space,
		evaluation->data->values, value_ret));
	return CCS_SUCCESS;
}

ccs_error_t
ccs_tree_evaluation_get_objective_values(
	ccs_tree_evaluation_t evaluation,
	size_t                num_values,
	ccs_datum_t          *values,
	size_t               *num_values_ret)
{
	CCS_CHECK_OBJ(evaluation, CCS_OBJECT_TYPE_TREE_EVALUATION);
	CCS_CHECK_ARY(num_values, values);
	CCS_REFUTE(!values && !num_values_ret, CCS_INVALID_VALUE);
	size_t count;
	CCS_VALIDATE(ccs_objective_space_get_objectives(
		evaluation->data->objective_space, 0, NULL, NULL, &count));
	if (values) {
		CCS_REFUTE(count < num_values, CCS_INVALID_VALUE);
		for (size_t i = 0; i < count; i++) {
			ccs_expression_t     expression;
			ccs_objective_type_t type;

			CCS_VALIDATE(ccs_objective_space_get_objective(
				evaluation->data->objective_space, i,
				&expression, &type));
			CCS_VALIDATE(ccs_expression_eval(
				expression,
				(ccs_context_t)evaluation->data->objective_space,
				evaluation->data->values, values + i));
		}
		for (size_t i = count; i < num_values; i++)
			values[i] = ccs_none;
	}
	if (num_values_ret)
		*num_values_ret = count;
	return CCS_SUCCESS;
}

ccs_error_t
ccs_tree_evaluation_hash(ccs_tree_evaluation_t evaluation, ccs_hash_t *hash_ret)
{
	CCS_CHECK_OBJ(evaluation, CCS_OBJECT_TYPE_TREE_EVALUATION);
	_ccs_tree_evaluation_ops_t *ops =
		ccs_tree_evaluation_get_ops(evaluation);
	CCS_VALIDATE(ops->hash(evaluation->data, hash_ret));
	return CCS_SUCCESS;
}

ccs_error_t
ccs_tree_evaluation_cmp(
	ccs_tree_evaluation_t evaluation,
	ccs_tree_evaluation_t other_evaluation,
	int                  *cmp_ret)
{
	CCS_CHECK_OBJ(evaluation, CCS_OBJECT_TYPE_TREE_EVALUATION);
	CCS_CHECK_OBJ(other_evaluation, CCS_OBJECT_TYPE_TREE_EVALUATION);
	CCS_CHECK_PTR(cmp_ret);
	if (evaluation == other_evaluation) {
		*cmp_ret = 0;
		return CCS_SUCCESS;
	}
	_ccs_tree_evaluation_ops_t *ops =
		ccs_tree_evaluation_get_ops(evaluation);
	CCS_VALIDATE(ops->cmp(evaluation->data, other_evaluation, cmp_ret));
	return CCS_SUCCESS;
}

static inline int
_numeric_compare(const ccs_datum_t *a, const ccs_datum_t *b)
{
	if (a->type == CCS_FLOAT) {
		return a->value.f < b->value.f ? -1 :
		       a->value.f > b->value.f ? 1 :
						 0;
	} else {
		return a->value.i < b->value.i ? -1 :
		       a->value.i > b->value.i ? 1 :
						 0;
	}
}

//Could be using memoization here.
ccs_error_t
ccs_tree_evaluation_compare(
	ccs_tree_evaluation_t evaluation,
	ccs_tree_evaluation_t other_evaluation,
	ccs_comparison_t     *result_ret)
{
	CCS_CHECK_OBJ(evaluation, CCS_OBJECT_TYPE_TREE_EVALUATION);
	CCS_CHECK_OBJ(other_evaluation, CCS_OBJECT_TYPE_TREE_EVALUATION);
	CCS_CHECK_PTR(result_ret);
	if (evaluation == other_evaluation) {
		*result_ret = CCS_EQUIVALENT;
		return CCS_SUCCESS;
	}
	CCS_REFUTE(
		evaluation->data->result || other_evaluation->data->result,
		CCS_INVALID_OBJECT);
	CCS_REFUTE(
		evaluation->data->objective_space !=
			other_evaluation->data->objective_space,
		CCS_INVALID_OBJECT);
	size_t count;
	CCS_VALIDATE(ccs_objective_space_get_objectives(
		evaluation->data->objective_space, 0, NULL, NULL, &count));
	*result_ret = CCS_EQUIVALENT;
	for (size_t i = 0; i < count; i++) {
		ccs_expression_t     expression;
		ccs_objective_type_t type;
		ccs_datum_t          values[2];
		int                  cmp;

		CCS_VALIDATE(ccs_objective_space_get_objective(
			evaluation->data->objective_space, i, &expression,
			&type));
		CCS_VALIDATE(ccs_expression_eval(
			expression,
			(ccs_context_t)evaluation->data->objective_space,
			evaluation->data->values, values));
		CCS_VALIDATE(ccs_expression_eval(
			expression,
			(ccs_context_t)evaluation->data->objective_space,
			other_evaluation->data->values, values + 1));
		// Maybe relax to allow comparing Numerical values of different
		// types.
		if ((values[0].type != CCS_INTEGER &&
		     values[0].type != CCS_FLOAT) ||
		    values[0].type != values[1].type) {
			*result_ret = CCS_NOT_COMPARABLE;
			return CCS_SUCCESS;
		}
		cmp = _numeric_compare(values, values + 1);
		if (cmp) {
			if (type == CCS_MAXIMIZE)
				cmp = -cmp;
			if (*result_ret == CCS_EQUIVALENT)
				*result_ret = (ccs_comparison_t)cmp;
			else if (*result_ret != cmp) {
				*result_ret = CCS_NOT_COMPARABLE;
				return CCS_SUCCESS;
			}
		}
	}
	return CCS_SUCCESS;
}
