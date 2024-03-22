#ifndef _CONTEXT_INTERNAL_H
#define _CONTEXT_INTERNAL_H
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

struct _ccs_parameter_index_hash_s {
	ccs_parameter_t parameter;
	const char     *name;
	size_t          index;
	UT_hash_handle  hh_name;
	UT_hash_handle  hh_handle;
};
typedef struct _ccs_parameter_index_hash_s _ccs_parameter_index_hash_t;

struct _ccs_context_data_s {
	const char                  *name;
	size_t                       num_parameters;
	ccs_parameter_t             *parameters;
	_ccs_parameter_index_hash_t *hash_elems;
	_ccs_parameter_index_hash_t *name_hash;
	_ccs_parameter_index_hash_t *handle_hash;
};

struct _ccs_context_s {
	_ccs_object_internal_t obj;
	_ccs_context_data_t   *data;
};

static inline ccs_result_t
_ccs_context_get_parameter_index(
	ccs_context_t   context,
	ccs_parameter_t parameter,
	ccs_bool_t     *found_ret,
	size_t         *index_ret)
{
	CCS_CHECK_PTR(index_ret);
	_ccs_context_data_t         *data = context->data;
	_ccs_parameter_index_hash_t *wrapper;
	HASH_FIND(
		hh_handle, data->handle_hash, &parameter,
		sizeof(ccs_parameter_t), wrapper);
	if (!found_ret) {
		CCS_REFUTE(!wrapper, CCS_RESULT_ERROR_INVALID_PARAMETER);
		*index_ret = wrapper->index;
	} else {
		if (wrapper) {
			*found_ret = CCS_TRUE;
			*index_ret = wrapper->index;
		} else {
			*found_ret = CCS_FALSE;
		}
	}
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_context_get_num_parameters(
	ccs_context_t context,
	size_t       *num_parameters_ret)
{
	CCS_CHECK_PTR(num_parameters_ret);
	*num_parameters_ret = context->data->num_parameters;
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_context_get_parameter(
	ccs_context_t    context,
	size_t           index,
	ccs_parameter_t *parameter_ret)
{
	CCS_CHECK_PTR(parameter_ret);
	CCS_REFUTE(
		index >= context->data->num_parameters,
		CCS_RESULT_ERROR_OUT_OF_BOUNDS);
	*parameter_ret = context->data->parameters[index];
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_context_get_parameter_by_name(
	ccs_context_t    context,
	const char      *name,
	ccs_parameter_t *parameter_ret)
{
	CCS_CHECK_PTR(name);
	CCS_CHECK_PTR(parameter_ret);
	_ccs_parameter_index_hash_t *wrapper;
	size_t                       sz_name;
	sz_name = strlen(name);
	HASH_FIND(hh_name, context->data->name_hash, name, sz_name, wrapper);
	*parameter_ret = wrapper ? wrapper->parameter : NULL;
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_context_get_parameter_index_by_name(
	ccs_context_t context,
	const char   *name,
	ccs_bool_t   *found_ret,
	size_t       *index_ret)
{
	CCS_CHECK_PTR(name);
	CCS_CHECK_PTR(index_ret);
	_ccs_parameter_index_hash_t *wrapper;
	size_t                       sz_name;
	sz_name = strlen(name);
	HASH_FIND(hh_name, context->data->name_hash, name, sz_name, wrapper);
	if (!found_ret) {
		CCS_REFUTE(!wrapper, CCS_RESULT_ERROR_INVALID_NAME);
		*index_ret = wrapper->index;
	} else {
		if (wrapper) {
			*found_ret = CCS_TRUE;
			*index_ret = wrapper->index;
		} else {
			*found_ret = CCS_FALSE;
		}
	}
	return CCS_RESULT_SUCCESS;
}

#undef uthash_nonfatal_oom
#define uthash_nonfatal_oom(elt)                                               \
	{                                                                      \
		CCS_RAISE(                                                     \
			CCS_RESULT_ERROR_OUT_OF_MEMORY,                        \
			"Not enough memory to allocate hash");                 \
	}
static inline ccs_result_t
_ccs_context_add_parameter(
	ccs_context_t   context,
	ccs_parameter_t parameter,
	size_t          index)
{
	const char                  *name;
	size_t                       sz_name;
	_ccs_parameter_index_hash_t *parameter_hash;
	CCS_VALIDATE(ccs_parameter_get_name(parameter, &name));
	sz_name = strlen(name);
	HASH_FIND(
		hh_name, context->data->name_hash, name, sz_name,
		parameter_hash);
	CCS_REFUTE_MSG(
		parameter_hash, CCS_RESULT_ERROR_INVALID_PARAMETER,
		"Duplicate parameter name '%s' found", name);

	parameter_hash            = context->data->hash_elems + index;
	parameter_hash->parameter = parameter;
	parameter_hash->name      = name;
	parameter_hash->index     = index;

	HASH_ADD(
		hh_handle, context->data->handle_hash, parameter,
		sizeof(ccs_parameter_t), parameter_hash);
	HASH_ADD_KEYPTR(
		hh_name, context->data->name_hash, parameter_hash->name,
		sz_name, parameter_hash);

	CCS_VALIDATE(ccs_retain_object(parameter));
	context->data->parameters[index] = parameter;
	return CCS_RESULT_SUCCESS;
}
#undef uthash_nonfatal_oom

static inline ccs_result_t
_ccs_context_get_parameters(
	ccs_context_t    context,
	size_t           num_parameters,
	ccs_parameter_t *parameters,
	size_t          *num_parameters_ret)
{
	CCS_CHECK_ARY(num_parameters, parameters);
	CCS_REFUTE(
		!num_parameters_ret && !parameters,
		CCS_RESULT_ERROR_INVALID_VALUE);
	ccs_parameter_t *params = context->data->parameters;
	size_t           size   = context->data->num_parameters;
	if (parameters) {
		CCS_REFUTE(
			num_parameters < size, CCS_RESULT_ERROR_INVALID_VALUE);
		for (size_t i = 0; i < size; i++)
			parameters[i] = params[i];
		for (size_t i = size; i < num_parameters; i++)
			parameters[i] = NULL;
	}
	if (num_parameters_ret)
		*num_parameters_ret = size;
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_context_get_parameter_indexes(
	ccs_context_t    context,
	size_t           num_parameters,
	ccs_parameter_t *parameters,
	ccs_bool_t      *found,
	size_t          *indexes)
{
	CCS_CHECK_ARY(num_parameters, parameters);
	CCS_CHECK_ARY(num_parameters, indexes);
	_ccs_parameter_index_hash_t *wrapper;
	if (found) {
		for (size_t i = 0; i < num_parameters; i++) {
			HASH_FIND(
				hh_handle, context->data->handle_hash,
				parameters + i, sizeof(ccs_parameter_t),
				wrapper);
			if (wrapper) {
				found[i]   = CCS_TRUE;
				indexes[i] = wrapper->index;
			} else {
				found[i] = CCS_FALSE;
			}
		}
	} else {
		for (size_t i = 0; i < num_parameters; i++) {
			HASH_FIND(
				hh_handle, context->data->handle_hash,
				parameters + i, sizeof(ccs_parameter_t),
				wrapper);
			CCS_REFUTE(
				!wrapper, CCS_RESULT_ERROR_INVALID_PARAMETER);
			indexes[i] = wrapper->index;
		}
	}
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_context_get_name(ccs_context_t context, const char **name_ret)
{
	CCS_CHECK_PTR(name_ret);
	*name_ret = context->data->name;
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_context_validate_value(
	ccs_context_t context,
	size_t        index,
	ccs_datum_t   value,
	ccs_datum_t  *value_ret)
{
	CCS_CHECK_PTR(value_ret);
	CCS_REFUTE(
		index >= context->data->num_parameters,
		CCS_RESULT_ERROR_OUT_OF_BOUNDS);
	ccs_bool_t valid;
	CCS_VALIDATE(ccs_parameter_validate_value(
		context->data->parameters[index], value, value_ret, &valid));
	CCS_REFUTE(!valid, CCS_RESULT_ERROR_INVALID_VALUE);
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_serialize_bin_size_ccs_context_data(
	_ccs_context_data_t             *data,
	size_t                          *cum_size,
	_ccs_object_serialize_options_t *opts)
{
	*cum_size += _ccs_serialize_bin_size_string(data->name);
	*cum_size += _ccs_serialize_bin_size_size(data->num_parameters);
	for (size_t i = 0; i < data->num_parameters; i++)
		CCS_VALIDATE(data->parameters[i]->obj.ops->serialize_size(
			data->parameters[i], CCS_SERIALIZE_FORMAT_BINARY,
			cum_size, opts));
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_serialize_bin_ccs_context_data(
	_ccs_context_data_t             *data,
	size_t                          *buffer_size,
	char                           **buffer,
	_ccs_object_serialize_options_t *opts)
{
	CCS_VALIDATE(
		_ccs_serialize_bin_string(data->name, buffer_size, buffer));
	CCS_VALIDATE(_ccs_serialize_bin_size(
		data->num_parameters, buffer_size, buffer));
	for (size_t i = 0; i < data->num_parameters; i++)
		CCS_VALIDATE(data->parameters[i]->obj.ops->serialize(
			data->parameters[i], CCS_SERIALIZE_FORMAT_BINARY,
			buffer_size, buffer, opts));
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_serialize_bin_size_ccs_context(
	ccs_context_t                    context,
	size_t                          *cum_size,
	_ccs_object_serialize_options_t *opts)
{
	*cum_size += _ccs_serialize_bin_size_ccs_object_internal(
		(_ccs_object_internal_t *)context);
	CCS_VALIDATE(_ccs_serialize_bin_size_ccs_context_data(
		context->data, cum_size, opts));
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_serialize_bin_ccs_context(
	ccs_context_t                    context,
	size_t                          *buffer_size,
	char                           **buffer,
	_ccs_object_serialize_options_t *opts)
{
	CCS_VALIDATE(_ccs_serialize_bin_ccs_object_internal(
		(_ccs_object_internal_t *)context, buffer_size, buffer));
	CCS_VALIDATE(_ccs_serialize_bin_ccs_context_data(
		context->data, buffer_size, buffer, opts));
	return CCS_RESULT_SUCCESS;
}

#endif //_CONTEXT_INTERNAL_H
