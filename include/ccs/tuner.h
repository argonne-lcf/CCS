#ifndef _CCS_TUNER_H
#define _CCS_TUNER_H

#ifdef __cplusplus
extern "C" {
#endif

extern ccs_error_t
ccs_tuner_ask(ccs_tuner_t          tuner,
              size_t               num_configurations,
              ccs_configuration_t *configurations,
              size_t              *num_configurations_ret);

extern ccs_error_t
ccs_tuner_tell(ccs_tuner_t       tuner,
               size_t            num_evaluations,
               ccs_evaluation_t *evaluations);

extern ccs_error_t
ccs_tuner_get_optimal(ccs_tuner_t          tuner,
                      ccs_configuration_t *configuration_ret);

#ifdef __cplusplus
}
#endif

#endif //_CCS_TUNER_H
