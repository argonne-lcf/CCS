#ifndef _TREE_INTERNAL_H
#define _TREE_INTERNAL_H
#include "distribution_internal.h"

#define CCS_CHECK_TREE(o, t) do { \
	CCS_CHECK_OBJ(o, CCS_TREE); \
	CCS_REFUTE(((_ccs_tree_common_data_t*)(o->data))->type != (t), CCS_INVALID_TREE); \
} while (0)

#define CCS_CHECK_TREE_ERR_GOTO(err, o, t, label) do { \
	CCS_CHECK_OBJ_ERR_GOTO(err, o, CCS_TREE, label); \
	CCS_REFUTE_ERR_GOTO(err, ((_ccs_tree_common_data_t*)(o->data))->type != (t), CCS_INVALID_TREE, label); \
} while (0)

struct _ccs_tree_data_s;
typedef struct _ccs_tree_data_s _ccs_tree_data_t;

struct _ccs_tree_ops_s {
	_ccs_object_ops_t obj_ops;
};
typedef struct _ccs_tree_ops_s _ccs_tree_ops_t;

struct _ccs_tree_s {
	_ccs_object_internal_t  obj;
	_ccs_tree_data_t       *data;
};

struct _ccs_tree_common_data_s {
	ccs_tree_type_t     type;
	size_t              arity;
	ccs_float_t        *weights; // Storage for children sum_weights * children bias and own weight at weights[arity]
	ccs_tree_t         *children;
	ccs_float_t         bias;
	ccs_datum_t         value;
	// Helper values
	ccs_float_t         sum_weights;
	ccs_distribution_t  distribution;
	ccs_tree_t          parent;
	size_t              index;  // if parent == NULL index contains tree_space handle
};
typedef struct _ccs_tree_common_data_s _ccs_tree_common_data_t;

static inline ccs_error_t
_ccs_serialize_bin_size_ccs_tree_common_data(
		_ccs_tree_common_data_t         *data,
		size_t                          *cum_size,
                _ccs_object_serialize_options_t *opts) {
	*cum_size += _ccs_serialize_bin_size_ccs_tree_type(data->type) +
		_ccs_serialize_bin_size_size(data->arity) +
		_ccs_serialize_bin_size_ccs_float(data->weights[data->arity]);
	for (size_t i = 0; i < data->arity; i++) {
		_ccs_serialize_bin_size_ccs_bool(data->children[i] != NULL);
		CCS_VALIDATE(data->children[i]->obj.ops->serialize_size(
			data->children[i], CCS_SERIALIZE_FORMAT_BINARY, cum_size, opts));
	}
	*cum_size += _ccs_serialize_bin_size_ccs_float(data->bias) +
		_ccs_serialize_bin_size_ccs_datum(data->value);
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
	CCS_VALIDATE(_ccs_serialize_bin_size(
		data->arity, buffer_size, buffer));
	CCS_VALIDATE(_ccs_serialize_bin_ccs_float(
		data->weights[data->arity], buffer_size, buffer));
	for (size_t i = 0; i < data->arity; i++) {
		CCS_VALIDATE(_ccs_serialize_bin_ccs_bool(
			data->children[i] != NULL, buffer_size, buffer));
		CCS_VALIDATE(data->children[i]->obj.ops->serialize(
			data->children[i], CCS_SERIALIZE_FORMAT_BINARY, buffer_size, buffer, opts));
	}
	CCS_VALIDATE(_ccs_serialize_bin_ccs_float(
		data->bias, buffer_size, buffer));
	CCS_VALIDATE(_ccs_serialize_bin_ccs_datum(
		data->value, buffer_size, buffer));
	return CCS_SUCCESS;
}
#endif //_TREE_INTERNAL_H
