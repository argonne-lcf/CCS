#ifndef _CCS_INTERVAL_H
#define _CCS_INTERVAL_H

#ifdef __cplusplus
extern "C" {
#endif

struct ccs_interval_s {
	ccs_numeric_type_t type;
	ccs_numeric_t   lower;
	ccs_numeric_t   upper;
	ccs_bool_t      lower_included;
	ccs_bool_t      upper_included;
};

typedef struct ccs_interval_s ccs_interval_t;

extern ccs_result_t
ccs_interval_empty(ccs_interval_t *interval, ccs_bool_t *empty_ret);

extern ccs_result_t
ccs_interval_intersect(ccs_interval_t *interval1,
		       ccs_interval_t *interval2,
                       ccs_interval_t *interval_res);

extern ccs_result_t
ccs_interval_union(ccs_interval_t *interval1,
		   ccs_interval_t *interval2,
                   ccs_interval_t *interval_res);

extern ccs_result_t
ccs_interval_equal(ccs_interval_t *interval1,
                   ccs_interval_t *interval2,
                   ccs_bool_t     *equal_res);

extern ccs_bool_t
ccs_interval_include(ccs_interval_t *interval, ccs_numeric_t value);
#ifdef __cplusplus
}
#endif

#endif //_CCS_INTERVAL_H

