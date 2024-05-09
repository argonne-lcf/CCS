#ifndef _TREE_CONFIGURATION_INTERNAL_H
#define _TREE_CONFIGURATION_INTERNAL_H

struct _ccs_tree_configuration_data_s;
typedef struct _ccs_tree_configuration_data_s _ccs_tree_configuration_data_t;

struct _ccs_tree_configuration_ops_s {
	_ccs_object_ops_t obj_ops;

	ccs_result_t (*hash)(
		ccs_tree_configuration_t configuration,
		ccs_hash_t              *hash_ret);

	ccs_result_t (*cmp)(
		ccs_tree_configuration_t configuration,
		ccs_tree_configuration_t other,
		int                     *cmp_ret);
};
typedef struct _ccs_tree_configuration_ops_s _ccs_tree_configuration_ops_t;

struct _ccs_tree_configuration_s {
	_ccs_object_internal_t          obj;
	_ccs_tree_configuration_data_t *data;
};

struct _ccs_tree_configuration_data_s {
	ccs_tree_space_t tree_space;
	size_t           position_size;
	size_t          *position;
};

#endif //_TREE_CONFIGURATION_INTERNAL_H
