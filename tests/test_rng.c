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

int main(int argc, char *argv[]) {
	ccs_init();
	test_rng_create_with_type();
	return 0;
}
