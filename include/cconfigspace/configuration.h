#ifndef _CCS_CONFIGURATION_H
#define _CCS_CONFIGURATION_H

#ifdef __cplusplus
extern "C" {
#endif

// Configuration Interface

//   Creators, Management
extern ccs_error_t
ccs_create_configuration(ccs_configuration_space_t configuration_space,
                         size_t                    num_values,
                         ccs_datum_t              *values,
                         void                     *user_data,
                         ccs_configuration_t      *configuration_ret);

//   Accessors
extern ccs_error_t
ccs_configuration_get_configuration_space(ccs_configuration_t        configuration,
                                          ccs_configuration_space_t *configuration_space_ret);

extern ccs_error_t
ccs_configuration_get_user_data(ccs_configuration_t   configuration,
                                void                **user_data_ret);

extern ccs_error_t
ccs_configuration_get_value(ccs_configuration_t  configuration,
                            size_t               index,
                            ccs_datum_t         *value_ret);

extern ccs_error_t
ccs_configuration_set_value(ccs_configuration_t configuration,
                            size_t              index,
                            ccs_datum_t         value);

extern ccs_error_t
ccs_configuration_get_values(ccs_configuration_t  configuration,
                             size_t               num_values,
                             ccs_datum_t         *values,
                             size_t              *num_values_ret);

extern ccs_error_t
ccs_configuration_get_value_by_name(ccs_configuration_t  configuration,
                                    const char          *name,
                                    ccs_datum_t         *value_ret);

extern ccs_error_t
ccs_configuration_check(ccs_configuration_t configuration);

extern ccs_error_t
ccs_configuration_hash(ccs_configuration_t  configuration,
                       ccs_hash_t          *hash_ret);

extern ccs_error_t
ccs_configuration_cmp(ccs_configuration_t  configuration,
                      ccs_configuration_t  other_configuration,
                      int                 *equal_ret);
#ifdef __cplusplus
}
#endif

#endif //_CCS_CONFIGURATION_H
