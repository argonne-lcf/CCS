#ifndef _CCS_TREE_CONFIGURATION_H
#define _CCS_TREE_CONFIGURATION_H

#ifdef __cplusplus
extern "C" {
#endif

extern ccs_error_t
ccs_create_tree_configuration(
	ccs_tree_space_t          tree_space,
	size_t                    position_size,
	size_t                   *position,
	ccs_tree_configuration_t *configuration_ret);

extern ccs_error_t
ccs_tree_configuration_get_tree_space(
	ccs_tree_configuration_t  configuration,
	ccs_tree_space_t         *tree_space_ret);

extern ccs_error_t
ccs_tree_configuration_get_position(
	ccs_tree_configuration_t  configuration,
	size_t                    position_size,
	size_t                   *position,
	size_t                   *position_size_ret);

extern ccs_error_t
ccs_tree_configuration_get_values(
	ccs_tree_configuration_t  configuration,
	size_t                    num_values,
	ccs_datum_t              *values,
	size_t                   *num_values_ret);

extern ccs_error_t
ccs_tree_configuration_get_node(
	ccs_tree_configuration_t  configuration,
	ccs_tree_t               *node_ret);

extern ccs_error_t
ccs_tree_configuration_check(
	ccs_tree_configuration_t  configuration,
	ccs_bool_t               *is_valid_ret);

extern ccs_error_t
ccs_tree_configuration_hash(
	ccs_tree_configuration_t  configuration,
	ccs_hash_t               *hash_ret);

extern ccs_error_t
ccs_tree_configuration_cmp(
	ccs_tree_configuration_t  configuration,
	ccs_tree_configuration_t  other_configuration,
	int                      *cmp_ret);

#ifdef __cplusplus
}
#endif

#endif //_CCS_TREE_CONFIGURATION_H
