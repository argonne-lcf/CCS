#ifndef _CCS_TREE_SPACE_H
#define _CCS_TREE_SPACE_H

#ifdef __cplusplus
extern "C" {
#endif

enum ccs_tree_space_type_e {
	CCS_TREE_SPACE_TYPE_STATIC,
	CCS_TREE_SPACE_TYPE_DYNAMIC,
	CCS_TREE_SPACE_TYPE_MAX,
	CCS_TREE_SPACE_TYPE_FORCE_32BIT = INT_MAX
};
typedef enum ccs_tree_space_type_e ccs_tree_space_type_t;

extern ccs_error_t
ccs_create_static_tree_space(
	const char       *name,
	ccs_tree_t        tree,
	ccs_tree_space_t *tree_space_ret);

struct ccs_dynamic_tree_space_vector_s {
	ccs_error_t (*arity)(
		ccs_tree_space_t          tree_space,
		ccs_tree_configuration_t  configuration,
		size_t                   *arity_ret);

	ccs_error_t (*values)(
		ccs_tree_space_t          tree_space,
		ccs_tree_configuration_t  configuration,
		size_t                    num_values,
		ccs_datum_t              *values,
		size_t                   *num_values_ret);

	ccs_error_t (*child)(
		ccs_tree_space_t          tree_space,
		ccs_tree_configuration_t  configuration,
		size_t                    child_index,
		void                     *user_data,
		ccs_tree_configuration_t *child_ret);
};
typedef struct ccs_dynamic_tree_space_vector_s ccs_dynamic_tree_space_vector_t;

extern ccs_error_t
ccs_create_dynamic_tree_space(
	const char                      *name,
	ccs_dynamic_tree_space_vector_t *vector,
	void                            *tree_space_data,
	ccs_tree_space_t                *tree_space_ret);

extern ccs_error_t
ccs_tree_space_get_type(
	ccs_tree_space_t       tree_space,
	ccs_tree_space_type_t *type_ret);

extern ccs_error_t
ccs_tree_space_get_name(
	ccs_tree_space_t   tree_space,
	const char       **name_ret);

extern ccs_error_t
ccs_tree_space_set_rng(
	ccs_tree_space_t tree_space,
	ccs_rng_t        rng);

extern ccs_error_t
ccs_tree_space_get_rng(
	ccs_tree_space_t  tree_space,
	ccs_rng_t        *rng_ret);

extern ccs_error_t
ccs_tree_space_get_tree(
	ccs_tree_space_t  tree_space,
	ccs_tree_t       *tree_ret);

extern ccs_error_t
ccs_tree_space_check_configuration(
	ccs_tree_space_t         tree_space,
	ccs_tree_configuration_t configuration);

extern ccs_error_t
ccs_tree_space_sample(
	ccs_tree_space_t          tree_space,
	ccs_tree_configuration_t *configuration_ret);

extern ccs_error_t
ccs_tree_space_samples(
	ccs_tree_space_t          tree_space,
	size_t                    num_configurations,
	ccs_tree_configuration_t *configurations);

#ifdef __cplusplus
}
#endif

#endif //_CCS_TREE_SPACE_H
