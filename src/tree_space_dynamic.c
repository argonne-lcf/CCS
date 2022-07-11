#include "cconfigspace_internal.h"
#include "tree_space_internal.h"
#include "tree_internal.h"

struct _ccs_tree_space_dynamic_data_s {
	_ccs_tree_space_common_data_t    common_data;
	ccs_dynamic_tree_space_vector_t  vector;
	void                            *tree_space_data;
};
typedef struct _ccs_tree_space_dynamic_data_s _ccs_tree_space_dynamic_data_t;

static ccs_error_t
_ccs_tree_space_dynamic_del(ccs_object_t o) {
	struct _ccs_tree_space_dynamic_data_s *data =
		(struct _ccs_tree_space_dynamic_data_s *)(((ccs_tree_space_t)o)->data);
	ccs_release_object(data->common_data.rng);
	ccs_release_object(data->common_data.tree);
	return CCS_SUCCESS;
}

static inline ccs_error_t
_ccs_serialize_bin_size_ccs_tree_space_dynamic_data(
		_ccs_tree_space_dynamic_data_t  *data,
		size_t                          *cum_size,
		_ccs_object_serialize_options_t *opts) {
	CCS_VALIDATE(_ccs_serialize_bin_size_ccs_tree_space_common_data(
		&data->common_data, cum_size, opts));
	return CCS_SUCCESS;
}

static inline ccs_error_t
_ccs_serialize_bin_ccs_tree_space_dynamic_data(
		_ccs_tree_space_dynamic_data_t   *data,
		size_t                           *buffer_size,
		char                            **buffer,
		_ccs_object_serialize_options_t  *opts) {
	CCS_VALIDATE(_ccs_serialize_bin_ccs_tree_space_common_data(
		&data->common_data, buffer_size, buffer, opts));
	return CCS_SUCCESS;
}

static inline ccs_error_t
_ccs_serialize_bin_size_ccs_tree_space_dynamic(
		ccs_tree_space_t                tree_space,
		size_t                          *cum_size,
		_ccs_object_serialize_options_t *opts) {
	_ccs_tree_space_dynamic_data_t *data =
		(_ccs_tree_space_dynamic_data_t *)tree_space->data;
	*cum_size += _ccs_serialize_bin_size_ccs_object_internal(
		(_ccs_object_internal_t *)tree_space);
	CCS_VALIDATE(_ccs_serialize_bin_size_ccs_tree_space_dynamic_data(
		data, cum_size, opts));
	size_t state_size = 0;
	if (data->vector.serialize_user_state)
		CCS_VALIDATE(data->vector.serialize_user_state(
			tree_space, 0, NULL, &state_size));
	*cum_size += _ccs_serialize_bin_size_size(state_size);
	*cum_size += state_size;
	return CCS_SUCCESS;
}

static inline ccs_error_t
_ccs_serialize_bin_ccs_tree_space_dynamic(
		ccs_tree_space_t                  tree_space,
		size_t                           *buffer_size,
		char                            **buffer,
		_ccs_object_serialize_options_t  *opts) {
	_ccs_tree_space_dynamic_data_t *data =
		(_ccs_tree_space_dynamic_data_t *)tree_space->data;
	CCS_VALIDATE(_ccs_serialize_bin_ccs_object_internal(
		 (_ccs_object_internal_t *)tree_space, buffer_size, buffer));
	CCS_VALIDATE(_ccs_serialize_bin_ccs_tree_space_dynamic_data(
		data, buffer_size, buffer, opts));
	size_t state_size = 0;
	if (data->vector.serialize_user_state)
		CCS_VALIDATE(data->vector.serialize_user_state(
			tree_space, 0, NULL, &state_size));
	CCS_VALIDATE(_ccs_serialize_bin_size(
		state_size, buffer_size, buffer));
	if (state_size) {
		CCS_REFUTE(*buffer_size < state_size, CCS_NOT_ENOUGH_DATA);
		CCS_VALIDATE(data->vector.serialize_user_state(
			tree_space, state_size, *buffer, NULL));
		*buffer_size -= state_size;
		*buffer += state_size;
	}
	return CCS_SUCCESS;
}

static ccs_error_t
_ccs_tree_space_dynamic_serialize_size(
		ccs_object_t                     object,
		ccs_serialize_format_t           format,
		size_t                          *cum_size,
		_ccs_object_serialize_options_t *opts) {
	switch(format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE(_ccs_serialize_bin_size_ccs_tree_space_dynamic(
			(ccs_tree_space_t)object, cum_size, opts));
		break;
	default:
		CCS_RAISE(CCS_INVALID_VALUE, "Unsupported serialization format: %d", format);
	}
	CCS_VALIDATE(_ccs_object_serialize_user_data_size(
		object, format, cum_size, opts));
	return CCS_SUCCESS;
}

static ccs_error_t
_ccs_tree_space_dynamic_serialize(
		ccs_object_t                      object,
		ccs_serialize_format_t            format,
		size_t                           *buffer_size,
		char                            **buffer,
		_ccs_object_serialize_options_t  *opts) {
	switch(format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE(_ccs_serialize_bin_ccs_tree_space_dynamic(
		    (ccs_tree_space_t)object, buffer_size, buffer, opts));
		break;
	default:
		CCS_RAISE(CCS_INVALID_VALUE, "Unsupported serialization format: %d", format);
	}
	CCS_VALIDATE(_ccs_object_serialize_user_data(
		object, format, buffer_size, buffer, opts));
	return CCS_SUCCESS;
}

static inline ccs_error_t
_ccs_tree_space_tree_get_child(
		ccs_tree_space_t                tree_space,
		ccs_tree_t                      parent,
		size_t                          index,
		ccs_tree_t                     *child,
		ccs_error_t (*child_cb)(
			ccs_tree_space_t  tree_space,
			ccs_tree_t        parent,
			size_t            child_index,
			ccs_tree_t       *child_ret)) {
	CCS_VALIDATE(ccs_tree_get_child(parent, index, child));
	if (!*child) {
		CCS_VALIDATE(child_cb(tree_space, parent, index, child));
		CCS_VALIDATE(ccs_tree_set_child(parent, index, *child));
		CCS_VALIDATE(ccs_release_object(*child));
	}
	return CCS_SUCCESS;
}

static ccs_error_t
_ccs_tree_space_dynamic_get_node_at_position(
		ccs_tree_space_t  tree_space,
		size_t            position_size,
		const size_t     *position,
		ccs_tree_t       *tree_ret) {
	_ccs_tree_space_dynamic_data_t *data =
		(_ccs_tree_space_dynamic_data_t *)tree_space->data;
	ccs_tree_t parent = data->common_data.tree;
	ccs_tree_t child = NULL;
	for (size_t i = 0; i < position_size; i++) {
		CCS_VALIDATE(_ccs_tree_space_tree_get_child(tree_space, parent, position[i], &child, data->vector.child));
		parent = child;
	}
	*tree_ret = child;
	return CCS_SUCCESS;
}

static ccs_error_t
_ccs_tree_space_dynamic_get_values_at_position(
		ccs_tree_space_t  tree_space,
		size_t            position_size,
		const size_t     *position,
		size_t            num_values,
		ccs_datum_t      *values) {
	CCS_CHECK_ARY(position_size, position);
	CCS_CHECK_ARY(num_values, values);
	CCS_REFUTE(num_values < position_size + 1, CCS_INVALID_VALUE);
	_ccs_tree_space_dynamic_data_t *data =
		(_ccs_tree_space_dynamic_data_t *)tree_space->data;
	ccs_tree_t parent = data->common_data.tree;
	ccs_tree_t child = NULL;
	*values++ = parent->data->value;
	for (size_t i = 0; i < position_size; i++) {
		CCS_VALIDATE(_ccs_tree_space_tree_get_child(tree_space, parent, position[i], &child, data->vector.child));
		*values++ = child->data->value;
		parent = child;
	}
	for (size_t i = position_size; i < num_values; i++)
		*values++ = ccs_none;
	return CCS_SUCCESS;
}

static ccs_error_t
_ccs_tree_space_dynamic_check_position(
		ccs_tree_space_t  tree_space,
		size_t            position_size,
		const size_t     *position,
		ccs_bool_t       *is_valid_ret) {
	CCS_CHECK_ARY(position_size, position);
	CCS_CHECK_PTR(is_valid_ret);
	_ccs_tree_space_dynamic_data_t *data =
		(_ccs_tree_space_dynamic_data_t *)tree_space->data;
	ccs_tree_t parent = data->common_data.tree;
	ccs_tree_t child = NULL;
	*is_valid_ret = CCS_FALSE;
	for (size_t i = 0; i < position_size; i++) {
		if (position[i] >= parent->data->arity)
			*is_valid_ret = CCS_FALSE;
		CCS_VALIDATE(_ccs_tree_space_tree_get_child(tree_space, parent, position[i], &child, data->vector.child));
		parent = child;
	}
	*is_valid_ret = CCS_TRUE;
	return CCS_SUCCESS;
}

static _ccs_tree_space_ops_t _ccs_tree_space_dynamic_ops = {
	{ &_ccs_tree_space_dynamic_del,
	  &_ccs_tree_space_dynamic_serialize_size,
	  &_ccs_tree_space_dynamic_serialize },
	&_ccs_tree_space_dynamic_get_node_at_position,
	&_ccs_tree_space_dynamic_get_values_at_position,
	&_ccs_tree_space_dynamic_check_position
};

ccs_error_t
ccs_create_dynamic_tree_space(
		const char                      *name,
		ccs_tree_t                       tree,
		ccs_dynamic_tree_space_vector_t *vector,
		void                            *tree_space_data,
		ccs_tree_space_t                *tree_space_ret) {
	CCS_CHECK_PTR(name);
	CCS_CHECK_OBJ(tree, CCS_TREE);
	CCS_CHECK_PTR(vector);
	CCS_CHECK_PTR(vector->child);
	CCS_CHECK_PTR(tree_space_ret);
	ccs_error_t err;
	uintptr_t mem = (uintptr_t)calloc(1,
		sizeof(struct _ccs_tree_space_s) +
		sizeof(struct _ccs_tree_space_dynamic_data_s) +
		strlen(name) + 1);
	CCS_REFUTE(!mem, CCS_OUT_OF_MEMORY);
	ccs_rng_t rng;
	CCS_VALIDATE_ERR_GOTO(err, ccs_create_rng(&rng), errmem);
	CCS_VALIDATE_ERR_GOTO(err, ccs_retain_object(tree), err_rng);

	ccs_tree_space_t tree_space;
	tree_space = (ccs_tree_space_t)mem;
	_ccs_object_init(&(tree_space->obj), CCS_TREE_SPACE,
		(_ccs_object_ops_t *)&_ccs_tree_space_dynamic_ops);
	_ccs_tree_space_dynamic_data_t *data;
	data = (struct _ccs_tree_space_dynamic_data_s *)(mem +
		sizeof(struct _ccs_tree_space_s));
	data->common_data.type = CCS_TREE_SPACE_TYPE_DYNAMIC;
	data->common_data.name = (const char *)(mem +
		sizeof(struct _ccs_tree_space_s) +
		sizeof(struct _ccs_tree_space_dynamic_data_s));
	data->common_data.rng = rng;
	data->common_data.tree = tree;
	strcpy((char *)(data->common_data.name), name);
	data->tree_space_data = tree_space_data;
	memcpy(&data->vector, vector, sizeof(data->vector));
	tree_space->data = (_ccs_tree_space_data_t *)data;
	*tree_space_ret = tree_space;
	return CCS_SUCCESS;
err_rng:
	ccs_release_object(rng);
errmem:
	free((void *)mem);
	return err;
}

