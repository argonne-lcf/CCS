#include "cconfigspace_internal.h"
#include "features_space_internal.h"
#include "features_internal.h"
#include "utlist.h"

static ccs_result_t
_ccs_features_space_del(ccs_object_t object) {
	ccs_features_space_t features_space = (ccs_features_space_t)object;
	UT_array *array = features_space->data->hyperparameters;
	_ccs_hyperparameter_wrapper_t *wrapper = NULL;
	while ( (wrapper = (_ccs_hyperparameter_wrapper_t *)utarray_next(array, wrapper)) ) {
		ccs_release_object(wrapper->hyperparameter);
	}
	HASH_CLEAR(hh_name, features_space->data->name_hash);
	HASH_CLEAR(hh_handle, features_space->data->handle_hash);
	utarray_free(features_space->data->hyperparameters);
	return CCS_SUCCESS;
}

static _ccs_features_space_ops_t _features_space_ops =
    { { {&_ccs_features_space_del} } };

static const UT_icd _hyperparameter_wrapper_icd = {
	sizeof(_ccs_hyperparameter_wrapper_t),
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
ccs_create_features_space(const char                *name,
                          void                      *user_data,
                          ccs_features_space_t *features_space_ret) {
	ccs_result_t err;
	CCS_CHECK_PTR(name);
	CCS_CHECK_PTR(features_space_ret);
	uintptr_t mem = (uintptr_t)calloc(1, sizeof(struct _ccs_features_space_s) + sizeof(struct _ccs_features_space_data_s) + strlen(name) + 1);
	if (!mem)
		return -CCS_OUT_OF_MEMORY;

	ccs_features_space_t feat_space = (ccs_features_space_t)mem;
	_ccs_object_init(&(feat_space->obj), CCS_FEATURES_SPACE,
		(_ccs_object_ops_t *)&_features_space_ops);
	feat_space->data = (struct _ccs_features_space_data_s*)(mem +
		sizeof(struct _ccs_features_space_s));
	feat_space->data->name = (const char *)(mem +
		sizeof(struct _ccs_features_space_s) +
		sizeof(struct _ccs_features_space_data_s));
	feat_space->data->user_data = user_data;
	utarray_new(feat_space->data->hyperparameters, &_hyperparameter_wrapper_icd);
	strcpy((char *)(feat_space->data->name), name);
	*features_space_ret = feat_space;
	return CCS_SUCCESS;
arrays:
	if (feat_space->data->hyperparameters)
		utarray_free(feat_space->data->hyperparameters);
	free((void *)mem);
	return err;
}

ccs_result_t
ccs_features_space_get_name(ccs_features_space_t   features_space,
                            const char                **name_ret) {
	CCS_CHECK_OBJ(features_space, CCS_FEATURES_SPACE);
	return _ccs_context_get_name((ccs_context_t)features_space, name_ret);
}

ccs_result_t
ccs_features_space_get_user_data(ccs_features_space_t   features_space,
                                 void                      **user_data_ret) {
	CCS_CHECK_OBJ(features_space, CCS_FEATURES_SPACE);
	return _ccs_context_get_user_data((ccs_context_t)features_space, user_data_ret);
}

#undef  utarray_oom
#define utarray_oom() { \
	err = -CCS_OUT_OF_MEMORY; \
	goto errorhyper; \
}
#undef uthash_nonfatal_oom
#define uthash_nonfatal_oom(elt) { \
	err = -CCS_OUT_OF_MEMORY; \
	goto errorutarray; \
}
ccs_result_t
ccs_features_space_add_hyperparameter(ccs_features_space_t features_space,
                                      ccs_hyperparameter_t hyperparameter) {
	CCS_CHECK_OBJ(features_space, CCS_FEATURES_SPACE);
	CCS_CHECK_OBJ(hyperparameter, CCS_HYPERPARAMETER);
	ccs_result_t err;
	const char *name;
	size_t sz_name;
	_ccs_hyperparameter_wrapper_t *p_hyper_wrapper;
	CCS_VALIDATE(ccs_hyperparameter_get_name(hyperparameter, &name));
	sz_name = strlen(name);
	HASH_FIND(hh_name, features_space->data->name_hash,
	          name, sz_name, p_hyper_wrapper);
	if (p_hyper_wrapper)
		return -CCS_INVALID_HYPERPARAMETER;
	UT_array *hyperparameters;
	unsigned int index;
	_ccs_hyperparameter_wrapper_t hyper_wrapper;
	hyper_wrapper.hyperparameter = hyperparameter;
	CCS_VALIDATE(ccs_retain_object(hyperparameter));

	hyperparameters = features_space->data->hyperparameters;
	index = utarray_len(hyperparameters);
	hyper_wrapper.index = index;
	hyper_wrapper.name = name;

	p_hyper_wrapper = (_ccs_hyperparameter_wrapper_t*)utarray_front(hyperparameters);
	utarray_push_back(hyperparameters, &hyper_wrapper);

	// Check for address change and rehash if needed
	if (p_hyper_wrapper != (_ccs_hyperparameter_wrapper_t*)utarray_front(hyperparameters)) {
		HASH_CLEAR(hh_name, features_space->data->name_hash);
		HASH_CLEAR(hh_handle, features_space->data->handle_hash);
		p_hyper_wrapper = NULL;
		while ( (p_hyper_wrapper = (_ccs_hyperparameter_wrapper_t*)utarray_next(hyperparameters, p_hyper_wrapper)) ) {
			HASH_ADD_KEYPTR( hh_name, features_space->data->name_hash,
			                 p_hyper_wrapper->name, strlen(p_hyper_wrapper->name), p_hyper_wrapper );
			HASH_ADD( hh_handle, features_space->data->handle_hash,
			          hyperparameter, sizeof(ccs_hyperparameter_t), p_hyper_wrapper );
		}
	} else {
		p_hyper_wrapper = (_ccs_hyperparameter_wrapper_t*)utarray_back(hyperparameters);
		HASH_ADD_KEYPTR( hh_name, features_space->data->name_hash,
		                 p_hyper_wrapper->name, strlen(p_hyper_wrapper->name), p_hyper_wrapper );
		HASH_ADD( hh_handle, features_space->data->handle_hash,
		          hyperparameter, sizeof(ccs_hyperparameter_t), p_hyper_wrapper );
	}

	return CCS_SUCCESS;
errorutarray:
	utarray_pop_back(hyperparameters);
errorhyper:
	ccs_release_object(hyperparameter);
	return err;
}
#undef  utarray_oom
#define utarray_oom() exit(-1)

ccs_result_t
ccs_features_space_add_hyperparameters(ccs_features_space_t  features_space,
                                       size_t                num_hyperparameters,
                                       ccs_hyperparameter_t *hyperparameters) {
	CCS_CHECK_OBJ(features_space, CCS_FEATURES_SPACE);
	CCS_CHECK_ARY(num_hyperparameters, hyperparameters);
	for (size_t i = 0; i < num_hyperparameters; i++) {
		ccs_result_t err =
		    ccs_features_space_add_hyperparameter(features_space,
		                                          hyperparameters[i]);
		if (err)
			return err;
	}
	return CCS_SUCCESS;
}

ccs_result_t
ccs_features_space_get_num_hyperparameters(
		ccs_features_space_t  features_space,
		size_t               *num_hyperparameters_ret) {
	CCS_CHECK_OBJ(features_space, CCS_FEATURES_SPACE);
	return _ccs_context_get_num_hyperparameters(
		(ccs_context_t)features_space, num_hyperparameters_ret);
}

ccs_result_t
ccs_features_space_get_hyperparameter(ccs_features_space_t  features_space,
                                      size_t                index,
                                      ccs_hyperparameter_t *hyperparameter_ret) {
	CCS_CHECK_OBJ(features_space, CCS_FEATURES_SPACE);
	return _ccs_context_get_hyperparameter(
		(ccs_context_t)features_space, index, hyperparameter_ret);
}

ccs_result_t
ccs_features_space_get_hyperparameter_by_name(
		ccs_features_space_t  features_space,
		const char *          name,
		ccs_hyperparameter_t *hyperparameter_ret) {
	CCS_CHECK_OBJ(features_space, CCS_FEATURES_SPACE);
	return _ccs_context_get_hyperparameter_by_name(
		(ccs_context_t)features_space, name, hyperparameter_ret);
}

ccs_result_t
ccs_features_space_get_hyperparameter_index_by_name(
		ccs_features_space_t  features_space,
		const char           *name,
		size_t               *index_ret) {
	CCS_CHECK_OBJ(features_space, CCS_FEATURES_SPACE);
	return _ccs_context_get_hyperparameter_index_by_name(
		(ccs_context_t)features_space, name, index_ret);
}

ccs_result_t
ccs_features_space_get_hyperparameter_index(
		ccs_features_space_t  features_space,
		ccs_hyperparameter_t  hyperparameter,
		size_t               *index_ret) {
	CCS_CHECK_OBJ(features_space, CCS_FEATURES_SPACE);
	CCS_CHECK_OBJ(hyperparameter, CCS_HYPERPARAMETER);
	return _ccs_context_get_hyperparameter_index(
		(ccs_context_t)(features_space),
		hyperparameter, index_ret);
}

ccs_result_t
ccs_features_space_get_hyperparameter_indexes(
		ccs_features_space_t  features_space,
		size_t                num_hyperparameters,
		ccs_hyperparameter_t *hyperparameters,
		size_t               *indexes) {
	CCS_CHECK_OBJ(features_space, CCS_FEATURES_SPACE);
	return _ccs_context_get_hyperparameter_indexes(
		(ccs_context_t)features_space, num_hyperparameters,
		 hyperparameters, indexes);
}

ccs_result_t
ccs_features_space_get_hyperparameters(ccs_features_space_t  features_space,
                                       size_t                num_hyperparameters,
                                       ccs_hyperparameter_t *hyperparameters,
                                       size_t               *num_hyperparameters_ret) {
	CCS_CHECK_OBJ(features_space, CCS_FEATURES_SPACE);
	return _ccs_context_get_hyperparameters(
		(ccs_context_t)features_space, num_hyperparameters,
		hyperparameters, num_hyperparameters_ret);
}

ccs_result_t
ccs_features_space_validate_value(ccs_features_space_t  features_space,
                                  size_t                index,
                                  ccs_datum_t           value,
                                  ccs_datum_t          *value_ret) {
	CCS_CHECK_OBJ(features_space, CCS_FEATURES_SPACE);
	return _ccs_context_validate_value((ccs_context_t)features_space,
	                                   index, value, value_ret);
}

static inline ccs_result_t
_check_features(ccs_features_space_t  features_space,
                size_t                num_values,
                ccs_datum_t          *values) {
	ccs_result_t err;
	UT_array *array = features_space->data->hyperparameters;
	if (num_values != utarray_len(array))
		return -CCS_INVALID_FEATURES;
	for (size_t i = 0; i < num_values; i++) {
		ccs_bool_t res;
		_ccs_hyperparameter_wrapper_t *wrapper =
			(_ccs_hyperparameter_wrapper_t *)utarray_eltptr(array, i);
		err = ccs_hyperparameter_check_value(wrapper->hyperparameter,
	                                             values[i], &res);
		if (unlikely(err))
			return err;
		if (res == CCS_FALSE)
			return -CCS_INVALID_FEATURES;
	}
	return CCS_SUCCESS;
}

ccs_result_t
ccs_features_space_check_features(ccs_features_space_t features_space,
                                  ccs_features_t       features) {
	CCS_CHECK_OBJ(features_space, CCS_FEATURES_SPACE);
	CCS_CHECK_OBJ(features, CCS_FEATURES);
	if (features->data->features_space != features_space)
		return -CCS_INVALID_FEATURES;
	return _check_features(features_space,
	                       features->data->num_values,
	                       features->data->values);
}

ccs_result_t
ccs_features_space_check_features_values(ccs_features_space_t  features_space,
                                         size_t                num_values,
                                         ccs_datum_t          *values) {
	CCS_CHECK_OBJ(features_space, CCS_CONFIGURATION_SPACE);
	CCS_CHECK_ARY(num_values, values);
	return _check_features(features_space, num_values, values);
}
