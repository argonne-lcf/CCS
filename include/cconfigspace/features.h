#ifndef _CCS_FEATURES
#define _CCS_FEATURES

#ifdef __cplusplus
extern "C" {
#endif

extern ccs_result_t
ccs_create_features(ccs_features_space_t features_space,
                    size_t               num_values,
                    ccs_datum_t         *values,
                    void                *user_data,
                    ccs_features_t      *features_ret);

//   Accessors
extern ccs_result_t
ccs_features_get_features_space(ccs_features_t        features,
                                ccs_features_space_t *features_space_ret);

extern ccs_result_t
ccs_features_get_user_data(ccs_features_t   features,
                           void           **user_data_ret);

extern ccs_result_t
ccs_features_get_value(ccs_features_t  features,
                       size_t          index,
                       ccs_datum_t    *value_ret);

extern ccs_result_t
ccs_features_set_value(ccs_features_t features,
                       size_t         index,
                       ccs_datum_t    value);

extern ccs_result_t
ccs_features_get_values(ccs_features_t  features,
                        size_t          num_values,
                        ccs_datum_t    *values,
                        size_t         *num_values_ret);

extern ccs_result_t
ccs_features_get_value_by_name(ccs_features_t  features,
                               const char     *name,
                               ccs_datum_t    *value_ret);

extern ccs_result_t
ccs_features_check(ccs_features_t features);

extern ccs_result_t
ccs_features_hash(ccs_features_t  features,
                  ccs_hash_t     *hash_ret);

extern ccs_result_t
ccs_features_cmp(ccs_features_t  features,
                 ccs_features_t  other_features,
                 int            *equal_ret);

#ifdef __cplusplus
}
#endif

#endif //_CCS_FEATURES
