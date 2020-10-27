#ifndef _CCS_TREE_CONFIGURATION_H
#define _CCS_TREE_CONFIGURATION_H

#ifdef __cplusplus
extern "C" {
#endif

extern ccs_result_t
ccs_create_tree_configuration(ccs_tree_space_t          tree_space,
                              size_t                    position_size,
                              size_t                   *position,
                              void                     *user_data,
                              ccs_tree_configuration_t *configuration_ret);

extern ccs_result_t
ccs_tree_configuration_get_tree_space(ccs_tree_configuration_t  configuration,
                                      ccs_tree_space_t         *tree_space_ret);

extern ccs_result_t
ccs_tree_configuration_get_user_data(ccs_tree_configuration_t   configuration,
                                     void                     **user_data_ret);

extern ccs_result_t
ccs_tree_configuration_get_position(ccs_tree_configuration_t  configuration,
                                    size_t                    position_size,
                                    size_t                   *position,
                                    size_t                   *position_size_ret);

extern ccs_result_t
ccs_tree_configuration_get_values(ccs_tree_configuration_t  configuration,
                                  size_t               num_values,
                                  ccs_datum_t         *values,
                                  size_t              *num_values_ret);

extern ccs_result_t
ccs_tree_configuration_get_arity(ccs_tree_configuration_t  configuration,
                                 size_t                   *arity_ret);

extern ccs_result_t
ccs_tree_configuration_get_child(ccs_tree_configuration_t  configuration,
                                 size_t                    child_index,
                                 void                     *user_data,
                                 ccs_tree_configuration_t *child_ret);

extern ccs_result_t
ccs_tree_configuration_cmp(ccs_tree_configuration_t  configuration,
                           ccs_tree_configuration_t  other_configuration,
                           int                      *equal_ret);

#ifdef __cplusplus
}
#endif

#endif //_CCS_TREE_CONFIGURATION_H
