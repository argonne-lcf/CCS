#ifdef HASH_NONFATAL_OOM
#undef HASH_NONFATAL_OOM
#endif
#define HASH_NONFATAL_OOM 1
#include "uthash.h"

#include "cconfigspace_internal.h"
#include "tree_internal.h"

struct _ccs_tree_wrapper_s {
	size_t         index;
	ccs_tree_t     tree;
	UT_hash_handle hh;
};
typedef struct _ccs_tree_wrapper_s _ccs_tree_wrapper_t;

static inline
int _ccs_tree_wrapper_cmp(const _ccs_tree_wrapper_t *a, const _ccs_tree_wrapper_t *b) {
	return a->index < b->index ? -1 : a->index > b->index ? 1 : 0;
}

struct _ccs_tree_dynamic_data_s {
	_ccs_tree_common_data_t  common_data;
	_ccs_tree_wrapper_t     *index_hash;
	_ccs_tree_wrapper_t      wrapper;
	ccs_bool_t               sorted;
};
typedef struct _ccs_tree_dynamic_data_s _ccs_tree_dynamic_data_t;

static ccs_error_t
_ccs_tree_dynamic_del(ccs_object_t o) {
	struct _ccs_tree_dynamic_data_s *data =
		(struct _ccs_tree_dynamic_data_s *)(((ccs_tree_t)o)->data);
	_ccs_tree_wrapper_t *current, *tmp;
	HASH_ITER(hh, data->index_hash, current, tmp) {
		HASH_DEL(data->index_hash, current);
		struct _ccs_tree_dynamic_data_s *cd =
			(struct _ccs_tree_dynamic_data_s *)(current->tree->data);
		cd->common_data.parent = NULL;
		cd->common_data.index = 0;
		cd->wrapper.index = 0;
		cd->wrapper.tree = NULL;
		ccs_release_object(current->tree);
	}
	return CCS_SUCCESS;
}

static inline ccs_error_t
_ccs_serialize_bin_size_ccs_tree_dynamic_data(
		_ccs_tree_dynamic_data_t        *data,
		size_t                          *cum_size,
		_ccs_object_serialize_options_t *opts) {
	CCS_VALIDATE(_ccs_serialize_bin_size_ccs_tree_common_data(
		&data->common_data, cum_size, opts));
	*cum_size += _ccs_serialize_bin_size_size(data->wrapper.index);
	_ccs_tree_wrapper_t *current, *tmp;
	HASH_ITER(hh, data->index_hash, current, tmp) {
		*cum_size += _ccs_serialize_bin_size_size(current->index);
		CCS_VALIDATE(current->tree->obj.ops->serialize_size(
			current->tree, CCS_SERIALIZE_FORMAT_BINARY, cum_size, opts));
	}
	return CCS_SUCCESS;
}

static inline ccs_error_t
_ccs_serialize_bin_ccs_tree_dynamic_data(
		_ccs_tree_dynamic_data_t         *data,
		size_t                           *buffer_size,
		char                            **buffer,
		_ccs_object_serialize_options_t  *opts) {
	CCS_VALIDATE(_ccs_serialize_bin_ccs_tree_common_data(
		&data->common_data, buffer_size, buffer, opts));
	CCS_VALIDATE(_ccs_serialize_bin_size(
		HASH_CNT(hh, data->index_hash), buffer_size, buffer));
	_ccs_tree_wrapper_t *current, *tmp;
	HASH_ITER(hh, data->index_hash, current, tmp) {
		CCS_VALIDATE(_ccs_serialize_bin_size(
			current->index, buffer_size, buffer));
		CCS_VALIDATE(current->tree->obj.ops->serialize(
			current->tree, CCS_SERIALIZE_FORMAT_BINARY, buffer_size, buffer, opts));
	}
	return CCS_SUCCESS;
}

static inline ccs_error_t
_ccs_serialize_bin_size_ccs_tree_dynamic(
		ccs_tree_t                       tree,
		size_t                          *cum_size,
		_ccs_object_serialize_options_t *opts) {
	_ccs_tree_dynamic_data_t *data =
		(_ccs_tree_dynamic_data_t *)tree->data;
	*cum_size += _ccs_serialize_bin_size_ccs_object_internal(
		(_ccs_object_internal_t *)tree);
	CCS_VALIDATE(_ccs_serialize_bin_size_ccs_tree_dynamic_data(
		data, cum_size, opts));
	return CCS_SUCCESS;
}

static inline ccs_error_t
_ccs_serialize_bin_ccs_tree_dynamic(
		ccs_tree_t                        tree,
		size_t                           *buffer_size,
		char                            **buffer,
		_ccs_object_serialize_options_t  *opts) {
	_ccs_tree_dynamic_data_t *data =
		(_ccs_tree_dynamic_data_t *)tree->data;
	CCS_VALIDATE(_ccs_serialize_bin_ccs_object_internal(
		 (_ccs_object_internal_t *)tree, buffer_size, buffer));
	CCS_VALIDATE(_ccs_serialize_bin_ccs_tree_dynamic_data(
		data, buffer_size, buffer, opts));
	return CCS_SUCCESS;
}

static ccs_error_t
_ccs_tree_dynamic_serialize_size(
		ccs_object_t                     object,
		ccs_serialize_format_t           format,
		size_t                          *cum_size,
		_ccs_object_serialize_options_t *opts) {
	switch(format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE(_ccs_serialize_bin_size_ccs_tree_dynamic(
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
_ccs_tree_dynamic_serialize(
		ccs_object_t                      object,
		ccs_serialize_format_t            format,
		size_t                           *buffer_size,
		char                            **buffer,
		_ccs_object_serialize_options_t  *opts) {
	switch(format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE(_ccs_serialize_bin_ccs_tree_dynamic(
		    (ccs_tree_t)object, buffer_size, buffer, opts));
		break;
	default:
		CCS_RAISE(CCS_INVALID_VALUE, "Unsupported serialization format: %d", format);
	}
	CCS_VALIDATE(_ccs_object_serialize_user_data(
		object, format, buffer_size, buffer, opts));
	return CCS_SUCCESS;
}


#undef uthash_nonfatal_oom
#define uthash_nonfatal_oom(elt) { \
	CCS_RAISE_ERR_GOTO(err, CCS_OUT_OF_MEMORY, retain, "Out of memory to allocate hash"); \
}
static ccs_error_t
 _ccs_tree_dynamic_set_child(
		ccs_tree_t        tree,
		size_t            index,
		ccs_tree_t        child) {
	CCS_CHECK_TREE(child, CCS_TREE_TYPE_DYNAMIC);
	CCS_VALIDATE(ccs_retain_object(child));
	_ccs_tree_dynamic_data_t *d =
		(_ccs_tree_dynamic_data_t *)tree->data;
	_ccs_tree_dynamic_data_t *cd =
		(_ccs_tree_dynamic_data_t *)child->data;
	CCS_REFUTE(cd->common_data.parent, CCS_INVALID_TREE);
	ccs_error_t err = CCS_SUCCESS;
	CCS_VALIDATE(ccs_retain_object(child));
	cd->wrapper.index = index;
	cd->wrapper.tree = child;
	HASH_ADD(hh, d->index_hash, index, sizeof(index), &cd->wrapper);
	cd->common_data.parent = tree;
	cd->common_data.index = index;
	d->sorted = CCS_FALSE;
	return CCS_SUCCESS;
retain:
	cd->wrapper.index = 0;
	cd->wrapper.tree = NULL;
	ccs_release_object(child);
	return err;
}

static ccs_error_t
_ccs_tree_dynamic_get_child(
		ccs_tree_t        tree,
		size_t            index,
		ccs_tree_t       *child_ret) {
	_ccs_tree_dynamic_data_t *d =
		(_ccs_tree_dynamic_data_t *)tree->data;
	_ccs_tree_wrapper_t *wrap;
	HASH_FIND(hh, d->index_hash, &index, sizeof(index), wrap);
	if (!wrap)
		*child_ret = NULL;
	else
		*child_ret = wrap->tree;
	return CCS_SUCCESS;
}

static _ccs_tree_ops_t _ccs_tree_dynamic_ops = {
	{ &_ccs_tree_dynamic_del,
	  &_ccs_tree_dynamic_serialize_size,
	  &_ccs_tree_dynamic_serialize },
	&_ccs_tree_dynamic_set_child,
	&_ccs_tree_dynamic_get_child
};

ccs_error_t
ccs_create_dynamic_tree(
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
		sizeof(_ccs_tree_dynamic_data_t) +
		size_strs);
	CCS_REFUTE(!mem, CCS_OUT_OF_MEMORY);
	ccs_tree_t tree = (ccs_tree_t)mem;
	_ccs_object_init(&(tree->obj), CCS_TREE, (_ccs_object_ops_t *)&_ccs_tree_dynamic_ops);
	_ccs_tree_dynamic_data_t *data =
		(_ccs_tree_dynamic_data_t *)(mem + sizeof(struct _ccs_tree_s));
	data->common_data.type = CCS_TREE_TYPE_DYNAMIC;
	data->common_data.arity = arity;
	data->common_data.parent = NULL;
	data->common_data.distribution = NULL;
	data->index_hash = NULL;
	data->sorted = CCS_FALSE;
	if (value.type == CCS_STRING) {
		char *str_pool = (char *)(mem +
			sizeof(struct _ccs_tree_s) +
			sizeof(_ccs_tree_dynamic_data_t));
		data->common_data.value = ccs_string(str_pool);
		strcpy(str_pool, value.value.s);
	} else {
		data->common_data.value = value;
		data->common_data.value.flags = CCS_FLAG_DEFAULT;
	}
	tree->data = (_ccs_tree_data_t *)data;
	*tree_ret = tree;
	return CCS_SUCCESS;
}

ccs_error_t
ccs_dynamic_tree_get_children(
		ccs_tree_t  tree,
		size_t      num_children,
		size_t     *indices,
		ccs_tree_t *children,
		size_t     *num_children_ret) {
	CCS_CHECK_TREE(tree, CCS_TREE_TYPE_DYNAMIC);
	CCS_CHECK_ARY(num_children, indices);
	CCS_CHECK_ARY(num_children, children);
	CCS_REFUTE(!num_children_ret && !children && !indices, CCS_INVALID_VALUE);
	_ccs_tree_dynamic_data_t *d =
		(_ccs_tree_dynamic_data_t *)tree->data;
	size_t count = HASH_CNT(hh, d->index_hash);
	if (indices) {
		if (!d->sorted) {
			HASH_SRT(hh, d->index_hash, _ccs_tree_wrapper_cmp);
			d->sorted = CCS_TRUE;
		}
		_ccs_tree_wrapper_t *current, *tmp;
		size_t index = 0;
		HASH_ITER(hh, d->index_hash, current, tmp) {
			indices[index] = current->index;
			children[index] = current->tree;
		}
		for (size_t i = count; i < num_children; i++) {
			indices[i] = 0;
			children[i] = NULL;
		}
	}
	if (num_children_ret)
		*num_children_ret = count;
	return CCS_SUCCESS;
}
