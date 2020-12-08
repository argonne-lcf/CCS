#ifndef _CCS_TREE_H
#define _CCS_TREE_H

#ifdef __cplusplus
extern "C" {
#endif

extern ccs_result_t
ccs_create_tree(size_t       arity,
                ccs_datum_t  value,
                void        *user_data,
                ccs_tree_t  *tree_ret);

extern ccs_result_t
ccs_tree_get_user_data(ccs_tree_t   tree,
                       void       **user_data_ret);

extern ccs_result_t
ccs_tree_get_value(ccs_tree_t   tree,
                   ccs_datum_t *value_ret);

extern ccs_result_t
ccs_tree_get_arity(ccs_tree_t  tree,
                   size_t     *arity_ret);

extern ccs_result_t
ccs_tree_get_child(ccs_tree_t  tree,
                   size_t      index,
                   ccs_tree_t *child_ret);

extern ccs_result_t
ccs_tree_set_child(ccs_tree_t tree,
                   size_t     index,
                   ccs_tree_t child);

extern ccs_result_t
ccs_tree_get_children(ccs_tree_t  tree,
                      size_t      num_children,
                      size_t     *indices,
                      ccs_tree_t *children,
                      size_t     *num_children_ret);

#ifdef __cplusplus
}
#endif

#endif //_CCS_TREE_SPACE_H
