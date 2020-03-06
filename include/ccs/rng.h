#ifndef _CCS_RNG_H
#define _CCS_RNG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <gsl/gsl_rng.h>

extern ccs_error_t
ccs_rng_create(ccs_rng_t *rng_ret);

extern ccs_error_t
ccs_rng_create_with_type(const gsl_rng_type *rng_type,
                         ccs_rng_t          *rng_ret);

extern ccs_error_t
ccs_rng_get_type(ccs_rng_t            rng,
                 const gsl_rng_type **rng_type_ret);

extern ccs_error_t
ccs_rng_set_seed(ccs_rng_t         rng,
                 unsigned long int seed);

extern ccs_error_t
ccs_rng_get(ccs_rng_t          rng,
            unsigned long int *value_ret);

extern ccs_error_t
ccs_rng_uniform(ccs_rng_t    rng,
                ccs_float_t *value_ret);

extern ccs_error_t
ccs_rng_get_gsl_rng(ccs_rng_t   rng,
                    gsl_rng   **gsl_rng_ret);

extern ccs_error_t
ccs_rng_min(ccs_rng_t          rng,
            unsigned long int *value_ret);

extern ccs_error_t
ccs_rng_max(ccs_rng_t          rng,
            unsigned long int *value_ret);

#ifdef __cplusplus
}
#endif

#endif //_CCS_RNG_H
