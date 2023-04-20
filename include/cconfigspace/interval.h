#ifndef _CCS_INTERVAL_H
#define _CCS_INTERVAL_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file interval.h
 * Interval define ranges over numeric values.
 */

/**
 * A structure defining an interval over numeric values
 */
struct ccs_interval_s {
	/** The type of numeric value */
	ccs_numeric_type_t type;
	/** The lower bound of the interval */
	ccs_numeric_t      lower;
	/** The upper bound of the interval */
	ccs_numeric_t      upper;
	/** Is the lower bound included in the interval */
	ccs_bool_t         lower_included;
	/** Is the upper boud included in the interval */
	ccs_bool_t         upper_included;
};

/**
 * A commodity type to represent CCS intervals.
 */
typedef struct ccs_interval_s ccs_interval_t;

/**
 * Check if an interval is empty.
 * @param[in] interval a pointer to the interval to check
 * @param[out] empty_ret a pointer to the variable that will contain the check
 *                       result
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_VALUE if \p interval or \p empty_ret are NULL
 */
extern ccs_result_t
ccs_interval_empty(ccs_interval_t *interval, ccs_bool_t *empty_ret);

/**
 * Compute the intersection of two intervals.
 * @param[in] interval1 a pointer to the first interval
 * @param[in] interval2 a pointer to the second interval
 * @param[out] interval_res a pointer to the variable that will contain the
 *                          intersection of \p interval1 and \p interval2
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_VALUE if \p interval1 or \p interval2 or\p interval_res
 *                             are NULL
 * @return #CCS_INVALID_TYPE if \p interval1 and \p interval2 are intervals
 *                            over different data types
 */
extern ccs_result_t
ccs_interval_intersect(
	ccs_interval_t *interval1,
	ccs_interval_t *interval2,
	ccs_interval_t *interval_res);

/**
 * Compute the union of two intervals.
 * @param[in] interval1 a pointer to the first interval
 * @param[in] interval2 a pointer to the second interval
 * @param[out] interval_res a pointer to the variable that will contain the
 *                          union of \p interval1 and \p interval2
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_VALUE if \p interval1 or \p interval2 or \p
 *                             interval_res are NULL
 * @return #CCS_INVALID_TYPE if \p interval1 and \p interval2 are intervals
 *                            over different data types
 */
extern ccs_result_t
ccs_interval_union(
	ccs_interval_t *interval1,
	ccs_interval_t *interval2,
	ccs_interval_t *interval_res);

/**
 * Test the equality of two intervals.
 * @param[in] interval1 a pointer to the first interval
 * @param[in] interval2 a pointer to the second interval
 * @param[out] equal_res a pointer to the variable that will contain the test
 *                       result
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_VALUE if \p interval1 or \p interval2 or \p equal_res
 *                             are NULL
 * @return #CCS_INVALID_TYPE if \p interval1 and \p interval2 are intervals
 *                            over different data types
 */
extern ccs_result_t
ccs_interval_equal(
	ccs_interval_t *interval1,
	ccs_interval_t *interval2,
	ccs_bool_t     *equal_res);

/**
 * Test the inclusion of a numeric value into an interval. The user must pass a
 * numeric of the same type as the interval, else results are undefined.
 * @param[in] interval a pointer to the interval
 * @param[in] value the value to check for inclusion in the interval
 * @return #CCS_TRUE if the interval include the value
 * @return #CCS_FALSE if the interval does not include the value; or if \p
 *                    interval is NULL
 */
extern ccs_bool_t
ccs_interval_include(ccs_interval_t *interval, ccs_numeric_t value);
#ifdef __cplusplus
}
#endif

#endif //_CCS_INTERVAL_H
