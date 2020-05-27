#ifndef _CCS_CONFIGURATION_SPACE
#define _CCS_CONFIGURATION_SPACE

#ifdef __cplusplus
extern "C" {
#endif

// Configuration Space Interface

//   Creators
extern ccs_error_t
ccs_create_configuration_space(const char                *name,
                               void                      *user_data,
                               ccs_configuration_space_t *configuration_space_ret);

//   Accessors
extern ccs_error_t
ccs_configuration_space_get_name(ccs_configuration_space_t   configuration_space,
                                 const char                **name_ret);

extern ccs_error_t
ccs_configuration_space_get_user_data(ccs_configuration_space_t   configuration_space,
                                      void                      **user_data_ret);

extern ccs_error_t
ccs_configuration_space_set_rng(ccs_configuration_space_t configuration_space,
                                ccs_rng_t                 rng);

extern ccs_error_t
ccs_configuration_space_get_rng(ccs_configuration_space_t  configuration_space,
                                ccs_rng_t                 *rng_ret);

//   List Accessors
extern ccs_error_t
ccs_configuration_space_add_hyperparameter(ccs_configuration_space_t configuration_space,
                                           ccs_hyperparameter_t      hyperparameter,
                                           ccs_distribution_t        distribution);

extern ccs_error_t
ccs_configuration_space_add_hyperparameters(ccs_configuration_space_t  configuration_space,
                                            size_t                     num_hyperparameters,
                                            ccs_hyperparameter_t      *hyperparameters,
                                            ccs_distribution_t        *distributions);

extern ccs_error_t
ccs_configuration_space_get_num_hyperparameters(ccs_configuration_space_t  configuration_space,
                                                size_t                     *num_hyperparameters_ret);

extern ccs_error_t
ccs_configuration_space_get_hyperparameter(ccs_configuration_space_t  configuration_space,
                                           size_t                     index,
                                           ccs_hyperparameter_t      *hyperparameter_ret);

extern ccs_error_t
ccs_configuration_space_get_hyperparameter_by_name(
		ccs_configuration_space_t  configuration_space,
		const char *               name,
		ccs_hyperparameter_t      *hyperparameter_ret);

extern ccs_error_t
ccs_configuration_space_get_hyperparameter_index_by_name(
		ccs_configuration_space_t  configuration_space,
		const char                *name,
		size_t                    *index_ret);

extern ccs_error_t
ccs_configuration_space_get_hyperparameters(ccs_configuration_space_t  configuration_space,
                                            size_t                     num_hyperparameters,
                                            ccs_hyperparameter_t      *hyperparameters,
                                            size_t                    *num_hyperparameters_ret);

extern ccs_error_t
ccs_configuration_space_add_condition(ccs_configuration_space_t configuration_space,
                                      ccs_condition_t           condition);

extern ccs_error_t
ccs_configuration_space_add_conditions(ccs_configuration_space_t  configuration_space,
                                       size_t                     num_conditions,
                                       ccs_condition_t           *conditions);

extern ccs_error_t
ccs_configuration_space_get_num_conditions(ccs_configuration_space_t  configuration_space,
                                           size_t                    *num_conditions_ret);

extern ccs_error_t
ccs_configuration_space_get_condition(ccs_configuration_space_t configuration_space,
                                      size_t                    index,
                                      ccs_condition_t          *condition_ret);

extern ccs_error_t
ccs_configuration_space_get_conditions(ccs_configuration_space_t  configuration_space,
                                       size_t                     num_conditions,
                                       ccs_condition_t           *conditions,
                                       size_t                    *num_conditions_ret);

extern ccs_error_t
ccs_configuration_space_add_forbidden_clause(ccs_configuration_space_t configuration_space,
                                             ccs_forbidden_clause_t    forbidden_clause);

extern ccs_error_t
ccs_configuration_space_add_forbidden_clauses(ccs_configuration_space_t  configuration_space,
                                              size_t                     num_forbidden_clauses,
                                              ccs_forbidden_clause_t    *forbidden_clauses);

extern ccs_error_t
ccs_configuration_space_get_num_forbidden_clauses(ccs_configuration_space_t  configuration_space,
                                                  size_t                    *num_forbidden_clauses_ret);

extern ccs_error_t
ccs_configuration_space_get_forbidden_clause(ccs_configuration_space_t configuration_space,
                                             size_t                    index,
                                             ccs_forbidden_clause_t   *forbidden_clause_ret);

extern ccs_error_t
ccs_configuration_space_get_forbidden_clauses(ccs_configuration_space_t  configuration_space,
                                              size_t                     num_forbidden_clauses,
                                              ccs_forbidden_clause_t    *forbidden_clauses,
                                              size_t                    *num_forbidden_clauses_ret);

//   Configuration related functions
extern ccs_error_t
ccs_configuration_space_check_configuration(ccs_configuration_space_t configuration_space,
                                            ccs_configuration_t       configuration);

extern ccs_error_t
ccs_configuration_space_check_configuration_values(ccs_configuration_space_t  configuration_space,
                                                   size_t                     num_values,
                                                   ccs_datum_t               *values);

extern ccs_error_t
ccs_configuration_space_get_default_configuration(ccs_configuration_space_t  configuration_space,
                                                  ccs_configuration_t       *configuration_ret);

extern ccs_error_t
ccs_configuration_space_sample_configuration(ccs_configuration_space_t  configuration_space,
                                             ccs_configuration_t       *configuration);

extern ccs_error_t
ccs_configuration_space_sample_configurations(ccs_configuration_space_t  configuration_space,
                                              size_t                     num_configurations,
                                              ccs_configuration_t       *configurations);

//   Hyperparameter related functions
extern ccs_error_t
ccs_configuration_space_get_active_hyperparameters(ccs_configuration_space_t  configuration_space,
                                                   size_t                     num_hyperparameters,
                                                   ccs_hyperparameter_t      *hyperparameters,
                                                   size_t                    *num_hyperparameters_ret);

extern ccs_error_t
ccs_configuration_space_get_conditional_hyperparameters(ccs_configuration_space_t  configuration_space,
                                                        size_t                     num_hyperparameters,
                                                        ccs_hyperparameter_t      *hyperparameters,
                                                        size_t                    *num_hyperparameters_ret);

extern ccs_error_t
ccs_configuration_space_get_unconditional_hyperparameters(ccs_configuration_space_t  configuration_space,
                                                          size_t                     num_hyperparameters,
                                                          ccs_hyperparameter_t      *hyperparameters,
                                                          size_t                    *num_hyperparameters_ret);

extern ccs_error_t
ccs_configuration_space_get_child_conditions_of(ccs_configuration_space_t  configuration_space,
                                                ccs_hyperparameter_t       hyperparameter,
                                                size_t                     num_conditions,
                                                ccs_condition_t           *conditions,
                                                size_t                    *num_conditions_ret);

extern ccs_error_t
ccs_configuration_space_get_children_of(ccs_configuration_space_t  configuration_space,
                                        ccs_hyperparameter_t       hyperparameter,
                                        size_t                     num_hyperparameters,
                                        ccs_hyperparameter_t      *hyperparameters,
                                        size_t                    *num_hyperparameters_ret);

extern ccs_error_t
ccs_configuration_space_get_parent_conditions_of(ccs_configuration_space_t  configuration_space,
                                                 ccs_hyperparameter_t       hyperparameter,
                                                 size_t                     num_conditions,
                                                 ccs_condition_t           *conditions,
                                                 size_t                    *num_conditions_ret);

extern ccs_error_t
ccs_configuration_space_get_parents_of(ccs_configuration_space_t  configuration_space,
                                       ccs_hyperparameter_t       hyperparameter,
                                       size_t                     num_hyperparameters,
                                       ccs_hyperparameter_t      *hyperparameters,
                                       size_t                    *num_hyperparameters_ret);

#ifdef __cplusplus
}
#endif

#endif //_CCS_CONFIGURATION_SPACE
