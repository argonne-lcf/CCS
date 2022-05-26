#include "cconfigspace_internal.h"
#include "configuration_space_internal.h"
#include "configuration_internal.h"
#include "distribution_internal.h"
#include "expression_internal.h"
#include "rng_internal.h"
#include "utlist.h"

static ccs_result_t
_generate_constraints(ccs_configuration_space_t configuration_space);

static ccs_result_t
_ccs_configuration_space_del(ccs_object_t object) {
	ccs_configuration_space_t configuration_space = (ccs_configuration_space_t)object;
	UT_array *array = configuration_space->data->hyperparameters;
	_ccs_hyperparameter_wrapper_cs_t *wrapper = NULL;
	while ( (wrapper = (_ccs_hyperparameter_wrapper_cs_t *)utarray_next(array, wrapper)) ) {
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
	_ccs_hyperparameter_index_hash_t *elem, *tmpelem;
	HASH_ITER(hh_handle, configuration_space->data->handle_hash, elem, tmpelem) {
		HASH_DELETE(hh_handle, configuration_space->data->handle_hash, elem);
		free(elem);
	}
	utarray_free(configuration_space->data->hyperparameters);
	utarray_free(configuration_space->data->forbidden_clauses);
	utarray_free(configuration_space->data->sorted_indexes);
	_ccs_distribution_wrapper_t *dw, *tmp;
	DL_FOREACH_SAFE(configuration_space->data->distribution_list, dw, tmp) {
		DL_DELETE(configuration_space->data->distribution_list, dw);
		ccs_release_object(dw->distribution);
		free(dw);
	}
	ccs_release_object(configuration_space->data->rng);
	return CCS_SUCCESS;
}

static inline ccs_result_t
_ccs_serialize_bin_size_ccs_configuration_space_data(
		_ccs_configuration_space_data_t *data,
		size_t                          *cum_size) {
	size_t condition_count;
	_ccs_hyperparameter_wrapper_cs_t *wrapper;
	size_t distribution_count;
	_ccs_distribution_wrapper_t *dw;
	ccs_expression_t *expr;

	*cum_size += _ccs_serialize_bin_size_string(data->name);
	*cum_size += _ccs_serialize_bin_size_uint64(
		utarray_len(data->hyperparameters));

	condition_count = 0;
	wrapper = NULL;
	while ( (wrapper = (_ccs_hyperparameter_wrapper_cs_t *)utarray_next(data->hyperparameters, wrapper)) )
		if (wrapper->condition)
			condition_count++;
	*cum_size += _ccs_serialize_bin_size_uint64(condition_count);

	DL_COUNT(data->distribution_list, dw, distribution_count);
	*cum_size += _ccs_serialize_bin_size_uint64(distribution_count);

	*cum_size += _ccs_serialize_bin_size_uint64(
		utarray_len(data->forbidden_clauses));

	/* rng */
	CCS_VALIDATE(data->rng->obj.ops->serialize_size(
		data->rng, CCS_SERIALIZE_FORMAT_BINARY, cum_size));

	/* hyperparameters */
	wrapper = NULL;
	while ( (wrapper = (_ccs_hyperparameter_wrapper_cs_t *)utarray_next(data->hyperparameters, wrapper)) )
		CCS_VALIDATE(wrapper->hyperparameter->obj.ops->serialize_size(
			wrapper->hyperparameter, CCS_SERIALIZE_FORMAT_BINARY, cum_size));

	/* conditions */
	condition_count = 0;
	wrapper = NULL;
	while ( (wrapper = (_ccs_hyperparameter_wrapper_cs_t *)utarray_next(data->hyperparameters, wrapper)) ) {
		if (wrapper->condition) {
			/* hyperparam index and condition */
			*cum_size += _ccs_serialize_bin_size_uint64(condition_count);
			CCS_VALIDATE(wrapper->condition->obj.ops->serialize_size(
				wrapper->condition, CCS_SERIALIZE_FORMAT_BINARY, cum_size));
		}
		condition_count++;
	}

	/* distributions */
	dw = NULL;
	DL_FOREACH(data->distribution_list, dw) {
		CCS_VALIDATE(dw->distribution->obj.ops->serialize_size(
			dw->distribution, CCS_SERIALIZE_FORMAT_BINARY, cum_size));
		*cum_size += _ccs_serialize_bin_size_uint64(dw->dimension);
		for (size_t i = 0; i < dw->dimension; i++)
			*cum_size += _ccs_serialize_bin_size_uint64(dw->hyperparameter_indexes[i]);
	}

	/* forbidden clauses */
	expr = NULL;
	while ( (expr = (ccs_expression_t *)utarray_next(data->forbidden_clauses, expr)) )
		CCS_VALIDATE((*expr)->obj.ops->serialize_size(
			*expr, CCS_SERIALIZE_FORMAT_BINARY, cum_size));
	return CCS_SUCCESS;
}

static inline ccs_result_t
_ccs_serialize_bin_ccs_configuration_space_data(
		_ccs_configuration_space_data_t  *data,
		size_t                           *buffer_size,
		char                            **buffer) {
	size_t condition_count;
	_ccs_hyperparameter_wrapper_cs_t *wrapper;
	size_t distribution_count;
	_ccs_distribution_wrapper_t *dw;
	ccs_expression_t *expr;

	CCS_VALIDATE(_ccs_serialize_bin_string(
		data->name, buffer_size, buffer));
	CCS_VALIDATE(_ccs_serialize_bin_uint64(
		utarray_len(data->hyperparameters), buffer_size, buffer));

	condition_count = 0;
	wrapper = NULL;
	while ( (wrapper = (_ccs_hyperparameter_wrapper_cs_t *)utarray_next(data->hyperparameters, wrapper)) )
		if (wrapper->condition)
			condition_count++;
	CCS_VALIDATE(_ccs_serialize_bin_uint64(
		condition_count, buffer_size, buffer));

	DL_COUNT(data->distribution_list, dw, distribution_count);
	CCS_VALIDATE(_ccs_serialize_bin_uint64(
		distribution_count, buffer_size, buffer));

	CCS_VALIDATE(_ccs_serialize_bin_uint64(
		utarray_len(data->forbidden_clauses), buffer_size, buffer));

	/* rng */
	CCS_VALIDATE(data->rng->obj.ops->serialize(
		data->rng, CCS_SERIALIZE_FORMAT_BINARY, buffer_size, buffer));

	/* hyperparameters */
	wrapper = NULL;
	while ( (wrapper = (_ccs_hyperparameter_wrapper_cs_t *)utarray_next(data->hyperparameters, wrapper)) )
		CCS_VALIDATE(wrapper->hyperparameter->obj.ops->serialize(
			wrapper->hyperparameter, CCS_SERIALIZE_FORMAT_BINARY, buffer_size, buffer));

	/* conditions */
	condition_count = 0;
	wrapper = NULL;
	while ( (wrapper = (_ccs_hyperparameter_wrapper_cs_t *)utarray_next(data->hyperparameters, wrapper)) ) {
		if (wrapper->condition) {
			CCS_VALIDATE(_ccs_serialize_bin_uint64(
				condition_count, buffer_size, buffer));
			CCS_VALIDATE(wrapper->condition->obj.ops->serialize(
				wrapper->condition, CCS_SERIALIZE_FORMAT_BINARY, buffer_size, buffer));
		}
		condition_count++;
	}

	/* distributions */
	dw = NULL;
	DL_FOREACH(data->distribution_list, dw) {
		CCS_VALIDATE(dw->distribution->obj.ops->serialize(
			dw->distribution, CCS_SERIALIZE_FORMAT_BINARY, buffer_size, buffer));
		CCS_VALIDATE(_ccs_serialize_bin_uint64(
			dw->dimension, buffer_size, buffer));
		for (size_t i = 0; i < dw->dimension; i++)
			CCS_VALIDATE(_ccs_serialize_bin_uint64(
				dw->hyperparameter_indexes[i], buffer_size, buffer));
	}

	/* forbidden clauses */
	expr = NULL;
	while ( (expr = (ccs_expression_t *)utarray_next(data->forbidden_clauses, expr)) )
		CCS_VALIDATE((*expr)->obj.ops->serialize(
			*expr, CCS_SERIALIZE_FORMAT_BINARY, buffer_size, buffer));
	return CCS_SUCCESS;
}

static inline ccs_result_t
_ccs_serialize_bin_size_ccs_configuration_space(
		ccs_configuration_space_t  configuration_space,
		size_t                    *cum_size) {
	_ccs_configuration_space_data_t *data =
		(_ccs_configuration_space_data_t *)(configuration_space->data);
	*cum_size += _ccs_serialize_bin_size_ccs_object_internal(
		(_ccs_object_internal_t *)configuration_space);
	CCS_VALIDATE(_ccs_serialize_bin_size_ccs_configuration_space_data(
		data, cum_size));
	return CCS_SUCCESS;
}

static inline ccs_result_t
_ccs_serialize_bin_ccs_configuration_space(
		ccs_configuration_space_t   configuration_space,
		size_t                     *buffer_size,
		char                      **buffer) {
	_ccs_configuration_space_data_t *data =
		(_ccs_configuration_space_data_t *)(configuration_space->data);
	CCS_VALIDATE(_ccs_serialize_bin_ccs_object_internal(
		(_ccs_object_internal_t *)configuration_space, buffer_size, buffer));
	CCS_VALIDATE(_ccs_serialize_bin_ccs_configuration_space_data(
		data, buffer_size, buffer));
	return CCS_SUCCESS;
}

static ccs_result_t
_ccs_configuration_space_serialize_size(
		ccs_object_t            object,
		ccs_serialize_format_t  format,
		size_t                 *cum_size) {
	switch(format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE(_ccs_serialize_bin_size_ccs_configuration_space(
			(ccs_configuration_space_t)object, cum_size));
		break;
	default:
		return -CCS_INVALID_VALUE;
	}
	return CCS_SUCCESS;
}

static ccs_result_t
_ccs_configuration_space_serialize(
		ccs_object_t             object,
		ccs_serialize_format_t   format,
		size_t                  *buffer_size,
		char                   **buffer) {
	switch(format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE(_ccs_serialize_bin_ccs_configuration_space(
		    (ccs_configuration_space_t)object, buffer_size, buffer));
		break;
	default:
		return -CCS_INVALID_VALUE;
	}
	return CCS_SUCCESS;
}

static _ccs_configuration_space_ops_t _configuration_space_ops =
    { { {&_ccs_configuration_space_del,
         &_ccs_configuration_space_serialize_size,
         &_ccs_configuration_space_serialize} } };

static const UT_icd _hyperparameter_wrapper_icd = {
	sizeof(_ccs_hyperparameter_wrapper_cs_t),
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
	err = -CCS_OUT_OF_MEMORY; \
	goto errarrays; \
}
ccs_result_t
ccs_create_configuration_space(const char                *name,
                               void                      *user_data,
                               ccs_configuration_space_t *configuration_space_ret) {
	CCS_CHECK_PTR(name);
	CCS_CHECK_PTR(configuration_space_ret);
	ccs_result_t err;
	uintptr_t mem = (uintptr_t)calloc(1,
	  sizeof(struct _ccs_configuration_space_s) +
	  sizeof(struct _ccs_configuration_space_data_s) +
	  strlen(name) + 1);
	if (!mem)
		return -CCS_OUT_OF_MEMORY;
	ccs_rng_t rng;
	CCS_VALIDATE_ERR_GOTO(err, ccs_create_rng(&rng), errmem);

	ccs_configuration_space_t config_space;
	config_space = (ccs_configuration_space_t)mem;
	_ccs_object_init(&(config_space->obj),
	                 CCS_CONFIGURATION_SPACE, user_data,
	                 (_ccs_object_ops_t *)&_configuration_space_ops);
	config_space->data =
	  (struct _ccs_configuration_space_data_s*)(
	    mem + sizeof(struct _ccs_configuration_space_s));
	config_space->data->name =
	  (const char *)(mem + sizeof(struct _ccs_configuration_space_s) +
	                 sizeof(struct _ccs_configuration_space_data_s));
	config_space->data->rng = rng;
	utarray_new(config_space->data->hyperparameters,
	            &_hyperparameter_wrapper_icd);
	utarray_new(config_space->data->forbidden_clauses,
	            &_forbidden_clauses_icd);
	utarray_new(config_space->data->sorted_indexes, &_size_t_icd);
	config_space->data->graph_ok = CCS_TRUE;
	strcpy((char *)(config_space->data->name), name);
	*configuration_space_ret = config_space;
	return CCS_SUCCESS;
errarrays:
	if (config_space->data->hyperparameters)
		utarray_free(config_space->data->hyperparameters);
	if (config_space->data->forbidden_clauses)
		utarray_free(config_space->data->forbidden_clauses);
	if (config_space->data->sorted_indexes)
		utarray_free(config_space->data->sorted_indexes);
errmem:
	free((void *)mem);
	return err;
}

ccs_result_t
ccs_configuration_space_get_name(ccs_configuration_space_t   configuration_space,
                                 const char                **name_ret) {
	CCS_CHECK_OBJ(configuration_space, CCS_CONFIGURATION_SPACE);
	return _ccs_context_get_name((ccs_context_t)configuration_space, name_ret);
}

ccs_result_t
ccs_configuration_space_set_rng(ccs_configuration_space_t configuration_space,
                                ccs_rng_t                 rng) {
	CCS_CHECK_OBJ(configuration_space, CCS_CONFIGURATION_SPACE);
	CCS_CHECK_OBJ(rng, CCS_RNG);
	CCS_VALIDATE(ccs_retain_object(rng));
	ccs_rng_t tmp = configuration_space->data->rng;
	configuration_space->data->rng = rng;
	CCS_VALIDATE(ccs_release_object(tmp));
	return CCS_SUCCESS;
}

ccs_result_t
ccs_configuration_space_get_rng(ccs_configuration_space_t  configuration_space,
                                ccs_rng_t                 *rng_ret) {
	CCS_CHECK_OBJ(configuration_space, CCS_CONFIGURATION_SPACE);
	CCS_CHECK_PTR(rng_ret);
	*rng_ret = configuration_space->data->rng;
	return CCS_SUCCESS;
}


#undef  utarray_oom
#define utarray_oom() { \
	err = -CCS_OUT_OF_MEMORY; \
	goto errordistrib_wrapper; \
}
#undef uthash_nonfatal_oom
#define uthash_nonfatal_oom(elt) { \
	err = -CCS_OUT_OF_MEMORY; \
	goto errorutarray; \
}
ccs_result_t
ccs_configuration_space_add_hyperparameter(ccs_configuration_space_t configuration_space,
                                           ccs_hyperparameter_t      hyperparameter,
                                           ccs_distribution_t        distribution) {
	CCS_CHECK_OBJ(configuration_space, CCS_CONFIGURATION_SPACE);
	CCS_CHECK_OBJ(hyperparameter, CCS_HYPERPARAMETER);
	ccs_result_t err;
	ccs_hyperparameter_type_t type;

	CCS_VALIDATE(ccs_hyperparameter_get_type(hyperparameter, &type));
	if (CCS_HYPERPARAMETER_TYPE_STRING == type)
		return -CCS_INVALID_HYPERPARAMETER;

	const char *name;
	size_t sz_name;
	_ccs_hyperparameter_index_hash_t *hyper_hash;
	CCS_VALIDATE(ccs_hyperparameter_get_name(hyperparameter, &name));
	sz_name = strlen(name);
	HASH_FIND(hh_name, configuration_space->data->name_hash,
	          name, sz_name, hyper_hash);
	if (hyper_hash)
		return -CCS_INVALID_HYPERPARAMETER;
	UT_array *hyperparameters;
	size_t index;
	size_t dimension;
	_ccs_hyperparameter_wrapper_cs_t hyper_wrapper;
	_ccs_distribution_wrapper_t *distrib_wrapper;
	uintptr_t pmem;
	hyper_wrapper.hyperparameter = hyperparameter;
	CCS_VALIDATE(ccs_retain_object(hyperparameter));

	if (distribution) {
		CCS_VALIDATE_ERR_GOTO(err,
		  ccs_distribution_get_dimension(distribution, &dimension),
		  errorhyper);
		if (dimension != 1) {
			err = -CCS_INVALID_DISTRIBUTION;
			goto errorhyper;
		}
		CCS_VALIDATE_ERR_GOTO(err, ccs_retain_object(distribution), errorhyper);
	} else {
		CCS_VALIDATE_ERR_GOTO(err,
		  ccs_hyperparameter_get_default_distribution(hyperparameter, &distribution),
		  errorhyper);
		dimension = 1;
	}
	pmem = (uintptr_t)malloc(sizeof(_ccs_distribution_wrapper_t) + sizeof(size_t)*dimension);
	if (!pmem) {
		err = -CCS_OUT_OF_MEMORY;
		goto errordistrib;
	}
        distrib_wrapper = (_ccs_distribution_wrapper_t *)pmem;
	distrib_wrapper->distribution = distribution;
	distrib_wrapper->dimension = dimension;
	distrib_wrapper->hyperparameter_indexes = (size_t *)(pmem + sizeof(_ccs_distribution_wrapper_t));

	hyper_hash = (_ccs_hyperparameter_index_hash_t *)malloc(sizeof(_ccs_hyperparameter_index_hash_t));
	if (!hyper_hash) {
		err = -CCS_OUT_OF_MEMORY;
		goto errordistrib_wrapper;
	}

	hyperparameters = configuration_space->data->hyperparameters;
	index = utarray_len(hyperparameters);
	hyper_hash->hyperparameter = hyperparameter;
	hyper_hash->name = name;
	hyper_hash->index = index;
	distrib_wrapper->hyperparameter_indexes[0] = index;
	hyper_wrapper.distribution_index = 0;
	hyper_wrapper.distribution = distrib_wrapper;
	hyper_wrapper.condition = NULL;
	hyper_wrapper.parents = NULL;
	hyper_wrapper.children = NULL;
	utarray_new(hyper_wrapper.parents, &_size_t_icd);
	utarray_new(hyper_wrapper.children, &_size_t_icd);

	utarray_push_back(hyperparameters, &hyper_wrapper);
	utarray_push_back(configuration_space->data->sorted_indexes, &index);

	HASH_ADD_KEYPTR( hh_name, configuration_space->data->name_hash,
	                 hyper_hash->name, sz_name, hyper_hash );
	HASH_ADD( hh_handle, configuration_space->data->handle_hash,
	          hyperparameter, sizeof(ccs_hyperparameter_t), hyper_hash );
	DL_APPEND( configuration_space->data->distribution_list, distrib_wrapper );

	return CCS_SUCCESS;
errorutarray:
	utarray_pop_back(hyperparameters);
errordistrib_wrapper:
	if (hyper_hash)
		free(hyper_hash);
	if (hyper_wrapper.parents)
		utarray_free(hyper_wrapper.parents);
	if (hyper_wrapper.children)
		utarray_free(hyper_wrapper.children);
	free(distrib_wrapper);
errordistrib:
	ccs_release_object(distribution);
errorhyper:
	ccs_release_object(hyperparameter);
	return err;
}
#undef  utarray_oom
#define utarray_oom() exit(-1)

ccs_result_t
ccs_configuration_space_set_distribution(ccs_configuration_space_t  configuration_space,
                                         ccs_distribution_t         distribution,
                                         size_t                    *indexes) {
	CCS_CHECK_OBJ(configuration_space, CCS_CONFIGURATION_SPACE);
	CCS_CHECK_OBJ(distribution, CCS_DISTRIBUTION);
	CCS_CHECK_PTR(indexes);

	_ccs_distribution_wrapper_t *dwrapper;
	_ccs_hyperparameter_wrapper_cs_t *hwrapper;
	ccs_result_t err;
	UT_array *hyperparameters = configuration_space->data->hyperparameters;
	size_t num_hyperparameters = utarray_len(hyperparameters);
	size_t dim;
	CCS_VALIDATE(ccs_distribution_get_dimension(distribution, &dim));

	for (size_t i = 0; i < dim; i++) {
		if (indexes[i] >= num_hyperparameters)
			return -CCS_INVALID_VALUE;
		// Check duplicate entries
		for (size_t j = 0;  j < i; j++)
			if (indexes[i] == indexes[j])
				return -CCS_INVALID_VALUE;
		for (size_t j = i + 1;  j < dim; j++)
			if (indexes[i] == indexes[j])
				return -CCS_INVALID_VALUE;
	}

	uintptr_t cur_mem;
	uintptr_t mem = (uintptr_t)malloc(sizeof(void *)*num_hyperparameters*2 +
		sizeof(size_t)*num_hyperparameters);
	if (!mem)
		return -CCS_OUT_OF_MEMORY;
	cur_mem = mem;
	_ccs_distribution_wrapper_t **p_dwrappers_to_del =
		 (_ccs_distribution_wrapper_t **)cur_mem;
	cur_mem += sizeof(_ccs_distribution_wrapper_t *)*num_hyperparameters;
	_ccs_distribution_wrapper_t **p_dwrappers_to_add =
		 (_ccs_distribution_wrapper_t **)cur_mem;
	cur_mem += sizeof(_ccs_distribution_wrapper_t *)*num_hyperparameters;
	size_t *hypers_without_distrib = (size_t *)cur_mem;
	cur_mem += sizeof(size_t)*num_hyperparameters;

	size_t to_add_count = 0;
	size_t to_del_count = 0;
	size_t without_distrib_count = 0;

	for (size_t i = 0; i < dim; i++) {
		int add = 1;
		hwrapper = (_ccs_hyperparameter_wrapper_cs_t *)utarray_eltptr(hyperparameters, indexes[i]);
		for (size_t j = 0; j < to_del_count; j++)
			if (p_dwrappers_to_del[j] == hwrapper->distribution) {
				add = 0;
				break;
			}
		if (add)
			p_dwrappers_to_del[to_del_count++] = hwrapper->distribution;
	}
	for (size_t i = 0; i < to_del_count; i++) {
		for (size_t j = 0; j < p_dwrappers_to_del[i]->dimension; j++) {
			hypers_without_distrib[without_distrib_count++] = p_dwrappers_to_del[i]->hyperparameter_indexes[j];
		}
	}

	uintptr_t dmem = (uintptr_t)malloc(sizeof(_ccs_distribution_wrapper_t) + sizeof(size_t)*dim);
	if (!dmem) {
		err = -CCS_OUT_OF_MEMORY;
		goto memory;
	}

	dwrapper = (_ccs_distribution_wrapper_t *)dmem;
	dwrapper->distribution = distribution;
	dwrapper->dimension = dim;
	dwrapper->hyperparameter_indexes = (size_t *)(dmem + sizeof(_ccs_distribution_wrapper_t));
	for (size_t i = 0; i < dim; i++) {
		dwrapper->hyperparameter_indexes[i] = indexes[i];
		size_t indx = 0;
		for (size_t j = 0; j < without_distrib_count; j++, indx++)
			if (hypers_without_distrib[j] == indexes[i])
				break;
		for (size_t j = indx + 1; j < without_distrib_count; j++)
			hypers_without_distrib[j-1] = hypers_without_distrib[j];
		without_distrib_count--;
	}
	CCS_VALIDATE_ERR_GOTO(err, ccs_retain_object(distribution), errdmem);

	p_dwrappers_to_add[0] = dwrapper;
	to_add_count = 1;
	for (size_t i = 0; i < without_distrib_count; i++) {
		dmem = (uintptr_t)malloc(sizeof(_ccs_distribution_wrapper_t) + sizeof(size_t));
		if (!dmem) {
			err = -CCS_OUT_OF_MEMORY;
			goto memory;
		}
		dwrapper = (_ccs_distribution_wrapper_t *)dmem;
		dwrapper->hyperparameter_indexes = (size_t *)(dmem + sizeof(_ccs_distribution_wrapper_t));
		dwrapper->dimension = 1;
		dwrapper->hyperparameter_indexes[0] = hypers_without_distrib[i];
		hwrapper = (_ccs_hyperparameter_wrapper_cs_t *)utarray_eltptr(hyperparameters, hypers_without_distrib[i]);
		CCS_VALIDATE_ERR_GOTO(err, ccs_hyperparameter_get_default_distribution(
			hwrapper->hyperparameter, &(dwrapper->distribution)), dwrappers);
		p_dwrappers_to_add[to_add_count++] = dwrapper;
	}

	for (size_t i = 0; i < to_del_count; i++) {
		DL_DELETE(configuration_space->data->distribution_list, p_dwrappers_to_del[i]);
		ccs_release_object(p_dwrappers_to_del[i]->distribution);
		free(p_dwrappers_to_del[i]);
	}
	for (size_t i = 0; i < to_add_count; i++) {
		DL_APPEND( configuration_space->data->distribution_list, p_dwrappers_to_add[i]);
		for (size_t j = 0; j < p_dwrappers_to_add[i]->dimension; j++) {
			hwrapper = (_ccs_hyperparameter_wrapper_cs_t *)utarray_eltptr(hyperparameters, p_dwrappers_to_add[i]->hyperparameter_indexes[j]);
			hwrapper->distribution_index = j;
			hwrapper->distribution = p_dwrappers_to_add[i];
		}
	}

	free((void *)mem);
	return CCS_SUCCESS;
dwrappers:
	for (size_t i = 0; i < to_add_count; i++) {
		ccs_release_object(p_dwrappers_to_add[i]->distribution);
		free(p_dwrappers_to_add[i]);
	}
errdmem:
	if (dmem)
		free((void *)dmem);
memory:
	free((void *)mem);
	return err;
}

extern ccs_result_t
ccs_configuration_space_get_hyperparameter_distribution(
		ccs_configuration_space_t  configuration_space,
		size_t                     index,
		ccs_distribution_t        *distribution_ret,
		size_t                    *index_ret) {
	CCS_CHECK_OBJ(configuration_space, CCS_CONFIGURATION_SPACE);
	CCS_CHECK_PTR(distribution_ret);
	CCS_CHECK_PTR(index_ret);

	_ccs_hyperparameter_wrapper_cs_t *wrapper = (_ccs_hyperparameter_wrapper_cs_t*)
		utarray_eltptr(configuration_space->data->hyperparameters, (unsigned int)index);
	if (!wrapper)
		return -CCS_OUT_OF_BOUNDS;
	*distribution_ret = wrapper->distribution->distribution;
	*index_ret = wrapper->distribution_index;
	return CCS_SUCCESS;
}

ccs_result_t
ccs_configuration_space_add_hyperparameters(ccs_configuration_space_t  configuration_space,
                                            size_t                     num_hyperparameters,
                                            ccs_hyperparameter_t      *hyperparameters,
                                            ccs_distribution_t        *distributions) {
	CCS_CHECK_OBJ(configuration_space, CCS_CONFIGURATION_SPACE);
	CCS_CHECK_ARY(num_hyperparameters, hyperparameters);
	for (size_t i = 0; i < num_hyperparameters; i++) {
		ccs_distribution_t distribution = NULL;
		if (distributions)
			distribution = distributions[i];
		CCS_VALIDATE(ccs_configuration_space_add_hyperparameter(
		    configuration_space, hyperparameters[i], distribution));
	}
	return CCS_SUCCESS;
}

ccs_result_t
ccs_configuration_space_get_num_hyperparameters(ccs_configuration_space_t  configuration_space,
                                                size_t                     *num_hyperparameters_ret) {
	CCS_CHECK_OBJ(configuration_space, CCS_CONFIGURATION_SPACE);
	return _ccs_context_get_num_hyperparameters(
		(ccs_context_t)configuration_space, num_hyperparameters_ret);
}

ccs_result_t
ccs_configuration_space_get_hyperparameter(ccs_configuration_space_t  configuration_space,
                                           size_t                     index,
                                           ccs_hyperparameter_t      *hyperparameter_ret) {
	CCS_CHECK_OBJ(configuration_space, CCS_CONFIGURATION_SPACE);
	return _ccs_context_get_hyperparameter(
		(ccs_context_t)configuration_space, index, hyperparameter_ret);
}

ccs_result_t
ccs_configuration_space_get_hyperparameter_by_name(
		ccs_configuration_space_t  configuration_space,
		const char *               name,
		ccs_hyperparameter_t      *hyperparameter_ret) {
	CCS_CHECK_OBJ(configuration_space, CCS_CONFIGURATION_SPACE);
	return _ccs_context_get_hyperparameter_by_name(
		(ccs_context_t)configuration_space, name, hyperparameter_ret);
}

ccs_result_t
ccs_configuration_space_get_hyperparameter_index_by_name(
		ccs_configuration_space_t  configuration_space,
		const char                *name,
		size_t                    *index_ret) {
	CCS_CHECK_OBJ(configuration_space, CCS_CONFIGURATION_SPACE);
	return _ccs_context_get_hyperparameter_index_by_name(
		(ccs_context_t)configuration_space, name, index_ret);
}

ccs_result_t
ccs_configuration_space_get_hyperparameter_index(
		ccs_configuration_space_t  configuration_space,
		ccs_hyperparameter_t       hyperparameter,
		size_t                    *index_ret) {
	CCS_CHECK_OBJ(configuration_space, CCS_CONFIGURATION_SPACE);
	return _ccs_context_get_hyperparameter_index(
		(ccs_context_t)(configuration_space),
		hyperparameter, index_ret);
}

ccs_result_t
ccs_configuration_space_get_hyperparameter_indexes(
		ccs_configuration_space_t  configuration_space,
		size_t                     num_hyperparameters,
		ccs_hyperparameter_t      *hyperparameters,
		size_t                    *indexes) {
	CCS_CHECK_OBJ(configuration_space, CCS_CONFIGURATION_SPACE);
	return _ccs_context_get_hyperparameter_indexes(
		(ccs_context_t)configuration_space, num_hyperparameters,
		 hyperparameters, indexes);
}

ccs_result_t
ccs_configuration_space_get_hyperparameters(
		ccs_configuration_space_t  configuration_space,
		size_t                     num_hyperparameters,
		ccs_hyperparameter_t      *hyperparameters,
		size_t                    *num_hyperparameters_ret) {
	CCS_CHECK_OBJ(configuration_space, CCS_CONFIGURATION_SPACE);
	return _ccs_context_get_hyperparameters(
		(ccs_context_t)configuration_space, num_hyperparameters,
		hyperparameters, num_hyperparameters_ret);
}

ccs_result_t
ccs_configuration_space_validate_value(ccs_configuration_space_t  configuration_space,
                                       size_t                     index,
                                       ccs_datum_t                value,
                                       ccs_datum_t               *value_ret) {
	CCS_CHECK_OBJ(configuration_space, CCS_CONFIGURATION_SPACE);
	return _ccs_context_validate_value((ccs_context_t)configuration_space,
	                                   index, value, value_ret);
}

static ccs_result_t
_set_actives(ccs_configuration_space_t configuration_space,
             ccs_configuration_t       configuration) {
	size_t *p_index = NULL;
	UT_array *indexes = configuration_space->data->sorted_indexes;
	UT_array *array = configuration_space->data->hyperparameters;
	ccs_datum_t *values = configuration->data->values;
	while ( (p_index = (size_t *)utarray_next(indexes, p_index)) ) {
		_ccs_hyperparameter_wrapper_cs_t *wrapper = NULL;
		wrapper = (_ccs_hyperparameter_wrapper_cs_t *)utarray_eltptr(array, *p_index);
		if (!wrapper->condition)
			continue;
		ccs_datum_t result;
		ccs_result_t err;
		err = ccs_expression_eval(wrapper->condition,
		                          (ccs_context_t)configuration_space,
		                          values, &result);
		if (err) {
			if (err != -CCS_INACTIVE_HYPERPARAMETER)
				return err;
                        values[*p_index] = ccs_inactive;
		} else if (!(result.type == CCS_BOOLEAN && result.value.i == CCS_TRUE))
			values[*p_index] = ccs_inactive;
	}
	return CCS_SUCCESS;
}

ccs_result_t
ccs_configuration_space_get_default_configuration(ccs_configuration_space_t  configuration_space,
                                                  ccs_configuration_t       *configuration_ret) {
	CCS_CHECK_OBJ(configuration_space, CCS_CONFIGURATION_SPACE);
	CCS_CHECK_PTR(configuration_ret);
	ccs_result_t err;
	ccs_configuration_t config;
	CCS_VALIDATE(ccs_create_configuration(configuration_space, 0, NULL, NULL, &config));
	UT_array *array = configuration_space->data->hyperparameters;
	_ccs_hyperparameter_wrapper_cs_t *wrapper = NULL;
	ccs_datum_t *values = config->data->values;
	while ( (wrapper = (_ccs_hyperparameter_wrapper_cs_t *)utarray_next(array, wrapper)) )
		CCS_VALIDATE_ERR_GOTO(err, ccs_hyperparameter_get_default_value(
		    wrapper->hyperparameter, values++), errc);
	CCS_VALIDATE_ERR_GOTO(err, _set_actives(configuration_space, config), errc);
	*configuration_ret = config;
	return CCS_SUCCESS;
errc:
	ccs_release_object(config);
	return err;
}

static ccs_result_t
_test_forbidden(ccs_configuration_space_t  configuration_space,
                ccs_datum_t               *values,
		ccs_bool_t                *is_valid) {
	ccs_result_t err;
	UT_array *array = configuration_space->data->forbidden_clauses;
	ccs_expression_t *p_expression = NULL;
	*is_valid = CCS_FALSE;
	while ( (p_expression = (ccs_expression_t *)
	               utarray_next(array, p_expression)) ) {
		ccs_datum_t result;
		err = ccs_expression_eval(*p_expression,
		                          (ccs_context_t)configuration_space,
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

static inline ccs_result_t
_check_configuration(ccs_configuration_space_t  configuration_space,
                     size_t                     num_values,
                     ccs_datum_t               *values) {
	UT_array *indexes = configuration_space->data->sorted_indexes;
	UT_array *array = configuration_space->data->hyperparameters;
	if (num_values != utarray_len(array))
		return -CCS_INVALID_CONFIGURATION;
	size_t *p_index = NULL;
	while ( (p_index = (size_t *)utarray_next(indexes, p_index)) ) {
		ccs_bool_t active = CCS_TRUE;
		_ccs_hyperparameter_wrapper_cs_t *wrapper = NULL;
		wrapper = (_ccs_hyperparameter_wrapper_cs_t *)utarray_eltptr(array, *p_index);
		if (wrapper->condition) {
			ccs_datum_t result;
			ccs_result_t err;
			err = ccs_expression_eval(
				wrapper->condition,
				(ccs_context_t)configuration_space,
				values, &result);
			if (err) {
				if (err != -CCS_INACTIVE_HYPERPARAMETER)
					return err;
                                active = CCS_FALSE;
			} else if (!(result.type == CCS_BOOLEAN && result.value.i == CCS_TRUE)) {
				active = CCS_FALSE;
			}
		}
		if (active != (values[*p_index].type == CCS_INACTIVE ? CCS_FALSE : CCS_TRUE))
			return -CCS_INVALID_CONFIGURATION;
		if (active) {
			ccs_bool_t res;
			CCS_VALIDATE(ccs_hyperparameter_check_value(
			    wrapper->hyperparameter, values[*p_index], &res));
			if (res == CCS_FALSE)
				return -CCS_INVALID_CONFIGURATION;
		}
	}
	ccs_bool_t valid;
	CCS_VALIDATE(_test_forbidden(configuration_space, values, &valid));
	if (!valid)
		return -CCS_INVALID_CONFIGURATION;
	return CCS_SUCCESS;
}

ccs_result_t
ccs_configuration_space_check_configuration(ccs_configuration_space_t configuration_space,
                                            ccs_configuration_t       configuration) {
	CCS_CHECK_OBJ(configuration_space, CCS_CONFIGURATION_SPACE);
	CCS_CHECK_OBJ(configuration, CCS_CONFIGURATION);
	if (configuration->data->configuration_space != configuration_space)
		return -CCS_INVALID_CONFIGURATION;
	if (!configuration_space->data->graph_ok)
		CCS_VALIDATE(_generate_constraints(configuration_space));
	return _check_configuration(configuration_space,
	                            configuration->data->num_values,
	                            configuration->data->values);
}

ccs_result_t
ccs_configuration_space_check_configuration_values(ccs_configuration_space_t  configuration_space,
                                                   size_t                     num_values,
                                                   ccs_datum_t               *values) {
	CCS_CHECK_OBJ(configuration_space, CCS_CONFIGURATION_SPACE);
	CCS_CHECK_ARY(num_values, values);
	if (!configuration_space->data->graph_ok)
		CCS_VALIDATE(_generate_constraints(configuration_space));
	return _check_configuration(configuration_space, num_values, values);
}


static ccs_result_t
_sample(ccs_configuration_space_t  configuration_space,
        ccs_configuration_t        config,
        ccs_bool_t                *found) {
	ccs_result_t err;
	ccs_rng_t rng = configuration_space->data->rng;
	UT_array *array = configuration_space->data->hyperparameters;
	_ccs_distribution_wrapper_t *dwrapper = NULL;
	_ccs_hyperparameter_wrapper_cs_t *hwrapper = NULL;
	ccs_datum_t *values = config->data->values;

	size_t num_hyperparameters = utarray_len(array);
	ccs_datum_t *p_values;
	ccs_hyperparameter_t *hps;
	uintptr_t mem;
	mem = (uintptr_t) malloc(num_hyperparameters * (sizeof(ccs_datum_t) + sizeof(ccs_hyperparameter_t)));
	if (!mem)
		return -CCS_OUT_OF_MEMORY;

	p_values = (ccs_datum_t *)mem;
	hps = (ccs_hyperparameter_t *)(mem + num_hyperparameters*sizeof(ccs_datum_t));
	DL_FOREACH(configuration_space->data->distribution_list, dwrapper) {
		for (size_t i = 0; i < dwrapper->dimension; i++) {
			size_t hindex = dwrapper->hyperparameter_indexes[i];
			hwrapper = (_ccs_hyperparameter_wrapper_cs_t *)utarray_eltptr(array, hindex);
			hps[i] = hwrapper->hyperparameter;
		}
		CCS_VALIDATE_ERR_GOTO(err, ccs_distribution_hyperparameters_sample(
			dwrapper->distribution, rng, hps, p_values), memory);
		for (size_t i = 0; i < dwrapper->dimension; i++) {
			size_t hindex = dwrapper->hyperparameter_indexes[i];
			values[hindex] = p_values[i];
		}
	}
	CCS_VALIDATE_ERR_GOTO(err, _set_actives(configuration_space, config), memory);
	CCS_VALIDATE_ERR_GOTO(err, _test_forbidden(configuration_space, config->data->values, found), memory);
	free((void *)mem);
	return CCS_SUCCESS;
memory:
	free((void *)mem);
	return err;
}

//static ccs_result_t
//_sample(ccs_configuration_space_t  configuration_space,
//        ccs_configuration_t        config,
//        ccs_bool_t                *found) {
//	ccs_rng_t rng = configuration_space->data->rng;
//	UT_array *array = configuration_space->data->hyperparameters;
//	_ccs_hyperparameter_wrapper_cs_t *wrapper = NULL;
//	ccs_datum_t *values = config->data->values;
//	while ( (wrapper = (_ccs_hyperparameter_wrapper_cs_t *)
//	                       utarray_next(array, wrapper)) ) {
//		CCS_VALIDATE(ccs_hyperparameter_sample(wrapper->hyperparameter,
//		    wrapper->distribution->distribution, rng, values++));
//	}
//	CCS_VALIDATE(_set_actives(configuration_space, config));
//	CCS_VALIDATE(_test_forbidden(configuration_space, config->data->values, found));
//	return CCS_SUCCESS;
//}

ccs_result_t
ccs_configuration_space_sample(ccs_configuration_space_t  configuration_space,
                               ccs_configuration_t       *configuration_ret) {
	CCS_CHECK_OBJ(configuration_space, CCS_CONFIGURATION_SPACE);
	CCS_CHECK_PTR(configuration_ret);
	ccs_result_t err;
	ccs_configuration_t config;
	if (!configuration_space->data->graph_ok)
		CCS_VALIDATE(_generate_constraints(configuration_space));
	CCS_VALIDATE(ccs_create_configuration(configuration_space, 0, NULL, NULL, &config));
	ccs_bool_t found;
	int counter = 0;
	do {
		CCS_VALIDATE_ERR_GOTO(err, _sample(configuration_space, config, &found), errc);
		counter++;
	} while (!found && counter < 100);
	if (!found) {
		err = -CCS_SAMPLING_UNSUCCESSFUL;
		goto errc;
	}
	*configuration_ret = config;
	return CCS_SUCCESS;
errc:
	ccs_release_object(config);
	return err;
}

ccs_result_t
ccs_configuration_space_samples(ccs_configuration_space_t  configuration_space,
                                size_t                     num_configurations,
                                ccs_configuration_t       *configurations) {
	CCS_CHECK_OBJ(configuration_space, CCS_CONFIGURATION_SPACE);
	CCS_CHECK_ARY(num_configurations, configurations);
	if (!num_configurations)
		return CCS_SUCCESS;
	ccs_result_t err;
	if (!configuration_space->data->graph_ok)
		CCS_VALIDATE(_generate_constraints(configuration_space));
	size_t     counter = 0;
	size_t     count = 0;
	ccs_bool_t found;
	ccs_configuration_t config = NULL;
	// Naive implementation
	//See below for more efficient ideas...
	for (size_t i = 0; i < num_configurations; i++)
		configurations[i] = NULL;
	while (count < num_configurations && counter < 100 * num_configurations) {
		if (!config)
			CCS_VALIDATE(ccs_create_configuration(configuration_space, 0, NULL, NULL, &config));
		CCS_VALIDATE_ERR_GOTO(err, _sample(configuration_space, config, &found), errc);
		counter++;
		if (found) {
			configurations[count++] = config;
			config = NULL;
		}
	}
	if (count < num_configurations)
		return -CCS_SAMPLING_UNSUCCESSFUL;
	return CCS_SUCCESS;
errc:
	ccs_release_object(config);
	return err;
}
//	UT_array *array = configuration_space->data->hyperparameters;
//	size_t num_hyper = utarray_len(array);
//	ccs_datum_t *values = (ccs_datum_t *)calloc(1, sizeof(ccs_datum_t)*num_configurations*num_hyper);
//	ccs_datum_t *p_values = values;
//	ccs_rng_t rng = configuration_space->data->rng;
//	_ccs_hyperparameter_wrapper_cs_t *wrapper = NULL;
//	while ( (wrapper = (_ccs_hyperparameter_wrapper_cs_t *)utarray_next(array, wrapper)) ) {
//		err = ccs_hyperparameter_samples(wrapper->hyperparameter,
//		                                 wrapper->distribution->distribution,
//						 rng, num_configurations, p_values);
//		if (CCS_UNLIKELY(err)) {
//			free(values);
//			return err;
//		}
//		p_values += num_configurations;
//	}
//	size_t i;
//	for(i = 0; i < num_configurations; i++) {
//		err = ccs_create_configuration(configuration_space, 0, NULL, NULL, configurations + i);
//		if (CCS_UNLIKELY(err)) {
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
	return -CCS_OUT_OF_MEMORY; \
}
static ccs_result_t
_topological_sort(ccs_configuration_space_t configuration_space) {
	utarray_clear(configuration_space->data->sorted_indexes);
	UT_array *array = configuration_space->data->hyperparameters;
	size_t count = utarray_len(array);

	struct _hyper_list_s *list = (struct _hyper_list_s *)calloc(1,
		sizeof(struct _hyper_list_s) * count);
	if (!list)
		return -CCS_OUT_OF_MEMORY;
	struct _hyper_list_s *queue = NULL;

	_ccs_hyperparameter_wrapper_cs_t *wrapper = NULL;
	size_t index = 0;
	while ( (wrapper = (_ccs_hyperparameter_wrapper_cs_t *)utarray_next(array, wrapper)) ) {
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
		wrapper = (_ccs_hyperparameter_wrapper_cs_t *)
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
	err = -CCS_OUT_OF_MEMORY; \
	goto errmem; \
}
static ccs_result_t
_recompute_graph(ccs_configuration_space_t configuration_space) {
	_ccs_hyperparameter_wrapper_cs_t *wrapper = NULL;
	UT_array *array = configuration_space->data->hyperparameters;
	while ( (wrapper = (_ccs_hyperparameter_wrapper_cs_t *)utarray_next(array, wrapper)) ) {
		utarray_clear(wrapper->parents);
		utarray_clear(wrapper->children);
	}
	wrapper = NULL;
	intptr_t mem = 0;
	ccs_result_t err;
	for (size_t index = 0; index < utarray_len(array); index++) {
		wrapper = (_ccs_hyperparameter_wrapper_cs_t *)utarray_eltptr(array, (unsigned int)index);
		if (!wrapper->condition)
			continue;
		size_t count;
		CCS_VALIDATE_ERR_GOTO(err, ccs_expression_get_hyperparameters(wrapper->condition, 0, NULL, &count), errmem);
		if (count == 0)
			continue;
		ccs_hyperparameter_t *parents = NULL;
		size_t               *parents_index = NULL;
		_ccs_hyperparameter_wrapper_cs_t *parent_wrapper = NULL;
		intptr_t oldmem = mem;
		mem = (intptr_t)realloc((void *)oldmem, count *
			(sizeof(ccs_hyperparameter_t) + sizeof(size_t)));
		if (!mem) {
			mem = oldmem;
			err = -CCS_OUT_OF_MEMORY;
			goto errmem;
		}
		parents = (ccs_hyperparameter_t *)mem;
		parents_index = (size_t *)(mem + count * sizeof(ccs_hyperparameter_t));
		CCS_VALIDATE_ERR_GOTO(err, ccs_expression_get_hyperparameters(
		    wrapper->condition, count, parents, NULL), errmem);
		CCS_VALIDATE_ERR_GOTO(err, ccs_configuration_space_get_hyperparameter_indexes(
		    configuration_space, count, parents, parents_index), errmem);
		for (size_t i = 0; i < count; i++) {
			utarray_push_back(wrapper->parents, parents_index + i);
			parent_wrapper = (_ccs_hyperparameter_wrapper_cs_t *)utarray_eltptr(array, parents_index[i]);
			utarray_push_back(parent_wrapper->children, &index);
		}
	}
	wrapper = NULL;
	while ( (wrapper = (_ccs_hyperparameter_wrapper_cs_t *)utarray_next(array, wrapper)) ) {
		_uniq_size_t_array(wrapper->parents);
		_uniq_size_t_array(wrapper->children);
        }
	err = CCS_SUCCESS;
errmem:
	if (mem)
		free((void *)mem);
	return err;
}
#undef  utarray_oom
#define utarray_oom() { \
	exit(-1); \
}

static ccs_result_t
_generate_constraints(ccs_configuration_space_t configuration_space) {
	CCS_VALIDATE(_recompute_graph(configuration_space));
	CCS_VALIDATE(_topological_sort(configuration_space));
	configuration_space->data->graph_ok = CCS_TRUE;
	return CCS_SUCCESS;
}

ccs_result_t
ccs_configuration_space_set_condition(ccs_configuration_space_t configuration_space,
                                      size_t                    hyperparameter_index,
                                      ccs_expression_t          expression) {
	CCS_CHECK_OBJ(configuration_space, CCS_CONFIGURATION_SPACE);
	CCS_CHECK_OBJ(expression, CCS_EXPRESSION);
	_ccs_hyperparameter_wrapper_cs_t *wrapper = (_ccs_hyperparameter_wrapper_cs_t*)
	    utarray_eltptr(configuration_space->data->hyperparameters,
	                   (unsigned int)hyperparameter_index);
	if (!wrapper)
		return -CCS_OUT_OF_BOUNDS;
	if (wrapper->condition)
		return -CCS_INVALID_HYPERPARAMETER;
	ccs_result_t err;
	CCS_VALIDATE(ccs_retain_object(expression));
	wrapper->condition = expression;
	// Recompute the whole graph for now
	configuration_space->data->graph_ok = CCS_FALSE;
	CCS_VALIDATE_ERR_GOTO(err, _generate_constraints(configuration_space), erre);
	return CCS_SUCCESS;
erre:
	ccs_release_object(expression);
	wrapper->condition = NULL;
	return err;
}

ccs_result_t
ccs_configuration_space_get_condition(ccs_configuration_space_t  configuration_space,
                                      size_t                     hyperparameter_index,
                                      ccs_expression_t          *expression_ret) {
	CCS_CHECK_OBJ(configuration_space, CCS_CONFIGURATION_SPACE);
	CCS_CHECK_PTR(expression_ret);
	_ccs_hyperparameter_wrapper_cs_t *wrapper = (_ccs_hyperparameter_wrapper_cs_t*)
	    utarray_eltptr(configuration_space->data->hyperparameters,
	                   (unsigned int)hyperparameter_index);
	if (!wrapper)
		return -CCS_OUT_OF_BOUNDS;
	*expression_ret = wrapper->condition;
	return CCS_SUCCESS;
}

ccs_result_t
ccs_configuration_space_get_conditions(ccs_configuration_space_t  configuration_space,
                                       size_t                     num_expressions,
                                       ccs_expression_t          *expressions,
                                       size_t                    *num_expressions_ret) {
	CCS_CHECK_OBJ(configuration_space, CCS_CONFIGURATION_SPACE);
	CCS_CHECK_ARY(num_expressions, expressions);
	if (!expressions && !num_expressions_ret)
		return -CCS_INVALID_VALUE;
	UT_array *array = configuration_space->data->hyperparameters;
	size_t size = utarray_len(array);
	if (expressions) {
		if (num_expressions < size)
			return -CCS_INVALID_VALUE;
		_ccs_hyperparameter_wrapper_cs_t *wrapper = NULL;
		size_t index = 0;
		while ( (wrapper = (_ccs_hyperparameter_wrapper_cs_t *)utarray_next(array, wrapper)) )
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
	return -CCS_OUT_OF_MEMORY; \
}
ccs_result_t
ccs_configuration_space_add_forbidden_clause(ccs_configuration_space_t configuration_space,
                                             ccs_expression_t          expression) {
	CCS_CHECK_OBJ(configuration_space, CCS_CONFIGURATION_SPACE);
	CCS_CHECK_OBJ(expression, CCS_EXPRESSION);
	ccs_result_t err;
	CCS_VALIDATE(ccs_expression_check_context(
	    expression, (ccs_context_t)configuration_space));
	ccs_datum_t d;
	ccs_configuration_t config;
	CCS_VALIDATE(ccs_configuration_space_get_default_configuration(
	    configuration_space, &config));

	err = ccs_expression_eval(expression, (ccs_context_t)configuration_space,
	                          config->data->values,
	                          &d);
	ccs_release_object(config);
	if (err && err != -CCS_INACTIVE_HYPERPARAMETER)
		return err;
	if (!err && d.type == CCS_BOOLEAN && d.value.i == CCS_TRUE)
		return -CCS_INVALID_CONFIGURATION;
	CCS_VALIDATE(ccs_retain_object(expression));
	utarray_push_back(configuration_space->data->forbidden_clauses, &expression);
	return CCS_SUCCESS;
}
#undef  utarray_oom
#define utarray_oom() exit(-1)

ccs_result_t
ccs_configuration_space_add_forbidden_clauses(ccs_configuration_space_t  configuration_space,
                                              size_t                     num_expressions,
                                              ccs_expression_t          *expressions) {
	CCS_CHECK_OBJ(configuration_space, CCS_CONFIGURATION_SPACE);
	CCS_CHECK_ARY(num_expressions, expressions);
	for (size_t i = 0; i < num_expressions; i++)
		CCS_VALIDATE(ccs_configuration_space_add_forbidden_clause(
		    configuration_space, expressions[i]));
	return CCS_SUCCESS;
}

ccs_result_t
ccs_configuration_space_get_forbidden_clause(ccs_configuration_space_t  configuration_space,
                                             size_t                     index,
                                             ccs_expression_t          *expression_ret) {
	CCS_CHECK_OBJ(configuration_space, CCS_CONFIGURATION_SPACE);
	CCS_CHECK_PTR(expression_ret);
	ccs_expression_t *p_expr = (ccs_expression_t*)
	    utarray_eltptr(configuration_space->data->forbidden_clauses,
	                   (unsigned int)index);
	if (!p_expr)
		return -CCS_OUT_OF_BOUNDS;
	*expression_ret = *p_expr;
	return CCS_SUCCESS;
}

ccs_result_t
ccs_configuration_space_get_forbidden_clauses(ccs_configuration_space_t  configuration_space,
                                              size_t                     num_expressions,
                                              ccs_expression_t          *expressions,
                                              size_t                    *num_expressions_ret) {
	CCS_CHECK_OBJ(configuration_space, CCS_CONFIGURATION_SPACE);
	CCS_CHECK_ARY(num_expressions, expressions);
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
