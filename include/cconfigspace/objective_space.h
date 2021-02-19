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

extern ccs_result_t
ccs_create_objective_space(const char            *name,
                           void                  *user_data,
                           ccs_objective_space_t *objective_space_ret);

extern ccs_result_t
ccs_objective_space_get_name(ccs_objective_space_t   objective_space,
                             const char            **name_ret);

extern ccs_result_t
ccs_objective_space_get_user_data(ccs_objective_space_t   objective_space,
                                  void                  **user_data_ret);

extern ccs_result_t
ccs_objective_space_add_hyperparameter(ccs_objective_space_t objective_space,
                                       ccs_hyperparameter_t  hyperparameter);

extern ccs_result_t
ccs_objective_space_add_hyperparameters(ccs_objective_space_t  objective_space,
                                        size_t                 num_hyperparameters,
                                        ccs_hyperparameter_t  *hyperparameters);

extern ccs_result_t
ccs_objective_space_get_num_hyperparameters(
		ccs_objective_space_t  objective_space,
		size_t                *num_hyperparameters_ret);

extern ccs_result_t
ccs_objective_space_get_hyperparameter(ccs_objective_space_t  objective_space,
                                       size_t                 index,
                                       ccs_hyperparameter_t  *hyperparameter_ret);

extern ccs_result_t
ccs_objective_space_get_hyperparameter_by_name(
		ccs_objective_space_t  objective_space,
		const char *           name,
		ccs_hyperparameter_t  *hyperparameter_ret);

extern ccs_result_t
ccs_objective_space_get_hyperparameter_index_by_name(
		ccs_objective_space_t  objective_space,
		const char            *name,
		size_t                *index_ret);

extern ccs_result_t
ccs_objective_space_get_hyperparameter_index(
		ccs_objective_space_t  objective_space,
		ccs_hyperparameter_t   hyperparameter,
		size_t                *index_ret);

extern ccs_result_t
ccs_objective_space_get_hyperparameter_indexes(
		ccs_objective_space_t  objective_space,
		size_t                 num_hyperparameters,
		ccs_hyperparameter_t  *hyperparameters,
		size_t                *indexes);

extern ccs_result_t
ccs_objective_space_get_hyperparameters(ccs_objective_space_t  objective_space,
                                        size_t                 num_hyperparameters,
                                        ccs_hyperparameter_t  *hyperparameters,
                                        size_t                *num_hyperparameters_ret);

extern ccs_result_t
ccs_objective_space_check_evaluation(ccs_objective_space_t objective_space,
                                     ccs_evaluation_t      evaluation);

extern ccs_result_t
ccs_objective_space_check_evaluation_values(ccs_objective_space_t  objective_space,
                                            size_t                 num_values,
                                            ccs_datum_t           *values);

extern ccs_result_t
ccs_objective_space_validate_value(ccs_objective_space_t  objective_space,
                                   size_t                 index,
                                   ccs_datum_t            value,
                                   ccs_datum_t           *value_ret);

extern ccs_result_t
ccs_objective_space_add_objective(ccs_objective_space_t objective_space,
                                  ccs_expression_t      expression,
                                  ccs_objective_type_t  type);

extern ccs_result_t
ccs_objective_space_add_objectives(ccs_objective_space_t  objective_space,
                                   size_t                 num_objectives,
                                   ccs_expression_t      *expressions,
                                   ccs_objective_type_t  *types);

extern ccs_result_t
ccs_objective_space_get_objective(ccs_objective_space_t  objective_space,
                                  size_t                 index,
                                  ccs_expression_t      *expression_ret,
                                  ccs_objective_type_t  *type_ret);
extern ccs_result_t
ccs_objective_space_get_objectives(ccs_objective_space_t  objective_space,
                                   size_t                 num_objectives,
                                   ccs_expression_t      *expressions,
                                   ccs_objective_type_t  *types,
                                   size_t                *num_objectives_ret);

#ifdef __cplusplus
}
#endif

#endif //_CCS_OBJECTIVE_SPACE
