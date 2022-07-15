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
	ccs_error_t (*del)(
		ccs_tree_space_t  tree_space);

	ccs_error_t (*get_child)(
		ccs_tree_space_t  tree_space,
		ccs_tree_t        parent,
		size_t            child_index,
		ccs_tree_t       *child_ret);

	ccs_error_t (*serialize_user_state)(
		ccs_tree_space_t  tree_space,
		size_t            sate_size,
		void             *state,
		size_t           *state_size_ret);

	ccs_error_t (*deserialize_state)(
		ccs_tree_space_t  tree_space,
		size_t            state_size,
		const void       *state);
};
typedef struct ccs_dynamic_tree_space_vector_s ccs_dynamic_tree_space_vector_t;

extern ccs_error_t
ccs_create_dynamic_tree_space(
	const char                      *name,
	ccs_tree_t                       tree,
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
ccs_tree_space_get_node_at_position(
	ccs_tree_space_t  tree_space,
	size_t            position_size,
	const size_t     *position,
	ccs_tree_t       *tree_ret);

extern ccs_error_t
ccs_tree_space_get_values_at_position(
	ccs_tree_space_t  tree_space,
	size_t            position_size,
	const size_t     *position,
	size_t            num_values,
	ccs_datum_t      *values);

extern ccs_error_t
ccs_tree_space_check_position(
	ccs_tree_space_t  tree_space,
	size_t            position_size,
	const size_t     *position,
	ccs_bool_t       *is_valid_ret);

extern ccs_error_t
ccs_tree_space_check_configuration(
	ccs_tree_space_t          tree_space,
	ccs_tree_configuration_t  configuration,
	ccs_bool_t               *is_valid_ret);

extern ccs_error_t
ccs_tree_space_sample(
	ccs_tree_space_t          tree_space,
	ccs_tree_configuration_t *configuration_ret);

extern ccs_error_t
ccs_tree_space_samples(
	ccs_tree_space_t          tree_space,
	size_t                    num_configurations,
	ccs_tree_configuration_t *configurations);

extern ccs_error_t
ccs_dynamic_tree_space_get_tree_space_data(
	ccs_tree_space_t   tree_space,
	void             **tree_space_data_ret);

#ifdef __cplusplus
}
#endif

#endif //_CCS_TREE_SPACE_H
