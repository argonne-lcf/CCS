#include "cconfigspace_internal.h"
#include "tree_internal.h"

static ccs_result_t
_ccs_tree_del(ccs_object_t object) {
	_ccs_tree_data_t * data = ((ccs_tree_t)object)->data;
	if (data->index_hash) {
		_ccs_tree_wrapper_t *current, *tmp;
		HASH_ITER(hh, data->index_hash, current, tmp) {
			HASH_DEL(data->index_hash, current);
			ccs_release_object(current->tree);
			free(current);
		}
	}
	return CCS_SUCCESS;
}
static struct _ccs_tree_ops_s _tree_ops = { {&_ccs_tree_del} };

ccs_result_t
ccs_create_tree(size_t       arity,
                ccs_datum_t  value,
                void        *user_data,
                ccs_tree_t  *tree_ret) {
	if (value.type < CCS_NONE || value.type > CCS_STRING)
		return -CCS_INVALID_VALUE;
	CCS_CHECK_PTR(tree_ret);
	size_t size_str = 0;
	if (value.type == CCS_STRING && value.value.s) {
		size_str = strlen(value.value.s) + 1;
	}
	uintptr_t mem = (uintptr_t)calloc(1,
		sizeof(struct _ccs_tree_s) +
		sizeof(struct _ccs_tree_data_s) +
		size_str);
	if (!mem)
		return -CCS_OUT_OF_MEMORY;

	ccs_tree_t tree = (ccs_tree_t)mem;
	_ccs_object_init(&(tree->obj), CCS_TREE, (_ccs_object_ops_t *)&_tree_ops);
	tree->data = (struct _ccs_tree_data_s *)(mem + sizeof(struct _ccs_tree_s));
	tree->data->arity = arity;
	tree->data->user_data = user_data;
	tree->data->index_hash = NULL;
	tree->data->sorted = 0;
	if (size_str) {
		char *str_pool = (char *)(mem +
			sizeof(struct _ccs_tree_s) +
			sizeof(struct _ccs_tree_data_s));
		tree->data->value.type = CCS_STRING;
		tree->data->value.value.s = str_pool;
		strcpy(str_pool, value.value.s);
	} else {
		tree->data->value = value;
	}
	*tree_ret = tree;
	return CCS_SUCCESS;
}

ccs_result_t
ccs_tree_get_user_data(ccs_tree_t   tree,
                       void       **user_data_ret) {
	CCS_CHECK_OBJ(tree, CCS_TREE);
	CCS_CHECK_PTR(user_data_ret);
	*user_data_ret = tree->data->user_data;
	return CCS_SUCCESS;
}

ccs_result_t
ccs_tree_get_value(ccs_tree_t   tree,
                   ccs_datum_t *value_ret) {
	CCS_CHECK_OBJ(tree, CCS_TREE);
	CCS_CHECK_PTR(value_ret);
	*value_ret = tree->data->value;
	return CCS_SUCCESS;
}

ccs_result_t
ccs_tree_get_arity(ccs_tree_t  tree,
                   size_t     *arity_ret) {
	CCS_CHECK_OBJ(tree, CCS_TREE);
	CCS_CHECK_PTR(arity_ret);
	*arity_ret = tree->data->arity;
	return CCS_SUCCESS;
}

ccs_result_t
ccs_tree_get_child(ccs_tree_t  tree,
                   size_t      index,
                   ccs_tree_t *child_ret) {
	CCS_CHECK_OBJ(tree, CCS_TREE);
	CCS_CHECK_PTR(child_ret);
	if (index >= tree->data->arity)
		return -CCS_OUT_OF_BOUNDS;
	_ccs_tree_wrapper_t *wrap;
	HASH_FIND(hh, tree->data->index_hash, &index, sizeof(index), wrap);
	if (!wrap)
		*child_ret = NULL;
	else
		*child_ret = wrap->tree;
	return CCS_SUCCESS;
}

#undef uthash_nonfatal_oom
#define uthash_nonfatal_oom(elt) { \
	err = -CCS_OUT_OF_MEMORY; \
	goto retain; \
}
ccs_result_t
ccs_tree_set_child(ccs_tree_t tree,
                   size_t     index,
                   ccs_tree_t child) {
	CCS_CHECK_OBJ(tree, CCS_TREE);
	if (index >= tree->data->arity)
		return -CCS_OUT_OF_BOUNDS;
	CCS_CHECK_OBJ(child, CCS_TREE);

	ccs_result_t err = CCS_SUCCESS;
	_ccs_tree_wrapper_t *wrap =
		(_ccs_tree_wrapper_t *)malloc(sizeof(_ccs_tree_wrapper_t));
	if (!wrap)
		return -CCS_OUT_OF_MEMORY;
	wrap->index = index;
	wrap->tree = child;
	err = ccs_retain_object(child);
	if (err)
		goto wrap;
	HASH_ADD(hh, tree->data->index_hash, index, sizeof(index), wrap);
	return CCS_SUCCESS;
retain:
	ccs_release_object(child);
wrap:
	free(wrap);
	return err;
}
