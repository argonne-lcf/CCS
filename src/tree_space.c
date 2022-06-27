#include "cconfigspace_internal.h"
#include "tree_space_internal.h"

static inline _ccs_tree_space_ops_t *
_ccs_tree_space_get_ops(ccs_tree_space_t tree_space) {
	return (_ccs_tree_space_ops_t *)tree_space->obj.ops;
}

ccs_error_t
ccs_tree_space_get_type(
		ccs_tree_space_t       tree_space,
		ccs_tree_space_type_t *type_ret) {
	CCS_CHECK_OBJ(tree_space, CCS_TREE_SPACE);
	CCS_CHECK_PTR(type_ret);
	*type_ret = ((_ccs_tree_space_common_data_t *)(tree_space->data))->type;
	return CCS_SUCCESS;
}

ccs_error_t
ccs_tree_space_get_name(
		ccs_tree_space_t   tree_space,
		const char       **name_ret) {
	CCS_CHECK_OBJ(tree_space, CCS_TREE_SPACE);
	CCS_CHECK_PTR(name_ret);
	*name_ret = ((_ccs_tree_space_common_data_t *)(tree_space->data))->name;
	return CCS_SUCCESS;
}

ccs_error_t
ccs_tree_space_set_rng(
		ccs_tree_space_t tree_space,
		ccs_rng_t        rng) {
	CCS_CHECK_OBJ(tree_space, CCS_TREE_SPACE);
	CCS_CHECK_OBJ(rng,        CCS_RNG);
	CCS_VALIDATE(ccs_retain_object(rng));
	ccs_rng_t tmp = ((_ccs_tree_space_common_data_t *)(tree_space->data))->rng;
	((_ccs_tree_space_common_data_t *)(tree_space->data))->rng = rng;
	CCS_VALIDATE(ccs_release_object(tmp));
	return CCS_SUCCESS;
}

ccs_error_t
ccs_tree_space_get_rng(
		ccs_tree_space_t  tree_space,
		ccs_rng_t        *rng_ret) {
	CCS_CHECK_OBJ(tree_space, CCS_TREE_SPACE);
	CCS_CHECK_PTR(rng_ret);
	*rng_ret = ((_ccs_tree_space_common_data_t *)(tree_space->data))->rng;
	return CCS_SUCCESS;
}

ccs_error_t
ccs_tree_space_get_tree(
		ccs_tree_space_t  tree_space,
		ccs_tree_t        *tree_ret) {
	CCS_CHECK_OBJ(tree_space, CCS_TREE_SPACE);
	CCS_CHECK_PTR(tree_ret);
	*tree_ret = ((_ccs_tree_space_common_data_t *)(tree_space->data))->tree;
	return CCS_SUCCESS;
}   
