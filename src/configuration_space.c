#include "cconfigspace_internal.h"
#include "configuration_space_internal.h"
#include "configuration_internal.h"

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
	}
	HASH_CLEAR(hh_name, configuration_space->data->name_hash);
	utarray_free(array);
	_ccs_distribution_wrapper_t *dw;
	_ccs_distribution_wrapper_t *tmp;
	DL_FOREACH_SAFE(configuration_space->data->distribution_list, dw, tmp) {
		DL_DELETE(configuration_space->data->distribution_list, dw);
		ccs_release_object(dw->distribution);
		free(dw);
	}
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
	config_space->data->distribution_list = NULL;
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
	goto errordistrib_wrapper; \
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
	const char *name;
	size_t sz_name;
	_ccs_hyperparameter_wrapper_t *p_hyper_wrapper;
	err = ccs_hyperparameter_get_name(hyperparameter, &name);
	if (err)
		goto error;
	sz_name = strlen(name);
	HASH_FIND(hh_name, configuration_space->data->name_hash,
	          name, sz_name, p_hyper_wrapper);
	if (p_hyper_wrapper) {
		err = -CCS_INVALID_HYPERPARAMETER;
		goto error;
	}
	UT_array *hyperparameters;
	unsigned int index;
	size_t dimension;
	_ccs_hyperparameter_wrapper_t hyper_wrapper;
	_ccs_distribution_wrapper_t *distrib_wrapper;
	uintptr_t pmem;
	hyper_wrapper.hyperparameter = hyperparameter;
	err = ccs_retain_object(hyperparameter);
	if (err)
		goto error;

	if (distribution) {
		err = ccs_distribution_get_dimension(distribution, &dimension);
		if (err)
			goto errorhyper;
		if (dimension != 1) {
			err = -CCS_INVALID_DISTRIBUTION;
			goto errorhyper;
		}
		err = ccs_retain_object(distribution);
		if (err)
			goto errorhyper;
	} else {
		err = ccs_hyperparameter_get_default_distribution(hyperparameter, &distribution);
		if (err)
			goto errorhyper;
		dimension = 1;
	}
	pmem = (uintptr_t)malloc(sizeof(_ccs_distribution_wrapper_t) + sizeof(size_t)*dimension);
	if (!pmem) {
		err = -CCS_ENOMEM;
		goto errordistrib;
	}
        distrib_wrapper = (_ccs_distribution_wrapper_t *)pmem;
	distrib_wrapper->distribution = distribution;
	distrib_wrapper->dimension = dimension;
	distrib_wrapper->hyperparameter_indexes = (size_t *)(pmem + sizeof(_ccs_distribution_wrapper_t));
	distrib_wrapper->hyperparameter_indexes[0] = 0;
	hyperparameters = configuration_space->data->hyperparameters;
	index = utarray_len(hyperparameters);
	hyper_wrapper.index = index;
	hyper_wrapper.distribution_index = 0;
	hyper_wrapper.distribution = distrib_wrapper;
	hyper_wrapper.name = name;
	utarray_push_back(hyperparameters, &hyper_wrapper);

	p_hyper_wrapper =
	   (_ccs_hyperparameter_wrapper_t*)utarray_eltptr(hyperparameters, index);
	HASH_ADD_KEYPTR( hh_name, configuration_space->data->name_hash,
	                 name, sz_name, p_hyper_wrapper );
	DL_APPEND(configuration_space->data->distribution_list, distrib_wrapper);

	return CCS_SUCCESS;
errorutarray:
	utarray_pop_back(hyperparameters);
errordistrib_wrapper:
	free(distrib_wrapper);
errordistrib:
	ccs_release_object(distribution);
errorhyper:
	ccs_release_object(hyperparameter);
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
		return -CCS_OUT_OF_BOUNDS;
	*hyperparameter_ret = wrapper->hyperparameter;
	return CCS_SUCCESS;
}

ccs_error_t
ccs_configuration_space_get_hyperparameter_by_name(
		ccs_configuration_space_t  configuration_space,
		const char *               name,
		ccs_hyperparameter_t      *hyperparameter_ret) {
	if (!configuration_space || !configuration_space->data)
		return -CCS_INVALID_OBJECT;
	if (!hyperparameter_ret)
		return -CCS_INVALID_VALUE;
	_ccs_hyperparameter_wrapper_t *wrapper;
	size_t sz_name;
	sz_name = strlen(name);
	HASH_FIND(hh_name, configuration_space->data->name_hash,
	          name, sz_name, wrapper);
	if (!wrapper)
		return -CCS_INVALID_NAME;
	*hyperparameter_ret = wrapper->hyperparameter;
	return CCS_SUCCESS;
}

ccs_error_t
ccs_configuration_space_get_hyperparameter_index_by_name(
		ccs_configuration_space_t  configuration_space,
		const char                *name,
		size_t                    *index_ret) {
	if (!configuration_space || !configuration_space->data)
		return -CCS_INVALID_OBJECT;
	if (!index_ret)
		return -CCS_INVALID_VALUE;
	_ccs_hyperparameter_wrapper_t *wrapper;
	size_t sz_name;
	sz_name = strlen(name);
	HASH_FIND(hh_name, configuration_space->data->name_hash,
	          name, sz_name, wrapper);
	if (!wrapper)
		return -CCS_INVALID_NAME;
	*index_ret = wrapper->index;
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
		if (num_hyperparameters < size)
			return -CCS_INVALID_VALUE;
		_ccs_hyperparameter_wrapper_t *wrapper = NULL;
		size_t index = 0;
		while ( (wrapper = (_ccs_hyperparameter_wrapper_t *)utarray_next(array, wrapper)) )
			hyperparameters[index++] = wrapper->hyperparameter;
		for (size_t i = size; i < num_hyperparameters; i++)
			hyperparameters[i] = NULL;
	}
	if (num_hyperparameters_ret)
		*num_hyperparameters_ret = size;
	return CCS_SUCCESS;
}

ccs_error_t
ccs_configuration_space_get_default_configuration(ccs_configuration_space_t  configuration_space,
                                                  ccs_configuration_t       *configuration_ret) {
	if (!configuration_space || !configuration_space->data)
		return -CCS_INVALID_OBJECT;
	if (!configuration_ret)
		return -CCS_INVALID_VALUE;
	ccs_error_t err;
	ccs_configuration_t config;
	err = ccs_create_configuration(configuration_space, 0, NULL, NULL, &config);
	if (err)
		return err;
	UT_array *array = configuration_space->data->hyperparameters;
	size_t index = 0;
	_ccs_hyperparameter_wrapper_t *wrapper = NULL;
	ccs_datum_t d;
	while ( (wrapper = (_ccs_hyperparameter_wrapper_t *)utarray_next(array, wrapper)) ) {
		err = ccs_hyperparameter_get_default_value(wrapper->hyperparameter, &d);
		if (unlikely(err))
			goto error;
		err = ccs_configuration_set_value(config, index++, d);
		if (unlikely(err))
			goto error;
	}
	*configuration_ret = config;
	return CCS_SUCCESS;
error:
	ccs_release_object(config);
	return err;
}

ccs_error_t
ccs_configuration_space_check_configuration(ccs_configuration_space_t configuration_space,
                                            ccs_configuration_t       configuration) {
	if (!configuration_space || !configuration_space->data)
		return -CCS_INVALID_OBJECT;
	if (!configuration || !configuration->data)
		return -CCS_INVALID_OBJECT;
	if (configuration->data->configuration_space != configuration_space)
		return -CCS_INVALID_CONFIGURATION;
	size_t index = 0;
	UT_array *array = configuration_space->data->hyperparameters;
	if (configuration->data->num_values != utarray_len(array))
		return -CCS_INVALID_CONFIGURATION;
	_ccs_hyperparameter_wrapper_t *wrapper = NULL;
	ccs_datum_t *values = configuration->data->values;
	while ( (wrapper = (_ccs_hyperparameter_wrapper_t *)utarray_next(array, wrapper)) ) {
		ccs_bool_t res;
		ccs_error_t err;
		err = ccs_hyperparameter_check_value(wrapper->hyperparameter,
		                                     values[index++], &res);
		if (unlikely(err))
			return err;
		if (res == CCS_FALSE)
			return -CCS_INVALID_CONFIGURATION;
	}
	return CCS_SUCCESS;
}


