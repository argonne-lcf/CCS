#ifndef _CONTEXT_INTERNAL_H
#define _CONTEXT_INTERNAL_H
#include "utarray.h"
#ifdef HASH_NONFATAL_OOM
#undef HASH_NONFATAL_OOM
#endif
#define HASH_NONFATAL_OOM 1
#include "uthash.h"
#include "parameter_internal.h"

typedef struct _ccs_context_data_s _ccs_context_data_t;

struct _ccs_context_ops_s {
	_ccs_object_ops_t obj_ops;
};
typedef struct _ccs_context_ops_s _ccs_context_ops_t;

struct _ccs_parameter_wrapper_s {
	ccs_parameter_t         parameter;
};
typedef struct _ccs_parameter_wrapper_s _ccs_parameter_wrapper_t;

struct _ccs_parameter_index_hash_s {
	ccs_parameter_t  parameter;
	const char           *name;
	size_t                index;
	UT_hash_handle        hh_name;
	UT_hash_handle        hh_handle;
};
typedef struct _ccs_parameter_index_hash_s _ccs_parameter_index_hash_t;

struct _ccs_context_data_s {
	const char                       *name;
	UT_array                         *parameters;
	_ccs_parameter_index_hash_t *name_hash;
	_ccs_parameter_index_hash_t *handle_hash;
};

struct _ccs_context_s {
	_ccs_object_internal_t  obj;
	_ccs_context_data_t    *data;
};

static inline ccs_error_t
_ccs_context_get_parameter_index(
		ccs_context_t         context,
		ccs_parameter_t  parameter,
		size_t               *index_ret) {
	CCS_CHECK_PTR(index_ret);
	_ccs_context_data_t *data = context->data;
	_ccs_parameter_index_hash_t *wrapper;
	HASH_FIND(hh_handle, data->handle_hash, &parameter,
	          sizeof(ccs_parameter_t), wrapper);
	CCS_REFUTE(!wrapper, CCS_INVALID_PARAMETER);
	*index_ret = wrapper->index;
	return CCS_SUCCESS;
}

static inline ccs_error_t
_ccs_context_get_num_parameters(
		ccs_context_t  context,
		size_t        *num_parameters_ret) {
	CCS_CHECK_PTR(num_parameters_ret);
	*num_parameters_ret = utarray_len(context->data->parameters);
	return CCS_SUCCESS;
}

static inline ccs_error_t
_ccs_context_get_parameter(
		ccs_context_t         context,
		size_t                index,
		ccs_parameter_t *parameter_ret) {
	CCS_CHECK_PTR(parameter_ret);
	_ccs_parameter_wrapper_t *wrapper = (_ccs_parameter_wrapper_t*)
	    utarray_eltptr(context->data->parameters, (unsigned int)index);
	CCS_REFUTE(!wrapper, CCS_OUT_OF_BOUNDS);
	*parameter_ret = wrapper->parameter;
	return CCS_SUCCESS;
}

static inline ccs_error_t
_ccs_context_get_parameter_by_name(
		ccs_context_t         context,
		const char *          name,
		ccs_parameter_t *parameter_ret) {
	CCS_CHECK_PTR(name);
	CCS_CHECK_PTR(parameter_ret);
	_ccs_parameter_index_hash_t *wrapper;
	size_t sz_name;
	sz_name = strlen(name);
	HASH_FIND(hh_name, context->data->name_hash,
	          name, sz_name, wrapper);
	CCS_REFUTE(!wrapper, CCS_INVALID_NAME);
	*parameter_ret = wrapper->parameter;
	return CCS_SUCCESS;
}

static inline ccs_error_t
_ccs_context_get_parameter_index_by_name(
		ccs_context_t  context,
		const char    *name,
		size_t        *index_ret) {
	CCS_CHECK_PTR(name);
	CCS_CHECK_PTR(index_ret);
	_ccs_parameter_index_hash_t *wrapper;
	size_t sz_name;
	sz_name = strlen(name);
	HASH_FIND(hh_name, context->data->name_hash,
	          name, sz_name, wrapper);
	CCS_REFUTE(!wrapper, CCS_INVALID_NAME);
	*index_ret = wrapper->index;
	return CCS_SUCCESS;
}

static inline ccs_error_t
_ccs_context_get_parameters(
		ccs_context_t          context,
		size_t                 num_parameters,
		ccs_parameter_t  *parameters,
		size_t                *num_parameters_ret) {
	CCS_CHECK_ARY(num_parameters, parameters);
	CCS_REFUTE(!num_parameters_ret && !parameters, CCS_INVALID_VALUE);
	UT_array *array = context->data->parameters;
	size_t size = utarray_len(array);
	if (parameters) {
		CCS_REFUTE(num_parameters < size, CCS_INVALID_VALUE);
		_ccs_parameter_wrapper_t *wrapper = NULL;
		size_t index = 0;
		while ( (wrapper = (_ccs_parameter_wrapper_t *)utarray_next(array, wrapper)) )
			parameters[index++] = wrapper->parameter;
		for (size_t i = size; i < num_parameters; i++)
			parameters[i] = NULL;
	}
	if (num_parameters_ret)
		*num_parameters_ret = size;
	return CCS_SUCCESS;
}

static inline ccs_error_t
_ccs_context_get_parameter_indexes(
		ccs_context_t          context,
		size_t                 num_parameters,
		ccs_parameter_t  *parameters,
		size_t                *indexes) {
	CCS_CHECK_ARY(num_parameters, parameters);
	CCS_CHECK_ARY(num_parameters, indexes);
	_ccs_parameter_index_hash_t *wrapper;
	for(size_t i = 0; i < num_parameters; i++) {
		HASH_FIND(hh_handle, context->data->handle_hash,
			parameters + i, sizeof(ccs_parameter_t), wrapper);
		CCS_REFUTE(!wrapper, CCS_INVALID_PARAMETER);
		indexes[i] = wrapper->index;
	}
	return CCS_SUCCESS;
}

static inline ccs_error_t
_ccs_context_get_name(ccs_context_t   context,
                      const char    **name_ret) {
	CCS_CHECK_PTR(name_ret);
	*name_ret = context->data->name;
	return CCS_SUCCESS;
}

static inline ccs_error_t
_ccs_context_validate_value(ccs_context_t  context,
                            size_t         index,
                            ccs_datum_t    value,
                            ccs_datum_t   *value_ret) {
	CCS_CHECK_PTR(value_ret);
	_ccs_parameter_wrapper_t *wrapper = (_ccs_parameter_wrapper_t*)
	    utarray_eltptr(context->data->parameters, (unsigned int)index);
	CCS_REFUTE(!wrapper, CCS_OUT_OF_BOUNDS);
	ccs_bool_t valid;
	CCS_VALIDATE(ccs_parameter_validate_value(wrapper->parameter,
	                                               value, value_ret, &valid));
	CCS_REFUTE(!valid, CCS_INVALID_VALUE);
	return CCS_SUCCESS;
}

static inline ccs_error_t
_ccs_serialize_bin_size_ccs_context_data(
		_ccs_context_data_t             *data,
		size_t                          *cum_size,
		_ccs_object_serialize_options_t *opts) {
	*cum_size += _ccs_serialize_bin_size_string(data->name);
	*cum_size += _ccs_serialize_bin_size_size(
		utarray_len(data->parameters));
	_ccs_parameter_wrapper_t *wrapper = NULL;
	while ( (wrapper = (_ccs_parameter_wrapper_t *)utarray_next(data->parameters, wrapper)) )
		CCS_VALIDATE(wrapper->parameter->obj.ops->serialize_size(
			wrapper->parameter, CCS_SERIALIZE_FORMAT_BINARY, cum_size, opts));
	return CCS_SUCCESS;
}

static inline ccs_error_t
_ccs_serialize_bin_ccs_context_data(
		_ccs_context_data_t              *data,
		size_t                           *buffer_size,
		char                            **buffer,
		_ccs_object_serialize_options_t  *opts) {
	CCS_VALIDATE(_ccs_serialize_bin_string(
		data->name, buffer_size, buffer));
	CCS_VALIDATE(_ccs_serialize_bin_size(
		utarray_len(data->parameters), buffer_size, buffer));
	_ccs_parameter_wrapper_t *wrapper = NULL;
	while ( (wrapper = (_ccs_parameter_wrapper_t *)utarray_next(data->parameters, wrapper)) )
		CCS_VALIDATE(wrapper->parameter->obj.ops->serialize(
			wrapper->parameter, CCS_SERIALIZE_FORMAT_BINARY, buffer_size, buffer, opts));
	return CCS_SUCCESS;
}

static inline ccs_error_t
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

static inline ccs_error_t
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
