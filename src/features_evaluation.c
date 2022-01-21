#include "cconfigspace_internal.h"
#include "features_evaluation_internal.h"
#include <string.h>

static inline _ccs_features_evaluation_ops_t *
ccs_features_evaluation_get_ops(ccs_features_evaluation_t evaluation) {
	return (_ccs_features_evaluation_ops_t *)evaluation->obj.ops;
}

static ccs_result_t
_ccs_features_evaluation_del(ccs_object_t object) {
	ccs_features_evaluation_t evaluation = (ccs_features_evaluation_t)object;
	ccs_release_object(evaluation->data->objective_space);
	ccs_release_object(evaluation->data->configuration);
	ccs_release_object(evaluation->data->features);
	return CCS_SUCCESS;
}

static ccs_result_t
_ccs_features_evaluation_hash(_ccs_features_evaluation_data_t  *data,
                              ccs_hash_t                       *hash_ret) {
	ccs_hash_t h, ht;
	CCS_VALIDATE(_ccs_binding_hash((_ccs_binding_data_t *)data, &h));
	CCS_VALIDATE(ccs_configuration_hash(data->configuration, &ht));
	h = _hash_combine(h, ht);
	CCS_VALIDATE(ccs_features_hash(data->features, &ht));
	h = _hash_combine(h, ht);
	HASH_JEN(&(data->error), sizeof(data->error), ht);
	h = _hash_combine(h, ht);
	*hash_ret = h;
	return CCS_SUCCESS;
}

static ccs_result_t
_ccs_features_evaluation_cmp(_ccs_features_evaluation_data_t *data,
                             ccs_features_evaluation_t        other,
                             int                             *cmp_ret) {
	CCS_VALIDATE(_ccs_binding_cmp((_ccs_binding_data_t *)data,
	                              (ccs_binding_t)other, cmp_ret));
	if (*cmp_ret)
		return CCS_SUCCESS;
	_ccs_features_evaluation_data_t *other_data = other->data;
	*cmp_ret = data->error < other_data->error ? -1 :
	           data->error > other_data->error ?  1 : 0;
	if (*cmp_ret)
		return CCS_SUCCESS;
	CCS_VALIDATE(ccs_configuration_cmp(data->configuration,
	                                   other_data->configuration, cmp_ret));
	if (*cmp_ret)
		return CCS_SUCCESS;
	return ccs_features_cmp(data->features, other_data->features, cmp_ret);
}

static _ccs_features_evaluation_ops_t _features_evaluation_ops =
    { { &_ccs_features_evaluation_del, NULL, NULL },
      &_ccs_features_evaluation_hash,
      &_ccs_features_evaluation_cmp };

ccs_result_t
ccs_create_features_evaluation(ccs_objective_space_t      objective_space,
                               ccs_configuration_t        configuration,
                               ccs_features_t             features,
                               ccs_result_t               error,
                               size_t                     num_values,
                               ccs_datum_t               *values,
                               void                      *user_data,
                               ccs_features_evaluation_t *evaluation_ret) {
	CCS_CHECK_OBJ(objective_space, CCS_OBJECTIVE_SPACE);
	CCS_CHECK_OBJ(configuration, CCS_CONFIGURATION);
	CCS_CHECK_OBJ(features, CCS_FEATURES);
	CCS_CHECK_PTR(evaluation_ret);
	CCS_CHECK_ARY(num_values, values);
	size_t num;
	CCS_VALIDATE(ccs_objective_space_get_num_hyperparameters(objective_space, &num));
	if (values && num != num_values)
		return -CCS_INVALID_VALUE;
	uintptr_t mem = (uintptr_t)calloc(1,
		sizeof(struct _ccs_features_evaluation_s) +
		sizeof(struct _ccs_features_evaluation_data_s) +
		num * sizeof(ccs_datum_t));
	if (!mem)
		return -CCS_OUT_OF_MEMORY;
	ccs_result_t err;
	CCS_VALIDATE_ERR_GOTO(err, ccs_retain_object(objective_space), errmemory);
	CCS_VALIDATE_ERR_GOTO(err, ccs_retain_object(configuration), errospace);
	CCS_VALIDATE_ERR_GOTO(err, ccs_retain_object(features), errconfig);

	ccs_features_evaluation_t eval;
        eval = (ccs_features_evaluation_t)mem;
	_ccs_object_init(&(eval->obj), CCS_FEATURES_EVALUATION, (_ccs_object_ops_t*)&_features_evaluation_ops);
	eval->data = (struct _ccs_features_evaluation_data_s*)(mem + sizeof(struct _ccs_features_evaluation_s));
	eval->data->user_data = user_data;
	eval->data->num_values = num;
	eval->data->objective_space = objective_space;
	eval->data->configuration = configuration;
	eval->data->features = features;
	eval->data->error = error;
	eval->data->values = (ccs_datum_t *)(mem + sizeof(struct _ccs_features_evaluation_s) +
	                                           sizeof(struct _ccs_features_evaluation_data_s));
	if (values) {
		memcpy(eval->data->values, values, num*sizeof(ccs_datum_t));
		for (size_t i = 0; i < num_values; i++) {
			if (values[i].flags & CCS_FLAG_TRANSIENT) {
				CCS_VALIDATE_ERR_GOTO(err, ccs_objective_space_validate_value(
					objective_space, i, values[i], eval->data->values + i), errfeat);
			}
		}
	}
	*evaluation_ret = eval;
	return CCS_SUCCESS;
errfeat:
	ccs_release_object(features);
errconfig:
	ccs_release_object(configuration);
errospace:
	ccs_release_object(objective_space);
errmemory:
	free((void *)mem);
	return err;
}

ccs_result_t
ccs_features_evaluation_get_objective_space(ccs_features_evaluation_t       evaluation,
                                            ccs_objective_space_t *objective_space_ret) {
	CCS_CHECK_OBJ(evaluation, CCS_FEATURES_EVALUATION);
	return _ccs_binding_get_context(
		(ccs_binding_t)evaluation, (ccs_context_t *)objective_space_ret);
}

ccs_result_t
ccs_features_evaluation_get_configuration(ccs_features_evaluation_t  evaluation,
                                          ccs_configuration_t       *configuration_ret) {
	CCS_CHECK_OBJ(evaluation, CCS_FEATURES_EVALUATION);
	CCS_CHECK_PTR(configuration_ret);
	*configuration_ret = evaluation->data->configuration;
	return CCS_SUCCESS;
}

ccs_result_t
ccs_features_evaluation_get_features(ccs_features_evaluation_t  evaluation,
                                     ccs_features_t            *features_ret) {
	CCS_CHECK_OBJ(evaluation, CCS_FEATURES_EVALUATION);
	CCS_CHECK_PTR(features_ret);
	*features_ret = evaluation->data->features;
	return CCS_SUCCESS;
}

ccs_result_t
ccs_features_evaluation_get_user_data(ccs_features_evaluation_t   evaluation,
                                      void                      **user_data_ret) {
	CCS_CHECK_OBJ(evaluation, CCS_FEATURES_EVALUATION);
	return _ccs_binding_get_user_data(
		(ccs_binding_t)evaluation, user_data_ret);
}

ccs_result_t
ccs_features_evaluation_get_error(ccs_features_evaluation_t  evaluation,
                                  ccs_result_t              *error_ret) {
	CCS_CHECK_OBJ(evaluation, CCS_FEATURES_EVALUATION);
	CCS_CHECK_PTR(error_ret);
	*error_ret = evaluation->data->error;
	return CCS_SUCCESS;
}

ccs_result_t
ccs_features_evaluation_set_error(ccs_features_evaluation_t evaluation,
                                  ccs_result_t              error) {
	CCS_CHECK_OBJ(evaluation, CCS_FEATURES_EVALUATION);
	evaluation->data->error = error;
	return CCS_SUCCESS;
}

ccs_result_t
ccs_features_evaluation_get_value(ccs_features_evaluation_t  evaluation,
                                  size_t                     index,
                                  ccs_datum_t               *value_ret) {
	CCS_CHECK_OBJ(evaluation, CCS_FEATURES_EVALUATION);
	return _ccs_binding_get_value(
		(ccs_binding_t)evaluation, index, value_ret);
}

ccs_result_t
ccs_features_evaluation_set_value(ccs_features_evaluation_t evaluation,
                                  size_t                    index,
                                  ccs_datum_t               value) {
	CCS_CHECK_OBJ(evaluation, CCS_FEATURES_EVALUATION);
	return _ccs_binding_set_value(
		(ccs_binding_t)evaluation, index, value);
}

ccs_result_t
ccs_features_evaluation_get_values(ccs_features_evaluation_t  evaluation,
                                   size_t                     num_values,
                                   ccs_datum_t               *values,
                                   size_t                    *num_values_ret) {
	CCS_CHECK_OBJ(evaluation, CCS_FEATURES_EVALUATION);
	return _ccs_binding_get_values(
		(ccs_binding_t)evaluation, num_values, values, num_values_ret);
}

ccs_result_t
ccs_features_evaluation_get_value_by_name(ccs_features_evaluation_t  evaluation,
                                          const char                *name,
                                          ccs_datum_t               *value_ret) {
	CCS_CHECK_OBJ(evaluation, CCS_FEATURES_EVALUATION);
	return _ccs_binding_get_value_by_name(
		(ccs_binding_t)evaluation, name, value_ret);
}

ccs_result_t
ccs_features_evaluation_check(ccs_features_evaluation_t  evaluation) {
	CCS_CHECK_OBJ(evaluation, CCS_FEATURES_EVALUATION);
	return ccs_objective_space_check_evaluation_values(
		evaluation->data->objective_space, evaluation->data->num_values, evaluation->data->values);
}

ccs_result_t
ccs_features_evaluation_get_objective_value(ccs_features_evaluation_t  evaluation,
                                            size_t                     index,
                                            ccs_datum_t               *value_ret) {
	CCS_CHECK_OBJ(evaluation, CCS_FEATURES_EVALUATION);
	CCS_CHECK_PTR(value_ret);
	ccs_expression_t     expression;
	ccs_objective_type_t type;
	CCS_VALIDATE(ccs_objective_space_get_objective(
	               evaluation->data->objective_space, index, &expression, &type));
	return ccs_expression_eval(expression,
	                           (ccs_context_t)evaluation->data->objective_space,
	                           evaluation->data->values, value_ret);
}

ccs_result_t
ccs_features_evaluation_get_objective_values(ccs_features_evaluation_t  evaluation,
                                             size_t                     num_values,
                                             ccs_datum_t               *values,
                                             size_t                    *num_values_ret) {
	CCS_CHECK_OBJ(evaluation, CCS_FEATURES_EVALUATION);
	CCS_CHECK_ARY(num_values, values);
	if (!values && !num_values_ret)
		return -CCS_INVALID_VALUE;
	size_t count;
	CCS_VALIDATE(ccs_objective_space_get_objectives(
	               evaluation->data->objective_space, 0, NULL, NULL, &count));
	if (values) {
		if (count < num_values)
			return -CCS_INVALID_VALUE;
		for (size_t i = 0; i < count; i++) {
			ccs_expression_t     expression;
			ccs_objective_type_t type;

			CCS_VALIDATE(ccs_objective_space_get_objective(
				evaluation->data->objective_space, i, &expression, &type));
			CCS_VALIDATE(ccs_expression_eval(expression,
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

static inline int
_numeric_compare(const ccs_datum_t *a, const ccs_datum_t *b) {
	if (a->type == CCS_FLOAT) {
		return a->value.f < b->value.f ? -1 : a->value.f > b->value.f ? 1 : 0;
	} else {
		return a->value.i < b->value.i ? -1 : a->value.i > b->value.i ? 1 : 0;
	}
}

ccs_result_t
ccs_features_evaluation_hash(ccs_features_evaluation_t  evaluation,
                             ccs_hash_t          *hash_ret) {
	CCS_CHECK_OBJ(evaluation, CCS_FEATURES_EVALUATION);
	_ccs_features_evaluation_ops_t *ops = ccs_features_evaluation_get_ops(evaluation);
	return ops->hash(evaluation->data, hash_ret);
}

ccs_result_t
ccs_features_evaluation_cmp(ccs_features_evaluation_t  evaluation,
                            ccs_features_evaluation_t  other_evaluation,
                            int                       *cmp_ret) {
	CCS_CHECK_OBJ(evaluation, CCS_FEATURES_EVALUATION);
	CCS_CHECK_OBJ(other_evaluation, CCS_FEATURES_EVALUATION);
	CCS_CHECK_PTR(cmp_ret);
	if (evaluation == other_evaluation) {
		*cmp_ret = 0;
		return CCS_SUCCESS;
	}
	_ccs_features_evaluation_ops_t *ops = ccs_features_evaluation_get_ops(evaluation);
	return ops->cmp(evaluation->data, other_evaluation, cmp_ret);
}


//Could be using memoization here.
ccs_result_t
ccs_features_evaluation_compare(ccs_features_evaluation_t  evaluation,
                                ccs_features_evaluation_t  other_evaluation,
                                ccs_comparison_t          *result_ret) {
	CCS_CHECK_OBJ(evaluation, CCS_FEATURES_EVALUATION);
	CCS_CHECK_OBJ(other_evaluation, CCS_FEATURES_EVALUATION);
	CCS_CHECK_PTR(result_ret);
	if (evaluation == other_evaluation) {
		*result_ret = CCS_EQUIVALENT;
		return CCS_SUCCESS;
	}
	if(evaluation->data->error || other_evaluation->data->error)
		return -CCS_INVALID_OBJECT;
	if (evaluation->data->objective_space != other_evaluation->data->objective_space)
		return -CCS_INVALID_OBJECT;
	size_t count;
	int eql;
	CCS_VALIDATE(ccs_objective_space_get_objectives(
	               evaluation->data->objective_space, 0, NULL, NULL, &count));
	CCS_VALIDATE(ccs_features_cmp(evaluation->data->features,
	                       other_evaluation->data->features, &eql));
	if (0 != eql) {
		*result_ret = CCS_NOT_COMPARABLE;
		return CCS_SUCCESS;
	}

	*result_ret = CCS_EQUIVALENT;
	for (size_t i = 0; i < count; i++) {
		ccs_expression_t     expression;
		ccs_objective_type_t type;
		ccs_datum_t          values[2];
		int cmp;

		CCS_VALIDATE(ccs_objective_space_get_objective(
			evaluation->data->objective_space, i, &expression, &type));
		CCS_VALIDATE(ccs_expression_eval(expression,
				(ccs_context_t)evaluation->data->objective_space,
				evaluation->data->values, values));
		CCS_VALIDATE(ccs_expression_eval(expression,
				(ccs_context_t)evaluation->data->objective_space,
				other_evaluation->data->values, values + 1));
		if ((values[0].type != CCS_INTEGER && values[0].type != CCS_FLOAT) ||
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
