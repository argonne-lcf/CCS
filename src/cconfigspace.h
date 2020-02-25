typedef struct cs_configuration_space_s *cs_configuration_space_t;
typedef struct cs_configuration_s *cs_configuration_t;
typedef struct cs_hyper_parameter_s *cs_hyper_parameter_t;
typedef struct cs_condition_s *cs_condition_t;
typedef struct cs_forbidden_clause_s *cs_forbidden_clause_t;

enum cs_error_e {
  CS_SUCCESS
};

typedef enum cs_error_e cs_error_t;


// Configuration Space Interface

//   Creators/deletion
extern cs_error_t
cs_create_configuration_space(cs_configuration_space_t *configuration_space_ret);

extern cs_error_t
cs_release_configuration_space(cs_configuration_space_t configuration_space);

//   Accessors
extern cs_error_t
cs_configuration_space_set_name(cs_configuration_space_t  configuration_space,
                                const char               *name);

extern cs_error_t
cs_configuration_space_get_name(cs_configuration_space_t   configuration_space,
                                char                     **name);

extern cs_error_t
cs_configuration_space_set_user_data(cs_configuration_space_t  configuration_space,
                                     void                     *user_data);

extern cs_error_t
cs_configuration_space_get_user_data(cs_configuration_space_t   configuration_space,
                                     void                     **user_data);

extern cs_error_t
cs_configuration_space_set_seed(cs_configuration_space_t configuration_space,
                                int64_t                  seed);

extern cs_error_t
cs_configuration_space_get_seed(cs_configuration_space_t  configuration_space,
                                int64_t                  *seed);

//   List Accessors
extern cs_error_t
cs_configuration_space_add_hyper_parameter(cs_configuration_space_t config_space,
                                           cs_hyper_parameter_t     hyper_parameter);

extern cs_error_t
cs_configuration_space_add_hyper_parameters(cs_configuration_space_t  config_space,
                                            size_t                    num_hyper_parameters,
                                            cs_hyper_parameter_t     *hyper_parameters);

extern cs_error_t
cs_configuration_space_get_num_hyper_parameters(cs_configuration_space_t  config_space,
                                                size_t                   *num_hyper_parameters);

extern cs_error_t
cs_configuration_space_get_hyper_parameter(cs_configuration_space_t config_space,
                                           size_t                   index);

extern cs_error_t
cs_configuration_space_get_hyper_parameters(cs_configuration_space_t  config_space,
                                            size_t                    num_hyper_parameters,
                                            cs_hyper_parameter_t     *hyper_parameters,
                                            size_t                   *num_hyper_parameters_ret);

extern cs_error_t
cs_configuration_space_add_condition(cs_configuration_space_t config_space,
                                     cs_condition_t           condition);

extern cs_error_t
cs_configuration_space_add_conditions(cs_configuration_space_t  config_space,
                                      size_t                    num_conditions,
                                      cs_condition_t           *conditions);

extern cs_error_t
cs_configuration_space_get_conditions(cs_configuration_space_t  config_space,
                                      size_t                    num_conditions,
                                      cs_condition_t           *conditions,
                                      size_t                   *num_conditions_ret);

extern cs_error_t
cs_configuration_space_add_forbidden_clause(cs_configuration_space_t config_space,
                                            cs_forbidden_clause_t    forbidden_clause);

extern cs_error_t
cs_configuration_space_add_forbidden_clauses(cs_configuration_space_t  config_space, 
                                             size_t                    num_forbidden_clauses,
                                             cs_forbidden_clause_t    *forbidden_clauses);

extern cs_error_t
cs_configuration_space_get_forbidden_clauses(cs_configuration_space_t  config_space, 
                                             size_t                    num_forbidden_clauses,
                                             cs_forbidden_clause_t    *forbidden_clauses
                                             size_t                   *num_forbidden_clauses_ret);

