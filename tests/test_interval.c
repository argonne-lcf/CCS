#include <assert.h>
#include <cconfigspace.h>

void test_empty_float() {
	ccs_interval_t interval;
	ccs_bool_t     empty;
	ccs_result_t   err;

	interval.type = CCS_NUM_FLOAT;
	interval.lower.f = -3.0;
	interval.upper.f = 5.0;
	interval.lower_included = CCS_TRUE;
	interval.upper_included = CCS_TRUE;

	err = ccs_interval_empty(&interval, &empty);
	assert( err == CCS_SUCCESS );
	assert( !empty );

	interval.upper.f = -5.0;
	err = ccs_interval_empty(&interval, &empty);
	assert( err == CCS_SUCCESS );
	assert( empty );

	interval.upper.f = -3.0;
	err = ccs_interval_empty(&interval, &empty);
	assert( err == CCS_SUCCESS );
	assert( !empty );

	interval.lower_included = CCS_FALSE;
	err = ccs_interval_empty(&interval, &empty);
	assert( err == CCS_SUCCESS );
	assert( empty );

	interval.upper_included = CCS_FALSE;
	err = ccs_interval_empty(&interval, &empty);
	assert( err == CCS_SUCCESS );
	assert( empty );

	interval.lower_included = CCS_TRUE;
	err = ccs_interval_empty(&interval, &empty);
	assert( err == CCS_SUCCESS );
	assert( empty );
}

void test_empty_int() {
	ccs_interval_t interval;
	ccs_bool_t     empty;
	ccs_result_t   err;

	interval.type = CCS_NUM_INTEGER;
	interval.lower.i = -3;
	interval.upper.i = 5;
	interval.lower_included = CCS_TRUE;
	interval.upper_included = CCS_TRUE;

	err = ccs_interval_empty(&interval, &empty);
	assert( err == CCS_SUCCESS );
	assert( !empty );

	interval.upper.i = -5;
	err = ccs_interval_empty(&interval, &empty);
	assert( err == CCS_SUCCESS );
	assert( empty );

	interval.upper.i = -3;
	err = ccs_interval_empty(&interval, &empty);
	assert( err == CCS_SUCCESS );
	assert( !empty );

	interval.lower_included = CCS_FALSE;
	err = ccs_interval_empty(&interval, &empty);
	assert( err == CCS_SUCCESS );
	assert( empty );

	interval.upper_included = CCS_FALSE;
	err = ccs_interval_empty(&interval, &empty);
	assert( err == CCS_SUCCESS );
	assert( empty );

	interval.lower_included = CCS_TRUE;
	err = ccs_interval_empty(&interval, &empty);
	assert( err == CCS_SUCCESS );
	assert( empty );
}

void test_intersect_float() {
	ccs_interval_t interval1, interval2, intersection;
	ccs_bool_t     empty;
	ccs_result_t   err;

	interval1.type = CCS_NUM_FLOAT;
	interval1.lower.f = -3.0;
	interval1.upper.f = 5.0;
	interval1.lower_included = CCS_TRUE;
	interval1.upper_included = CCS_FALSE;

	interval2.type = CCS_NUM_FLOAT;
	interval2.lower.f = 2.0;
	interval2.upper.f = 7.0;
	interval2.lower_included = CCS_TRUE;
	interval2.upper_included = CCS_TRUE;

	err = ccs_interval_intersect(&interval1, &interval2, &intersection);
	assert( err == CCS_SUCCESS );
	err = ccs_interval_empty(&intersection, &empty);
	assert( err == CCS_SUCCESS );
	assert( !empty );
	assert( intersection.type == CCS_NUM_FLOAT );
	assert( intersection.lower.f == 2.0 );
	assert( intersection.lower_included == CCS_TRUE );
	assert( intersection.upper.f == 5.0 );
	assert( intersection.upper_included == CCS_FALSE );

}

void test_intersect_int() {
	ccs_interval_t interval1, interval2, intersection;
	ccs_bool_t     empty;
	ccs_result_t   err;

	interval1.type = CCS_NUM_INTEGER;
	interval1.lower.i = -3;
	interval1.upper.i = 5;
	interval1.lower_included = CCS_TRUE;
	interval1.upper_included = CCS_FALSE;

	interval2.type = CCS_NUM_INTEGER;
	interval2.lower.i = 2;
	interval2.upper.i = 7;
	interval2.lower_included = CCS_TRUE;
	interval2.upper_included = CCS_TRUE;

	err = ccs_interval_intersect(&interval1, &interval2, &intersection);
	assert( err == CCS_SUCCESS );
	err = ccs_interval_empty(&intersection, &empty);
	assert( err == CCS_SUCCESS );
	assert( !empty );
	assert( intersection.type == CCS_NUM_INTEGER );
	assert( intersection.lower.i == 2 );
	assert( intersection.lower_included == CCS_TRUE);
	assert( intersection.upper.i == 5 );
	assert( intersection.upper_included == CCS_FALSE);

}

void test_union_float() {
	ccs_interval_t interval1, interval2, u;
	ccs_bool_t     empty;
	ccs_result_t   err;

	interval1.type = CCS_NUM_FLOAT;
	interval1.lower.f = -3.0;
	interval1.upper.f = 5.0;
	interval1.lower_included = CCS_TRUE;
	interval1.upper_included = CCS_FALSE;

	interval2.type = CCS_NUM_FLOAT;
	interval2.lower.f = 2.0;
	interval2.upper.f = 7.0;
	interval2.lower_included = CCS_TRUE;
	interval2.upper_included = CCS_TRUE;

	err = ccs_interval_union(&interval1, &interval2, &u);
	assert( err == CCS_SUCCESS );
	err = ccs_interval_empty(&u, &empty);
	assert( err == CCS_SUCCESS );
	assert( !empty );
	assert( u.type == CCS_NUM_FLOAT );
	assert( u.lower.f == -3.0 );
	assert( u.lower_included == CCS_TRUE );
	assert( u.upper.f == 7.0 );
	assert( u.upper_included == CCS_TRUE );

}

void test_union_int() {
	ccs_interval_t interval1, interval2, u;
	ccs_bool_t     empty;
	ccs_result_t   err;

	interval1.type = CCS_NUM_INTEGER;
	interval1.lower.i = -3;
	interval1.upper.i = 5;
	interval1.lower_included = CCS_TRUE;
	interval1.upper_included = CCS_FALSE;

	interval2.type = CCS_NUM_INTEGER;
	interval2.lower.i = 2;
	interval2.upper.i = 7;
	interval2.lower_included = CCS_TRUE;
	interval2.upper_included = CCS_TRUE;

	err = ccs_interval_union(&interval1, &interval2, &u);
	assert( err == CCS_SUCCESS );
	err = ccs_interval_empty(&u, &empty);
	assert( err == CCS_SUCCESS );
	assert( !empty );
	assert( u.type == CCS_NUM_INTEGER );
	assert( u.lower.i == -3 );
	assert( u.lower_included == CCS_TRUE);
	assert( u.upper.i == 7 );
	assert( u.upper_included == CCS_TRUE);

}

void test_equal_float() {
	ccs_interval_t interval1, interval2;
	ccs_bool_t     equal;
	ccs_result_t   err;

	interval1.type = CCS_NUM_FLOAT;
	interval1.lower.f = -3.0;
	interval1.upper.f = 5.0;
	interval1.lower_included = CCS_TRUE;
	interval1.upper_included = CCS_FALSE;

	interval2.type = CCS_NUM_FLOAT;
	interval2.lower.f = 2.0;
	interval2.upper.f = 7.0;
	interval2.lower_included = CCS_TRUE;
	interval2.upper_included = CCS_TRUE;

	err = ccs_interval_equal(&interval1, &interval2, &equal);
	assert( err == CCS_SUCCESS );
	assert( !equal );

	interval2.lower.f = -3.0;
	interval2.upper.f = 5.0;
	err = ccs_interval_equal(&interval1, &interval2, &equal);
	assert( err == CCS_SUCCESS );
	assert( !equal );

	interval2.upper_included = CCS_FALSE;
	err = ccs_interval_equal(&interval1, &interval2, &equal);
	assert( err == CCS_SUCCESS );
	assert( equal );
}

void test_equal_int() {
	ccs_interval_t interval1, interval2;
	ccs_bool_t     equal;
	ccs_result_t   err;

	interval1.type = CCS_NUM_INTEGER;
	interval1.lower.i = -3;
	interval1.upper.i = 5;
	interval1.lower_included = CCS_TRUE;
	interval1.upper_included = CCS_FALSE;

	interval2.type = CCS_NUM_INTEGER;
	interval2.lower.i = 2;
	interval2.upper.i = 7;
	interval2.lower_included = CCS_TRUE;
	interval2.upper_included = CCS_TRUE;

	err = ccs_interval_equal(&interval1, &interval2, &equal);
	assert( err == CCS_SUCCESS );
	assert( !equal );

	interval2.lower.i = -3;
	interval2.upper.i = 4;
	err = ccs_interval_equal(&interval1, &interval2, &equal);
	assert( err == CCS_SUCCESS );
	assert( equal );

	interval2.upper.i = 4;
	interval2.upper_included = CCS_FALSE;
	err = ccs_interval_equal(&interval1, &interval2, &equal);
	assert( err == CCS_SUCCESS );
	assert( !equal );

	interval2.upper.i = 5;
	err = ccs_interval_equal(&interval1, &interval2, &equal);
	assert( err == CCS_SUCCESS );
	assert( equal );
}

void test_interval_include_float() {
	ccs_interval_t interval;

	interval.type = CCS_NUM_FLOAT;
	interval.lower.f = -3.0;
	interval.upper.f = 5.0;
	interval.lower_included = CCS_TRUE;
	interval.upper_included = CCS_FALSE;

	assert( ccs_interval_include(&interval, CCSF(0.0)) );
	assert( ccs_interval_include(&interval, CCSF(-3.0)) );
	assert( !ccs_interval_include(&interval, CCSF(-3.1)) );
	assert( ccs_interval_include(&interval, CCSF(4.9)) );
	assert( !ccs_interval_include(&interval, CCSF(5.0)) );
}

void test_interval_include_int() {
	ccs_interval_t interval;

	interval.type = CCS_NUM_INTEGER;
	interval.lower.i = -3;
	interval.upper.i = 5;
	interval.lower_included = CCS_TRUE;
	interval.upper_included = CCS_FALSE;

	assert( ccs_interval_include(&interval, CCSI(0)) );
	assert( ccs_interval_include(&interval, CCSI(-3)) );
	assert( !ccs_interval_include(&interval, CCSI(-4)) );
	assert( ccs_interval_include(&interval, CCSI(4)) );
	assert( !ccs_interval_include(&interval, CCSI(5)) );
}

int main() {
	ccs_init();
	test_empty_float();
	test_empty_int();
	test_intersect_float();
	test_intersect_int();
	test_union_float();
	test_union_int();
	test_equal_float();
	test_equal_int();
	test_interval_include_float();
	test_interval_include_int();
	ccs_fini();
	return 0;
}

