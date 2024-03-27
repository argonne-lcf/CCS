#ifndef _BINDING_INTERNAL_H
#define _BINDING_INTERNAL_H
#include "datum_hash.h"

typedef struct _ccs_binding_data_s _ccs_binding_data_t;

struct _ccs_binding_ops_s {
	_ccs_object_ops_t obj_ops;

	ccs_result_t (*hash)(ccs_binding_t binding, ccs_hash_t *hash_ret);

	ccs_result_t (
		*cmp)(ccs_binding_t binding, ccs_binding_t other, int *cmp_ret);
};
typedef struct _ccs_binding_ops_s _ccs_binding_ops_t;

struct _ccs_binding_data_s {
	ccs_context_t context;
	size_t        num_values;
	ccs_datum_t  *values;
};

struct _ccs_binding_s {
	_ccs_object_internal_t obj;
	_ccs_binding_data_t   *data;
};

static inline ccs_result_t
_ccs_binding_get_context(ccs_binding_t binding, ccs_context_t *context_ret)
{
	CCS_CHECK_PTR(context_ret);
	*context_ret = binding->data->context;
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_binding_get_value(
	ccs_binding_t binding,
	size_t        index,
	ccs_datum_t  *value_ret)
{
	CCS_CHECK_PTR(value_ret);
	CCS_REFUTE(
		index >= binding->data->num_values,
		CCS_RESULT_ERROR_OUT_OF_BOUNDS);
	*value_ret = binding->data->values[index];
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_binding_get_values(
	ccs_binding_t binding,
	size_t        num_values,
	ccs_datum_t  *values,
	size_t       *num_values_ret)
{
	CCS_CHECK_ARY(num_values, values);
	CCS_REFUTE(!num_values_ret && !values, CCS_RESULT_ERROR_INVALID_VALUE);
	size_t num = binding->data->num_values;
	if (values) {
		CCS_REFUTE(num_values < num, CCS_RESULT_ERROR_INVALID_VALUE);
		memcpy(values, binding->data->values,
		       num * sizeof(ccs_datum_t));
		for (size_t i = num; i < num_values; i++) {
			values[i].type    = CCS_DATA_TYPE_NONE;
			values[i].value.i = 0;
		}
	}
	if (num_values_ret)
		*num_values_ret = num;
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_binding_get_value_by_name(
	ccs_binding_t binding,
	const char   *name,
	ccs_bool_t   *found_ret,
	ccs_datum_t  *value_ret)
{
	CCS_CHECK_PTR(name);
	size_t index;
	CCS_VALIDATE(ccs_context_get_parameter_index_by_name(
		binding->data->context, name, found_ret, &index));
	if (found_ret && !*found_ret)
		return CCS_RESULT_SUCCESS;
	CCS_VALIDATE(_ccs_binding_get_value(binding, index, value_ret));
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_binding_get_value_by_parameter(
	ccs_binding_t   binding,
	ccs_parameter_t parameter,
	ccs_bool_t     *found_ret,
	ccs_datum_t    *value_ret)
{
	size_t index;
	CCS_VALIDATE(ccs_context_get_parameter_index(
		binding->data->context, parameter, found_ret, &index));
	if (found_ret && !*found_ret)
		return CCS_RESULT_SUCCESS;
	CCS_VALIDATE(_ccs_binding_get_value(binding, index, value_ret));
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_binding_hash(ccs_binding_t binding, ccs_hash_t *hash_ret)
{
	_ccs_binding_data_t *data = binding->data;
	ccs_hash_t           h, ht;
	HASH_JEN(&(data->context), sizeof(data->context), h);
	HASH_JEN(&(data->num_values), sizeof(data->num_values), ht);
	h = _hash_combine(h, ht);
	for (size_t i = 0; i < data->num_values; i++) {
		ht = _hash_datum(data->values + i);
		h  = _hash_combine(h, ht);
	}
	*hash_ret = h;
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_binding_cmp(
	ccs_binding_t binding,
	ccs_binding_t other_binding,
	int          *cmp_ret)
{
	_ccs_binding_data_t *data       = binding->data;
	_ccs_binding_data_t *other_data = other_binding->data;
	if (data == other_data) {
		*cmp_ret = 0;
		return CCS_RESULT_SUCCESS;
	}
	*cmp_ret = data->context < other_data->context ? -1 :
		   data->context > other_data->context ? 1 :
							 0;
	if (*cmp_ret)
		return CCS_RESULT_SUCCESS;
	*cmp_ret = data->num_values < other_data->num_values ? -1 :
		   data->num_values > other_data->num_values ? 1 :
							       0;
	if (*cmp_ret)
		return CCS_RESULT_SUCCESS;
	for (size_t i = 0; i < data->num_values; i++) {
		if ((*cmp_ret = _datum_cmp(
			     data->values + i, other_data->values + i)))
			return CCS_RESULT_SUCCESS;
	}
	return CCS_RESULT_SUCCESS;
}

static inline size_t
_ccs_serialize_bin_size_ccs_binding_data(_ccs_binding_data_t *data)
{
	size_t sz = _ccs_serialize_bin_size_ccs_object(data->context);
	sz += _ccs_serialize_bin_size_size(data->num_values);
	for (size_t i = 0; i < data->num_values; i++)
		sz += _ccs_serialize_bin_size_ccs_datum(data->values[i]);
	return sz;
}

static inline ccs_result_t
_ccs_serialize_bin_ccs_binding_data(
	_ccs_binding_data_t *data,
	size_t              *buffer_size,
	char               **buffer)
{
	CCS_VALIDATE(_ccs_serialize_bin_ccs_object(
		data->context, buffer_size, buffer));
	CCS_VALIDATE(
		_ccs_serialize_bin_size(data->num_values, buffer_size, buffer));
	for (size_t i = 0; i < data->num_values; i++)
		CCS_VALIDATE(_ccs_serialize_bin_ccs_datum(
			data->values[i], buffer_size, buffer));
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_serialize_bin_size_ccs_binding(ccs_binding_t binding, size_t *cum_size)
{
	*cum_size += _ccs_serialize_bin_size_ccs_object_internal(
		(_ccs_object_internal_t *)binding);
	*cum_size += _ccs_serialize_bin_size_ccs_binding_data(binding->data);
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_serialize_bin_ccs_binding(
	ccs_binding_t binding,
	size_t       *buffer_size,
	char        **buffer)
{
	CCS_VALIDATE(_ccs_serialize_bin_ccs_object_internal(
		(_ccs_object_internal_t *)binding, buffer_size, buffer));
	CCS_VALIDATE(_ccs_serialize_bin_ccs_binding_data(
		binding->data, buffer_size, buffer));
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_deserialize_bin_ccs_binding_data(
	_ccs_binding_data_t *data,
	uint32_t             version,
	size_t              *buffer_size,
	const char         **buffer)
{
	(void)version;
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_object(
		(ccs_object_t *)&data->context, buffer_size, buffer));
	CCS_VALIDATE(_ccs_deserialize_bin_size(
		&data->num_values, buffer_size, buffer));
	if (data->num_values) {
		data->values = (ccs_datum_t *)calloc(
			data->num_values, sizeof(ccs_datum_t));
		CCS_REFUTE(!data->values, CCS_RESULT_ERROR_OUT_OF_MEMORY);
		for (size_t i = 0; i < data->num_values; i++)
			CCS_VALIDATE(_ccs_deserialize_bin_ccs_datum(
				data->values + i, buffer_size, buffer));
	}
	return CCS_RESULT_SUCCESS;
}

#endif //_BINDING_INTERNAL_H
