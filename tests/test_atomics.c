#include <stdlib.h>
#include <assert.h>
#include <cconfigspace.h>
#include <pthread.h>
#include <string.h>

#define REPEAT 10000000


void * code(void *ptr) {
	ccs_hyperparameter_t hyperparameter = (ccs_hyperparameter_t)ptr;
	for (int i = 0; i < REPEAT; i++) {
		assert(CCS_SUCCESS == ccs_retain_object(hyperparameter));
		assert(CCS_SUCCESS == ccs_release_object(hyperparameter));
	}
	return NULL;
}

void test_parallel_retain_release() {
	ccs_hyperparameter_t hyperparameter;
	pthread_t            thread1, thread2;
	ccs_result_t         err;

	err = ccs_create_numerical_hyperparameter("my_param", CCS_NUM_FLOAT,
	                                          CCSF(-5.0), CCSF(5.0),
	                                          CCSF(0.0), CCSF(1.0),
	                                          (void *)0xdeadbeef,
	                                          &hyperparameter);
	assert( err == CCS_SUCCESS );

	pthread_create(&thread1, NULL, &code, hyperparameter);
	pthread_create(&thread2, NULL, &code, hyperparameter);
	pthread_join(thread1, NULL);
	pthread_join(thread2, NULL);
	err = ccs_release_object(hyperparameter);
	assert( err == CCS_SUCCESS );
}

int main() {
	ccs_init();
	test_parallel_retain_release();
	ccs_fini();
	return 0;
}
