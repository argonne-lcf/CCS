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
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_VALUE if \p rng_ret is NULL
 * @return -#CCS_OUT_OF_MEMORY if there was not enough memory to allocate the
 *                             new map
 */
extern ccs_result_t
ccs_create_map(ccs_map_t *map_ret);

extern ccs_result_t
ccs_map_set(ccs_map_t   map,
            ccs_datum_t key,
            ccs_datum_t value);

extern ccs_result_t
ccs_map_exist(ccs_map_t    map,
              ccs_datum_t  key,
              ccs_bool_t  *exist);

extern ccs_result_t
ccs_map_get(ccs_map_t    map,
            ccs_datum_t  key,
            ccs_datum_t *value_ret);

extern ccs_result_t
ccs_map_del(ccs_map_t    map,
            ccs_datum_t  key);

extern ccs_result_t
ccs_map_get_keys(ccs_map_t    map,
                 size_t       num_keys,
                 ccs_datum_t *keys,
                 size_t      *num_keys_ret);

extern ccs_result_t
ccs_map_get_values(ccs_map_t    map,
                   size_t       num_values,
                   ccs_datum_t *values,
                   size_t      *num_values_ret);

extern ccs_result_t
ccs_map_get_pairs(ccs_map_t    map,
                  size_t       num_pairs,
                  ccs_datum_t *keys,
                  ccs_datum_t *values,
                  size_t      *num_pairs_ret);

#ifdef __cplusplus
}
#endif

#endif //_CCS_MAP_H
