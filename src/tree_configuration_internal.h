#ifndef _TREE_CONFIGURATION_INTERNAL_H
#define _TREE_CONFIGURATION_INTERNAL_H

struct _ccs_tree_configuration_data_s;
typedef struct _ccs_tree_configuration_data_s _ccs_tree_configuration_data_t;

struct _ccs_tree_configuration_ops_s {
	_ccs_object_ops_t obj_ops;
};
typedef struct _ccs_tree_configuration_ops_s _ccs_tree_configuration_ops_t;

struct _ccs_tree_configuration_s {
	_ccs_object_internal_t          obj;
	_ccs_tree_configuration_data_t *data;
};

struct _ccs_tree_configuration_data_s {
	ccs_tree_space_t  tree_space;
	size_t            position_size;
	size_t           *position;
};

#endif //_TREE_CONFIGURATION_INTERNAL_H
