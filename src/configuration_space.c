#include "cconfigspace_internal.h"
#include "configuration_space_internal.h"

static inline _ccs_configuration_space_ops_t *
ccs_configuration_space_get_ops(ccs_configuration_space_t configuration_space) {
	return (_ccs_configuration_space_ops_t *)configuration_space->obj.ops;
}

static ccs_error_t
_ccs_configuration_space_del(ccs_object_t object) {
	ccs_configuration_space_t configuration_space = (ccs_configuration_space_t)object;
	UT_array *array = configuration_space->data->hyperparameters;
	_ccs_hyperparameter_wrapper_t *wrapper = NULL;
	while ( (wrapper = (_ccs_hyperparameter_wrapper_t *)utarray_next(array, wrapper)) ) {
		ccs_release_object(wrapper->hyperparameter);
		ccs_release_object(wrapper->distribution);
	}
	HASH_CLEAR(hh_name, configuration_space->data->name_hash);
	utarray_free(array);
	ccs_release_object(configuration_space->data->rng);
	return CCS_SUCCESS;
}

static _ccs_configuration_space_ops_t _configuration_space_ops =
    { {&_ccs_configuration_space_del} };

static const UT_icd _hyperparameter_wrapper_icd = {
	sizeof(_ccs_hyperparameter_wrapper_t),
	NULL,
	NULL,
	NULL,
};

#undef  utarray_oom
#define utarray_oom() { \
	ccs_release_object(config_space->data->rng); \
	free((void *)mem); \
	return -CCS_ENOMEM; \
}
ccs_error_t
ccs_create_configuration_space(const char                *name,
                               void                      *user_data,
                               ccs_configuration_space_t *configuration_space_ret) {
	if (!name || !configuration_space_ret)
		return -CCS_INVALID_VALUE;
	uintptr_t mem = (uintptr_t)calloc(1, sizeof(struct _ccs_configuration_space_s) + sizeof(struct _ccs_configuration_space_data_s) + strlen(name) + 1);
	if (!mem)
		return -CCS_ENOMEM;
	ccs_rng_t rng;
	ccs_error_t err = ccs_rng_create(&rng);
	if (err) {
		free((void *)mem);
		return err;
	}

	ccs_configuration_space_t config_space = (ccs_configuration_space_t)mem;
	_ccs_object_init(&(config_space->obj), CCS_CONFIGURATION_SPACE, (_ccs_object_ops_t *)&_configuration_space_ops);
	config_space->data = (struct _ccs_configuration_space_data_s*)(mem + sizeof(struct _ccs_configuration_space_s));
	config_space->data->name = (const char *)(mem + sizeof(struct _ccs_configuration_space_s) + sizeof(struct _ccs_configuration_space_data_s));
	config_space->data->user_data = user_data;
	config_space->data->rng = rng;
	utarray_new(config_space->data->hyperparameters, &_hyperparameter_wrapper_icd);
	config_space->data->name_hash = NULL;
	strcpy((char *)(config_space->data->name), name);
	*configuration_space_ret = config_space;
	return CCS_SUCCESS;
}

ccs_error_t
ccs_configuration_space_get_name(ccs_configuration_space_t   configuration_space,
                                 const char                **name_ret) {
	if (!configuration_space || !configuration_space->data)
		return -CCS_INVALID_OBJECT;
	if (!name_ret)
		return -CCS_INVALID_VALUE;
	*name_ret = configuration_space->data->name;
	return CCS_SUCCESS;
}

ccs_error_t
ccs_configuration_space_get_user_data(ccs_configuration_space_t   configuration_space,
                                      void                      **user_data_ret) {
	if (!configuration_space || !configuration_space->data)
		return -CCS_INVALID_OBJECT;
	if (!user_data_ret)
		return -CCS_INVALID_VALUE;
	*user_data_ret = configuration_space->data->user_data;
	return CCS_SUCCESS;
}

ccs_error_t
ccs_configuration_space_set_rng(ccs_configuration_space_t configuration_space,
                                ccs_rng_t                 rng) {
	if (!configuration_space || !configuration_space->data || !rng)
		return -CCS_INVALID_OBJECT;
	ccs_error_t err;
	err = ccs_release_object(configuration_space->data->rng);
	if(err)
		return err;
	configuration_space->data->rng = rng;
	return CCS_SUCCESS;
}

ccs_error_t
ccs_configuration_space_get_rng(ccs_configuration_space_t  configuration_space,
                                ccs_rng_t                 *rng_ret) {
	if (!configuration_space || !configuration_space->data)
		return -CCS_INVALID_OBJECT;
	if (!rng_ret)
		return -CCS_INVALID_VALUE;
	*rng_ret = configuration_space->data->rng;
	return CCS_SUCCESS;
}


#undef  utarray_oom
#define utarray_oom() { \
	err = -CCS_ENOMEM; \
	goto errordistrib; \
}
#undef uthash_nonfatal_oom
#define uthash_nonfatal_oom(elt) { \
	err = -CCS_ENOMEM; \
	goto errorutarray; \
}
ccs_error_t
ccs_configuration_space_add_hyperparameter(ccs_configuration_space_t configuration_space,
                                           ccs_hyperparameter_t      hyperparameter,
                                           ccs_distribution_t        distribution) {
	if (!configuration_space || !configuration_space->data || !hyperparameter)
		return -CCS_INVALID_OBJECT;
	ccs_error_t err;
	UT_array *hyperparameters;
	unsigned int index;
	_ccs_hyperparameter_wrapper_t hyper_wrapper;
	hyper_wrapper.hyperparameter = hyperparameter;
	if (distribution) {
		ccs_retain_object(distribution);
		hyper_wrapper.distribution = distribution;
	} else {
		err = ccs_hyperparameter_get_default_distribution(hyperparameter, &hyper_wrapper.distribution);
		if (err)
			goto error;
	}
	hyperparameters = configuration_space->data->hyperparameters;
	index = utarray_len(hyperparameters);
	hyper_wrapper.index = index;
	err = ccs_hyperparameter_get_name(hyperparameter, &hyper_wrapper.name);
	if (err)
		goto errordistrib;
	utarray_push_back(hyperparameters, &hyper_wrapper);

	_ccs_hyperparameter_wrapper_t *p_hyper_wrapper =
	   (_ccs_hyperparameter_wrapper_t*)utarray_eltptr(hyperparameters, index);
	HASH_ADD_KEYPTR( hh_name, configuration_space->data->name_hash,
	                 p_hyper_wrapper->name, strlen(p_hyper_wrapper->name),
	                 p_hyper_wrapper );

	return CCS_SUCCESS;
errorutarray:
	utarray_pop_back(hyperparameters);
errordistrib:
	ccs_release_object(hyper_wrapper.distribution);
error:
	return err;
}
#undef  utarray_oom
#define utarray_oom() exit(-1)

ccs_error_t
ccs_configuration_space_add_hyperparameters(ccs_configuration_space_t  configuration_space,
                                            size_t                     num_hyperparameters,
                                            ccs_hyperparameter_t      *hyperparameters,
                                            ccs_distribution_t        *distributions) {
	if (!configuration_space)
		return -CCS_INVALID_OBJECT;
	if (num_hyperparameters > 0 && !hyperparameters)
		return -CCS_INVALID_VALUE;
	for (size_t i = 0; i < num_hyperparameters; i++) {
		ccs_distribution_t distribution = NULL;
		if (distributions)
			distribution = distributions[i];
		ccs_error_t err =
		    ccs_configuration_space_add_hyperparameter( configuration_space,
		                                                hyperparameters[i],
		                                                distribution );
		if (err)
			return err;
	}
	return CCS_SUCCESS;
}

ccs_error_t
ccs_configuration_space_get_num_hyperparameters(ccs_configuration_space_t  configuration_space,
                                                size_t                     *num_hyperparameters_ret) {
	if (!configuration_space || !configuration_space->data)
		return -CCS_INVALID_OBJECT;
	if (!num_hyperparameters_ret)
		return -CCS_INVALID_VALUE;
	*num_hyperparameters_ret = utarray_len(configuration_space->data->hyperparameters);
	return CCS_SUCCESS;
}

ccs_error_t
ccs_configuration_space_get_hyperparameter(ccs_configuration_space_t  configuration_space,
                                           size_t                     index,
                                           ccs_hyperparameter_t      *hyperparameter_ret) {
	if (!configuration_space || !configuration_space->data)
		return -CCS_INVALID_OBJECT;
	if (!hyperparameter_ret)
		return -CCS_INVALID_VALUE;
	_ccs_hyperparameter_wrapper_t *wrapper = (_ccs_hyperparameter_wrapper_t*)
	    utarray_eltptr(configuration_space->data->hyperparameters, (unsigned int)index);
	if (!wrapper)
		return -CCS_INVALID_VALUE;
	*hyperparameter_ret = wrapper->hyperparameter;
	return CCS_SUCCESS;
}

ccs_error_t
ccs_configuration_space_get_hyperparameters(ccs_configuration_space_t  configuration_space,
                                            size_t                     num_hyperparameters
,
                                            ccs_hyperparameter_t      *hyperparameters,
                                            size_t                    *num_hyperparameters_ret) {
	if (!configuration_space || !configuration_space->data)
		return -CCS_INVALID_OBJECT;
	if (num_hyperparameters && !hyperparameters)
		return -CCS_INVALID_VALUE;
	if (hyperparameters && !num_hyperparameters)
		return -CCS_INVALID_VALUE;
	if (!num_hyperparameters_ret && !hyperparameters)
		return -CCS_INVALID_VALUE;
	UT_array *array = configuration_space->data->hyperparameters;
	size_t size = utarray_len(array);
	if (hyperparameters) {
		_ccs_hyperparameter_wrapper_t *wrapper = NULL;
		size_t index = 0;
		while ( (wrapper = (_ccs_hyperparameter_wrapper_t *)utarray_next(array, wrapper)) )
			hyperparameters[index] = wrapper->hyperparameter;
		for (size_t i = size; i < num_hyperparameters; i++)
			hyperparameters[i] = NULL;
	}
	if (num_hyperparameters_ret)
		*num_hyperparameters_ret = size;
	return CCS_SUCCESS;
}

