#include "cconfigspace_internal.h"
#include "evaluation_internal.h"
#include "search_configuration_internal.h"
#include "configuration_internal.h"
#include "objective_space_internal.h"
#include <string.h>

static inline _ccs_evaluation_ops_t *
ccs_evaluation_get_ops(ccs_evaluation_t binding)
{
	return (_ccs_evaluation_ops_t *)binding->obj.ops;
}

static ccs_result_t
_ccs_evaluation_del(ccs_object_t object)
{
	ccs_evaluation_t evaluation = (ccs_evaluation_t)object;
	ccs_release_object(evaluation->data->objective_space);
	ccs_release_object(evaluation->data->configuration);
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_serialize_bin_size_ccs_evaluation_data(
	_ccs_evaluation_data_t          *data,
	size_t                          *cum_size,
	_ccs_object_serialize_options_t *opts)
{
	*cum_size += _ccs_serialize_bin_size_ccs_binding_data(
		(_ccs_binding_data_t *)data);
	CCS_VALIDATE(_ccs_object_serialize_size_with_opts(
		data->configuration, CCS_SERIALIZE_FORMAT_BINARY, cum_size,
		opts));
	*cum_size +=
		_ccs_serialize_bin_size_ccs_evaluation_result(data->result);
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_serialize_bin_ccs_evaluation_data(
	_ccs_evaluation_data_t          *data,
	size_t                          *buffer_size,
	char                           **buffer,
	_ccs_object_serialize_options_t *opts)
{
	CCS_VALIDATE(_ccs_serialize_bin_ccs_binding_data(
		(_ccs_binding_data_t *)data, buffer_size, buffer));
	CCS_VALIDATE(_ccs_object_serialize_with_opts(
		data->configuration, CCS_SERIALIZE_FORMAT_BINARY, buffer_size,
		buffer, opts));
	CCS_VALIDATE(_ccs_serialize_bin_ccs_evaluation_result(
		data->result, buffer_size, buffer));
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_serialize_bin_size_ccs_evaluation(
	ccs_evaluation_t                 evaluation,
	size_t                          *cum_size,
	_ccs_object_serialize_options_t *opts)
{
	*cum_size += _ccs_serialize_bin_size_ccs_object_internal(
		(_ccs_object_internal_t *)evaluation);
	CCS_VALIDATE(_ccs_serialize_bin_size_ccs_evaluation_data(
		evaluation->data, cum_size, opts));
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_serialize_bin_ccs_evaluation(
	ccs_evaluation_t                 evaluation,
	size_t                          *buffer_size,
	char                           **buffer,
	_ccs_object_serialize_options_t *opts)
{
	CCS_VALIDATE(_ccs_serialize_bin_ccs_object_internal(
		(_ccs_object_internal_t *)evaluation, buffer_size, buffer));
	CCS_VALIDATE(_ccs_serialize_bin_ccs_evaluation_data(
		evaluation->data, buffer_size, buffer, opts));
	return CCS_RESULT_SUCCESS;
}

static ccs_result_t
_ccs_evaluation_serialize_size(
	ccs_object_t                     object,
	ccs_serialize_format_t           format,
	size_t                          *cum_size,
	_ccs_object_serialize_options_t *opts)
{
	switch (format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE(_ccs_serialize_bin_size_ccs_evaluation(
			(ccs_evaluation_t)object, cum_size, opts));
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
_ccs_evaluation_serialize(
	ccs_object_t                     object,
	ccs_serialize_format_t           format,
	size_t                          *buffer_size,
	char                           **buffer,
	_ccs_object_serialize_options_t *opts)
{
	switch (format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE(_ccs_serialize_bin_ccs_evaluation(
			(ccs_evaluation_t)object, buffer_size, buffer, opts));
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

static ccs_result_t
_ccs_evaluation_hash(ccs_evaluation_t evaluation, ccs_hash_t *hash_ret)
{
	_ccs_evaluation_data_t *data = evaluation->data;
	ccs_hash_t              h, ht;
	CCS_VALIDATE(_ccs_binding_hash((ccs_binding_t)evaluation, &h));
	CCS_VALIDATE(_ccs_search_configuration_hash(data->configuration, &ht));
	h = _hash_combine(h, ht);
	HASH_JEN(&(data->result), sizeof(data->result), ht);
	h         = _hash_combine(h, ht);
	*hash_ret = h;
	return CCS_RESULT_SUCCESS;
}

static ccs_result_t
_ccs_evaluation_cmp(
	ccs_evaluation_t evaluation,
	ccs_evaluation_t other,
	int             *cmp_ret)
{
	_ccs_evaluation_data_t *data       = evaluation->data;
	_ccs_evaluation_data_t *other_data = other->data;
	*cmp_ret = data->result < other_data->result ? -1 :
		   data->result > other_data->result ? 1 :
						       0;
	if (*cmp_ret)
		return CCS_RESULT_SUCCESS;
	CCS_VALIDATE(_ccs_search_configuration_cmp(
		data->configuration, other_data->configuration, cmp_ret));
	if (*cmp_ret)
		return CCS_RESULT_SUCCESS;
	CCS_VALIDATE(_ccs_binding_cmp(
		(ccs_binding_t)evaluation, (ccs_binding_t)other, cmp_ret));
	return CCS_RESULT_SUCCESS;
}

static int
_numeric_compare(const ccs_datum_t *a, const ccs_datum_t *b)
{
	if (a->type == CCS_DATA_TYPE_FLOAT) {
		return a->value.f < b->value.f ? -1 :
		       a->value.f > b->value.f ? 1 :
						 0;
	} else {
		return a->value.i < b->value.i ? -1 :
		       a->value.i > b->value.i ? 1 :
						 0;
	}
}

static ccs_result_t
_ccs_evaluation_compare(
	ccs_evaluation_t  evaluation,
	ccs_evaluation_t  other_evaluation,
	ccs_comparison_t *result_ret)
{
	size_t         count;
	int            eql;
	ccs_features_t features, other_features;
	CCS_VALIDATE(_ccs_search_configuration_get_features(
		evaluation->data->configuration, &features));
	CCS_VALIDATE(_ccs_search_configuration_get_features(
		other_evaluation->data->configuration, &other_features));
	if (features != other_features) {
		CCS_VALIDATE(_ccs_binding_cmp(
			(ccs_binding_t)features, (ccs_binding_t)other_features,
			&eql));
		if (0 != eql) {
			*result_ret = CCS_COMPARISON_NOT_COMPARABLE;
			return CCS_RESULT_SUCCESS;
		}
	}
	CCS_VALIDATE(ccs_objective_space_get_objectives(
		evaluation->data->objective_space, 0, NULL, NULL, &count));
	*result_ret = CCS_COMPARISON_EQUIVALENT;
	for (size_t i = 0; i < count; i++) {
		ccs_expression_t     expression;
		ccs_objective_type_t type;
		ccs_datum_t          values[2];
		int                  cmp;

		CCS_VALIDATE(ccs_objective_space_get_objective(
			evaluation->data->objective_space, i, &expression,
			&type));
		CCS_VALIDATE(ccs_expression_eval(
			expression, 1, (ccs_binding_t *)&evaluation, values));
		CCS_VALIDATE(ccs_expression_eval(
			expression, 1, (ccs_binding_t *)&other_evaluation,
			values + 1));
		if ((values[0].type != CCS_DATA_TYPE_INT &&
		     values[0].type != CCS_DATA_TYPE_FLOAT) ||
		    values[0].type != values[1].type) {
			*result_ret = CCS_COMPARISON_NOT_COMPARABLE;
			return CCS_RESULT_SUCCESS;
		}
		cmp = _numeric_compare(values, values + 1);
		if (cmp) {
			if (type == CCS_OBJECTIVE_TYPE_MAXIMIZE)
				cmp = -cmp;
			if (*result_ret == CCS_COMPARISON_EQUIVALENT)
				*result_ret = (ccs_comparison_t)cmp;
			else if (*result_ret != cmp) {
				*result_ret = CCS_COMPARISON_NOT_COMPARABLE;
				return CCS_RESULT_SUCCESS;
			}
		}
	}
	return CCS_RESULT_SUCCESS;
}

static _ccs_evaluation_ops_t _evaluation_ops = {
	{&_ccs_evaluation_del, &_ccs_evaluation_serialize_size,
	 &_ccs_evaluation_serialize},
	&_ccs_evaluation_hash,
	&_ccs_evaluation_cmp,
	&_ccs_evaluation_compare};

ccs_result_t
ccs_create_evaluation(
	ccs_objective_space_t      objective_space,
	ccs_search_configuration_t configuration,
	ccs_evaluation_result_t    result,
	size_t                     num_values,
	ccs_datum_t               *values,
	ccs_evaluation_t          *evaluation_ret)
{
	CCS_CHECK_OBJ(objective_space, CCS_OBJECT_TYPE_OBJECTIVE_SPACE);
	CCS_CHECK_SEARCH_CONFIGURATION(configuration);
	CCS_REFUTE(
		objective_space->data->search_space !=
			configuration->data->space,
		CCS_RESULT_ERROR_INVALID_CONFIGURATION);
	CCS_CHECK_PTR(evaluation_ret);
	CCS_CHECK_ARY(num_values, values);
	ccs_result_t err;
	size_t       num_parameters = objective_space->data->num_parameters;
	CCS_REFUTE(
		values && num_parameters != num_values,
		CCS_RESULT_ERROR_INVALID_VALUE);
	uintptr_t mem = (uintptr_t)calloc(
		1, sizeof(struct _ccs_evaluation_s) +
			   sizeof(struct _ccs_evaluation_data_s) +
			   num_parameters * sizeof(ccs_datum_t));
	CCS_REFUTE(!mem, CCS_RESULT_ERROR_OUT_OF_MEMORY);
	CCS_VALIDATE_ERR_GOTO(err, ccs_retain_object(objective_space), errmem);
	CCS_VALIDATE_ERR_GOTO(err, ccs_retain_object(configuration), erros);
	ccs_evaluation_t eval;
	eval = (ccs_evaluation_t)mem;
	_ccs_object_init(
		&(eval->obj), CCS_OBJECT_TYPE_EVALUATION,
		(_ccs_object_ops_t *)&_evaluation_ops);
	eval->data                  = (struct _ccs_evaluation_data_s
                              *)(mem + sizeof(struct _ccs_evaluation_s));
	eval->data->num_values      = num_parameters;
	eval->data->objective_space = objective_space;
	eval->data->configuration   = configuration;
	eval->data->result          = result;
	eval->data->values =
		(ccs_datum_t
			 *)(mem + sizeof(struct _ccs_evaluation_s) + sizeof(struct _ccs_evaluation_data_s));
	if (values) {
		memcpy(eval->data->values, values,
		       num_parameters * sizeof(ccs_datum_t));
		for (size_t i = 0; i < num_values; i++)
			if (values[i].flags & CCS_DATUM_FLAG_TRANSIENT)
				CCS_VALIDATE_ERR_GOTO(
					err,
					ccs_context_validate_value(
						(ccs_context_t)objective_space,
						i, values[i],
						eval->data->values + i),
					errc);
	}
	*evaluation_ret = eval;
	return CCS_RESULT_SUCCESS;
errc:
	_ccs_object_deinit(&(eval->obj));
	ccs_release_object(configuration);
erros:
	ccs_release_object(objective_space);
errmem:
	free((void *)mem);
	return err;
}

ccs_result_t
ccs_evaluation_get_objective_space(
	ccs_evaluation_t       evaluation,
	ccs_objective_space_t *objective_space_ret)
{
	CCS_CHECK_OBJ(evaluation, CCS_OBJECT_TYPE_EVALUATION);
	CCS_CHECK_PTR(objective_space_ret);
	*objective_space_ret = evaluation->data->objective_space;
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
ccs_evaluation_get_result(
	ccs_evaluation_t         evaluation,
	ccs_evaluation_result_t *result_ret)
{
	CCS_CHECK_OBJ(evaluation, CCS_OBJECT_TYPE_EVALUATION);
	CCS_CHECK_PTR(result_ret);
	*result_ret = evaluation->data->result;
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
ccs_evaluation_get_configuration(
	ccs_evaluation_t            evaluation,
	ccs_search_configuration_t *configuration_ret)
{
	CCS_CHECK_OBJ(evaluation, CCS_OBJECT_TYPE_EVALUATION);
	CCS_CHECK_PTR(configuration_ret);
	*configuration_ret = evaluation->data->configuration;
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
ccs_evaluation_get_features(
	ccs_evaluation_t evaluation,
	ccs_features_t  *features_ret)
{
	CCS_CHECK_OBJ(evaluation, CCS_OBJECT_TYPE_EVALUATION);
	CCS_CHECK_PTR(features_ret);
	CCS_VALIDATE(_ccs_search_configuration_get_features(
		evaluation->data->configuration, features_ret));
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
ccs_evaluation_check(ccs_evaluation_t evaluation, ccs_bool_t *is_valid_ret)
{
	CCS_CHECK_OBJ(evaluation, CCS_OBJECT_TYPE_EVALUATION);
	CCS_VALIDATE(ccs_objective_space_check_evaluation(
		evaluation->data->objective_space, evaluation, is_valid_ret));
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
ccs_evaluation_get_objective_value(
	ccs_evaluation_t evaluation,
	size_t           index,
	ccs_datum_t     *value_ret)
{
	CCS_CHECK_OBJ(evaluation, CCS_OBJECT_TYPE_EVALUATION);
	CCS_CHECK_PTR(value_ret);
	ccs_expression_t     expression;
	ccs_objective_type_t type;
	CCS_VALIDATE(ccs_objective_space_get_objective(
		evaluation->data->objective_space, index, &expression, &type));
	CCS_VALIDATE(ccs_expression_eval(
		expression, 1, (ccs_binding_t *)&evaluation, value_ret));
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
ccs_evaluation_get_objective_values(
	ccs_evaluation_t evaluation,
	size_t           num_values,
	ccs_datum_t     *values,
	size_t          *num_values_ret)
{
	CCS_CHECK_OBJ(evaluation, CCS_OBJECT_TYPE_EVALUATION);
	CCS_CHECK_ARY(num_values, values);
	CCS_REFUTE(!values && !num_values_ret, CCS_RESULT_ERROR_INVALID_VALUE);
	size_t count;
	CCS_VALIDATE(ccs_objective_space_get_objectives(
		evaluation->data->objective_space, 0, NULL, NULL, &count));
	if (values) {
		CCS_REFUTE(count < num_values, CCS_RESULT_ERROR_INVALID_VALUE);
		for (size_t i = 0; i < count; i++) {
			ccs_expression_t     expression;
			ccs_objective_type_t type;

			CCS_VALIDATE(ccs_objective_space_get_objective(
				evaluation->data->objective_space, i,
				&expression, &type));
			CCS_VALIDATE(ccs_expression_eval(
				expression, 1, (ccs_binding_t *)&evaluation,
				values + i));
		}
		for (size_t i = count; i < num_values; i++)
			values[i] = ccs_none;
	}
	if (num_values_ret)
		*num_values_ret = count;
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
ccs_evaluation_compare(
	ccs_evaluation_t  evaluation,
	ccs_evaluation_t  other_evaluation,
	ccs_comparison_t *result_ret)
{
	CCS_CHECK_OBJ(evaluation, CCS_OBJECT_TYPE_EVALUATION);
	CCS_CHECK_OBJ(other_evaluation, CCS_OBJECT_TYPE_EVALUATION);
	CCS_REFUTE(
		evaluation->obj.type != other_evaluation->obj.type,
		CCS_RESULT_ERROR_INVALID_OBJECT);
	CCS_CHECK_PTR(result_ret);
	if (evaluation == other_evaluation) {
		*result_ret = CCS_COMPARISON_EQUIVALENT;
		return CCS_RESULT_SUCCESS;
	}
	CCS_REFUTE(
		evaluation->data->result || other_evaluation->data->result,
		CCS_RESULT_ERROR_INVALID_OBJECT);
	CCS_REFUTE(
		evaluation->data->objective_space !=
			other_evaluation->data->objective_space,
		CCS_RESULT_ERROR_INVALID_OBJECT);

	_ccs_evaluation_ops_t *ops = ccs_evaluation_get_ops(evaluation);
	CCS_VALIDATE(ops->compare(evaluation, other_evaluation, result_ret));
	return CCS_RESULT_SUCCESS;
}
