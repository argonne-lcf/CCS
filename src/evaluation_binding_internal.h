#ifndef _EVALUATION_BINDING_INTERNAL_H
#define _EVALUATION_BINDING_INTERNAL_H

typedef struct _ccs_evaluation_binding_data_s _ccs_evaluation_binding_data_t;

struct _ccs_evaluation_binding_ops_s {
	_ccs_object_ops_t obj_ops;

	ccs_result_t (*hash)(ccs_binding_t binding, ccs_hash_t *hash_ret);

	ccs_result_t (
		*cmp)(ccs_binding_t binding, ccs_binding_t other, int *cmp_ret);

	ccs_result_t (*compare)(
		ccs_evaluation_binding_t evaluation,
		ccs_evaluation_binding_t other_evaluation,
		ccs_comparison_t        *result_ret);
};
typedef struct _ccs_evaluation_binding_ops_s _ccs_evaluation_binding_ops_t;

struct _ccs_evaluation_binding_data_s {
	ccs_objective_space_t   objective_space;
	size_t                  num_values;
	ccs_datum_t            *values;
	ccs_evaluation_result_t result;
};

struct _ccs_evaluation_binding_s {
	_ccs_object_internal_t          obj;
	_ccs_evaluation_binding_data_t *data;
};

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
_ccs_evaluation_binding_compare(
	ccs_evaluation_binding_t evaluation,
	ccs_evaluation_binding_t other_evaluation,
	ccs_comparison_t        *result_ret)
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

#endif // _EVALUATION_BINDING_INTERNAL_H
