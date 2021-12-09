#include "cconfigspace_internal.h"
#include "objective_space_internal.h"
#include "evaluation_internal.h"

static ccs_result_t
_ccs_objective_space_del(ccs_object_t object) {
	ccs_objective_space_t objective_space = (ccs_objective_space_t)object;
	UT_array *array = objective_space->data->hyperparameters;
	_ccs_hyperparameter_wrapper_t *wrapper = NULL;
	while ( (wrapper = (_ccs_hyperparameter_wrapper_t *)utarray_next(array, wrapper)) ) {
		ccs_release_object(wrapper->hyperparameter);
	}
	array = objective_space->data->objectives;
	_ccs_objective_t *obj = NULL;
	while ( (obj = (_ccs_objective_t *)utarray_next(array, obj)) ) {
		ccs_release_object(obj->expression);
	}
	HASH_CLEAR(hh_name, objective_space->data->name_hash);
	_ccs_hyperparameter_index_hash_t *elem, *tmpelem;
	HASH_ITER(hh_handle, objective_space->data->handle_hash, elem, tmpelem) {
		HASH_DELETE(hh_handle, objective_space->data->handle_hash, elem);
		free(elem);
	}
	utarray_free(objective_space->data->hyperparameters);
	utarray_free(objective_space->data->objectives);
	return CCS_SUCCESS;
}

static _ccs_objective_space_ops_t _objective_space_ops =
    { { {&_ccs_objective_space_del} } };

static const UT_icd _hyperparameter_wrapper2_icd = {
	sizeof(_ccs_hyperparameter_wrapper_t),
	NULL,
	NULL,
	NULL,
};

static const UT_icd _objectives_icd = {
	sizeof(_ccs_objective_t),
	NULL,
	NULL,
	NULL,
};

#undef  utarray_oom
#define utarray_oom() { \
	err = -CCS_OUT_OF_MEMORY; \
	goto arrays; \
}

ccs_result_t
ccs_create_objective_space(const char            *name,
                           void                  *user_data,
                           ccs_objective_space_t *objective_space_ret) {
	CCS_CHECK_PTR(name);
	CCS_CHECK_PTR(objective_space_ret);

	uintptr_t mem = (uintptr_t)calloc(1, sizeof(struct _ccs_objective_space_s) + sizeof(struct _ccs_objective_space_data_s) + strlen(name) + 1);
	if (!mem)
		return -CCS_OUT_OF_MEMORY;
	ccs_result_t err;
	ccs_objective_space_t obj_space = (ccs_objective_space_t)mem;
	_ccs_object_init(&(obj_space->obj), CCS_OBJECTIVE_SPACE,
		(_ccs_object_ops_t *)&_objective_space_ops);
	obj_space->data = (struct _ccs_objective_space_data_s*)(mem +
		sizeof(struct _ccs_objective_space_s));
	obj_space->data->name = (const char *)(mem +
		sizeof(struct _ccs_objective_space_s) +
		sizeof(struct _ccs_objective_space_data_s));
	obj_space->data->user_data = user_data;
	utarray_new(obj_space->data->hyperparameters, &_hyperparameter_wrapper2_icd);
	utarray_new(obj_space->data->objectives, &_objectives_icd);
	strcpy((char *)(obj_space->data->name), name);
	*objective_space_ret = obj_space;
	return CCS_SUCCESS;
arrays:
	if (obj_space->data->hyperparameters)
		utarray_free(obj_space->data->hyperparameters);
	if (obj_space->data->objectives)
		utarray_free(obj_space->data->objectives);
	free((void *)mem);
	return err;
}

ccs_result_t
ccs_objective_space_get_name(ccs_objective_space_t   objective_space,
                             const char            **name_ret) {
	CCS_CHECK_OBJ(objective_space, CCS_OBJECTIVE_SPACE);
	return _ccs_context_get_name((ccs_context_t)objective_space, name_ret);
}

ccs_result_t
ccs_objective_space_get_user_data(ccs_objective_space_t   objective_space,
                                  void                  **user_data_ret) {
	CCS_CHECK_OBJ(objective_space, CCS_OBJECTIVE_SPACE);
	return _ccs_context_get_user_data((ccs_context_t)objective_space, user_data_ret);
}

#undef  utarray_oom
#define utarray_oom() { \
	err = -CCS_OUT_OF_MEMORY; \
	goto errormem; \
}
#undef uthash_nonfatal_oom
#define uthash_nonfatal_oom(elt) { \
	err = -CCS_OUT_OF_MEMORY; \
	goto errorutarray; \
}
ccs_result_t
ccs_objective_space_add_hyperparameter(ccs_objective_space_t objective_space,
                                       ccs_hyperparameter_t  hyperparameter) {
	CCS_CHECK_OBJ(objective_space, CCS_OBJECTIVE_SPACE);
	CCS_CHECK_OBJ(hyperparameter, CCS_HYPERPARAMETER);
	ccs_result_t err;
	const char *name;
	size_t sz_name;
	_ccs_hyperparameter_index_hash_t *hyper_hash;
	CCS_VALIDATE(ccs_hyperparameter_get_name(hyperparameter, &name));
	sz_name = strlen(name);
	HASH_FIND(hh_name, objective_space->data->name_hash,
	          name, sz_name, hyper_hash);
	if (hyper_hash)
		return -CCS_INVALID_HYPERPARAMETER;
	UT_array *hyperparameters;
	CCS_VALIDATE(ccs_retain_object(hyperparameter));
	_ccs_hyperparameter_wrapper_t hyper_wrapper;
	hyper_wrapper.hyperparameter = hyperparameter;

	hyperparameters = objective_space->data->hyperparameters;

	hyper_hash = (_ccs_hyperparameter_index_hash_t *)malloc(sizeof(_ccs_hyperparameter_index_hash_t));
	if (!hyper_hash) {
		err = -CCS_OUT_OF_MEMORY;
		goto errorhyper;
	}
	hyper_hash->hyperparameter = hyperparameter;
	hyper_hash->name = name;
	hyper_hash->index = utarray_len(hyperparameters);

	utarray_push_back(hyperparameters, &hyper_wrapper);

	HASH_ADD_KEYPTR( hh_name, objective_space->data->name_hash,
	                 hyper_hash->name, sz_name, hyper_hash );
	HASH_ADD( hh_handle, objective_space->data->handle_hash,
	          hyperparameter, sizeof(ccs_hyperparameter_t), hyper_hash );

	return CCS_SUCCESS;
errorutarray:
	utarray_pop_back(hyperparameters);
errormem:
	free(hyper_hash);
errorhyper:
	ccs_release_object(hyperparameter);
	return err;
}
#undef  utarray_oom
#define utarray_oom() exit(-1)

ccs_result_t
ccs_objective_space_add_hyperparameters(ccs_objective_space_t  objective_space,
                                        size_t                 num_hyperparameters,
                                        ccs_hyperparameter_t  *hyperparameters) {
	CCS_CHECK_OBJ(objective_space, CCS_OBJECTIVE_SPACE);
	CCS_CHECK_ARY(num_hyperparameters, hyperparameters);
	for (size_t i = 0; i < num_hyperparameters; i++)
		CCS_VALIDATE(
		    ccs_objective_space_add_hyperparameter( objective_space,
		                                            hyperparameters[i]));
	return CCS_SUCCESS;
}

ccs_result_t
ccs_objective_space_get_num_hyperparameters(
		ccs_objective_space_t  objective_space,
		size_t                *num_hyperparameters_ret) {
	CCS_CHECK_OBJ(objective_space, CCS_OBJECTIVE_SPACE);
	return _ccs_context_get_num_hyperparameters(
		(ccs_context_t)objective_space, num_hyperparameters_ret);
}

ccs_result_t
ccs_objective_space_get_hyperparameter(ccs_objective_space_t  objective_space,
                                       size_t                 index,
                                       ccs_hyperparameter_t  *hyperparameter_ret) {
	CCS_CHECK_OBJ(objective_space, CCS_OBJECTIVE_SPACE);
	return _ccs_context_get_hyperparameter(
		(ccs_context_t)objective_space, index, hyperparameter_ret);
}

ccs_result_t
ccs_objective_space_get_hyperparameter_by_name(
		ccs_objective_space_t  objective_space,
		const char *           name,
		ccs_hyperparameter_t  *hyperparameter_ret) {
	CCS_CHECK_OBJ(objective_space, CCS_OBJECTIVE_SPACE);
	return _ccs_context_get_hyperparameter_by_name(
		(ccs_context_t)objective_space, name, hyperparameter_ret);
}

ccs_result_t
ccs_objective_space_get_hyperparameter_index_by_name(
		ccs_objective_space_t  objective_space,
		const char            *name,
		size_t                *index_ret) {
	CCS_CHECK_OBJ(objective_space, CCS_OBJECTIVE_SPACE);
	return _ccs_context_get_hyperparameter_index_by_name(
		(ccs_context_t)objective_space, name, index_ret);
}

ccs_result_t
ccs_objective_space_get_hyperparameter_index(
		ccs_objective_space_t  objective_space,
		ccs_hyperparameter_t   hyperparameter,
		size_t                *index_ret) {
	CCS_CHECK_OBJ(objective_space, CCS_OBJECTIVE_SPACE);
	CCS_CHECK_OBJ(hyperparameter, CCS_HYPERPARAMETER);
	return _ccs_context_get_hyperparameter_index(
		(ccs_context_t)(objective_space),
		hyperparameter, index_ret);
}

ccs_result_t
ccs_objective_space_get_hyperparameter_indexes(
		ccs_objective_space_t  objective_space,
		size_t                 num_hyperparameters,
		ccs_hyperparameter_t  *hyperparameters,
		size_t                *indexes) {
	CCS_CHECK_OBJ(objective_space, CCS_OBJECTIVE_SPACE);
	return _ccs_context_get_hyperparameter_indexes(
		(ccs_context_t)objective_space, num_hyperparameters,
		 hyperparameters, indexes);
}

ccs_result_t
ccs_objective_space_get_hyperparameters(ccs_objective_space_t  objective_space,
                                        size_t                 num_hyperparameters,
                                        ccs_hyperparameter_t  *hyperparameters,
                                        size_t                *num_hyperparameters_ret) {
	CCS_CHECK_OBJ(objective_space, CCS_OBJECTIVE_SPACE);
	return _ccs_context_get_hyperparameters(
		(ccs_context_t)objective_space, num_hyperparameters,
		hyperparameters, num_hyperparameters_ret);
}

static inline ccs_result_t
_check_evaluation(ccs_objective_space_t  objective_space,
                  size_t                 num_values,
                  ccs_datum_t           *values) {
	UT_array *array = objective_space->data->hyperparameters;
	if (num_values != utarray_len(array))
		return -CCS_INVALID_EVALUATION;
	for (size_t i = 0; i < num_values; i++) {
		ccs_bool_t res;
		_ccs_hyperparameter_wrapper_t *wrapper =
			(_ccs_hyperparameter_wrapper_t *)utarray_eltptr(array, i);
		CCS_VALIDATE(ccs_hyperparameter_check_value(wrapper->hyperparameter,
	                                                    values[i], &res));
		if (CCS_UNLIKELY(res == CCS_FALSE))
			return -CCS_INVALID_EVALUATION;
	}
	return CCS_SUCCESS;
}

ccs_result_t
ccs_objective_space_check_evaluation(ccs_objective_space_t objective_space,
                                     ccs_evaluation_t      evaluation) {
	CCS_CHECK_OBJ(objective_space, CCS_OBJECTIVE_SPACE);
	CCS_CHECK_OBJ(evaluation, CCS_EVALUATION);
	if (evaluation->data->objective_space != objective_space)
		return -CCS_INVALID_EVALUATION;
	return _check_evaluation(objective_space,
	                         evaluation->data->num_values,
	                         evaluation->data->values);
}

ccs_result_t
ccs_objective_space_check_evaluation_values(ccs_objective_space_t  objective_space,
                                            size_t                 num_values,
                                            ccs_datum_t           *values) {
	CCS_CHECK_OBJ(objective_space, CCS_OBJECTIVE_SPACE);
	CCS_CHECK_ARY(num_values, values);
	return _check_evaluation(objective_space, num_values, values);
}

ccs_result_t
ccs_objective_space_validate_value(ccs_objective_space_t  objective_space,
                                   size_t                 index,
                                   ccs_datum_t            value,
                                   ccs_datum_t           *value_ret) {
	CCS_CHECK_OBJ(objective_space, CCS_OBJECTIVE_SPACE);
	return _ccs_context_validate_value((ccs_context_t)objective_space,
	                                   index, value, value_ret);
}

#undef  utarray_oom
#define utarray_oom() { \
	return -CCS_OUT_OF_MEMORY; \
}
ccs_result_t
ccs_objective_space_add_objective(ccs_objective_space_t objective_space,
                                  ccs_expression_t      expression,
                                  ccs_objective_type_t  type) {
	CCS_CHECK_OBJ(objective_space, CCS_OBJECTIVE_SPACE);
	CCS_VALIDATE(ccs_expression_check_context(
	  expression, (ccs_context_t)objective_space));
	CCS_VALIDATE(ccs_retain_object(expression));
	_ccs_objective_t objective;
	objective.expression = expression;
	objective.type = type;
	utarray_push_back(objective_space->data->objectives, &objective);
	return CCS_SUCCESS;
}

ccs_result_t
ccs_objective_space_add_objectives(ccs_objective_space_t  objective_space,
                                   size_t                 num_objectives,
                                   ccs_expression_t      *expressions,
                                   ccs_objective_type_t  *types) {
	CCS_CHECK_OBJ(objective_space, CCS_OBJECTIVE_SPACE);
	CCS_CHECK_ARY(num_objectives, expressions);
	CCS_CHECK_ARY(num_objectives, types);
	for (size_t i = 0; i < num_objectives; i++) {
		CCS_VALIDATE(ccs_expression_check_context(
		  expressions[i], (ccs_context_t)objective_space));
		CCS_VALIDATE(ccs_retain_object(expressions[i]));
		_ccs_objective_t objective;
		objective.expression = expressions[i];
		objective.type = types[i];
		utarray_push_back(objective_space->data->objectives, &objective);
	}
	return CCS_SUCCESS;
}
#undef  utarray_oom
#define utarray_oom() exit(-1)

ccs_result_t
ccs_objective_space_get_objective(ccs_objective_space_t  objective_space,
                                  size_t                 index,
                                  ccs_expression_t      *expression_ret,
                                  ccs_objective_type_t  *type_ret) {
	CCS_CHECK_OBJ(objective_space, CCS_OBJECTIVE_SPACE);
	CCS_CHECK_PTR(expression_ret);
	CCS_CHECK_PTR(type_ret);
	_ccs_objective_t *p_obj = (_ccs_objective_t*)
	    utarray_eltptr(objective_space->data->objectives,
	                   (unsigned int)index);
	if (!p_obj)
		return -CCS_OUT_OF_BOUNDS;
	*expression_ret = p_obj->expression;
	*type_ret       = p_obj->type;
	return CCS_SUCCESS;
}

ccs_result_t
ccs_objective_space_get_objectives(ccs_objective_space_t  objective_space,
                                   size_t                 num_objectives,
                                   ccs_expression_t      *expressions,
                                   ccs_objective_type_t  *types,
                                   size_t                *num_objectives_ret) {
	CCS_CHECK_OBJ(objective_space, CCS_OBJECTIVE_SPACE);
	CCS_CHECK_ARY(num_objectives, expressions);
	CCS_CHECK_ARY(num_objectives, types);
	if (!expressions && !num_objectives_ret)
		return -CCS_INVALID_VALUE;
	UT_array *array = objective_space->data->objectives;
	size_t size = utarray_len(array);
	if (expressions) {
		if (num_objectives < size)
			return -CCS_INVALID_VALUE;
		_ccs_objective_t *p_obj = NULL;
		size_t index = 0;
		while ( (p_obj = (_ccs_objective_t *)utarray_next(array, p_obj)) ) {
			expressions[index] = p_obj->expression;
			types[index]       = p_obj->type;
			index++;
		}
		for (size_t i = size; i < num_objectives; i++) {
			expressions[i] = NULL;
			types[i] = CCS_MINIMIZE;
		}
	}
	if (num_objectives_ret)
		*num_objectives_ret = size;
	return CCS_SUCCESS;
}
