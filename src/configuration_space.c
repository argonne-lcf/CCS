#include "cconfigspace_internal.h"
#include "configuration_space_internal.h"
#include "configuration_internal.h"

static ccs_error_t
_generate_constraints(ccs_configuration_space_t configuration_space);

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
		if (wrapper->condition)
			ccs_release_object(wrapper->condition);
		utarray_free(wrapper->parents);
		utarray_free(wrapper->children);
	}
	array = configuration_space->data->forbidden_clauses;
	ccs_expression_t *expr = NULL;
	while ( (expr = (ccs_expression_t *)utarray_next(array, expr)) ) {
		ccs_release_object(*expr);
	}
	HASH_CLEAR(hh_name, configuration_space->data->name_hash);
	HASH_CLEAR(hh_handle, configuration_space->data->handle_hash);
	utarray_free(configuration_space->data->hyperparameters);
	utarray_free(configuration_space->data->forbidden_clauses);
	utarray_free(configuration_space->data->sorted_indexes);
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

static const UT_icd _forbidden_clauses_icd = {
	sizeof(ccs_expression_t),
	NULL,
	NULL,
	NULL,
};

static UT_icd _size_t_icd = {
	sizeof(size_t),
	NULL,
	NULL,
	NULL
};

#undef  utarray_oom
#define utarray_oom() { \
	ccs_release_object(config_space->data->rng); \
	err = -CCS_ENOMEM; \
	goto arrays; \
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
	config_space->data->hyperparameters = NULL;
	config_space->data->forbidden_clauses = NULL;
	config_space->data->sorted_indexes = NULL;
	utarray_new(config_space->data->hyperparameters, &_hyperparameter_wrapper_icd);
	utarray_new(config_space->data->forbidden_clauses, &_forbidden_clauses_icd);
	utarray_new(config_space->data->sorted_indexes, &_size_t_icd);
	config_space->data->name_hash = NULL;
	config_space->data->distribution_list = NULL;
	config_space->data->graph_ok = CCS_TRUE;
	strcpy((char *)(config_space->data->name), name);
	*configuration_space_ret = config_space;
	return CCS_SUCCESS;
arrays:
	if (config_space->data->hyperparameters)
		utarray_free(config_space->data->hyperparameters);
	if (config_space->data->forbidden_clauses)
		utarray_free(config_space->data->forbidden_clauses);
	if (config_space->data->sorted_indexes)
		utarray_free(config_space->data->sorted_indexes);
	free((void *)mem);
	return err;
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
	hyperparameters = configuration_space->data->hyperparameters;
	index = utarray_len(hyperparameters);
	distrib_wrapper->hyperparameter_indexes[0] = index;
	hyper_wrapper.index = index;
	hyper_wrapper.distribution_index = 0;
	hyper_wrapper.distribution = distrib_wrapper;
	hyper_wrapper.name = name;
	hyper_wrapper.condition = NULL;
	hyper_wrapper.parents = NULL;
	hyper_wrapper.children = NULL;
	utarray_new(hyper_wrapper.parents, &_size_t_icd);
	utarray_new(hyper_wrapper.children, &_size_t_icd);
	utarray_push_back(hyperparameters, &hyper_wrapper);
	utarray_push_back(configuration_space->data->sorted_indexes, &(hyper_wrapper.index));

	p_hyper_wrapper =
	   (_ccs_hyperparameter_wrapper_t*)utarray_eltptr(hyperparameters, index);
	HASH_ADD_KEYPTR( hh_name, configuration_space->data->name_hash,
	                 name, sz_name, p_hyper_wrapper );
	HASH_ADD( hh_handle, configuration_space->data->handle_hash,
	          hyperparameter, sizeof(ccs_hyperparameter_t), p_hyper_wrapper );
	DL_APPEND( configuration_space->data->distribution_list, distrib_wrapper );

	return CCS_SUCCESS;
errorutarray:
	utarray_pop_back(hyperparameters);
errordistrib_wrapper:
	if (hyper_wrapper.parents)
		utarray_free(hyper_wrapper.parents);
	if (hyper_wrapper.children)
		utarray_free(hyper_wrapper.children);
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
ccs_configuration_space_get_hyperparameter_index(
		ccs_configuration_space_t  configuration_space,
		ccs_hyperparameter_t       hyperparameter,
		size_t                    *index_ret) {
	if (!configuration_space || !configuration_space->data)
		return -CCS_INVALID_OBJECT;
	if (!hyperparameter)
		return -CCS_INVALID_HYPERPARAMETER;
	if (!index_ret)
		return -CCS_INVALID_VALUE;
	_ccs_hyperparameter_wrapper_t *wrapper;
	HASH_FIND(hh_handle, configuration_space->data->handle_hash,
	          &hyperparameter, sizeof(ccs_hyperparameter_t), wrapper);
	if (!wrapper)
		return -CCS_INVALID_HYPERPARAMETER;
	*index_ret = wrapper->index;
	return CCS_SUCCESS;
}

ccs_error_t
ccs_configuration_space_get_hyperparameter_indexes(
		ccs_configuration_space_t  configuration_space,
		size_t                     num_hyperparameters,
		ccs_hyperparameter_t      *hyperparameters,
		size_t                    *indexes) {
	if (!configuration_space || !configuration_space->data)
		return -CCS_INVALID_OBJECT;
	if (num_hyperparameters && (!hyperparameters || !indexes ))
		return -CCS_INVALID_VALUE;
	_ccs_hyperparameter_wrapper_t *wrapper;
	for(size_t i = 0; i < num_hyperparameters; i++) {
		HASH_FIND(hh_handle, configuration_space->data->handle_hash,
			hyperparameters + i, sizeof(ccs_hyperparameter_t), wrapper);
		if (!wrapper)
			return -CCS_INVALID_HYPERPARAMETER;
		indexes[i] = wrapper->index;
	}
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
	_ccs_hyperparameter_wrapper_t *wrapper = NULL;
	ccs_datum_t *values = config->data->values;
	while ( (wrapper = (_ccs_hyperparameter_wrapper_t *)utarray_next(array, wrapper)) ) {
		err = ccs_hyperparameter_get_default_value(wrapper->hyperparameter,
		                                           values++);
		if (unlikely(err)) {
			ccs_release_object(config);
			return err;
		}
	}
	*configuration_ret = config;
	return CCS_SUCCESS;
}

static ccs_error_t
_set_actives(ccs_configuration_space_t configuration_space,
             ccs_configuration_t       configuration) {
	size_t *p_index = NULL;
	UT_array *indexes = configuration_space->data->sorted_indexes;
	UT_array *array = configuration_space->data->hyperparameters;
	ccs_datum_t *values = configuration->data->values;
	while ( (p_index = (size_t *)utarray_next(indexes, p_index)) ) {
		_ccs_hyperparameter_wrapper_t *wrapper = NULL;
		wrapper = (_ccs_hyperparameter_wrapper_t *)utarray_eltptr(array, *p_index);
		if (!wrapper->condition) {
			continue;
		}
		UT_array *parents = wrapper->parents;
		size_t *p_parent = NULL;
		while ( (p_parent = (size_t*)utarray_next(parents, p_parent)) ) {
			if (values[*p_parent].type == CCS_INACTIVE) {
				values[*p_index] = ccs_inactive;
				break;
			}
		}
		if (values[*p_index].type == CCS_INACTIVE)
			continue;
		ccs_datum_t result;
		ccs_error_t err;
		err = ccs_expression_eval(wrapper->condition, configuration_space,
		                          values, &result);
		if (err)
			return err;
		if (!(result.type == CCS_BOOLEAN && result.value.i == CCS_TRUE))
			values[*p_index] = ccs_inactive;
	}
	return CCS_SUCCESS;
}

static ccs_error_t
_test_forbidden(ccs_configuration_space_t  configuration_space,
                ccs_datum_t               *values,
		ccs_bool_t                *is_valid) {
	ccs_error_t err;
	UT_array *array = configuration_space->data->forbidden_clauses;
	ccs_expression_t *p_expression = NULL;
	*is_valid = CCS_FALSE;
	while ( (p_expression = (ccs_expression_t *)
	               utarray_next(array, p_expression)) ) {
		ccs_datum_t result;
		err = ccs_expression_eval(*p_expression, configuration_space,
		                          values, &result);
		if (err == -CCS_INACTIVE_HYPERPARAMETER)
			continue;
		else if (err)
			return err;
		if (result.type == CCS_BOOLEAN && result.value.i == CCS_TRUE)
			return CCS_SUCCESS;
	}
	*is_valid = CCS_TRUE;
	return CCS_SUCCESS;
}

static inline ccs_error_t
_check_configuration(ccs_configuration_space_t  configuration_space,
                     size_t                     num_values,
                     ccs_datum_t               *values) {
	ccs_error_t err;
	UT_array *indexes = configuration_space->data->sorted_indexes;
	UT_array *array = configuration_space->data->hyperparameters;
	if (num_values != utarray_len(array))
		return -CCS_INVALID_CONFIGURATION;
	size_t *p_index = NULL;
	while ( (p_index = (size_t *)utarray_next(indexes, p_index)) ) {
		ccs_bool_t active = CCS_TRUE;
		_ccs_hyperparameter_wrapper_t *wrapper = NULL;
		wrapper = (_ccs_hyperparameter_wrapper_t *)utarray_eltptr(array, *p_index);
		if (wrapper->condition) {
			UT_array *parents = wrapper->parents;
			size_t *p_parent = NULL;
			while ( (p_parent = (size_t*)utarray_next(parents, p_parent)) ) {
				if (values[*p_parent].type == CCS_INACTIVE) {
					active = CCS_FALSE;
					break;
				}
			}
			if (active) {
				ccs_datum_t result;
				ccs_error_t err;
				err = ccs_expression_eval(wrapper->condition,
				                          configuration_space,
		                                          values, &result);
				if (err)
					return err;
				if (!(result.type == CCS_BOOLEAN && result.value.i == CCS_TRUE)) {
					active = CCS_FALSE;
				}
			}
		}
		if (active != (values[*p_index].type == CCS_INACTIVE ? CCS_FALSE : CCS_TRUE))
			return -CCS_INVALID_CONFIGURATION;
		if (active) {
			ccs_bool_t res;
			err = ccs_hyperparameter_check_value(wrapper->hyperparameter,
		                                             values[*p_index], &res);
			if (unlikely(err))
				return err;
			if (res == CCS_FALSE)
				return -CCS_INVALID_CONFIGURATION;
		}
	}
	ccs_bool_t valid;
	err = _test_forbidden(configuration_space, values, &valid);
	if (err)
		return err;
	if (!valid)
		return -CCS_INVALID_CONFIGURATION;
	return CCS_SUCCESS;
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
	if (!configuration_space->data->graph_ok) {
		ccs_error_t err;
		err = _generate_constraints(configuration_space);
		if (err)
			return err;
	}
	return _check_configuration(configuration_space,
	                            configuration->data->num_values,
	                            configuration->data->values);
}

ccs_error_t
ccs_configuration_space_check_configuration_values(ccs_configuration_space_t  configuration_space,
                                                   size_t                     num_values,
                                                   ccs_datum_t               *values) {
	if (!configuration_space || !configuration_space->data)
		return -CCS_INVALID_OBJECT;
	if (!values)
		return -CCS_INVALID_VALUE;
	if (!configuration_space->data->graph_ok) {
		ccs_error_t err;
		err = _generate_constraints(configuration_space);
		if (err)
			return err;
	}
	return _check_configuration(configuration_space, num_values, values);
}


static ccs_error_t
_sample(ccs_configuration_space_t  configuration_space,
	ccs_configuration_t        config,
	ccs_bool_t                *found) {
	ccs_error_t err;
	ccs_rng_t rng = configuration_space->data->rng;
	UT_array *array = configuration_space->data->hyperparameters;
	_ccs_hyperparameter_wrapper_t *wrapper = NULL;
	ccs_datum_t *values = config->data->values;
	while ( (wrapper = (_ccs_hyperparameter_wrapper_t *)
	                       utarray_next(array, wrapper)) ) {
		err = ccs_hyperparameter_sample(wrapper->hyperparameter,
		                                wrapper->distribution->distribution,
		                                rng, values++);
		if (unlikely(err))
			return err;
	}
	err = _set_actives(configuration_space, config);
	if (err)
		return err;
	err = _test_forbidden(configuration_space, config->data->values, found);
	if (err)
		return err;
	return CCS_SUCCESS;
}

ccs_error_t
ccs_configuration_space_sample(ccs_configuration_space_t  configuration_space,
                               ccs_configuration_t       *configuration_ret) {
	if (!configuration_space || !configuration_space->data)
		return -CCS_INVALID_OBJECT;
	if (!configuration_ret)
		return -CCS_INVALID_VALUE;
	ccs_error_t err;
	ccs_configuration_t config;
	if (!configuration_space->data->graph_ok) {
		err = _generate_constraints(configuration_space);
		if (err)
			return err;
	}
	err = ccs_create_configuration(configuration_space, 0, NULL, NULL, &config);
	if (err)
		return err;
	ccs_bool_t found;
	int counter = 0;
	do {
		err = _sample(configuration_space, config, &found);
		if (err) {
			ccs_release_object(config);
			return err;
		}
		counter++;
	} while (!found && counter < 100);
	if (!found) {
		ccs_release_object(config);
		return -CCS_SAMPLING_UNSUCCESSFUL;
	}
	*configuration_ret = config;
	return CCS_SUCCESS;
}

ccs_error_t
ccs_configuration_space_samples(ccs_configuration_space_t  configuration_space,
                                size_t                     num_configurations,
                                ccs_configuration_t       *configurations) {
	if (!configuration_space || !configuration_space->data)
		return -CCS_INVALID_OBJECT;
	if (num_configurations && !configurations)
		return -CCS_INVALID_VALUE;
	if (!num_configurations)
		return CCS_SUCCESS;
	ccs_error_t err;
	if (!configuration_space->data->graph_ok) {
		err = _generate_constraints(configuration_space);
		if (err)
			return err;
	}
	size_t     counter = 0;
	size_t     count = 0;
	ccs_bool_t found;
	ccs_configuration_t config = NULL;
	// Naive implementation
	//See below for more efficient ideas...
	for (size_t i = 0; i < num_configurations; i++)
		configurations[i] = NULL;
	while (count < num_configurations && counter < 100 * num_configurations) {
		if (!config) {
			err = ccs_create_configuration(configuration_space, 0, NULL, NULL, &config);
			if (err)
				return err;
		}
		err = _sample(configuration_space, config, &found);
		if (err) {
			ccs_release_object(config);
			return err;
		}
		counter++;
		if (found) {
			configurations[count++] = config;
			config = NULL;
		}
	}
	if (count < num_configurations)
		return -CCS_SAMPLING_UNSUCCESSFUL;
	return CCS_SUCCESS;
//	UT_array *array = configuration_space->data->hyperparameters;
//	size_t num_hyper = utarray_len(array);
//	ccs_datum_t *values = (ccs_datum_t *)calloc(1, sizeof(ccs_datum_t)*num_configurations*num_hyper);
//	ccs_datum_t *p_values = values;
//	ccs_rng_t rng = configuration_space->data->rng;
//	_ccs_hyperparameter_wrapper_t *wrapper = NULL;
//	while ( (wrapper = (_ccs_hyperparameter_wrapper_t *)utarray_next(array, wrapper)) ) {
//		err = ccs_hyperparameter_samples(wrapper->hyperparameter,
//		                                 wrapper->distribution->distribution,
//						 rng, num_configurations, p_values);
//		if (unlikely(err)) {
//			free(values);
//			return err;
//		}
//		p_values += num_configurations;
//	}
//	size_t i;
//	for(i = 0; i < num_configurations; i++) {
//		err = ccs_create_configuration(configuration_space, 0, NULL, NULL, configurations + i);
//		if (unlikely(err)) {
//			free(values);
//			for(size_t j = 0; j < i; j++)
//				ccs_release_object(configurations + j);
//			return err;
//		}
//	}
//	for(i = 0; i < num_configurations; i++) {
//		for(size_t j = 0; j < num_hyper; j++)
//			configurations[i]->data->values[j] =
//				values[j*num_configurations + i];
//
//		err = _set_actives(configuration_space, configurations[i]);
//		if (err) {
//			free(values);
//			for(size_t j = 0; j < num_configurations; j++)
//				ccs_release_object(configurations + j);
//			return err;
//		}
//	}
//	free(values);
//	return CCS_SUCCESS;
}

static int _size_t_sort(const void *a, const void *b) {
	const size_t sa = *(const size_t *)a;
	const size_t sb = *(const size_t *)b;
	return sa < sb ? -1 : sa > sb ? 1 : 0;
}

static void _uniq_size_t_array(UT_array *array) {
	size_t count = utarray_len(array);
	if (count == 0)
		return;
	utarray_sort(array, &_size_t_sort);
	size_t real_count = 0;
	size_t *p = (size_t *)utarray_front(array);
	size_t *p2 = p;
	real_count++;
	while ( (p = (size_t *)utarray_next(array, p)) ) {
		if (*p != *p2) {
			p2 = (size_t *)utarray_next(array, p2);
			*p2 = *p;
			real_count++;
		}
	}
	utarray_resize(array, real_count);
}

struct _hyper_list_s;
struct _hyper_list_s {
	size_t in_edges;
	size_t index;
	struct _hyper_list_s *next;
	struct _hyper_list_s *prev;
};

#undef  utarray_oom
#define utarray_oom() { \
	free((void *)list); \
	return -CCS_ENOMEM; \
}
static ccs_error_t
_topological_sort(ccs_configuration_space_t configuration_space) {
	utarray_clear(configuration_space->data->sorted_indexes);
	UT_array *array = configuration_space->data->hyperparameters;
	size_t count = utarray_len(array);

	struct _hyper_list_s *list = (struct _hyper_list_s *)calloc(1,
		sizeof(struct _hyper_list_s) * count);
	if (!list)
		return -CCS_ENOMEM;
	struct _hyper_list_s *queue = NULL;

	_ccs_hyperparameter_wrapper_t *wrapper = NULL;
	size_t index = 0;
	while ( (wrapper = (_ccs_hyperparameter_wrapper_t *)utarray_next(array, wrapper)) ) {
		size_t in_edges = utarray_len(wrapper->parents);
		list[index].in_edges = in_edges;
		list[index].index = index;
		if (in_edges == 0)
			DL_APPEND(queue, list + index);
		index++;
	}
	size_t processed = 0;
	while(queue) {
		struct _hyper_list_s *e = queue;
		DL_DELETE(queue, queue);
		wrapper = (_ccs_hyperparameter_wrapper_t *)
			utarray_eltptr(array, e->index);
		size_t *child = NULL;
		while ( (child = (size_t *)utarray_next(wrapper->children, child)) ) {
			list[*child].in_edges--;
			if (list[*child].in_edges == 0) {
				DL_APPEND(queue, list + *child);
			}
		}
		utarray_push_back(configuration_space->data->sorted_indexes, &(e->index));
		processed++;
	};
	free(list);
	if (processed < count)
		return -CCS_INVALID_GRAPH;
	return CCS_SUCCESS;
}

#undef  utarray_oom
#define utarray_oom() { \
	free((void *)mem); \
	return -CCS_ENOMEM; \
}
static ccs_error_t
_recompute_graph(ccs_configuration_space_t configuration_space) {
	_ccs_hyperparameter_wrapper_t *wrapper = NULL;
	UT_array *array = configuration_space->data->hyperparameters;
	while ( (wrapper = (_ccs_hyperparameter_wrapper_t *)utarray_next(array, wrapper)) ) {
		utarray_clear(wrapper->parents);
		utarray_clear(wrapper->children);
	}
	wrapper = NULL;
	while ( (wrapper = (_ccs_hyperparameter_wrapper_t *)utarray_next(array, wrapper)) ) {
		if (!wrapper->condition)
			continue;
		size_t count;
		ccs_error_t err;
		err = ccs_expression_get_hyperparameters(wrapper->condition, 0, NULL, &count);
		if (err)
			return err;
		if (count == 0)
			continue;
		ccs_hyperparameter_t *parents = NULL;
		size_t               *parents_index = NULL;
		_ccs_hyperparameter_wrapper_t *parent_wrapper = NULL;
		intptr_t mem = (intptr_t)malloc(count *
			(sizeof(ccs_hyperparameter_t) + sizeof(size_t)));
		if (!mem)
			return -CCS_ENOMEM;
		parents = (ccs_hyperparameter_t *)mem;
		parents_index = (size_t *)(mem + count * sizeof(ccs_hyperparameter_t));
		err = ccs_expression_get_hyperparameters(wrapper->condition, count, parents, NULL);
		if (err) {
			free((void *)mem);
			return err;
		}
		err = ccs_configuration_space_get_hyperparameter_indexes(
			configuration_space, count, parents, parents_index);
		if (err) {
			free((void *)mem);
			return err;
		}
		for (size_t i = 0; i < count; i++) {
			utarray_push_back(wrapper->parents, parents_index + i);
			parent_wrapper = (_ccs_hyperparameter_wrapper_t *)utarray_eltptr(array, parents_index[i]);
			utarray_push_back(parent_wrapper->children, &(wrapper->index));
		}
		free((void *)mem);
	}
	wrapper = NULL;
	while ( (wrapper = (_ccs_hyperparameter_wrapper_t *)utarray_next(array, wrapper)) ) {
		_uniq_size_t_array(wrapper->parents);
		_uniq_size_t_array(wrapper->children);
        }
	return CCS_SUCCESS;
}
#undef  utarray_oom
#define utarray_oom() { \
	exit(-1); \
}

static ccs_error_t
_generate_constraints(ccs_configuration_space_t configuration_space) {
	ccs_error_t err;
	err = _recompute_graph(configuration_space);
	if (err)
		return err;
	err = _topological_sort(configuration_space);
	if (err)
		return err;
	configuration_space->data->graph_ok = CCS_TRUE;
	return CCS_SUCCESS;
}

ccs_error_t
ccs_configuration_space_set_condition(ccs_configuration_space_t configuration_space,
                                      size_t                    hyperparameter_index,
                                      ccs_expression_t          expression) {
	if (!configuration_space || !configuration_space->data || !expression)
		return -CCS_INVALID_OBJECT;
	_ccs_hyperparameter_wrapper_t *wrapper = (_ccs_hyperparameter_wrapper_t*)
	    utarray_eltptr(configuration_space->data->hyperparameters,
	                   (unsigned int)hyperparameter_index);
	if (!wrapper)
		return -CCS_OUT_OF_BOUNDS;
	if (wrapper->condition)
		return -CCS_INVALID_HYPERPARAMETER;
	ccs_error_t err;
	err = ccs_retain_object(expression);
	if (err)
		return err;
	wrapper->condition = expression;
	// Recompute the whole graph for now
	configuration_space->data->graph_ok = CCS_FALSE;
	err = _generate_constraints(configuration_space);
	if (err) {
		ccs_release_object(expression);
		wrapper->condition = NULL;
		return err;
	}
	return CCS_SUCCESS;
}

ccs_error_t
ccs_configuration_space_get_condition(ccs_configuration_space_t  configuration_space,
                                      size_t                     hyperparameter_index,
                                      ccs_expression_t          *expression_ret) {
	if (!configuration_space || !configuration_space->data)
		return -CCS_INVALID_OBJECT;
	if (!expression_ret)
		return -CCS_INVALID_VALUE;
	_ccs_hyperparameter_wrapper_t *wrapper = (_ccs_hyperparameter_wrapper_t*)
	    utarray_eltptr(configuration_space->data->hyperparameters,
	                   (unsigned int)hyperparameter_index);
	if (!wrapper)
		return -CCS_OUT_OF_BOUNDS;
	*expression_ret = wrapper->condition;
	return CCS_SUCCESS;
}

ccs_error_t
ccs_configuration_space_get_conditions(ccs_configuration_space_t  configuration_space,
                                       size_t                     num_expressions,
                                       ccs_expression_t          *expressions,
                                       size_t                    *num_expressions_ret) {
	if (!configuration_space || !configuration_space->data)
		return -CCS_INVALID_OBJECT;
	if (num_expressions && !expressions)
		return -CCS_INVALID_VALUE;
	if (!expressions && !num_expressions_ret)
		return -CCS_INVALID_VALUE;
	UT_array *array = configuration_space->data->hyperparameters;
	size_t size = utarray_len(array);
	if (expressions) {
		if (num_expressions < size)
			return -CCS_INVALID_VALUE;
		_ccs_hyperparameter_wrapper_t *wrapper = NULL;
		size_t index = 0;
		while ( (wrapper = (_ccs_hyperparameter_wrapper_t *)utarray_next(array, wrapper)) )
			expressions[index++] = wrapper->condition;
		for (size_t i = size; i < num_expressions; i++)
			expressions[i] = NULL;
	}
	if (num_expressions_ret)
		*num_expressions_ret = size;
	return CCS_SUCCESS;
}

#undef  utarray_oom
#define utarray_oom() { \
	return -CCS_ENOMEM; \
}
ccs_error_t
ccs_configuration_space_add_forbidden_clause(ccs_configuration_space_t configuration_space,
                                             ccs_expression_t          expression) {
	if (!configuration_space || !configuration_space->data)
		return -CCS_INVALID_OBJECT;
	ccs_error_t err;
	err = ccs_expression_check_context(expression, configuration_space);
	if (err)
		return err;
	err = ccs_retain_object(expression);
	if (err)
		return err;
	utarray_push_back(configuration_space->data->forbidden_clauses, &expression);
	return CCS_SUCCESS;
}

ccs_error_t
ccs_configuration_space_add_forbidden_clauses(ccs_configuration_space_t  configuration_space,
                                              size_t                     num_expressions,
                                              ccs_expression_t          *expressions) {
	if (!configuration_space || !configuration_space->data)
		return -CCS_INVALID_OBJECT;
	if (num_expressions && !expressions)
		return -CCS_INVALID_VALUE;
	for (size_t i = 0; i < num_expressions; i++) {
		ccs_error_t err;
		err = ccs_expression_check_context(expressions[i], configuration_space);
		if (err)
			return err;
		err = ccs_retain_object(expressions[i]);
		if (err)
			return err;
		utarray_push_back(configuration_space->data->forbidden_clauses, expressions + i);
	}
	return CCS_SUCCESS;
}
#undef  utarray_oom
#define utarray_oom() exit(-1)

ccs_error_t
ccs_configuration_space_get_forbidden_clause(ccs_configuration_space_t  configuration_space,
                                             size_t                     index,
                                             ccs_expression_t          *expression_ret) {
	if (!configuration_space || !configuration_space->data)
		return -CCS_INVALID_OBJECT;
	if (!expression_ret)
		return -CCS_INVALID_VALUE;
	ccs_expression_t *p_expr = (ccs_expression_t*)
	    utarray_eltptr(configuration_space->data->forbidden_clauses,
	                   (unsigned int)index);
	if (!p_expr)
		return -CCS_OUT_OF_BOUNDS;
	*expression_ret = *p_expr;
	return CCS_SUCCESS;
}

ccs_error_t
ccs_configuration_space_get_forbidden_clauses(ccs_configuration_space_t  configuration_space,
                                              size_t                     num_expressions,
                                              ccs_expression_t          *expressions,
                                              size_t                    *num_expressions_ret) {
	if (!configuration_space || !configuration_space->data)
		return -CCS_INVALID_OBJECT;
	if (num_expressions && !expressions)
		return -CCS_INVALID_VALUE;
	if (!expressions && !num_expressions_ret)
		return -CCS_INVALID_VALUE;
	UT_array *array = configuration_space->data->forbidden_clauses;
	size_t size = utarray_len(array);
	if (expressions) {
		if (num_expressions < size)
			return -CCS_INVALID_VALUE;
		ccs_expression_t *p_expr = NULL;
		size_t index = 0;
		while ( (p_expr = (ccs_expression_t *)utarray_next(array, p_expr)) )
			expressions[index++] = *p_expr;
		for (size_t i = size; i < num_expressions; i++)
			expressions[i] = NULL;
	}
	if (num_expressions_ret)
		*num_expressions_ret = size;
	return CCS_SUCCESS;
}
