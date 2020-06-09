#ifndef _CCS_OBJECTIVE_SPACE
#define _CCS_OBJECTIVE_SPACE

#ifdef __cplusplus
extern "C" {
#endif

enum ccs_objective_type_e {
	CCS_MINIMIZE,
	CCS_MAXIMIZE,
	CCS_OBJECTIVE_TYPE_MAX,
	CCS_OBJECTIVE_TYPE_FORCE_32BIT = INT_MAX
};

typedef enum ccs_objective_type_e ccs_objective_type_t;

extern ccs_error_t
ccs_create_objective_space(const char            *name,
                           void                  *user_data,
                           ccs_objective_space_t *objective_space_ret);

extern ccs_error_t
ccs_objective_space_get_name(ccs_objective_space_t   objective_space,
                             const char            **name_ret);

extern ccs_error_t
ccs_objective_space_get_user_data(ccs_objective_space_t   objective_space,
                                  void                  **user_data_ret);

extern ccs_error_t
ccs_objective_space_add_variable(ccs_objective_space_t objective_space,
                                 ccs_hyperparameter_t  result);

extern ccs_error_t
ccs_objective_space_add_objective(ccs_objective_space_t objective_space,
                                  ccs_expression_t      objective,
                                  ccs_objective_type_t  type);

extern ccs_error_t
ccs_objective_space_get_objective(ccs_objective_space_t  objective_space,
                                  ccs_expression_t      *objective_ret,
                                  ccs_objective_type_t  *type_ret);

#ifdef __cplusplus
}
#endif

#endif //_CCS_OBJECTIVE_SPACE
