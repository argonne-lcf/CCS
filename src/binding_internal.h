#ifndef _BINDING_INTERNAL_H
#define _BINDING_INTERNAL_H
#include "datum_hash.h"

typedef struct _ccs_binding_data_s _ccs_binding_data_t;

struct _ccs_binding_ops_s {
	_ccs_object_ops_t obj_ops;

	ccs_result_t (*hash)(
		_ccs_binding_data_t *data,
		ccs_hash_t          *hash_ret);

	ccs_result_t (*cmp)(
		_ccs_binding_data_t *data,
		ccs_binding_t        other,
		int                 *cmp_ret);
};
typedef struct _ccs_binding_ops_s _ccs_binding_ops_t;

struct _ccs_binding_data_s {
	ccs_context_t  context;
	size_t         num_values;
	ccs_datum_t   *values;
};

struct _ccs_binding_s {
	_ccs_object_internal_t  obj;
	_ccs_binding_data_t    *data;
};

static inline ccs_result_t
_ccs_binding_get_context(ccs_binding_t  binding,
                         ccs_context_t *context_ret) {
	CCS_CHECK_PTR(context_ret);
	*context_ret = binding->data->context;
	return CCS_SUCCESS;
}

static inline ccs_result_t
_ccs_binding_get_value(ccs_binding_t  binding,
                       size_t         index,
                       ccs_datum_t   *value_ret) {
	CCS_CHECK_PTR(value_ret);
	if (index >= binding->data->num_values)
		return -CCS_OUT_OF_BOUNDS;
	*value_ret = binding->data->values[index];
	return CCS_SUCCESS;
}

static inline ccs_result_t
_ccs_binding_set_value(ccs_binding_t binding,
                      size_t        index,
                      ccs_datum_t   value) {
	if (index >= binding->data->num_values)
		return -CCS_OUT_OF_BOUNDS;
	if (value.flags & CCS_FLAG_TRANSIENT)
		CCS_VALIDATE(ccs_context_validate_value(
			binding->data->context, index, value, &value));
	binding->data->values[index] = value;
	return CCS_SUCCESS;
}

static inline ccs_result_t
_ccs_binding_get_values(ccs_binding_t  binding,
                        size_t         num_values,
                        ccs_datum_t   *values,
                        size_t        *num_values_ret) {
	CCS_CHECK_ARY(num_values, values);
	if (!num_values_ret && !values)
		return -CCS_INVALID_VALUE;
	size_t num = binding->data->num_values;
	if (values) {
		if (num_values < num)
			return -CCS_INVALID_VALUE;
		memcpy(values, binding->data->values, num*sizeof(ccs_datum_t));
		for (size_t i = num; i < num_values; i++) {
			values[i].type = CCS_NONE;
			values[i].value.i = 0;
		}
	}
	if (num_values_ret)
		*num_values_ret = num;
	return CCS_SUCCESS;
}

static inline ccs_result_t
_ccs_binding_get_value_by_name(ccs_binding_t  binding,
                               const char    *name,
                               ccs_datum_t   *value_ret) {
	CCS_CHECK_PTR(name);
	size_t index;
	CCS_VALIDATE(ccs_context_get_hyperparameter_index_by_name(
	               binding->data->context, name, &index));
	*value_ret = binding->data->values[index];
	return CCS_SUCCESS;
}

static inline ccs_result_t
_ccs_binding_hash(_ccs_binding_data_t *data,
                  ccs_hash_t          *hash_ret) {
	CCS_CHECK_PTR(hash_ret);
	ccs_hash_t h, ht;
	HASH_JEN(&(data->context), sizeof(data->context), h);
	HASH_JEN(&(data->num_values), sizeof(data->num_values), ht);
	h = _hash_combine(h, ht);
	for (size_t i = 0; i < data->num_values; i++) {
		ht = _hash_datum(data->values + i);
		h = _hash_combine(h, ht);
	}
	*hash_ret = h;
	return CCS_SUCCESS;
}

static inline ccs_result_t
_ccs_binding_cmp(_ccs_binding_data_t  *data,
                 ccs_binding_t         other_binding,
                 int                  *cmp_ret) {
	CCS_CHECK_PTR(cmp_ret);
	_ccs_binding_data_t *other_data = other_binding->data;
	if (data == other_data) {
		*cmp_ret = 0;
		return CCS_SUCCESS;
	}
	*cmp_ret = data->context < other_data->context ? -1 :
	           data->context > other_data->context ?  1 : 0;
	if (*cmp_ret)
		return CCS_SUCCESS;
	*cmp_ret = data->num_values < other_data->num_values ? -1 :
	           data->num_values > other_data->num_values ?  1 : 0;
	if (*cmp_ret)
		return CCS_SUCCESS;
	for (size_t i = 0; i < data->num_values; i++) {
		if ( (*cmp_ret = _datum_cmp(data->values + i, other_data->values + i)) )
			return CCS_SUCCESS;
	}
	return CCS_SUCCESS;
}

#endif //_BINDING_INTERNAL_H
