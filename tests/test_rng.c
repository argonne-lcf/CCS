#include <stdlib.h>
#include <assert.h>
#include <cconfigspace.h>
#include <gsl/gsl_rng.h>

static void test_rng_create_with_type() {
	const gsl_rng_type **t1, **t2, *t;
	size_t               type_count = 0;
	int32_t              selected, refcount;
	ccs_rng_t            rng = NULL;
	ccs_error_t          err = CCS_SUCCESS;
	ccs_object_type_t    otype;

	t1 = t2 = gsl_rng_types_setup();
	while (*t1++)
		type_count++;
	selected = rand() % type_count;
	err = ccs_rng_create_with_type(t2[selected], NULL);
	assert( err == -CCS_INVALID_VALUE );
	err = ccs_rng_create_with_type(NULL, &rng);
	assert( err == -CCS_INVALID_VALUE );
	err = ccs_rng_create_with_type(t2[selected], &rng);
	assert( err == CCS_SUCCESS );
	assert( rng );
	err = ccs_rng_get_type(rng, &t);
	assert( err == CCS_SUCCESS );
	assert( t == t2[selected] );
	err = ccs_object_get_type(rng, &otype);
	assert( err == CCS_SUCCESS );
	assert( otype == CCS_RNG );
	err = ccs_object_get_refcount(rng, &refcount);
	assert( err == CCS_SUCCESS );
	assert( refcount == 1 );
	err = ccs_release_object(rng);
	assert( err == CCS_SUCCESS );
}

static void test_rng_create() {
	ccs_rng_t           rng = NULL;
	ccs_error_t         err = CCS_SUCCESS;
	const gsl_rng_type *t;

	err = ccs_rng_create(NULL);
	assert( err == -CCS_INVALID_VALUE );
	err = ccs_rng_create(&rng);
	assert( err == CCS_SUCCESS );
	assert( rng );
	err = ccs_rng_get_type(rng, &t);
	assert( err == CCS_SUCCESS );
	assert( t == gsl_rng_default );
	err = ccs_release_object(rng);
	assert( err == CCS_SUCCESS );
}

static void test_rng_min_max() {
	ccs_rng_t         rng = NULL;
	ccs_error_t       err = CCS_SUCCESS;
	unsigned long int imin = 0;
	unsigned long int imax = 0;
	err = ccs_rng_create(&rng);
	assert( err == CCS_SUCCESS );
	err = ccs_rng_min(NULL, &imin);
	assert( err == -CCS_INVALID_OBJECT );
	err = ccs_rng_min(rng, NULL);
	assert( err == -CCS_INVALID_VALUE );
	err = ccs_rng_min(rng, &imin);
	assert( err == CCS_SUCCESS );
	err = ccs_rng_max(NULL, &imax);
	assert( err == -CCS_INVALID_OBJECT );
	err = ccs_rng_max(rng, NULL);
	assert( err == -CCS_INVALID_VALUE );
	err = ccs_rng_max(rng, &imax);
	assert( err == CCS_SUCCESS );
	assert( imin < imax );
	err = ccs_release_object(rng);
	assert( err == CCS_SUCCESS );
}

static void test_rng_get() {
	ccs_rng_t         rng = NULL;
	ccs_error_t       err = CCS_SUCCESS;
	unsigned long int i = 0;
	unsigned long int imin = 0;
	unsigned long int imax = 0;

	err = ccs_rng_create(&rng);
	assert( err == CCS_SUCCESS );
	err = ccs_rng_min(rng, &imin);
	assert( err == CCS_SUCCESS );
	err = ccs_rng_max(rng, &imax);
	assert( err == CCS_SUCCESS );
	for (int j = 0; j < 100; j++) {
		err = ccs_rng_get(rng, &i);
		assert( err == CCS_SUCCESS );
		assert( i >= imin );
		assert( i <= imax );
	}
	err = ccs_release_object(rng);
	assert( err == CCS_SUCCESS );
}

static void test_rng_uniform() {
	ccs_rng_t   rng = NULL;
	ccs_error_t err = CCS_SUCCESS;
	double      d = -1.0;

	err = ccs_rng_create(&rng);
	assert( err == CCS_SUCCESS );
	for (int j = 0; j < 100; j++) {
		err = ccs_rng_uniform(rng, &d);
		assert( err == CCS_SUCCESS );
		assert( d >= 0.0 );
		assert( d <  1.0 );
	}
	err = ccs_release_object(rng);
	assert( err == CCS_SUCCESS );
}

int main(int argc, char *argv[]) {
	ccs_init();
	test_rng_create_with_type();
	test_rng_create();
	test_rng_min_max();
	test_rng_get();
	test_rng_uniform();
	return 0;
}
