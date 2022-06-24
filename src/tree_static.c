#include "cconfigspace_internal.h"
#include "tree_internal.h"

struct _ccs_tree_static_data_s {
	_ccs_tree_common_data_t  common_data;
	ccs_tree_t              *children;
};
typedef struct _ccs_tree_static_data_s _ccs_tree_static_data_t;

static ccs_error_t
_ccs_tree_static_del(ccs_object_t o) {
	struct _ccs_tree_static_data_s *data =
		(struct _ccs_tree_static_data_s *)(((ccs_tree_t)o)->data);
	for (size_t i = 0; i < data->common_data.arity; i++)
		if (data->children[i])
			ccs_release_object(data->children[i]);
	return CCS_SUCCESS;
}

static inline ccs_error_t
_ccs_serialize_bin_size_ccs_tree_static_data(
		_ccs_tree_static_data_t         *data,
		size_t                          *cum_size,
		_ccs_object_serialize_options_t *opts) {
	CCS_VALIDATE(_ccs_serialize_bin_size_ccs_tree_common_data(
		&data->common_data, cum_size, opts));
	for (size_t i = 0; i < data->common_data.arity; i++) {
		_ccs_serialize_bin_size_ccs_bool(data->children[i] != NULL);
		CCS_VALIDATE(data->children[i]->obj.ops->serialize_size(
			data->children[i], CCS_SERIALIZE_FORMAT_BINARY, cum_size, opts));
	}
	return CCS_SUCCESS;
}

static inline ccs_error_t
_ccs_serialize_bin_ccs_tree_static_data(
		_ccs_tree_static_data_t          *data,
		size_t                           *buffer_size,
		char                            **buffer,
		_ccs_object_serialize_options_t  *opts) {
	CCS_VALIDATE(_ccs_serialize_bin_ccs_tree_common_data(
		&data->common_data, buffer_size, buffer, opts));
	for (size_t i = 0; i < data->common_data.arity; i++) {
		CCS_VALIDATE(_ccs_serialize_bin_ccs_bool(
			data->children[i] != NULL, buffer_size, buffer));
		CCS_VALIDATE(data->children[i]->obj.ops->serialize(
			data->children[i], CCS_SERIALIZE_FORMAT_BINARY, buffer_size, buffer, opts));
	}
	return CCS_SUCCESS;
}

static inline ccs_error_t 
_ccs_serialize_bin_size_ccs_tree_static(
		ccs_tree_t                       tree,
		size_t                          *cum_size,
		_ccs_object_serialize_options_t *opts) {
	_ccs_tree_static_data_t *data =
		(_ccs_tree_static_data_t *)tree->data;
	*cum_size += _ccs_serialize_bin_size_ccs_object_internal(
		(_ccs_object_internal_t *)tree);
	CCS_VALIDATE(_ccs_serialize_bin_size_ccs_tree_static_data(
		data, cum_size, opts));
	return CCS_SUCCESS;
}

static inline ccs_error_t
_ccs_serialize_bin_ccs_tree_static(
		ccs_tree_t                        tree,
		size_t                           *buffer_size,
		char                            **buffer,
		_ccs_object_serialize_options_t  *opts) {
	_ccs_tree_static_data_t *data =
		(_ccs_tree_static_data_t *)tree->data;
	CCS_VALIDATE(_ccs_serialize_bin_ccs_object_internal(
		 (_ccs_object_internal_t *)tree, buffer_size, buffer));
	CCS_VALIDATE(_ccs_serialize_bin_ccs_tree_static_data(
		data, buffer_size, buffer, opts));
	return CCS_SUCCESS;
}

static ccs_error_t
_ccs_tree_static_serialize_size(
		ccs_object_t                     object,
		ccs_serialize_format_t           format,
		size_t                          *cum_size,
		_ccs_object_serialize_options_t *opts) {
	switch(format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE(_ccs_serialize_bin_size_ccs_tree_static(
			(ccs_tree_t)object, cum_size, opts));
		break;
	default:
		CCS_RAISE(CCS_INVALID_VALUE, "Unsupported serialization format: %d", format);
	}
	CCS_VALIDATE(_ccs_object_serialize_user_data_size(
		object, format, cum_size, opts));
	return CCS_SUCCESS;
}

static ccs_error_t
_ccs_tree_static_serialize(
		ccs_object_t                      object,
		ccs_serialize_format_t            format,
		size_t                           *buffer_size,
		char                            **buffer,
		_ccs_object_serialize_options_t  *opts) {
	switch(format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE(_ccs_serialize_bin_ccs_tree_static(
		    (ccs_tree_t)object, buffer_size, buffer, opts));
		break;
	default:
		CCS_RAISE(CCS_INVALID_VALUE, "Unsupported serialization format: %d", format);
	}
	CCS_VALIDATE(_ccs_object_serialize_user_data(
		object, format, buffer_size, buffer, opts));
	return CCS_SUCCESS;
}

static ccs_error_t
_ccs_tree_static_set_child(
		ccs_tree_t        tree,
		size_t            index,
		ccs_tree_t        child) {
	_ccs_tree_static_data_t *d =
		(_ccs_tree_static_data_t *)tree->data;
	CCS_CHECK_TREE(child, CCS_TREE_TYPE_STATIC);
	CCS_VALIDATE(ccs_retain_object(child));
	d->children[index] = child;
	return CCS_SUCCESS;
}

static ccs_error_t
_ccs_tree_static_get_child(
		ccs_tree_t        tree,
		size_t            index,
		ccs_tree_t       *child_ret) {
	_ccs_tree_static_data_t *d =
		(_ccs_tree_static_data_t *)tree->data;
	*child_ret = d->children[index];
	return CCS_SUCCESS;
}

static _ccs_tree_ops_t _ccs_tree_static_ops = {
	{ &_ccs_tree_static_del,
	  &_ccs_tree_static_serialize_size,
	  &_ccs_tree_static_serialize },
	&_ccs_tree_static_set_child,
	&_ccs_tree_static_get_child
};

ccs_error_t
ccs_create_static_tree(
		size_t       arity,
		ccs_datum_t  value,
		ccs_tree_t  *tree_ret) {
	CCS_CHECK_PTR(tree_ret);
	CCS_REFUTE(value.type > CCS_STRING, CCS_INVALID_VALUE);
	CCS_REFUTE(arity > CCS_INT_MAX, CCS_INVALID_VALUE);
	size_t size_strs = 0;
	if (value.type == CCS_STRING) {
		CCS_REFUTE(!value.value.s, CCS_INVALID_VALUE);
		size_strs += strlen(value.value.s) + 1;
	}

	uintptr_t mem = (uintptr_t)calloc(1,
		sizeof(struct _ccs_tree_s) +
		sizeof(_ccs_tree_static_data_t) +
		arity * sizeof(ccs_tree_t) +
		size_strs);
	CCS_REFUTE(!mem, CCS_OUT_OF_MEMORY);

	ccs_tree_t tree = (ccs_tree_t)mem;
	_ccs_object_init(&(tree->obj), CCS_TREE, (_ccs_object_ops_t *)&_ccs_tree_static_ops);
	_ccs_tree_static_data_t *data =
		(_ccs_tree_static_data_t *)(mem + sizeof(struct _ccs_tree_s));
	data->common_data.type = CCS_TREE_TYPE_STATIC;
	data->common_data.arity = arity;
	data->common_data.distribution = NULL;
	data->children = (ccs_tree_t *)(mem +
		sizeof(struct _ccs_tree_s) +
		sizeof(_ccs_tree_static_data_t));
	if (value.type == CCS_STRING) {
		data->common_data.value = ccs_string((char *)(mem +
			sizeof(struct _ccs_tree_s) +
			sizeof(_ccs_tree_static_data_t) +
			arity * sizeof(ccs_tree_t)));
		strcpy((char *)data->common_data.value.value.s, value.value.s);
	} else {
		data->common_data.value = value;
		data->common_data.value.flags = CCS_FLAG_DEFAULT;
	}
	tree->data = (_ccs_tree_data_t *)data;
	*tree_ret = tree;
	return CCS_SUCCESS;
}

ccs_error_t
ccs_static_tree_get_children(
		ccs_tree_t  tree,
		size_t      num_children,
		ccs_tree_t *children,
		size_t     *num_children_ret) {
	CCS_CHECK_TREE(tree, CCS_TREE_TYPE_STATIC);
	CCS_CHECK_ARY(num_children, children);
	CCS_REFUTE(!children && !num_children_ret, CCS_INVALID_VALUE);
	_ccs_tree_static_data_t *data =
		 (_ccs_tree_static_data_t *)tree->data;
	size_t arity = data->common_data.arity;
	if (children) {
		CCS_REFUTE(num_children < arity, CCS_INVALID_VALUE);
		for (size_t i = 0; i < arity; i++)
			children[i] = data->children[i];
		for (size_t i = arity; i < num_children; i++)
			children[i] = NULL;
	}
	if (num_children_ret)
		*num_children_ret = arity;
	return CCS_SUCCESS;
}
