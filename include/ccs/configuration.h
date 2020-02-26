// Configuration Interface

//   Creators, Management
extern ccs_error_t
ccs_create_configuration(ccs_configuration_space_t configuration_space,
                         size_t                    num_datum,
                         ccs_data_t               *vector,
                         void                     *user_data
                         ccs_configuration_t      *configuration_ret);

extern ccs_error_t
ccs_retain_configuration(ccs_configuration_t configuration);

extern ccs_error_t
ccs_release_configuration(ccs_configuration_t configuration);

//   Accessors
extern ccs_error_t
ccs_configuration_get_configuration_space(ccs_configuration_t        configuration,
                                          ccs_configuration_space_t *configuration_space_ret);

extern ccs_error_t
ccs_configuration_get_user_data(ccs_configuration_t   configuration,
                                void                **user_data);

extern ccs_error_t
ccs_configuration_get_value(ccs_configuration_t  configuration,
                            size_t               index,
                            ccs_data_t          *value);

extern ccs_error_t
ccs_configuration_get_vector(ccs_configuration_t  configuration,
                             size_t               num_datum,
                             ccs_data_t          *vector,
                             size_t              *num_datum_ret);

