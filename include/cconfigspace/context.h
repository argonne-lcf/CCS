#ifndef _CCS_CONTEXT_H
#define _CCS_CONTEXT_H

#ifdef __cplusplus
extern "C" {
#endif

extern ccs_result_t
ccs_context_get_name(ccs_context_t   context,
                     const char    **name_ret);

extern ccs_result_t
ccs_context_get_user_data(ccs_context_t   context,
                          void          **user_data_ret);

extern ccs_result_t
ccs_context_get_hyperparameter_index(ccs_context_t         context,
                                     ccs_hyperparameter_t  hyperparameter,
                                     size_t               *index_ret);

extern ccs_result_t
ccs_context_get_num_hyperparameters(ccs_context_t  context,
                                    size_t        *num_hyperparameters_ret);

extern ccs_result_t
ccs_context_get_hyperparameter(ccs_context_t         context,
                               size_t                index,
                               ccs_hyperparameter_t *hyperparameter_ret);

extern ccs_result_t
ccs_context_get_hyperparameter_by_name(ccs_context_t         context,
                                       const char *          name,
                                       ccs_hyperparameter_t *hyperparameter_ret);

extern ccs_result_t
ccs_context_get_hyperparameter_index_by_name(ccs_context_t  context,
                                             const char    *name,
                                             size_t        *index_ret);

extern ccs_result_t
ccs_context_get_hyperparameters(ccs_context_t          context,
                                size_t                 num_hyperparameters,
                                ccs_hyperparameter_t  *hyperparameters,
                                size_t                *num_hyperparameters_ret);

extern ccs_result_t
ccs_context_get_hyperparameter_indexes(
		ccs_context_t          context,
		size_t                 num_hyperparameters,
		ccs_hyperparameter_t  *hyperparameters,
		size_t                *indexes);

#ifdef __cplusplus
}
#endif

#endif //_CCS_CONTEXT_H
