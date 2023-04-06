#ifndef _CCS_RNG_H
#define _CCS_RNG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <gsl/gsl_rng.h>

/**
 * @file rng.h
 * CCS rng define random number generators. For now they are wrappers over gsl
 * random number generators.
 */

/**
 * Create a new random number generator using the gsl default type (see
 * gsl_rng_default).
 * @param [out] rng_ret a pointer to the variable that will contain the returned
 *              random number generator
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_VALUE if \p rng_ret is NULL
 * @return #CCS_OUT_OF_MEMORY if there was not enough memory to allocate the
 *                             new random number generator
 */
extern ccs_error_t
ccs_create_rng(
	ccs_rng_t *rng_ret);

/**
 * Create a new random number generator using the provided gsl type (see
 * gsl_rng_type).
 * @param [in] rng_type a pointer to the type of gsl random number generator to
 *                      use.
 * @param [out] rng_ret a pointer to the variable that will contain the returned
 *              random number generator
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_VALUE if \p rng_ret or \p rng_type are NULL
 * @return #CCS_OUT_OF_MEMORY if there was not enough memory to allocate the
 *                             new random number generator
 */
extern ccs_error_t
ccs_create_rng_with_type(
	const gsl_rng_type *rng_type,
	ccs_rng_t          *rng_ret);

/**
 * Get the gsl type of a random number generator.
 * @param [in] rng
 * @param [out] rng_type_ret a pointer that will contained a pointer to the
 *                           returned gsl random number generator type.
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_OBJECT if \p rng is not a valid CCS random number
 *                              generator
 * @return #CCS_INVALID_VALUE if \p rng_type_ret is NULL
 */
extern ccs_error_t
ccs_rng_get_type(
	ccs_rng_t            rng,
	const gsl_rng_type **rng_type_ret);

/**
 * Set the seed of a random number generator.
 * @param [in] rng
 * @param [in] seed the seed to use with the random number generator
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_OBJECT if \p rng is not a valid CCS random number
 *                              generator
 */
extern ccs_error_t
ccs_rng_set_seed(
	ccs_rng_t         rng,
	unsigned long int seed);

/**
 * Get a random integer from a random number generator. Integer is contained
 * between the value returned by #ccs_rng_min and #ccs_rng_max, both included.
 * @param [in] rng
 * @param [out] value_ret a pointer to the variable that will contain the
 *                        returned value
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_OBJECT if \p rng is not a valid CCS random number
 *                              generator
 * @return #CCS_INVALID_VALUE if \p value_ret is NULL
 */
extern ccs_error_t
ccs_rng_get(
	ccs_rng_t          rng,
	unsigned long int *value_ret);

/**
 * Get a random floating point value uniformly sampled in the interval [0.0,
 * 1.0).
 * @param [in] rng
 * @param [out] value_ret a pointer to the variable that will contain the
 *                        returned value
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_OBJECT if \p rng is not a valid CCS random number
 *                              generator
 * @return #CCS_INVALID_VALUE if \p value_ret is NULL
 */
extern ccs_error_t
ccs_rng_uniform(
	ccs_rng_t    rng,
	ccs_float_t *value_ret);

/**
 * Get the underlying gsl random number generator.
 * @param [in] rng
 * @param [out] gsl_rng_ret a pointer to the variable that will contain a
 *                          pointer to the underlying random number generator
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_OBJECT if \p rng is not a valid CCS random number
 *                              generator
 * @return #CCS_INVALID_VALUE if \p gsl_rng_ret is NULL
 */
extern ccs_error_t
ccs_rng_get_gsl_rng(
	ccs_rng_t   rng,
	gsl_rng   **gsl_rng_ret);

/**
 * Get the minimum value that can be returned by #ccs_rng_get.
 * @param [in] rng
 * @param [out] value_ret a pointer to the variable that will contain the
 *                        returned value
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_OBJECT if \p rng is not a valid CCS random number
 *                              generator
 * @return #CCS_INVALID_VALUE if \p value_ret is NULL
 */
extern ccs_error_t
ccs_rng_min(
	ccs_rng_t          rng,
	unsigned long int *value_ret);

/**
 * Get the maximum value that can be returned by #ccs_rng_get.
 * @param [in] rng
 * @param [out] value_ret a pointer to the variable that will contain the
 *                        returned value
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_OBJECT if \p rng is not a valid CCS random number
 *                              generator
 * @return #CCS_INVALID_VALUE if \p value_ret is NULL
 */
extern ccs_error_t
ccs_rng_max(
	ccs_rng_t          rng,
	unsigned long int *value_ret);

#ifdef __cplusplus
}
#endif

#endif //_CCS_RNG_H
