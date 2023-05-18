#include <stdlib.h>
#include <assert.h>
#include <cconfigspace.h>
#include <pthread.h>
#include <string.h>

#define REPEAT 10000000

void *
code(void *ptr)
{
	ccs_parameter_t parameter = (ccs_parameter_t)ptr;
	for (int i = 0; i < REPEAT; i++) {
		assert(CCS_RESULT_SUCCESS == ccs_retain_object(parameter));
		assert(CCS_RESULT_SUCCESS == ccs_release_object(parameter));
	}
	return NULL;
}

void
test_parallel_retain_release()
{
	ccs_parameter_t parameter;
	pthread_t       thread1, thread2;
	ccs_result_t    err;

	err = ccs_create_numerical_parameter(
		"my_param", CCS_NUMERIC_TYPE_FLOAT, CCSF(-5.0), CCSF(5.0),
		CCSF(0.0), CCSF(1.0), &parameter);
	assert(err == CCS_RESULT_SUCCESS);

	pthread_create(&thread1, NULL, &code, parameter);
	pthread_create(&thread2, NULL, &code, parameter);
	pthread_join(thread1, NULL);
	pthread_join(thread2, NULL);
	err = ccs_release_object(parameter);
	assert(err == CCS_RESULT_SUCCESS);
}

int
main()
{
	ccs_init();
	test_parallel_retain_release();
	ccs_clear_thread_error();
	return 0;
}
