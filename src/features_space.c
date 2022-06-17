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
	_ccs_hyperparameter_index_hash_t *elem, *tmpelem;
	HASH_ITER(hh_handle, features_space->data->handle_hash, elem, tmpelem) {
		HASH_DELETE(hh_handle, features_space->data->handle_hash, elem);
		free(elem);
	}
	utarray_free(features_space->data->hyperparameters);
	return CCS_SUCCESS;
}

static ccs_result_t
_ccs_features_space_serialize_size(
		ccs_object_t                     object,
		ccs_serialize_format_t           format,
		size_t                          *cum_size,
		_ccs_object_serialize_options_t *opts) {
	switch(format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE(_ccs_serialize_bin_size_ccs_context(
			(ccs_context_t)object, cum_size, opts));
		break;
	default:
		CCS_RAISE(CCS_INVALID_VALUE, "Unsupported serialization format: %d", format);
	}
	CCS_VALIDATE(_ccs_object_serialize_user_data_size(
		object, format, cum_size, opts));
	return CCS_SUCCESS;
}

static ccs_result_t
_ccs_features_space_serialize(
		ccs_object_t                      object,
		ccs_serialize_format_t            format,
		size_t                           *buffer_size,
		char                            **buffer,
		_ccs_object_serialize_options_t  *opts) {
	switch(format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE(_ccs_serialize_bin_ccs_context(
			(ccs_context_t)object, buffer_size, buffer, opts));
		break;
	default:
		CCS_RAISE(CCS_INVALID_VALUE, "Unsupported serialization format: %d", format);
	}
	CCS_VALIDATE(_ccs_object_serialize_user_data(
		object, format, buffer_size, buffer, opts));
	return CCS_SUCCESS;
}

static _ccs_features_space_ops_t _features_space_ops =
    { { {&_ccs_features_space_del,
         &_ccs_features_space_serialize_size,
         &_ccs_features_space_serialize} } };

static const UT_icd _hyperparameter_wrapper_icd = {
	sizeof(_ccs_hyperparameter_wrapper_t),
	NULL,
	NULL,
	NULL,
};

#undef  utarray_oom
#define utarray_oom() { \
	CCS_RAISE_ERR_GOTO(err, CCS_OUT_OF_MEMORY, arrays, "Not enough memory to allocate array"); \
}
ccs_result_t
ccs_create_features_space(const char           *name,
                          ccs_features_space_t *features_space_ret) {
	ccs_result_t err;
	CCS_CHECK_PTR(name);
	CCS_CHECK_PTR(features_space_ret);
	uintptr_t mem = (uintptr_t)calloc(1, sizeof(struct _ccs_features_space_s) + sizeof(struct _ccs_features_space_data_s) + strlen(name) + 1);
	CCS_REFUTE(!mem, CCS_OUT_OF_MEMORY);

	ccs_features_space_t feat_space = (ccs_features_space_t)mem;
	_ccs_object_init(&(feat_space->obj), CCS_FEATURES_SPACE,
		(_ccs_object_ops_t *)&_features_space_ops);
	feat_space->data = (struct _ccs_features_space_data_s*)(mem +
		sizeof(struct _ccs_features_space_s));
	feat_space->data->name = (const char *)(mem +
		sizeof(struct _ccs_features_space_s) +
		sizeof(struct _ccs_features_space_data_s));
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
	CCS_VALIDATE(_ccs_context_get_name((ccs_context_t)features_space, name_ret));
	return CCS_SUCCESS;
}

#undef  utarray_oom
#define utarray_oom() { \
	CCS_RAISE_ERR_GOTO(err, CCS_OUT_OF_MEMORY, errormem, "Not enough memory to allocate array"); \
}
#undef uthash_nonfatal_oom
#define uthash_nonfatal_oom(elt) { \
	CCS_RAISE_ERR_GOTO(err, CCS_OUT_OF_MEMORY, errorutarray, "Not enough memory to allocate hash"); \
}
ccs_result_t
ccs_features_space_add_hyperparameter(ccs_features_space_t features_space,
                                      ccs_hyperparameter_t hyperparameter) {
	CCS_CHECK_OBJ(features_space, CCS_FEATURES_SPACE);
	CCS_CHECK_OBJ(hyperparameter, CCS_HYPERPARAMETER);
	ccs_result_t err;
	const char *name;
	size_t sz_name;
	_ccs_hyperparameter_index_hash_t *hyper_hash;
	CCS_VALIDATE(ccs_hyperparameter_get_name(hyperparameter, &name));
	sz_name = strlen(name);
	HASH_FIND(hh_name, features_space->data->name_hash,
	          name, sz_name, hyper_hash);
	CCS_REFUTE_MSG(hyper_hash, CCS_INVALID_HYPERPARAMETER, "An hyperparameter with name '%s' already exists in the feature space", name);
	UT_array *hyperparameters;
	CCS_VALIDATE(ccs_retain_object(hyperparameter));
	_ccs_hyperparameter_wrapper_t hyper_wrapper;
	hyper_wrapper.hyperparameter = hyperparameter;

	hyperparameters = features_space->data->hyperparameters;
	hyper_hash = (_ccs_hyperparameter_index_hash_t *)malloc(sizeof(_ccs_hyperparameter_index_hash_t));
	CCS_REFUTE_ERR_GOTO(err, !hyper_hash, CCS_OUT_OF_MEMORY, errorhyper);
	hyper_hash->hyperparameter = hyperparameter;
	hyper_hash->name = name;
	hyper_hash->index = utarray_len(hyperparameters);

	utarray_push_back(hyperparameters, &hyper_wrapper);

	HASH_ADD_KEYPTR( hh_name, features_space->data->name_hash,
	                 hyper_hash->name, sz_name, hyper_hash );
	HASH_ADD( hh_handle, features_space->data->handle_hash,
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
ccs_features_space_add_hyperparameters(ccs_features_space_t  features_space,
                                       size_t                num_hyperparameters,
                                       ccs_hyperparameter_t *hyperparameters) {
	CCS_CHECK_OBJ(features_space, CCS_FEATURES_SPACE);
	CCS_CHECK_ARY(num_hyperparameters, hyperparameters);
	for (size_t i = 0; i < num_hyperparameters; i++)
		CCS_VALIDATE(ccs_features_space_add_hyperparameter(
		    features_space, hyperparameters[i]));
	return CCS_SUCCESS;
}

ccs_result_t
ccs_features_space_get_num_hyperparameters(
		ccs_features_space_t  features_space,
		size_t               *num_hyperparameters_ret) {
	CCS_CHECK_OBJ(features_space, CCS_FEATURES_SPACE);
	CCS_VALIDATE(_ccs_context_get_num_hyperparameters(
		(ccs_context_t)features_space, num_hyperparameters_ret));
	return CCS_SUCCESS;
}

ccs_result_t
ccs_features_space_get_hyperparameter(ccs_features_space_t  features_space,
                                      size_t                index,
                                      ccs_hyperparameter_t *hyperparameter_ret) {
	CCS_CHECK_OBJ(features_space, CCS_FEATURES_SPACE);
	CCS_VALIDATE(_ccs_context_get_hyperparameter(
		(ccs_context_t)features_space, index, hyperparameter_ret));
	return CCS_SUCCESS;
}

ccs_result_t
ccs_features_space_get_hyperparameter_by_name(
		ccs_features_space_t  features_space,
		const char *          name,
		ccs_hyperparameter_t *hyperparameter_ret) {
	CCS_CHECK_OBJ(features_space, CCS_FEATURES_SPACE);
	CCS_VALIDATE(_ccs_context_get_hyperparameter_by_name(
		(ccs_context_t)features_space, name, hyperparameter_ret));
	return CCS_SUCCESS;
}

ccs_result_t
ccs_features_space_get_hyperparameter_index_by_name(
		ccs_features_space_t  features_space,
		const char           *name,
		size_t               *index_ret) {
	CCS_CHECK_OBJ(features_space, CCS_FEATURES_SPACE);
	CCS_VALIDATE(_ccs_context_get_hyperparameter_index_by_name(
		(ccs_context_t)features_space, name, index_ret));
	return CCS_SUCCESS;
}

ccs_result_t
ccs_features_space_get_hyperparameter_index(
		ccs_features_space_t  features_space,
		ccs_hyperparameter_t  hyperparameter,
		size_t               *index_ret) {
	CCS_CHECK_OBJ(features_space, CCS_FEATURES_SPACE);
	CCS_CHECK_OBJ(hyperparameter, CCS_HYPERPARAMETER);
	CCS_VALIDATE(_ccs_context_get_hyperparameter_index(
		(ccs_context_t)(features_space),
		hyperparameter, index_ret));
	return CCS_SUCCESS;
}

ccs_result_t
ccs_features_space_get_hyperparameter_indexes(
		ccs_features_space_t  features_space,
		size_t                num_hyperparameters,
		ccs_hyperparameter_t *hyperparameters,
		size_t               *indexes) {
	CCS_CHECK_OBJ(features_space, CCS_FEATURES_SPACE);
	CCS_VALIDATE(_ccs_context_get_hyperparameter_indexes(
		(ccs_context_t)features_space, num_hyperparameters,
		 hyperparameters, indexes));
	return CCS_SUCCESS;
}

ccs_result_t
ccs_features_space_get_hyperparameters(ccs_features_space_t  features_space,
                                       size_t                num_hyperparameters,
                                       ccs_hyperparameter_t *hyperparameters,
                                       size_t               *num_hyperparameters_ret) {
	CCS_CHECK_OBJ(features_space, CCS_FEATURES_SPACE);
	CCS_VALIDATE(_ccs_context_get_hyperparameters(
		(ccs_context_t)features_space, num_hyperparameters,
		hyperparameters, num_hyperparameters_ret));
	return CCS_SUCCESS;
}

ccs_result_t
ccs_features_space_validate_value(ccs_features_space_t  features_space,
                                  size_t                index,
                                  ccs_datum_t           value,
                                  ccs_datum_t          *value_ret) {
	CCS_CHECK_OBJ(features_space, CCS_FEATURES_SPACE);
	CCS_VALIDATE(_ccs_context_validate_value(
		(ccs_context_t)features_space, index, value, value_ret));
	return CCS_SUCCESS;
}

static inline ccs_result_t
_check_features(ccs_features_space_t  features_space,
                size_t                num_values,
                ccs_datum_t          *values) {
	UT_array *array = features_space->data->hyperparameters;
	size_t num_hyperparameters = utarray_len(array);
	CCS_REFUTE(num_values != num_hyperparameters, CCS_INVALID_FEATURES);
	for (size_t i = 0; i < num_values; i++) {
		ccs_bool_t res;
		_ccs_hyperparameter_wrapper_t *wrapper =
			(_ccs_hyperparameter_wrapper_t *)utarray_eltptr(array, i);
		CCS_VALIDATE(ccs_hyperparameter_check_value(
		    wrapper->hyperparameter, values[i], &res));
		CCS_REFUTE(res == CCS_FALSE, CCS_INVALID_FEATURES);
	}
	return CCS_SUCCESS;
}

ccs_result_t
ccs_features_space_check_features(ccs_features_space_t features_space,
                                  ccs_features_t       features) {
	CCS_CHECK_OBJ(features_space, CCS_FEATURES_SPACE);
	CCS_CHECK_OBJ(features, CCS_FEATURES);
	CCS_REFUTE(features->data->features_space != features_space, CCS_INVALID_FEATURES);
	CCS_VALIDATE(_check_features(features_space,
		features->data->num_values, features->data->values));
	return CCS_SUCCESS;
}

ccs_result_t
ccs_features_space_check_features_values(ccs_features_space_t  features_space,
                                         size_t                num_values,
                                         ccs_datum_t          *values) {
	CCS_CHECK_OBJ(features_space, CCS_CONFIGURATION_SPACE);
	CCS_CHECK_ARY(num_values, values);
	CCS_VALIDATE(_check_features(features_space, num_values, values));
	return CCS_SUCCESS;
}
