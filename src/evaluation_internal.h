#ifndef _EVALUATION_INTERNAL_H
#define _EVALUATION_INTERNAL_H
#include "binding_internal.h"
#include "search_configuration_internal.h"

struct _ccs_evaluation_data_s;
typedef struct _ccs_evaluation_data_s _ccs_evaluation_data_t;

struct _ccs_evaluation_ops_s {
	_ccs_object_ops_t obj_ops;

	ccs_result_t (*hash)(ccs_evaluation_t evaluation, ccs_hash_t *hash_ret);

	ccs_result_t (*cmp)(
		ccs_evaluation_t evaluation,
		ccs_evaluation_t other,
		int             *cmp_ret);

	ccs_result_t (*compare)(
		ccs_evaluation_t  evaluation,
		ccs_evaluation_t  other_evaluation,
		ccs_comparison_t *result_ret);
};
typedef struct _ccs_evaluation_ops_s _ccs_evaluation_ops_t;

struct _ccs_evaluation_s {
	_ccs_object_internal_t  obj;
	_ccs_evaluation_data_t *data;
};

struct _ccs_evaluation_data_s {
	ccs_objective_space_t      objective_space;
	size_t                     num_values;
	ccs_datum_t               *values;
	ccs_evaluation_result_t    result;
	ccs_search_configuration_t configuration;
};

static inline ccs_result_t
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

static inline ccs_result_t
_ccs_evaluation_cmp(
	ccs_evaluation_t evaluation,
	ccs_evaluation_t other,
	int             *cmp_ret)
{
	CCS_VALIDATE(_ccs_binding_cmp(
		(ccs_binding_t)evaluation, (ccs_binding_t)other, cmp_ret));
	if (*cmp_ret)
		return CCS_RESULT_SUCCESS;
	_ccs_evaluation_data_t *data       = evaluation->data;
	_ccs_evaluation_data_t *other_data = other->data;
	*cmp_ret = data->result < other_data->result ? -1 :
		   data->result > other_data->result ? 1 :
						       0;
	if (*cmp_ret)
		return CCS_RESULT_SUCCESS;
	CCS_VALIDATE(_ccs_search_configuration_cmp(
		data->configuration, other_data->configuration, cmp_ret));
	return CCS_RESULT_SUCCESS;
}

static inline int
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

static inline ccs_result_t
_ccs_evaluation_compare(
	ccs_evaluation_t  evaluation,
	ccs_evaluation_t  other_evaluation,
	ccs_comparison_t *result_ret)
{
	size_t count;
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

#endif //_EVALUATION_INTERNAL_H
