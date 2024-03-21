#ifndef _CCS_MAP_H
#define _CCS_MAP_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file map.h
 * CCS map defines a key-value store.
 */

/**
 * Create a new map.
 * @param [out] map_ret a pointer to the variable that will contain the returned
 *                      map
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p rng_ret is NULL
 * @return #CCS_RESULT_ERROR_OUT_OF_MEMORY if there was not enough memory to
 * allocate the new map
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_create_map(ccs_map_t *map_ret);

/**
 * Associate a key to a value in a map
 * param[in,out] map
 * param[in] key if a transient string it will be memoized, if a CCS object it
 *               will be retained unless #CCS_DATUM_FLAG_ID is used.
 * param[in] value if a transient string it will be memoized, if a CCS object
 *                 it will be retained unless #CCS_DATUM_FLAG_ID is used.
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p map is not a valid CCS map
 * @return #CCS_RESULT_ERROR_OUT_OF_MEMORY if there was not enough memory to
 * allocate data structures
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_map_set(ccs_map_t map, ccs_datum_t key, ccs_datum_t value);

/**
 * Check if a key exists in a map
 * param[in] map
 * param[in] key
 * param[out] exist a pointer to a variable that will hold the result of the
 *                  search
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p map is not a valid CCS map
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p exist is NULL
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_map_exist(ccs_map_t map, ccs_datum_t key, ccs_bool_t *exist);

/**
 * Get the value associated with a key
 * param[in] map
 * param[in] key
 * param[out] value_ret a pointer to a variable that will hold the returned
 *                      value or ccs_none if not found
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p map is not a valid CCS map
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p value_ret is NULL
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_map_get(ccs_map_t map, ccs_datum_t key, ccs_datum_t *value_ret);

/**
 * Delete a key in a map
 * param[in,out] map
 * param[in] key
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p map is not a valid CCS map
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p key does not exist in \p map
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_map_del(ccs_map_t map, ccs_datum_t key);

/**
 * Get the keys contained in a map
 * @param[in] map
 * @param[in] num_keys the number of keys that can be added to \p
 *                     keys. If \p keys is not NULL, num_keys must be greater
 *                     than 0
 * @param[out] keys an array of \p num_keys ccs_datum_t that will contain the
 *                  returned keys of NULL. If the array is too big, extra
 *                  values will be set to ccs_none
 * @param[out] num_keys_ret a pointer to a variable that will contain the
 *                          number of keys that are or would be returned. Can
 *                          be NULL
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p map is not a valid CCS map
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p keys is NULL and \p num_keys is
 * greater than 0; or if \p keys is NULL and num_keys_ret is NULL; or if \p
 * num_keys is less than the number of keys that would be returned
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_map_get_keys(
	ccs_map_t    map,
	size_t       num_keys,
	ccs_datum_t *keys,
	size_t      *num_keys_ret);

/**
 * Get the values contained in a map
 * @param[in] map
 * @param[in] num_values the number of values that can be added to \p
 *                     values. If \p values is not NULL, num_values must be
 * greater than 0
 * @param[out] values an array of \p num_values ccs_datum_t that will contain
 *                    the returned values of NULL. If the array is too big,
 *                    extra values will be set to ccs_none
 * @param[out] num_values_ret a pointer to a variable that will contain the
 *                          number of values that are or would be returned. Can
 *                          be NULL
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p map is not a valid CCS map
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p values is NULL and \p
 * num_values is greater than 0; or if \p values is NULL and num_values_ret is
 * NULL; or if \p num_values is less than the number of values that would be
 * returned
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_map_get_values(
	ccs_map_t    map,
	size_t       num_values,
	ccs_datum_t *values,
	size_t      *num_values_ret);

/**
 * Get the keys and values contained in a map
 * @param[in] map
 * @param[in] num_pairs the number of keys that can be added to \p
 *                     keys. If \p keys is not NULL, num_pairs must be greater
 *                     than 0
 * @param[out] keys an array of \p num_pairs ccs_datum_t that will contain the
 *                  returned keys of NULL. If the array is too big, extra
 *                  values will be set to ccs_none
 * @param[out] values an array of \p num_pairs ccs_datum_t that will contain
 *                    the returned values of NULL. If the array is too big,
 *                    extra values will be set to ccs_none
 * @param[out] num_pairs_ret a pointer to a variable that will contain the
 *                          number of keys that are or would be returned. Can
 *                          be NULL
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p map is not a valid CCS map
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p keys is NULL and \p num_pairs
 * is greater than 0; or if \p keys is NULL and num_pairs_ret is NULL; or if \p
 * num_pairs is less than the number of values that would be returned; if \p
 * values is NULL and \p num_pairs is greater than 0; or if \p values is NULL
 * and num_pairs_ret is NULL; or if \p num_pairs is less than the number of
 * values that would be returned
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_map_get_pairs(
	ccs_map_t    map,
	size_t       num_pairs,
	ccs_datum_t *keys,
	ccs_datum_t *values,
	size_t      *num_pairs_ret);

/**
 * Remove all pairs from a map
 * @param[in,out] map
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p map is not a valid CCS map
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_map_clear(ccs_map_t map);

#ifdef __cplusplus
}
#endif

#endif //_CCS_MAP_H
