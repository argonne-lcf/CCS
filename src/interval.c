#include "cconfigspace_internal.h"

ccs_result_t
ccs_interval_empty(ccs_interval_t *interval, ccs_bool_t *empty_ret)
{
	CCS_REFUTE(!interval || !empty_ret, CCS_INVALID_VALUE);
	// Empty ranges
	// [l,u] when l> u; ]l,u] when l>=u
	// [l,u[ when l>=u; ]l,u[ when l>=u
	if (interval->type == CCS_NUMERIC_TYPE_INT) {
		if (interval->upper_included && interval->lower_included ?
			    interval->lower.i > interval->upper.i :
			    interval->lower.i >= interval->upper.i) {
			*empty_ret = CCS_TRUE;
		} else
			*empty_ret = CCS_FALSE;
	} else {
		if (interval->upper_included && interval->lower_included ?
			    interval->lower.f > interval->upper.f :
			    interval->lower.f >= interval->upper.f) {
			*empty_ret = CCS_TRUE;
		} else
			*empty_ret = CCS_FALSE;
	}
	return CCS_SUCCESS;
}
#define MERGE_MAX(l1, li1, l2, li2, l, li)                                     \
	{                                                                      \
		if (l1 > l2) {                                                 \
			l  = l1;                                               \
			li = li1;                                              \
		} else if (l2 > l1) {                                          \
			l  = l2;                                               \
			li = li2;                                              \
		} else {                                                       \
			if (li1) {                                             \
				l  = l2;                                       \
				li = li2;                                      \
			} else {                                               \
				l  = l1;                                       \
				li = li1;                                      \
			}                                                      \
		}                                                              \
	}

#define MERGE_MIN(l1, li1, l2, li2, l, li)                                     \
	{                                                                      \
		if (l1 < l2) {                                                 \
			l  = l1;                                               \
			li = li1;                                              \
		} else if (l2 < l1) {                                          \
			l  = l2;                                               \
			li = li2;                                              \
		} else {                                                       \
			if (li1) {                                             \
				l  = l2;                                       \
				li = li2;                                      \
			} else {                                               \
				l  = l1;                                       \
				li = li1;                                      \
			}                                                      \
		}                                                              \
	}

ccs_result_t
ccs_interval_intersect(
	ccs_interval_t *interval1,
	ccs_interval_t *interval2,
	ccs_interval_t *interval_res)
{
	CCS_REFUTE(
		!interval1 || !interval2 || !interval_res, CCS_INVALID_VALUE);
	CCS_REFUTE(interval1->type != interval2->type, CCS_INVALID_TYPE);
	if (interval1->type == CCS_NUMERIC_TYPE_FLOAT) {
		interval_res->type = CCS_NUMERIC_TYPE_FLOAT;
		MERGE_MAX(
			interval1->lower.f, interval1->lower_included,
			interval2->lower.f, interval2->lower_included,
			interval_res->lower.f, interval_res->lower_included);
		MERGE_MIN(
			interval1->upper.f, interval1->upper_included,
			interval2->upper.f, interval2->upper_included,
			interval_res->upper.f, interval_res->upper_included);
	} else {
		interval_res->type = CCS_NUMERIC_TYPE_INT;
		MERGE_MAX(
			interval1->lower.i, interval1->lower_included,
			interval2->lower.i, interval2->lower_included,
			interval_res->lower.i, interval_res->lower_included);
		MERGE_MIN(
			interval1->upper.i, interval1->upper_included,
			interval2->upper.i, interval2->upper_included,
			interval_res->upper.i, interval_res->upper_included);
	}
	return CCS_SUCCESS;
}

ccs_result_t
ccs_interval_union(
	ccs_interval_t *interval1,
	ccs_interval_t *interval2,
	ccs_interval_t *interval_res)
{
	CCS_REFUTE(
		!interval1 || !interval2 || !interval_res, CCS_INVALID_VALUE);
	CCS_REFUTE(interval1->type != interval2->type, CCS_INVALID_TYPE);
	if (interval1->type == CCS_NUMERIC_TYPE_FLOAT) {
		interval_res->type = CCS_NUMERIC_TYPE_FLOAT;
		MERGE_MIN(
			interval1->lower.f, interval1->lower_included,
			interval2->lower.f, interval2->lower_included,
			interval_res->lower.f, interval_res->lower_included);
		MERGE_MAX(
			interval1->upper.f, interval1->upper_included,
			interval2->upper.f, interval2->upper_included,
			interval_res->upper.f, interval_res->upper_included);
	} else {
		interval_res->type = CCS_NUMERIC_TYPE_INT;
		MERGE_MIN(
			interval1->lower.i, interval1->lower_included,
			interval2->lower.i, interval2->lower_included,
			interval_res->lower.i, interval_res->lower_included);
		MERGE_MAX(
			interval1->upper.i, interval1->upper_included,
			interval2->upper.i, interval2->upper_included,
			interval_res->upper.i, interval_res->upper_included);
	}
	return CCS_SUCCESS;
}

ccs_result_t
ccs_interval_equal(
	ccs_interval_t *interval1,
	ccs_interval_t *interval2,
	ccs_bool_t     *equal_res)
{
	CCS_REFUTE(!interval1 || !interval2 || !equal_res, CCS_INVALID_VALUE);
	CCS_REFUTE(interval1->type != interval2->type, CCS_INVALID_TYPE);
	if (interval1->type == CCS_NUMERIC_TYPE_FLOAT) {
		*equal_res =
			(interval1->lower.f == interval2->lower.f &&
			 interval1->upper.f == interval2->upper.f &&
			 interval1->lower_included ==
				 interval2->lower_included &&
			 interval1->upper_included ==
				 interval2->upper_included);
	} else {
		ccs_int_t l1, u1, l2, u2;
		l1         = interval1->lower_included ? interval1->lower.i :
							 interval1->lower.i + 1;
		u1         = interval1->upper_included ? interval1->upper.i :
							 interval1->upper.i - 1;
		l2         = interval2->lower_included ? interval2->lower.i :
							 interval2->lower.i + 1;
		u2         = interval2->upper_included ? interval2->upper.i :
							 interval2->upper.i - 1;
		*equal_res = (l1 == l2 && u1 == u2);
	}
	return CCS_SUCCESS;
}

ccs_bool_t
ccs_interval_include(ccs_interval_t *interval, ccs_numeric_t value)
{
	if (CCS_UNLIKELY(!interval))
		return CCS_FALSE;
	return _ccs_interval_include(interval, value);
}
