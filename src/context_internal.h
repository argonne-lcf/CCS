#ifndef _CONTEXT_INTERNAL_H
#define _CONTEXT_INTERNAL_H
#include "utarray.h"
#ifdef HASH_NONFATAL_OOM
#undef HASH_NONFATAL_OOM
#endif
#define HASH_NONFATAL_OOM 1
#include "uthash.h"
#include "hyperparameter_internal.h"

typedef struct _ccs_context_data_s _ccs_context_data_t;

struct _ccs_context_ops_s {
	_ccs_object_ops_t obj_ops;
};
typedef struct _ccs_context_ops_s _ccs_context_ops_t;

struct _ccs_hyperparameter_wrapper_s {
	ccs_hyperparameter_t         hyperparameter;
};
typedef struct _ccs_hyperparameter_wrapper_s _ccs_hyperparameter_wrapper_t;

struct _ccs_hyperparameter_index_hash_s {
	ccs_hyperparameter_t  hyperparameter;
	const char           *name;
	size_t                index;
	UT_hash_handle        hh_name;
	UT_hash_handle        hh_handle;
};
typedef struct _ccs_hyperparameter_index_hash_s _ccs_hyperparameter_index_hash_t;

struct _ccs_context_data_s {
	const char                       *name;
	UT_array                         *hyperparameters;
	_ccs_hyperparameter_index_hash_t *name_hash;
	_ccs_hyperparameter_index_hash_t *handle_hash;
};

struct _ccs_context_s {
	_ccs_object_internal_t  obj;
	_ccs_context_data_t    *data;
};

static inline ccs_result_t
_ccs_context_get_hyperparameter_index(
		ccs_context_t         context,
		ccs_hyperparameter_t  hyperparameter,
		size_t               *index_ret) {
	CCS_CHECK_PTR(index_ret);
	_ccs_context_data_t *data = context->data;
	_ccs_hyperparameter_index_hash_t *wrapper;
	HASH_FIND(hh_handle, data->handle_hash, &hyperparameter,
	          sizeof(ccs_hyperparameter_t), wrapper);
	if (!wrapper)
		return -CCS_INVALID_HYPERPARAMETER;
	*index_ret = wrapper->index;
	return CCS_SUCCESS;
}

static inline ccs_result_t
_ccs_context_get_num_hyperparameters(
		ccs_context_t  context,
		size_t        *num_hyperparameters_ret) {
	CCS_CHECK_PTR(num_hyperparameters_ret);
	*num_hyperparameters_ret = utarray_len(context->data->hyperparameters);
	return CCS_SUCCESS;
}

static inline ccs_result_t
_ccs_context_get_hyperparameter(
		ccs_context_t         context,
		size_t                index,
		ccs_hyperparameter_t *hyperparameter_ret) {
	CCS_CHECK_PTR(hyperparameter_ret);
	_ccs_hyperparameter_wrapper_t *wrapper = (_ccs_hyperparameter_wrapper_t*)
	    utarray_eltptr(context->data->hyperparameters, (unsigned int)index);
	if (!wrapper)
		return -CCS_OUT_OF_BOUNDS;
	*hyperparameter_ret = wrapper->hyperparameter;
	return CCS_SUCCESS;
}

static inline ccs_result_t
_ccs_context_get_hyperparameter_by_name(
		ccs_context_t         context,
		const char *          name,
		ccs_hyperparameter_t *hyperparameter_ret) {
	CCS_CHECK_PTR(name);
	CCS_CHECK_PTR(hyperparameter_ret);
	_ccs_hyperparameter_index_hash_t *wrapper;
	size_t sz_name;
	sz_name = strlen(name);
	HASH_FIND(hh_name, context->data->name_hash,
	          name, sz_name, wrapper);
	if (!wrapper)
		return -CCS_INVALID_NAME;
	*hyperparameter_ret = wrapper->hyperparameter;
	return CCS_SUCCESS;
}

static inline ccs_result_t
_ccs_context_get_hyperparameter_index_by_name(
		ccs_context_t  context,
		const char    *name,
		size_t        *index_ret) {
	CCS_CHECK_PTR(name);
	CCS_CHECK_PTR(index_ret);
	_ccs_hyperparameter_index_hash_t *wrapper;
	size_t sz_name;
	sz_name = strlen(name);
	HASH_FIND(hh_name, context->data->name_hash,
	          name, sz_name, wrapper);
	if (!wrapper)
		return -CCS_INVALID_NAME;
	*index_ret = wrapper->index;
	return CCS_SUCCESS;
}

static inline ccs_result_t
_ccs_context_get_hyperparameters(
		ccs_context_t          context,
		size_t                 num_hyperparameters,
		ccs_hyperparameter_t  *hyperparameters,
		size_t                *num_hyperparameters_ret) {
	CCS_CHECK_ARY(num_hyperparameters, hyperparameters);
	if (!num_hyperparameters_ret && !hyperparameters)
		return -CCS_INVALID_VALUE;
	UT_array *array = context->data->hyperparameters;
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

static inline ccs_result_t
_ccs_context_get_hyperparameter_indexes(
		ccs_context_t          context,
		size_t                 num_hyperparameters,
		ccs_hyperparameter_t  *hyperparameters,
		size_t                *indexes) {
	CCS_CHECK_ARY(num_hyperparameters, hyperparameters);
	CCS_CHECK_ARY(num_hyperparameters, indexes);
	_ccs_hyperparameter_index_hash_t *wrapper;
	for(size_t i = 0; i < num_hyperparameters; i++) {
		HASH_FIND(hh_handle, context->data->handle_hash,
			hyperparameters + i, sizeof(ccs_hyperparameter_t), wrapper);
		if (!wrapper)
			return -CCS_INVALID_HYPERPARAMETER;
		indexes[i] = wrapper->index;
	}
	return CCS_SUCCESS;
}

static inline ccs_result_t
_ccs_context_get_name(ccs_context_t   context,
                      const char    **name_ret) {
	CCS_CHECK_PTR(name_ret);
	*name_ret = context->data->name;
	return CCS_SUCCESS;
}

static inline ccs_result_t
_ccs_context_validate_value(ccs_context_t  context,
                            size_t         index,
                            ccs_datum_t    value,
                            ccs_datum_t   *value_ret) {
	CCS_CHECK_PTR(value_ret);
	_ccs_hyperparameter_wrapper_t *wrapper = (_ccs_hyperparameter_wrapper_t*)
	    utarray_eltptr(context->data->hyperparameters, (unsigned int)index);
	if (!wrapper)
		return -CCS_OUT_OF_BOUNDS;
	ccs_bool_t valid;
	CCS_VALIDATE(ccs_hyperparameter_validate_value(wrapper->hyperparameter,
	                                               value, value_ret, &valid));
	if (!valid)
		return -CCS_INVALID_VALUE;
	return CCS_SUCCESS;
}

static inline ccs_result_t
_ccs_serialize_bin_size_ccs_context_data(
		_ccs_context_data_t             *data,
		size_t                          *cum_size,
		_ccs_object_serialize_options_t *opts) {
	*cum_size += _ccs_serialize_bin_size_string(data->name);
	*cum_size += _ccs_serialize_bin_size_uint64(
		utarray_len(data->hyperparameters));
	_ccs_hyperparameter_wrapper_t *wrapper = NULL;
	while ( (wrapper = (_ccs_hyperparameter_wrapper_t *)utarray_next(data->hyperparameters, wrapper)) )
		CCS_VALIDATE(wrapper->hyperparameter->obj.ops->serialize_size(
			wrapper->hyperparameter, CCS_SERIALIZE_FORMAT_BINARY, cum_size, opts));
	return CCS_SUCCESS;
}

static inline ccs_result_t
_ccs_serialize_bin_ccs_context_data(
		_ccs_context_data_t              *data,
		size_t                           *buffer_size,
		char                            **buffer,
		_ccs_object_serialize_options_t  *opts) {
	CCS_VALIDATE(_ccs_serialize_bin_string(
		data->name, buffer_size, buffer));
	CCS_VALIDATE(_ccs_serialize_bin_uint64(
		utarray_len(data->hyperparameters), buffer_size, buffer));
	_ccs_hyperparameter_wrapper_t *wrapper = NULL;
	while ( (wrapper = (_ccs_hyperparameter_wrapper_t *)utarray_next(data->hyperparameters, wrapper)) )
		CCS_VALIDATE(wrapper->hyperparameter->obj.ops->serialize(
			wrapper->hyperparameter, CCS_SERIALIZE_FORMAT_BINARY, buffer_size, buffer, opts));
	return CCS_SUCCESS;
}

static inline ccs_result_t
_ccs_serialize_bin_size_ccs_context(
		ccs_context_t                    context,
		size_t                          *cum_size,
		_ccs_object_serialize_options_t *opts) {
	*cum_size += _ccs_serialize_bin_size_ccs_object_internal(
		(_ccs_object_internal_t *)context);
	CCS_VALIDATE(_ccs_serialize_bin_size_ccs_context_data(
		context->data, cum_size, opts));
	return CCS_SUCCESS;
}

static inline ccs_result_t
_ccs_serialize_bin_ccs_context(
		ccs_context_t                     context,
		size_t                           *buffer_size,
		char                            **buffer,
		_ccs_object_serialize_options_t  *opts) {
	CCS_VALIDATE(_ccs_serialize_bin_ccs_object_internal(
		(_ccs_object_internal_t *)context, buffer_size, buffer));
	CCS_VALIDATE(_ccs_serialize_bin_ccs_context_data(
		context->data, buffer_size, buffer, opts));
	return CCS_SUCCESS;
}

#endif //_CONTEXT_INTERNAL_H
