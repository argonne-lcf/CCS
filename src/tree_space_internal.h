#ifndef _TREE_SPACE_INTERNAL_H
#define _TREE_SPACE_INTERNAL_H
#include "tree_internal.h"
#include "rng_internal.h"

#define CCS_CHECK_TREE_SPACE(o, t)                                             \
	do {                                                                   \
		CCS_CHECK_OBJ(o, CCS_OBJECT_TYPE_TREE_SPACE);                  \
		CCS_REFUTE(                                                    \
			((_ccs_tree_space_common_data_t *)(o->data))->type !=  \
				(t),                                           \
			CCS_RESULT_ERROR_INVALID_TREE_SPACE);                  \
	} while (0)

struct _ccs_tree_space_data_s;
typedef struct _ccs_tree_space_data_s _ccs_tree_space_data_t;

struct _ccs_tree_space_ops_s {
	_ccs_object_ops_t obj_ops;

	ccs_result_t (*get_node_at_position)(
		ccs_tree_space_t tree_space,
		size_t           position_size,
		const size_t    *position,
		ccs_tree_t      *tree_ret);

	ccs_result_t (*get_values_at_position)(
		ccs_tree_space_t tree_space,
		size_t           position_size,
		const size_t    *position,
		size_t           num_values,
		ccs_datum_t     *values);

	ccs_result_t (*check_position)(
		ccs_tree_space_t tree_space,
		size_t           position_size,
		const size_t    *position,
		ccs_bool_t      *is_valid_ret);
};
typedef struct _ccs_tree_space_ops_s _ccs_tree_space_ops_t;

struct _ccs_tree_space_s {
	_ccs_object_internal_t  obj;
	_ccs_tree_space_data_t *data;
};

struct _ccs_tree_space_common_data_s {
	ccs_tree_space_type_t type;
	const char           *name;
	ccs_rng_t             rng;
	ccs_tree_t            tree;
};
typedef struct _ccs_tree_space_common_data_s _ccs_tree_space_common_data_t;

static inline ccs_result_t
_ccs_serialize_bin_size_ccs_tree_space_common_data(
	_ccs_tree_space_common_data_t   *data,
	size_t                          *cum_size,
	_ccs_object_serialize_options_t *opts)
{
	*cum_size += _ccs_serialize_bin_size_ccs_tree_space_type(data->type) +
		     _ccs_serialize_bin_size_string(data->name);
	CCS_VALIDATE(data->rng->obj.ops->serialize_size(
		data->rng, CCS_SERIALIZE_FORMAT_BINARY, cum_size, opts));
	CCS_VALIDATE(data->tree->obj.ops->serialize_size(
		data->tree, CCS_SERIALIZE_FORMAT_BINARY, cum_size, opts));
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_serialize_bin_ccs_tree_space_common_data(
	_ccs_tree_space_common_data_t   *data,
	size_t                          *buffer_size,
	char                           **buffer,
	_ccs_object_serialize_options_t *opts)
{
	CCS_VALIDATE(_ccs_serialize_bin_ccs_tree_space_type(
		data->type, buffer_size, buffer));
	CCS_VALIDATE(
		_ccs_serialize_bin_string(data->name, buffer_size, buffer));
	CCS_VALIDATE(data->rng->obj.ops->serialize(
		data->rng, CCS_SERIALIZE_FORMAT_BINARY, buffer_size, buffer,
		opts));
	CCS_VALIDATE(data->tree->obj.ops->serialize(
		data->tree, CCS_SERIALIZE_FORMAT_BINARY, buffer_size, buffer,
		opts));
	return CCS_RESULT_SUCCESS;
}

#endif //_TREE_SPACE_INTERNAL_H
