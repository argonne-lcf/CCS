#include "cconfigspace_internal.h"
#include "configuration_space_internal.h"
#include "configuration_internal.h"
#include "distribution_internal.h"
#include "expression_internal.h"
#include "rng_internal.h"
#include "utlist.h"

static ccs_error_t
_generate_constraints(ccs_configuration_space_t configuration_space);

static ccs_error_t
_ccs_configuration_space_del(ccs_object_t object)
{
	ccs_configuration_space_t configuration_space =
		(ccs_configuration_space_t)object;
	UT_array *array = configuration_space->data->parameters;
	_ccs_parameter_wrapper_cs_t *wrapper = NULL;
	while ((wrapper = (_ccs_parameter_wrapper_cs_t *)utarray_next(
			array, wrapper))) {
		ccs_release_object(wrapper->parameter);
		if (wrapper->condition)
			ccs_release_object(wrapper->condition);
		utarray_free(wrapper->parents);
		utarray_free(wrapper->children);
	}
	array                  = configuration_space->data->forbidden_clauses;
	ccs_expression_t *expr = NULL;
	while ((expr = (ccs_expression_t *)utarray_next(array, expr))) {
		ccs_release_object(*expr);
	}
	HASH_CLEAR(hh_name, configuration_space->data->name_hash);
	_ccs_parameter_index_hash_t *elem, *tmpelem;
	HASH_ITER(
		hh_handle,
		configuration_space->data->handle_hash,
		elem,
		tmpelem)
	{
		HASH_DELETE(
			hh_handle,
			configuration_space->data->handle_hash,
			elem);
		free(elem);
	}
	utarray_free(configuration_space->data->parameters);
	utarray_free(configuration_space->data->forbidden_clauses);
	utarray_free(configuration_space->data->sorted_indexes);
	_ccs_distribution_wrapper_t *dw, *tmp;
	DL_FOREACH_SAFE(configuration_space->data->distribution_list, dw, tmp)
	{
		DL_DELETE(configuration_space->data->distribution_list, dw);
		ccs_release_object(dw->distribution);
		free(dw);
	}
	ccs_release_object(configuration_space->data->rng);
	return CCS_SUCCESS;
}

static inline ccs_error_t
_ccs_serialize_bin_size_ccs_configuration_space_data(
	_ccs_configuration_space_data_t *data,
	size_t                          *cum_size,
	_ccs_object_serialize_options_t *opts)
{
	size_t                       condition_count;
	_ccs_parameter_wrapper_cs_t *wrapper;
	size_t                       distribution_count;
	_ccs_distribution_wrapper_t *dw;
	ccs_expression_t            *expr;

	*cum_size += _ccs_serialize_bin_size_string(data->name);
	*cum_size +=
		_ccs_serialize_bin_size_size(utarray_len(data->parameters));

	condition_count = 0;
	wrapper         = NULL;
	while ((wrapper = (_ccs_parameter_wrapper_cs_t *)utarray_next(
			data->parameters, wrapper)))
		if (wrapper->condition)
			condition_count++;
	*cum_size += _ccs_serialize_bin_size_size(condition_count);

	DL_COUNT(data->distribution_list, dw, distribution_count);
	*cum_size += _ccs_serialize_bin_size_size(distribution_count);

	*cum_size += _ccs_serialize_bin_size_size(
		utarray_len(data->forbidden_clauses));

	/* rng */
	CCS_VALIDATE(data->rng->obj.ops->serialize_size(
		data->rng, CCS_SERIALIZE_FORMAT_BINARY, cum_size, opts));

	/* parameters */
	wrapper = NULL;
	while ((wrapper = (_ccs_parameter_wrapper_cs_t *)utarray_next(
			data->parameters, wrapper)))
		CCS_VALIDATE(wrapper->parameter->obj.ops->serialize_size(
			wrapper->parameter,
			CCS_SERIALIZE_FORMAT_BINARY,
			cum_size,
			opts));

	/* conditions */
	condition_count = 0;
	wrapper         = NULL;
	while ((wrapper = (_ccs_parameter_wrapper_cs_t *)utarray_next(
			data->parameters, wrapper))) {
		if (wrapper->condition) {
			/* parameter index and condition */
			*cum_size +=
				_ccs_serialize_bin_size_size(condition_count);
			CCS_VALIDATE(
				wrapper->condition->obj.ops->serialize_size(
					wrapper->condition,
					CCS_SERIALIZE_FORMAT_BINARY,
					cum_size,
					opts));
		}
		condition_count++;
	}

	/* distributions */
	dw = NULL;
	DL_FOREACH(data->distribution_list, dw)
	{
		CCS_VALIDATE(dw->distribution->obj.ops->serialize_size(
			dw->distribution,
			CCS_SERIALIZE_FORMAT_BINARY,
			cum_size,
			opts));
		*cum_size += _ccs_serialize_bin_size_size(dw->dimension);
		for (size_t i = 0; i < dw->dimension; i++)
			*cum_size += _ccs_serialize_bin_size_size(
				dw->parameter_indexes[i]);
	}

	/* forbidden clauses */
	expr = NULL;
	while ((expr = (ccs_expression_t *)utarray_next(
			data->forbidden_clauses, expr)))
		CCS_VALIDATE((*expr)->obj.ops->serialize_size(
			*expr, CCS_SERIALIZE_FORMAT_BINARY, cum_size, opts));
	return CCS_SUCCESS;
}

static inline ccs_error_t
_ccs_serialize_bin_ccs_configuration_space_data(
	_ccs_configuration_space_data_t *data,
	size_t                          *buffer_size,
	char                           **buffer,
	_ccs_object_serialize_options_t *opts)
{
	size_t                       condition_count;
	_ccs_parameter_wrapper_cs_t *wrapper;
	size_t                       distribution_count;
	_ccs_distribution_wrapper_t *dw;
	ccs_expression_t            *expr;

	CCS_VALIDATE(
		_ccs_serialize_bin_string(data->name, buffer_size, buffer));
	CCS_VALIDATE(_ccs_serialize_bin_size(
		utarray_len(data->parameters), buffer_size, buffer));

	condition_count = 0;
	wrapper         = NULL;
	while ((wrapper = (_ccs_parameter_wrapper_cs_t *)utarray_next(
			data->parameters, wrapper)))
		if (wrapper->condition)
			condition_count++;
	CCS_VALIDATE(
		_ccs_serialize_bin_size(condition_count, buffer_size, buffer));

	DL_COUNT(data->distribution_list, dw, distribution_count);
	CCS_VALIDATE(_ccs_serialize_bin_size(
		distribution_count, buffer_size, buffer));

	CCS_VALIDATE(_ccs_serialize_bin_size(
		utarray_len(data->forbidden_clauses), buffer_size, buffer));

	/* rng */
	CCS_VALIDATE(data->rng->obj.ops->serialize(
		data->rng,
		CCS_SERIALIZE_FORMAT_BINARY,
		buffer_size,
		buffer,
		opts));

	/* parameters */
	wrapper = NULL;
	while ((wrapper = (_ccs_parameter_wrapper_cs_t *)utarray_next(
			data->parameters, wrapper)))
		CCS_VALIDATE(wrapper->parameter->obj.ops->serialize(
			wrapper->parameter,
			CCS_SERIALIZE_FORMAT_BINARY,
			buffer_size,
			buffer,
			opts));

	/* conditions */
	condition_count = 0;
	wrapper         = NULL;
	while ((wrapper = (_ccs_parameter_wrapper_cs_t *)utarray_next(
			data->parameters, wrapper))) {
		if (wrapper->condition) {
			CCS_VALIDATE(_ccs_serialize_bin_size(
				condition_count, buffer_size, buffer));
			CCS_VALIDATE(wrapper->condition->obj.ops->serialize(
				wrapper->condition,
				CCS_SERIALIZE_FORMAT_BINARY,
				buffer_size,
				buffer,
				opts));
		}
		condition_count++;
	}

	/* distributions */
	dw = NULL;
	DL_FOREACH(data->distribution_list, dw)
	{
		CCS_VALIDATE(dw->distribution->obj.ops->serialize(
			dw->distribution,
			CCS_SERIALIZE_FORMAT_BINARY,
			buffer_size,
			buffer,
			opts));
		CCS_VALIDATE(_ccs_serialize_bin_size(
			dw->dimension, buffer_size, buffer));
		for (size_t i = 0; i < dw->dimension; i++)
			CCS_VALIDATE(_ccs_serialize_bin_size(
				dw->parameter_indexes[i], buffer_size, buffer));
	}

	/* forbidden clauses */
	expr = NULL;
	while ((expr = (ccs_expression_t *)utarray_next(
			data->forbidden_clauses, expr)))
		CCS_VALIDATE((*expr)->obj.ops->serialize(
			*expr,
			CCS_SERIALIZE_FORMAT_BINARY,
			buffer_size,
			buffer,
			opts));
	return CCS_SUCCESS;
}

static inline ccs_error_t
_ccs_serialize_bin_size_ccs_configuration_space(
	ccs_configuration_space_t        configuration_space,
	size_t                          *cum_size,
	_ccs_object_serialize_options_t *opts)
{
	_ccs_configuration_space_data_t *data =
		(_ccs_configuration_space_data_t *)(configuration_space->data);
	*cum_size += _ccs_serialize_bin_size_ccs_object_internal(
		(_ccs_object_internal_t *)configuration_space);
	CCS_VALIDATE(_ccs_serialize_bin_size_ccs_configuration_space_data(
		data, cum_size, opts));
	return CCS_SUCCESS;
}

static inline ccs_error_t
_ccs_serialize_bin_ccs_configuration_space(
	ccs_configuration_space_t        configuration_space,
	size_t                          *buffer_size,
	char                           **buffer,
	_ccs_object_serialize_options_t *opts)
{
	_ccs_configuration_space_data_t *data =
		(_ccs_configuration_space_data_t *)(configuration_space->data);
	CCS_VALIDATE(_ccs_serialize_bin_ccs_object_internal(
		(_ccs_object_internal_t *)configuration_space,
		buffer_size,
		buffer));
	CCS_VALIDATE(_ccs_serialize_bin_ccs_configuration_space_data(
		data, buffer_size, buffer, opts));
	return CCS_SUCCESS;
}

static ccs_error_t
_ccs_configuration_space_serialize_size(
	ccs_object_t                     object,
	ccs_serialize_format_t           format,
	size_t                          *cum_size,
	_ccs_object_serialize_options_t *opts)
{
	switch (format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE(_ccs_serialize_bin_size_ccs_configuration_space(
			(ccs_configuration_space_t)object, cum_size, opts));
		break;
	default:
		CCS_RAISE(
			CCS_INVALID_VALUE,
			"Unsupported serialization format: %d",
			format);
	}
	CCS_VALIDATE(_ccs_object_serialize_user_data_size(
		object, format, cum_size, opts));
	return CCS_SUCCESS;
}

static ccs_error_t
_ccs_configuration_space_serialize(
	ccs_object_t                     object,
	ccs_serialize_format_t           format,
	size_t                          *buffer_size,
	char                           **buffer,
	_ccs_object_serialize_options_t *opts)
{
	switch (format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE(_ccs_serialize_bin_ccs_configuration_space(
			(ccs_configuration_space_t)object,
			buffer_size,
			buffer,
			opts));
		break;
	default:
		CCS_RAISE(
			CCS_INVALID_VALUE,
			"Unsupported serialization format: %d",
			format);
	}
	CCS_VALIDATE(_ccs_object_serialize_user_data(
		object, format, buffer_size, buffer, opts));
	return CCS_SUCCESS;
}

static _ccs_configuration_space_ops_t _configuration_space_ops = {
	{{&_ccs_configuration_space_del,
	  &_ccs_configuration_space_serialize_size,
	  &_ccs_configuration_space_serialize}}};

static const UT_icd _parameter_wrapper_icd = {
	sizeof(_ccs_parameter_wrapper_cs_t),
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

static UT_icd _size_t_icd = {sizeof(size_t), NULL, NULL, NULL};

#undef utarray_oom
#define utarray_oom()                                                          \
	{                                                                      \
		ccs_release_object(config_space->data->rng);                   \
		CCS_RAISE_ERR_GOTO(                                            \
			err,                                                   \
			CCS_OUT_OF_MEMORY,                                     \
			errarrays,                                             \
			"Out of memory to allocate array");                    \
	}
ccs_error_t
ccs_create_configuration_space(
	const char                *name,
	ccs_configuration_space_t *configuration_space_ret)
{
	CCS_CHECK_PTR(name);
	CCS_CHECK_PTR(configuration_space_ret);
	ccs_error_t err;
	uintptr_t   mem = (uintptr_t)calloc(
                1,
                sizeof(struct _ccs_configuration_space_s) +
                        sizeof(struct _ccs_configuration_space_data_s) +
                        strlen(name) + 1);
	CCS_REFUTE(!mem, CCS_OUT_OF_MEMORY);
	ccs_rng_t rng;
	CCS_VALIDATE_ERR_GOTO(err, ccs_create_rng(&rng), errmem);

	ccs_configuration_space_t config_space;
	config_space = (ccs_configuration_space_t)mem;
	_ccs_object_init(
		&(config_space->obj),
		CCS_CONFIGURATION_SPACE,
		(_ccs_object_ops_t *)&_configuration_space_ops);
	config_space->data =
		(struct _ccs_configuration_space_data_s
			 *)(mem + sizeof(struct _ccs_configuration_space_s));
	config_space->data->name =
		(const char
			 *)(mem + sizeof(struct _ccs_configuration_space_s) + sizeof(struct _ccs_configuration_space_data_s));
	config_space->data->rng = rng;
	utarray_new(config_space->data->parameters, &_parameter_wrapper_icd);
	utarray_new(
		config_space->data->forbidden_clauses, &_forbidden_clauses_icd);
	utarray_new(config_space->data->sorted_indexes, &_size_t_icd);
	config_space->data->graph_ok = CCS_TRUE;
	strcpy((char *)(config_space->data->name), name);
	*configuration_space_ret = config_space;
	return CCS_SUCCESS;
errarrays:
	if (config_space->data->parameters)
		utarray_free(config_space->data->parameters);
	if (config_space->data->forbidden_clauses)
		utarray_free(config_space->data->forbidden_clauses);
	if (config_space->data->sorted_indexes)
		utarray_free(config_space->data->sorted_indexes);
errmem:
	free((void *)mem);
	return err;
}

ccs_error_t
ccs_configuration_space_get_name(
	ccs_configuration_space_t configuration_space,
	const char              **name_ret)
{
	CCS_CHECK_OBJ(configuration_space, CCS_CONFIGURATION_SPACE);
	CCS_VALIDATE(_ccs_context_get_name(
		(ccs_context_t)configuration_space, name_ret));
	return CCS_SUCCESS;
}

ccs_error_t
ccs_configuration_space_set_rng(
	ccs_configuration_space_t configuration_space,
	ccs_rng_t                 rng)
{
	CCS_CHECK_OBJ(configuration_space, CCS_CONFIGURATION_SPACE);
	CCS_CHECK_OBJ(rng, CCS_RNG);
	CCS_VALIDATE(ccs_retain_object(rng));
	ccs_rng_t tmp                  = configuration_space->data->rng;
	configuration_space->data->rng = rng;
	CCS_VALIDATE(ccs_release_object(tmp));
	return CCS_SUCCESS;
}

ccs_error_t
ccs_configuration_space_get_rng(
	ccs_configuration_space_t configuration_space,
	ccs_rng_t                *rng_ret)
{
	CCS_CHECK_OBJ(configuration_space, CCS_CONFIGURATION_SPACE);
	CCS_CHECK_PTR(rng_ret);
	*rng_ret = configuration_space->data->rng;
	return CCS_SUCCESS;
}

#undef utarray_oom
#define utarray_oom()                                                          \
	{                                                                      \
		CCS_RAISE_ERR_GOTO(                                            \
			err,                                                   \
			CCS_OUT_OF_MEMORY,                                     \
			errordistrib_wrapper,                                  \
			"Out of memory to allocate array");                    \
	}
#undef uthash_nonfatal_oom
#define uthash_nonfatal_oom(elt)                                               \
	{                                                                      \
		CCS_RAISE_ERR_GOTO(                                            \
			err,                                                   \
			CCS_OUT_OF_MEMORY,                                     \
			errorutarray,                                          \
			"Out of memory to allocate hash");                     \
	}
ccs_error_t
ccs_configuration_space_add_parameter(
	ccs_configuration_space_t configuration_space,
	ccs_parameter_t           parameter,
	ccs_distribution_t        distribution)
{
	CCS_CHECK_OBJ(configuration_space, CCS_CONFIGURATION_SPACE);
	CCS_CHECK_OBJ(parameter, CCS_PARAMETER);
	ccs_error_t          err;
	ccs_parameter_type_t type;

	CCS_VALIDATE(ccs_parameter_get_type(parameter, &type));
	CCS_REFUTE(CCS_PARAMETER_TYPE_STRING == type, CCS_INVALID_PARAMETER);

	const char                  *name;
	size_t                       sz_name;
	_ccs_parameter_index_hash_t *parameter_hash;
	CCS_VALIDATE(ccs_parameter_get_name(parameter, &name));
	sz_name = strlen(name);
	HASH_FIND(
		hh_name,
		configuration_space->data->name_hash,
		name,
		sz_name,
		parameter_hash);
	CCS_REFUTE_MSG(
		parameter_hash,
		CCS_INVALID_PARAMETER,
		"An parameter with name '%s' already exists in the "
		"configuration space",
		name);
	UT_array                    *parameters;
	size_t                       index;
	size_t                       dimension;
	_ccs_parameter_wrapper_cs_t  parameter_wrapper;
	_ccs_distribution_wrapper_t *distrib_wrapper;
	uintptr_t                    pmem;
	parameter_wrapper.parameter = parameter;
	CCS_VALIDATE(ccs_retain_object(parameter));

	if (distribution) {
		CCS_VALIDATE_ERR_GOTO(
			err,
			ccs_distribution_get_dimension(
				distribution, &dimension),
			errorparameter);
		CCS_REFUTE_ERR_GOTO(
			err,
			dimension != 1,
			CCS_INVALID_DISTRIBUTION,
			errorparameter);
		CCS_VALIDATE_ERR_GOTO(
			err, ccs_retain_object(distribution), errorparameter);
	} else {
		CCS_VALIDATE_ERR_GOTO(
			err,
			ccs_parameter_get_default_distribution(
				parameter, &distribution),
			errorparameter);
		dimension = 1;
	}
	pmem = (uintptr_t)malloc(
		sizeof(_ccs_distribution_wrapper_t) +
		sizeof(size_t) * dimension);
	CCS_REFUTE_ERR_GOTO(err, !pmem, CCS_OUT_OF_MEMORY, errordistrib);
	distrib_wrapper               = (_ccs_distribution_wrapper_t *)pmem;
	distrib_wrapper->distribution = distribution;
	distrib_wrapper->dimension    = dimension;
	distrib_wrapper->parameter_indexes =
		(size_t *)(pmem + sizeof(_ccs_distribution_wrapper_t));

	parameter_hash = (_ccs_parameter_index_hash_t *)malloc(
		sizeof(_ccs_parameter_index_hash_t));
	CCS_REFUTE_ERR_GOTO(
		err, !parameter_hash, CCS_OUT_OF_MEMORY, errordistrib_wrapper);

	parameters                = configuration_space->data->parameters;
	index                     = utarray_len(parameters);
	parameter_hash->parameter = parameter;
	parameter_hash->name      = name;
	parameter_hash->index     = index;
	distrib_wrapper->parameter_indexes[0] = index;
	parameter_wrapper.distribution_index  = 0;
	parameter_wrapper.distribution        = distrib_wrapper;
	parameter_wrapper.condition           = NULL;
	parameter_wrapper.parents             = NULL;
	parameter_wrapper.children            = NULL;
	utarray_new(parameter_wrapper.parents, &_size_t_icd);
	utarray_new(parameter_wrapper.children, &_size_t_icd);

	utarray_push_back(parameters, &parameter_wrapper);
	utarray_push_back(configuration_space->data->sorted_indexes, &index);

	HASH_ADD_KEYPTR(
		hh_name,
		configuration_space->data->name_hash,
		parameter_hash->name,
		sz_name,
		parameter_hash);
	HASH_ADD(
		hh_handle,
		configuration_space->data->handle_hash,
		parameter,
		sizeof(ccs_parameter_t),
		parameter_hash);
	DL_APPEND(
		configuration_space->data->distribution_list, distrib_wrapper);

	return CCS_SUCCESS;
errorutarray:
	utarray_pop_back(parameters);
errordistrib_wrapper:
	if (parameter_hash)
		free(parameter_hash);
	if (parameter_wrapper.parents)
		utarray_free(parameter_wrapper.parents);
	if (parameter_wrapper.children)
		utarray_free(parameter_wrapper.children);
	free(distrib_wrapper);
errordistrib:
	ccs_release_object(distribution);
errorparameter:
	ccs_release_object(parameter);
	return err;
}

#undef utarray_oom
#define utarray_oom() exit(-1)

ccs_error_t
ccs_configuration_space_set_distribution(
	ccs_configuration_space_t configuration_space,
	ccs_distribution_t        distribution,
	size_t                   *indexes)
{
	CCS_CHECK_OBJ(configuration_space, CCS_CONFIGURATION_SPACE);
	CCS_CHECK_OBJ(distribution, CCS_DISTRIBUTION);
	CCS_CHECK_PTR(indexes);

	_ccs_distribution_wrapper_t *dwrapper;
	_ccs_parameter_wrapper_cs_t *hwrapper;
	ccs_error_t                  err;
	UT_array *parameters     = configuration_space->data->parameters;
	size_t    num_parameters = utarray_len(parameters);
	size_t    dim;
	CCS_VALIDATE(ccs_distribution_get_dimension(distribution, &dim));

	for (size_t i = 0; i < dim; i++) {
		CCS_REFUTE(indexes[i] >= num_parameters, CCS_INVALID_VALUE);
		// Check duplicate entries
		for (size_t j = i + 1; j < dim; j++)
			CCS_REFUTE(indexes[i] == indexes[j], CCS_INVALID_VALUE);
	}

	uintptr_t cur_mem;
	uintptr_t mem = (uintptr_t)malloc(
		sizeof(void *) * num_parameters * 2 +
		sizeof(size_t) * num_parameters);
	CCS_REFUTE(!mem, CCS_OUT_OF_MEMORY);
	cur_mem = mem;
	_ccs_distribution_wrapper_t **p_dwrappers_to_del =
		(_ccs_distribution_wrapper_t **)cur_mem;
	cur_mem += sizeof(_ccs_distribution_wrapper_t *) * num_parameters;
	_ccs_distribution_wrapper_t **p_dwrappers_to_add =
		(_ccs_distribution_wrapper_t **)cur_mem;
	cur_mem += sizeof(_ccs_distribution_wrapper_t *) * num_parameters;
	size_t *parameters_without_distrib = (size_t *)cur_mem;
	cur_mem += sizeof(size_t) * num_parameters;

	size_t to_add_count          = 0;
	size_t to_del_count          = 0;
	size_t without_distrib_count = 0;

	for (size_t i = 0; i < dim; i++) {
		int add  = 1;
		hwrapper = (_ccs_parameter_wrapper_cs_t *)utarray_eltptr(
			parameters, indexes[i]);
		for (size_t j = 0; j < to_del_count; j++)
			if (p_dwrappers_to_del[j] == hwrapper->distribution) {
				add = 0;
				break;
			}
		if (add)
			p_dwrappers_to_del[to_del_count++] =
				hwrapper->distribution;
	}
	for (size_t i = 0; i < to_del_count; i++) {
		for (size_t j = 0; j < p_dwrappers_to_del[i]->dimension; j++) {
			parameters_without_distrib[without_distrib_count++] =
				p_dwrappers_to_del[i]->parameter_indexes[j];
		}
	}

	uintptr_t dmem = (uintptr_t)malloc(
		sizeof(_ccs_distribution_wrapper_t) + sizeof(size_t) * dim);
	CCS_REFUTE_ERR_GOTO(err, !dmem, CCS_OUT_OF_MEMORY, memory);

	dwrapper               = (_ccs_distribution_wrapper_t *)dmem;
	dwrapper->distribution = distribution;
	dwrapper->dimension    = dim;
	dwrapper->parameter_indexes =
		(size_t *)(dmem + sizeof(_ccs_distribution_wrapper_t));
	for (size_t i = 0; i < dim; i++) {
		dwrapper->parameter_indexes[i] = indexes[i];
		size_t indx                    = 0;
		for (size_t j = 0; j < without_distrib_count; j++, indx++)
			if (parameters_without_distrib[j] == indexes[i])
				break;
		for (size_t j = indx + 1; j < without_distrib_count; j++)
			parameters_without_distrib[j - 1] =
				parameters_without_distrib[j];
		without_distrib_count--;
	}
	CCS_VALIDATE_ERR_GOTO(err, ccs_retain_object(distribution), errdmem);

	p_dwrappers_to_add[0] = dwrapper;
	to_add_count          = 1;
	for (size_t i = 0; i < without_distrib_count; i++) {
		dmem = (uintptr_t)malloc(
			sizeof(_ccs_distribution_wrapper_t) + sizeof(size_t));
		CCS_REFUTE_ERR_GOTO(err, !dmem, CCS_OUT_OF_MEMORY, memory);
		dwrapper = (_ccs_distribution_wrapper_t *)dmem;
		dwrapper->parameter_indexes =
			(size_t *)(dmem + sizeof(_ccs_distribution_wrapper_t));
		dwrapper->dimension            = 1;
		dwrapper->parameter_indexes[0] = parameters_without_distrib[i];
		hwrapper = (_ccs_parameter_wrapper_cs_t *)utarray_eltptr(
			parameters, parameters_without_distrib[i]);
		CCS_VALIDATE_ERR_GOTO(
			err,
			ccs_parameter_get_default_distribution(
				hwrapper->parameter, &(dwrapper->distribution)),
			dwrappers);
		p_dwrappers_to_add[to_add_count++] = dwrapper;
	}

	for (size_t i = 0; i < to_del_count; i++) {
		DL_DELETE(
			configuration_space->data->distribution_list,
			p_dwrappers_to_del[i]);
		ccs_release_object(p_dwrappers_to_del[i]->distribution);
		free(p_dwrappers_to_del[i]);
	}
	for (size_t i = 0; i < to_add_count; i++) {
		DL_APPEND(
			configuration_space->data->distribution_list,
			p_dwrappers_to_add[i]);
		for (size_t j = 0; j < p_dwrappers_to_add[i]->dimension; j++) {
			hwrapper =
				(_ccs_parameter_wrapper_cs_t *)utarray_eltptr(
					parameters,
					p_dwrappers_to_add[i]
						->parameter_indexes[j]);
			hwrapper->distribution_index = j;
			hwrapper->distribution       = p_dwrappers_to_add[i];
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

extern ccs_error_t
ccs_configuration_space_get_parameter_distribution(
	ccs_configuration_space_t configuration_space,
	size_t                    index,
	ccs_distribution_t       *distribution_ret,
	size_t                   *index_ret)
{
	CCS_CHECK_OBJ(configuration_space, CCS_CONFIGURATION_SPACE);
	CCS_CHECK_PTR(distribution_ret);
	CCS_CHECK_PTR(index_ret);

	_ccs_parameter_wrapper_cs_t *wrapper =
		(_ccs_parameter_wrapper_cs_t *)utarray_eltptr(
			configuration_space->data->parameters,
			(unsigned int)index);
	CCS_REFUTE(!wrapper, CCS_OUT_OF_BOUNDS);
	*distribution_ret = wrapper->distribution->distribution;
	*index_ret        = wrapper->distribution_index;
	return CCS_SUCCESS;
}

ccs_error_t
ccs_configuration_space_add_parameters(
	ccs_configuration_space_t configuration_space,
	size_t                    num_parameters,
	ccs_parameter_t          *parameters,
	ccs_distribution_t       *distributions)
{
	CCS_CHECK_OBJ(configuration_space, CCS_CONFIGURATION_SPACE);
	CCS_CHECK_ARY(num_parameters, parameters);
	for (size_t i = 0; i < num_parameters; i++) {
		ccs_distribution_t distribution = NULL;
		if (distributions)
			distribution = distributions[i];
		CCS_VALIDATE(ccs_configuration_space_add_parameter(
			configuration_space, parameters[i], distribution));
	}
	return CCS_SUCCESS;
}

ccs_error_t
ccs_configuration_space_get_num_parameters(
	ccs_configuration_space_t configuration_space,
	size_t                   *num_parameters_ret)
{
	CCS_CHECK_OBJ(configuration_space, CCS_CONFIGURATION_SPACE);
	CCS_VALIDATE(_ccs_context_get_num_parameters(
		(ccs_context_t)configuration_space, num_parameters_ret));
	return CCS_SUCCESS;
}

ccs_error_t
ccs_configuration_space_get_parameter(
	ccs_configuration_space_t configuration_space,
	size_t                    index,
	ccs_parameter_t          *parameter_ret)
{
	CCS_CHECK_OBJ(configuration_space, CCS_CONFIGURATION_SPACE);
	CCS_VALIDATE(_ccs_context_get_parameter(
		(ccs_context_t)configuration_space, index, parameter_ret));
	return CCS_SUCCESS;
}

ccs_error_t
ccs_configuration_space_get_parameter_by_name(
	ccs_configuration_space_t configuration_space,
	const char               *name,
	ccs_parameter_t          *parameter_ret)
{
	CCS_CHECK_OBJ(configuration_space, CCS_CONFIGURATION_SPACE);
	CCS_VALIDATE(_ccs_context_get_parameter_by_name(
		(ccs_context_t)configuration_space, name, parameter_ret));
	return CCS_SUCCESS;
}

ccs_error_t
ccs_configuration_space_get_parameter_index_by_name(
	ccs_configuration_space_t configuration_space,
	const char               *name,
	size_t                   *index_ret)
{
	CCS_CHECK_OBJ(configuration_space, CCS_CONFIGURATION_SPACE);
	CCS_VALIDATE(_ccs_context_get_parameter_index_by_name(
		(ccs_context_t)configuration_space, name, index_ret));
	return CCS_SUCCESS;
}

ccs_error_t
ccs_configuration_space_get_parameter_index(
	ccs_configuration_space_t configuration_space,
	ccs_parameter_t           parameter,
	size_t                   *index_ret)
{
	CCS_CHECK_OBJ(configuration_space, CCS_CONFIGURATION_SPACE);
	CCS_VALIDATE(_ccs_context_get_parameter_index(
		(ccs_context_t)(configuration_space), parameter, index_ret));
	return CCS_SUCCESS;
}

ccs_error_t
ccs_configuration_space_get_parameter_indexes(
	ccs_configuration_space_t configuration_space,
	size_t                    num_parameters,
	ccs_parameter_t          *parameters,
	size_t                   *indexes)
{
	CCS_CHECK_OBJ(configuration_space, CCS_CONFIGURATION_SPACE);
	CCS_VALIDATE(_ccs_context_get_parameter_indexes(
		(ccs_context_t)configuration_space,
		num_parameters,
		parameters,
		indexes));
	return CCS_SUCCESS;
}

ccs_error_t
ccs_configuration_space_get_parameters(
	ccs_configuration_space_t configuration_space,
	size_t                    num_parameters,
	ccs_parameter_t          *parameters,
	size_t                   *num_parameters_ret)
{
	CCS_CHECK_OBJ(configuration_space, CCS_CONFIGURATION_SPACE);
	CCS_VALIDATE(_ccs_context_get_parameters(
		(ccs_context_t)configuration_space,
		num_parameters,
		parameters,
		num_parameters_ret));
	return CCS_SUCCESS;
}

ccs_error_t
ccs_configuration_space_validate_value(
	ccs_configuration_space_t configuration_space,
	size_t                    index,
	ccs_datum_t               value,
	ccs_datum_t              *value_ret)
{
	CCS_CHECK_OBJ(configuration_space, CCS_CONFIGURATION_SPACE);
	CCS_VALIDATE(_ccs_context_validate_value(
		(ccs_context_t)configuration_space, index, value, value_ret));
	return CCS_SUCCESS;
}

static ccs_error_t
_set_actives(
	ccs_configuration_space_t configuration_space,
	ccs_configuration_t       configuration)
{
	size_t      *p_index = NULL;
	UT_array    *indexes = configuration_space->data->sorted_indexes;
	UT_array    *array   = configuration_space->data->parameters;
	ccs_datum_t *values  = configuration->data->values;
	while ((p_index = (size_t *)utarray_next(indexes, p_index))) {
		_ccs_parameter_wrapper_cs_t *wrapper = NULL;
		wrapper = (_ccs_parameter_wrapper_cs_t *)utarray_eltptr(
			array, *p_index);
		if (!wrapper->condition)
			continue;
		ccs_datum_t result;
		CCS_VALIDATE(ccs_expression_eval(
			wrapper->condition,
			(ccs_context_t)configuration_space,
			values,
			&result));
		if (!(result.type == CCS_BOOLEAN && result.value.i == CCS_TRUE))
			values[*p_index] = ccs_inactive;
	}
	return CCS_SUCCESS;
}

ccs_error_t
ccs_configuration_space_get_default_configuration(
	ccs_configuration_space_t configuration_space,
	ccs_configuration_t      *configuration_ret)
{
	CCS_CHECK_OBJ(configuration_space, CCS_CONFIGURATION_SPACE);
	CCS_CHECK_PTR(configuration_ret);
	ccs_error_t         err;
	ccs_configuration_t config;
	CCS_VALIDATE(ccs_create_configuration(
		configuration_space, 0, NULL, &config));
	UT_array *array = configuration_space->data->parameters;
	_ccs_parameter_wrapper_cs_t *wrapper = NULL;
	ccs_datum_t                 *values  = config->data->values;
	while ((wrapper = (_ccs_parameter_wrapper_cs_t *)utarray_next(
			array, wrapper)))
		CCS_VALIDATE_ERR_GOTO(
			err,
			ccs_parameter_get_default_value(
				wrapper->parameter, values++),
			errc);
	CCS_VALIDATE_ERR_GOTO(
		err, _set_actives(configuration_space, config), errc);
	*configuration_ret = config;
	return CCS_SUCCESS;
errc:
	ccs_release_object(config);
	return err;
}

static ccs_error_t
_test_forbidden(
	ccs_configuration_space_t configuration_space,
	ccs_datum_t              *values,
	ccs_bool_t               *is_valid)
{
	UT_array         *array = configuration_space->data->forbidden_clauses;
	ccs_expression_t *p_expression = NULL;
	*is_valid                      = CCS_FALSE;
	while ((p_expression = (ccs_expression_t *)utarray_next(
			array, p_expression))) {
		ccs_datum_t result;
		CCS_VALIDATE(ccs_expression_eval(
			*p_expression,
			(ccs_context_t)configuration_space,
			values,
			&result));
		if (result.type == CCS_INACTIVE)
			continue;
		if (result.type == CCS_BOOLEAN && result.value.i == CCS_TRUE)
			return CCS_SUCCESS;
	}
	*is_valid = CCS_TRUE;
	return CCS_SUCCESS;
}

static inline ccs_error_t
_check_configuration(
	ccs_configuration_space_t configuration_space,
	size_t                    num_values,
	ccs_datum_t              *values,
	ccs_bool_t               *is_valid_ret)
{
	UT_array *indexes = configuration_space->data->sorted_indexes;
	UT_array *array   = configuration_space->data->parameters;
	CCS_REFUTE(num_values != utarray_len(array), CCS_INVALID_CONFIGURATION);
	size_t *p_index = NULL;
	while ((p_index = (size_t *)utarray_next(indexes, p_index))) {
		ccs_bool_t                   active  = CCS_TRUE;
		_ccs_parameter_wrapper_cs_t *wrapper = NULL;
		wrapper = (_ccs_parameter_wrapper_cs_t *)utarray_eltptr(
			array, *p_index);
		if (wrapper->condition) {
			ccs_datum_t result;
			CCS_VALIDATE(ccs_expression_eval(
				wrapper->condition,
				(ccs_context_t)configuration_space,
				values,
				&result));
			if (!(result.type == CCS_BOOLEAN &&
			      result.value.i == CCS_TRUE))
				active = CCS_FALSE;
		}
		if (active != (values[*p_index].type == CCS_INACTIVE
				       ? CCS_FALSE
				       : CCS_TRUE)) {
			*is_valid_ret = CCS_FALSE;
			return CCS_SUCCESS;
		}
		if (active) {
			CCS_VALIDATE(ccs_parameter_check_value(
				wrapper->parameter,
				values[*p_index],
				is_valid_ret));
			if (*is_valid_ret == CCS_FALSE)
				return CCS_SUCCESS;
		}
	}
	CCS_VALIDATE(
		_test_forbidden(configuration_space, values, is_valid_ret));
	return CCS_SUCCESS;
}

ccs_error_t
ccs_configuration_space_check_configuration(
	ccs_configuration_space_t configuration_space,
	ccs_configuration_t       configuration,
	ccs_bool_t               *is_valid_ret)
{
	CCS_CHECK_OBJ(configuration_space, CCS_CONFIGURATION_SPACE);
	CCS_CHECK_OBJ(configuration, CCS_CONFIGURATION);
	CCS_REFUTE(
		configuration->data->configuration_space != configuration_space,
		CCS_INVALID_CONFIGURATION);
	if (!configuration_space->data->graph_ok)
		CCS_VALIDATE(_generate_constraints(configuration_space));
	CCS_VALIDATE(_check_configuration(
		configuration_space,
		configuration->data->num_values,
		configuration->data->values,
		is_valid_ret));
	return CCS_SUCCESS;
}

ccs_error_t
ccs_configuration_space_check_configuration_values(
	ccs_configuration_space_t configuration_space,
	size_t                    num_values,
	ccs_datum_t              *values,
	ccs_bool_t               *is_valid_ret)
{
	CCS_CHECK_OBJ(configuration_space, CCS_CONFIGURATION_SPACE);
	CCS_CHECK_ARY(num_values, values);
	if (!configuration_space->data->graph_ok)
		CCS_VALIDATE(_generate_constraints(configuration_space));
	CCS_VALIDATE(_check_configuration(
		configuration_space, num_values, values, is_valid_ret));
	return CCS_SUCCESS;
}

static ccs_error_t
_sample(ccs_configuration_space_t configuration_space,
	ccs_configuration_t       config,
	ccs_bool_t               *found)
{
	ccs_error_t err;
	ccs_rng_t   rng   = configuration_space->data->rng;
	UT_array   *array = configuration_space->data->parameters;
	_ccs_distribution_wrapper_t *dwrapper       = NULL;
	_ccs_parameter_wrapper_cs_t *hwrapper       = NULL;
	ccs_datum_t                 *values         = config->data->values;

	size_t                       num_parameters = utarray_len(array);
	ccs_datum_t                 *p_values;
	ccs_parameter_t             *hps;
	uintptr_t                    mem;
	mem = (uintptr_t)malloc(
		num_parameters *
		(sizeof(ccs_datum_t) + sizeof(ccs_parameter_t)));
	CCS_REFUTE(!mem, CCS_OUT_OF_MEMORY);

	p_values = (ccs_datum_t *)mem;
	hps = (ccs_parameter_t *)(mem + num_parameters * sizeof(ccs_datum_t));
	DL_FOREACH(configuration_space->data->distribution_list, dwrapper)
	{
		for (size_t i = 0; i < dwrapper->dimension; i++) {
			size_t hindex = dwrapper->parameter_indexes[i];
			hwrapper =
				(_ccs_parameter_wrapper_cs_t *)utarray_eltptr(
					array, hindex);
			hps[i] = hwrapper->parameter;
		}
		CCS_VALIDATE_ERR_GOTO(
			err,
			ccs_distribution_parameters_sample(
				dwrapper->distribution, rng, hps, p_values),
			memory);
		for (size_t i = 0; i < dwrapper->dimension; i++) {
			size_t hindex  = dwrapper->parameter_indexes[i];
			values[hindex] = p_values[i];
		}
	}
	CCS_VALIDATE_ERR_GOTO(
		err, _set_actives(configuration_space, config), memory);
	CCS_VALIDATE_ERR_GOTO(
		err,
		_test_forbidden(
			configuration_space, config->data->values, found),
		memory);
memory:
	free((void *)mem);
	return err;
}

// static ccs_error_t
//_sample(ccs_configuration_space_t  configuration_space,
//         ccs_configuration_t        config,
//         ccs_bool_t                *found) {
//       ccs_rng_t rng = configuration_space->data->rng;
//       UT_array *array = configuration_space->data->parameters;
//       _ccs_parameter_wrapper_cs_t *wrapper = NULL;
//       ccs_datum_t *values = config->data->values;
//       while ( (wrapper = (_ccs_parameter_wrapper_cs_t *)
//                              utarray_next(array, wrapper)) ) {
//               CCS_VALIDATE(ccs_parameter_sample(wrapper->parameter,
//                   wrapper->distribution->distribution, rng, values++));
//       }
//       CCS_VALIDATE(_set_actives(configuration_space, config));
//       CCS_VALIDATE(_test_forbidden(configuration_space, config->data->values,
//       found)); return CCS_SUCCESS;
// }

ccs_error_t
ccs_configuration_space_sample(
	ccs_configuration_space_t configuration_space,
	ccs_configuration_t      *configuration_ret)
{
	CCS_CHECK_OBJ(configuration_space, CCS_CONFIGURATION_SPACE);
	CCS_CHECK_PTR(configuration_ret);
	ccs_error_t         err;
	ccs_configuration_t config;
	if (!configuration_space->data->graph_ok)
		CCS_VALIDATE(_generate_constraints(configuration_space));
	CCS_VALIDATE(ccs_create_configuration(
		configuration_space, 0, NULL, &config));
	ccs_bool_t found;
	int        counter = 0;
	do {
		CCS_VALIDATE_ERR_GOTO(
			err,
			_sample(configuration_space, config, &found),
			errc);
		counter++;
	} while (!found && counter < 100);
	CCS_REFUTE_ERR_GOTO(err, !found, CCS_SAMPLING_UNSUCCESSFUL, errc);
	*configuration_ret = config;
	return CCS_SUCCESS;
errc:
	ccs_release_object(config);
	return err;
}

ccs_error_t
ccs_configuration_space_samples(
	ccs_configuration_space_t configuration_space,
	size_t                    num_configurations,
	ccs_configuration_t      *configurations)
{
	CCS_CHECK_OBJ(configuration_space, CCS_CONFIGURATION_SPACE);
	CCS_CHECK_ARY(num_configurations, configurations);
	if (!num_configurations)
		return CCS_SUCCESS;
	ccs_error_t err;
	if (!configuration_space->data->graph_ok)
		CCS_VALIDATE(_generate_constraints(configuration_space));
	size_t              counter = 0;
	size_t              count   = 0;
	ccs_bool_t          found;
	ccs_configuration_t config = NULL;
	// Naive implementation
	// See below for more efficient ideas...
	for (size_t i = 0; i < num_configurations; i++)
		configurations[i] = NULL;
	while (count < num_configurations &&
	       counter < 100 * num_configurations) {
		if (!config)
			CCS_VALIDATE(ccs_create_configuration(
				configuration_space, 0, NULL, &config));
		CCS_VALIDATE_ERR_GOTO(
			err,
			_sample(configuration_space, config, &found),
			errc);
		counter++;
		if (found) {
			configurations[count++] = config;
			config                  = NULL;
		}
	}
	CCS_REFUTE(count < num_configurations, CCS_SAMPLING_UNSUCCESSFUL);
	return CCS_SUCCESS;
errc:
	ccs_release_object(config);
	return err;
}

//      UT_array *array = configuration_space->data->parameters;
//      size_t num_parameter = utarray_len(array);
//      ccs_datum_t *values = (ccs_datum_t *)calloc(1,
//      sizeof(ccs_datum_t)*num_configurations*num_parameter); ccs_datum_t
//      *p_values = values; ccs_rng_t rng = configuration_space->data->rng;
//      _ccs_parameter_wrapper_cs_t *wrapper = NULL;
//      while ( (wrapper = (_ccs_parameter_wrapper_cs_t *)utarray_next(array,
//      wrapper)) ) {
//              err = ccs_parameter_samples(wrapper->parameter,
//                                               wrapper->distribution->distribution,
//                                               rng, num_configurations,
//                                               p_values);
//              if (CCS_UNLIKELY(err)) {
//                      free(values);
//                      return err;
//              }
//              p_values += num_configurations;
//      }
//      size_t i;
//      for(i = 0; i < num_configurations; i++) {
//              err = ccs_create_configuration(configuration_space, 0, NULL,
//              configurations + i); if (CCS_UNLIKELY(err)) {
//                      free(values);
//                      for(size_t j = 0; j < i; j++)
//                              ccs_release_object(configurations + j);
//                      return err;
//              }
//      }
//      for(i = 0; i < num_configurations; i++) {
//              for(size_t j = 0; j < num_parameter; j++)
//                      configurations[i]->data->values[j] =
//                              values[j*num_configurations + i];
//
//              err = _set_actives(configuration_space, configurations[i]);
//              if (err) {
//                      free(values);
//                      for(size_t j = 0; j < num_configurations; j++)
//                              ccs_release_object(configurations + j);
//                      return err;
//              }
//      }
//      free(values);
//      return CCS_SUCCESS;

static int
_size_t_sort(const void *a, const void *b)
{
	const size_t sa = *(const size_t *)a;
	const size_t sb = *(const size_t *)b;
	return sa < sb ? -1 : sa > sb ? 1 : 0;
}

static void
_uniq_size_t_array(UT_array *array)
{
	size_t count = utarray_len(array);
	if (count == 0)
		return;
	utarray_sort(array, &_size_t_sort);
	size_t  real_count = 0;
	size_t *p          = (size_t *)utarray_front(array);
	size_t *p2         = p;
	real_count++;
	while ((p = (size_t *)utarray_next(array, p))) {
		if (*p != *p2) {
			p2  = (size_t *)utarray_next(array, p2);
			*p2 = *p;
			real_count++;
		}
	}
	utarray_resize(array, real_count);
}

struct _parameter_list_s;
struct _parameter_list_s {
	size_t                    in_edges;
	size_t                    index;
	struct _parameter_list_s *next;
	struct _parameter_list_s *prev;
};

#undef utarray_oom
#define utarray_oom()                                                          \
	{                                                                      \
		free((void *)list);                                            \
		CCS_RAISE(                                                     \
			CCS_OUT_OF_MEMORY,                                     \
			"Not enough memory to allocate array");                \
	}
static ccs_error_t
_topological_sort(ccs_configuration_space_t configuration_space)
{
	utarray_clear(configuration_space->data->sorted_indexes);
	UT_array                 *array = configuration_space->data->parameters;
	size_t                    count = utarray_len(array);

	struct _parameter_list_s *list  = (struct _parameter_list_s *)calloc(
                1, sizeof(struct _parameter_list_s) * count);
	CCS_REFUTE(!list, CCS_OUT_OF_MEMORY);
	struct _parameter_list_s    *queue   = NULL;

	_ccs_parameter_wrapper_cs_t *wrapper = NULL;
	size_t                       index   = 0;
	while ((wrapper = (_ccs_parameter_wrapper_cs_t *)utarray_next(
			array, wrapper))) {
		size_t in_edges      = utarray_len(wrapper->parents);
		list[index].in_edges = in_edges;
		list[index].index    = index;
		if (in_edges == 0)
			DL_APPEND(queue, list + index);
		index++;
	}
	size_t processed = 0;
	while (queue) {
		struct _parameter_list_s *e = queue;
		DL_DELETE(queue, queue);
		wrapper = (_ccs_parameter_wrapper_cs_t *)utarray_eltptr(
			array, e->index);
		size_t *child = NULL;
		while ((child = (size_t *)utarray_next(
				wrapper->children, child))) {
			list[*child].in_edges--;
			if (list[*child].in_edges == 0) {
				DL_APPEND(queue, list + *child);
			}
		}
		utarray_push_back(
			configuration_space->data->sorted_indexes, &(e->index));
		processed++;
	};
	free(list);
	CCS_REFUTE(processed < count, CCS_INVALID_GRAPH);
	return CCS_SUCCESS;
}

#undef utarray_oom
#define utarray_oom()                                                          \
	{                                                                      \
		CCS_RAISE_ERR_GOTO(                                            \
			err,                                                   \
			CCS_OUT_OF_MEMORY,                                     \
			errmem,                                                \
			"Not enough memory to allocate array");                \
	}
static ccs_error_t
_recompute_graph(ccs_configuration_space_t configuration_space)
{
	_ccs_parameter_wrapper_cs_t *wrapper = NULL;
	UT_array *array = configuration_space->data->parameters;
	while ((wrapper = (_ccs_parameter_wrapper_cs_t *)utarray_next(
			array, wrapper))) {
		utarray_clear(wrapper->parents);
		utarray_clear(wrapper->children);
	}
	wrapper         = NULL;
	intptr_t    mem = 0;
	ccs_error_t err = CCS_SUCCESS;
	for (size_t index = 0; index < utarray_len(array); index++) {
		wrapper = (_ccs_parameter_wrapper_cs_t *)utarray_eltptr(
			array, (unsigned int)index);
		if (!wrapper->condition)
			continue;
		size_t count;
		CCS_VALIDATE_ERR_GOTO(
			err,
			ccs_expression_get_parameters(
				wrapper->condition, 0, NULL, &count),
			errmem);
		if (count == 0)
			continue;
		ccs_parameter_t             *parents        = NULL;
		size_t                      *parents_index  = NULL;
		_ccs_parameter_wrapper_cs_t *parent_wrapper = NULL;
		intptr_t                     oldmem         = mem;
		mem                                         = (intptr_t)realloc(
                        (void *)oldmem,
                        count * (sizeof(ccs_parameter_t) + sizeof(size_t)));
		if (!mem) {
			mem = oldmem;
			CCS_RAISE_ERR_GOTO(
				err,
				CCS_OUT_OF_MEMORY,
				errmem,
				"Not enough memory to reallocate array");
		}
		parents = (ccs_parameter_t *)mem;
		parents_index =
			(size_t *)(mem + count * sizeof(ccs_parameter_t));
		CCS_VALIDATE_ERR_GOTO(
			err,
			ccs_expression_get_parameters(
				wrapper->condition, count, parents, NULL),
			errmem);
		CCS_VALIDATE_ERR_GOTO(
			err,
			ccs_configuration_space_get_parameter_indexes(
				configuration_space,
				count,
				parents,
				parents_index),
			errmem);
		for (size_t i = 0; i < count; i++) {
			utarray_push_back(wrapper->parents, parents_index + i);
			parent_wrapper =
				(_ccs_parameter_wrapper_cs_t *)utarray_eltptr(
					array, parents_index[i]);
			utarray_push_back(parent_wrapper->children, &index);
		}
	}
	wrapper = NULL;
	while ((wrapper = (_ccs_parameter_wrapper_cs_t *)utarray_next(
			array, wrapper))) {
		_uniq_size_t_array(wrapper->parents);
		_uniq_size_t_array(wrapper->children);
	}
errmem:
	if (mem)
		free((void *)mem);
	return err;
}

#undef utarray_oom
#define utarray_oom()                                                          \
	{                                                                      \
		exit(-1);                                                      \
	}

static ccs_error_t
_generate_constraints(ccs_configuration_space_t configuration_space)
{
	CCS_VALIDATE(_recompute_graph(configuration_space));
	CCS_VALIDATE(_topological_sort(configuration_space));
	configuration_space->data->graph_ok = CCS_TRUE;
	return CCS_SUCCESS;
}

ccs_error_t
ccs_configuration_space_set_condition(
	ccs_configuration_space_t configuration_space,
	size_t                    parameter_index,
	ccs_expression_t          expression)
{
	CCS_CHECK_OBJ(configuration_space, CCS_CONFIGURATION_SPACE);
	CCS_CHECK_OBJ(expression, CCS_EXPRESSION);
	_ccs_parameter_wrapper_cs_t *wrapper =
		(_ccs_parameter_wrapper_cs_t *)utarray_eltptr(
			configuration_space->data->parameters,
			(unsigned int)parameter_index);
	CCS_REFUTE(!wrapper, CCS_OUT_OF_BOUNDS);
	CCS_REFUTE(wrapper->condition, CCS_INVALID_PARAMETER);
	ccs_error_t err;
	CCS_VALIDATE(ccs_retain_object(expression));
	wrapper->condition                  = expression;
	// Recompute the whole graph for now
	configuration_space->data->graph_ok = CCS_FALSE;
	CCS_VALIDATE_ERR_GOTO(
		err, _generate_constraints(configuration_space), erre);
	return CCS_SUCCESS;
erre:
	ccs_release_object(expression);
	wrapper->condition = NULL;
	return err;
}

ccs_error_t
ccs_configuration_space_get_condition(
	ccs_configuration_space_t configuration_space,
	size_t                    parameter_index,
	ccs_expression_t         *expression_ret)
{
	CCS_CHECK_OBJ(configuration_space, CCS_CONFIGURATION_SPACE);
	CCS_CHECK_PTR(expression_ret);
	_ccs_parameter_wrapper_cs_t *wrapper =
		(_ccs_parameter_wrapper_cs_t *)utarray_eltptr(
			configuration_space->data->parameters,
			(unsigned int)parameter_index);
	CCS_REFUTE(!wrapper, CCS_OUT_OF_BOUNDS);
	*expression_ret = wrapper->condition;
	return CCS_SUCCESS;
}

ccs_error_t
ccs_configuration_space_get_conditions(
	ccs_configuration_space_t configuration_space,
	size_t                    num_expressions,
	ccs_expression_t         *expressions,
	size_t                   *num_expressions_ret)
{
	CCS_CHECK_OBJ(configuration_space, CCS_CONFIGURATION_SPACE);
	CCS_CHECK_ARY(num_expressions, expressions);
	CCS_REFUTE(!expressions && !num_expressions_ret, CCS_INVALID_VALUE);
	UT_array *array          = configuration_space->data->parameters;
	size_t    num_parameters = utarray_len(array);
	if (expressions) {
		CCS_REFUTE(num_expressions < num_parameters, CCS_INVALID_VALUE);
		_ccs_parameter_wrapper_cs_t *wrapper = NULL;
		size_t                       index   = 0;
		while ((wrapper = (_ccs_parameter_wrapper_cs_t *)utarray_next(
				array, wrapper)))
			expressions[index++] = wrapper->condition;
		for (size_t i = num_parameters; i < num_expressions; i++)
			expressions[i] = NULL;
	}
	if (num_expressions_ret)
		*num_expressions_ret = num_parameters;
	return CCS_SUCCESS;
}

#undef utarray_oom
#define utarray_oom()                                                          \
	{                                                                      \
		CCS_RAISE_ERR_GOTO(                                            \
			err,                                                   \
			CCS_OUT_OF_MEMORY,                                     \
			end,                                                   \
			"Out of memory to allocate array");                    \
	}
ccs_error_t
ccs_configuration_space_add_forbidden_clause(
	ccs_configuration_space_t configuration_space,
	ccs_expression_t          expression)
{
	CCS_CHECK_OBJ(configuration_space, CCS_CONFIGURATION_SPACE);
	CCS_CHECK_OBJ(expression, CCS_EXPRESSION);
	ccs_error_t err = CCS_SUCCESS;
	CCS_VALIDATE(ccs_expression_check_context(
		expression, (ccs_context_t)configuration_space));
	ccs_datum_t         d;
	ccs_configuration_t config;
	CCS_VALIDATE(ccs_configuration_space_get_default_configuration(
		configuration_space, &config));

	CCS_VALIDATE_ERR_GOTO(
		err,
		ccs_expression_eval(
			expression,
			(ccs_context_t)configuration_space,
			config->data->values,
			&d),
		end);
	if (d.type == CCS_BOOLEAN && d.value.i == CCS_TRUE)
		CCS_RAISE_ERR_GOTO(
			err,
			CCS_INVALID_CONFIGURATION,
			end,
			"Default configuration is invalid");
	CCS_VALIDATE_ERR_GOTO(err, ccs_retain_object(expression), end);
	utarray_push_back(
		configuration_space->data->forbidden_clauses, &expression);
end:
	ccs_release_object(config);
	return err;
}

#undef utarray_oom
#define utarray_oom() exit(-1)

ccs_error_t
ccs_configuration_space_add_forbidden_clauses(
	ccs_configuration_space_t configuration_space,
	size_t                    num_expressions,
	ccs_expression_t         *expressions)
{
	CCS_CHECK_OBJ(configuration_space, CCS_CONFIGURATION_SPACE);
	CCS_CHECK_ARY(num_expressions, expressions);
	for (size_t i = 0; i < num_expressions; i++)
		CCS_VALIDATE(ccs_configuration_space_add_forbidden_clause(
			configuration_space, expressions[i]));
	return CCS_SUCCESS;
}

ccs_error_t
ccs_configuration_space_get_forbidden_clause(
	ccs_configuration_space_t configuration_space,
	size_t                    index,
	ccs_expression_t         *expression_ret)
{
	CCS_CHECK_OBJ(configuration_space, CCS_CONFIGURATION_SPACE);
	CCS_CHECK_PTR(expression_ret);
	ccs_expression_t *p_expr = (ccs_expression_t *)utarray_eltptr(
		configuration_space->data->forbidden_clauses,
		(unsigned int)index);
	CCS_REFUTE(!p_expr, CCS_OUT_OF_BOUNDS);
	*expression_ret = *p_expr;
	return CCS_SUCCESS;
}

ccs_error_t
ccs_configuration_space_get_forbidden_clauses(
	ccs_configuration_space_t configuration_space,
	size_t                    num_expressions,
	ccs_expression_t         *expressions,
	size_t                   *num_expressions_ret)
{
	CCS_CHECK_OBJ(configuration_space, CCS_CONFIGURATION_SPACE);
	CCS_CHECK_ARY(num_expressions, expressions);
	CCS_REFUTE(!expressions && !num_expressions_ret, CCS_INVALID_VALUE);
	UT_array *array = configuration_space->data->forbidden_clauses;
	size_t    num_forbidden_clauses = utarray_len(array);
	if (expressions) {
		CCS_REFUTE(
			num_expressions < num_forbidden_clauses,
			CCS_INVALID_VALUE);
		ccs_expression_t *p_expr = NULL;
		size_t            index  = 0;
		while ((p_expr = (ccs_expression_t *)utarray_next(
				array, p_expr)))
			expressions[index++] = *p_expr;
		for (size_t i = num_forbidden_clauses; i < num_expressions; i++)
			expressions[i] = NULL;
	}
	if (num_expressions_ret)
		*num_expressions_ret = num_forbidden_clauses;
	return CCS_SUCCESS;
}
