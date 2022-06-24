#ifndef _TREE_INTERNAL_H
#define _TREE_INTERNAL_H
#include "distribution_internal.h"

#define CCS_CHECK_TREE(o, t) do { \
	CCS_CHECK_OBJ(o, CCS_TREE); \
	CCS_REFUTE(((_ccs_tree_common_data_t*)(o->data))->type != (t), CCS_INVALID_TREE); \
} while (0)

struct _ccs_tree_data_s;
typedef struct _ccs_tree_data_s _ccs_tree_data_t;

struct _ccs_tree_ops_s {
	_ccs_object_ops_t obj_ops;

	ccs_error_t (*set_child)(
		ccs_tree_t        tree,
		size_t            index,
		ccs_tree_t        child);

	ccs_error_t (*get_child)(
		ccs_tree_t        tree,
		size_t            index,
		ccs_tree_t       *child_ret);
};
typedef struct _ccs_tree_ops_s _ccs_tree_ops_t;

struct _ccs_tree_s {
	_ccs_object_internal_t  obj;
	_ccs_tree_data_t       *data;
};

struct _ccs_tree_common_data_s {
	ccs_tree_type_t    type;
	size_t             arity;
	ccs_datum_t        value;
	ccs_distribution_t distribution;
};
typedef struct _ccs_tree_common_data_s _ccs_tree_common_data_t;

static inline ccs_error_t
_ccs_serialize_bin_size_ccs_tree_common_data(
		_ccs_tree_common_data_t         *data,
		size_t                          *cum_size,
                _ccs_object_serialize_options_t *opts) {
	*cum_size += _ccs_serialize_bin_size_ccs_tree_type(data->type) +
		_ccs_serialize_bin_size_uint64(data->arity) +
		_ccs_serialize_bin_size_ccs_datum(data->value) +
		_ccs_serialize_bin_size_ccs_bool(data->distribution != NULL);
	CCS_VALIDATE(data->distribution->obj.ops->serialize_size(
		data->distribution, CCS_SERIALIZE_FORMAT_BINARY, cum_size, opts));
	return CCS_SUCCESS;
}

static inline ccs_error_t
_ccs_serialize_bin_ccs_tree_common_data(
		_ccs_tree_common_data_t          *data,
		size_t                           *buffer_size,
		char                            **buffer,
		_ccs_object_serialize_options_t  *opts) {
	CCS_VALIDATE(_ccs_serialize_bin_ccs_tree_type(
		data->type, buffer_size, buffer));
	CCS_VALIDATE(_ccs_serialize_bin_uint64(
		data->arity, buffer_size, buffer));
	CCS_VALIDATE(_ccs_serialize_bin_ccs_datum(
		data->value, buffer_size, buffer));
	CCS_VALIDATE(_ccs_serialize_bin_ccs_bool(
		data->distribution != NULL, buffer_size, buffer));
	CCS_VALIDATE(data->distribution->obj.ops->serialize(
		data->distribution, CCS_SERIALIZE_FORMAT_BINARY, buffer_size, buffer, opts));
	return CCS_SUCCESS;
}
#endif //_TREE_INTERNAL_H
