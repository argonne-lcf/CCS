#ifndef _TREE_INTERNAL_H
#define _TREE_INTERNAL_H
#define HASH_NONFATAL_OOM 1
#include "uthash.h"

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

struct _ccs_tree_data_s {
	ccs_datum_t          value;
	size_t               arity;
	void                *user_data;
	_ccs_tree_wrapper_t *index_hash;
	int                  sorted;
};
typedef struct _ccs_tree_data_s _ccs_tree_data_t;

struct _ccs_tree_ops_s {
	_ccs_object_ops_t obj_ops;
};
typedef struct _ccs_tree_ops_s _ccs_tree_ops_t;

struct _ccs_tree_s {
	_ccs_object_internal_t obj;
	_ccs_tree_data_t *data;
};
#endif //_TREE_INTERNAL_H
