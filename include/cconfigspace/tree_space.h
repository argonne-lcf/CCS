#ifndef _CCS_TREE_SPACE_H
#define _CCS_TREE_SPACE_H

#ifdef __cplusplus
extern "C" {
#endif

enum ccs_tree_space_type_e {
	CCS_STATIC,
	CCS_DYNAMIC,
	CCS_TREE_SPACE_TYPE_MAX,
	CCS_TREE_SPACE_TYPE_FORCE_32BIT = INT_MAX
};
typedef enum ccs_tree_space_type_e ccs_tree_space_type_t;

extern ccs_result_t
ccs_create_static_tree_space(const char       *name,
                             ccs_tree_t        tree,
                             void             *user_data,
                             ccs_tree_space_t *tree_space_ret);

struct ccs_dynamic_tree_space_vector_s {
	ccs_result_t (*arity)(
		ccs_tree_space_t          tree_space,
		ccs_tree_configuration_t  configuration,
		size_t                   *arity_ret);

	ccs_result_t (*child)(
		ccs_tree_space_t          tree_space,
		ccs_tree_configuration_t  configuration,
		size_t                    child_index,
                void                     *user_data,
		ccs_tree_configuration_t *child_ret);
};
typedef struct ccs_dynamic_tree_space_vector_s ccs_dynamic_tree_space_vector_t;

extern ccs_result_t
ccs_create_dynamic_tree_space(const char                      *name,
                              ccs_dynamic_tree_space_vector_t *vector,
                              void                            *tree_space_data,
                              void                            *user_data,
                              ccs_tree_space_t                *tree_space_ret);

extern ccs_result_t
ccs_tree_space_get_type(ccs_tree_space_t       tree_space,
                        ccs_tree_space_type_t *type_ret);

extern ccs_result_t
ccs_tree_space_get_name(ccs_tree_space_t   tree_space,
                        const char       **name_ret);

extern ccs_result_t
ccs_tree_space_get_user_data(ccs_tree_space_t   tree_space,
                             void             **user_data_ret);

extern ccs_result_t
ccs_tree_space_check_configuration(ccs_tree_space_t         tree_space,
                                   ccs_tree_configuration_t configuration);

extern ccs_result_t
ccs_tree_space_get_configuration_arity(ccs_tree_space_t          tree_space,
                                       ccs_tree_configuration_t  configuration,
                                       size_t                   *arity_ret);

extern ccs_result_t
ccs_tree_space_get_configuration_child(ccs_tree_space_t          tree_space,
                                       ccs_tree_configuration_t  configuration,
                                       size_t                    child_index,
                                       void                     *user_data,
                                       ccs_tree_configuration_t *child_ret);

#ifdef __cplusplus
}
#endif

#endif //_CCS_TREE_SPACE_H
