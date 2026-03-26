#include <stdlib.h>
#include <assert.h>
#include <cconfigspace.h>
#include <pthread.h>
#include <string.h>
#include "test_utils.h"

#define REPEAT          10000000
#define NUM_THREADS     4
#define SAMPLES_PER_THR 1000

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
test_parallel_retain_release(void)
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

struct sample_args {
	ccs_configuration_space_t cspace;
	int                       count;
};

static void *
sample_thread(void *ptr)
{
	struct sample_args *args = (struct sample_args *)ptr;
	ccs_result_t        err;
	for (int i = 0; i < args->count; i++) {
		ccs_configuration_t config;
		err = ccs_configuration_space_sample(
			args->cspace, NULL, NULL, NULL, &config);
		assert(err == CCS_RESULT_SUCCESS);
		ccs_datum_t values[2];
		err = ccs_binding_get_values(
			(ccs_binding_t)config, 2, values, NULL);
		assert(err == CCS_RESULT_SUCCESS);
		assert(values[0].type == CCS_DATA_TYPE_FLOAT);
		assert(values[1].type == CCS_DATA_TYPE_FLOAT);
		err = ccs_release_object(config);
		assert(err == CCS_RESULT_SUCCESS);
	}
	return NULL;
}

void
test_parallel_sampling(void)
{
	ccs_configuration_space_t cspace;
	pthread_t                 threads[NUM_THREADS];
	struct sample_args        args[NUM_THREADS];
	int                       ret;

	cspace = create_2d_plane(NULL);

	for (int i = 0; i < NUM_THREADS; i++) {
		args[i].cspace = cspace;
		args[i].count  = SAMPLES_PER_THR;
		ret            = pthread_create(
                        &threads[i], NULL, sample_thread, &args[i]);
		assert(ret == 0);
	}
	for (int i = 0; i < NUM_THREADS; i++) {
		ret = pthread_join(threads[i], NULL);
		assert(ret == 0);
	}

	ccs_release_object(cspace);
}

struct tuner_ask_tell_args {
	ccs_tuner_t           tuner;
	ccs_objective_space_t ospace;
	int                   count;
};

static void *
tuner_ask_tell_thread(void *ptr)
{
	struct tuner_ask_tell_args *args = (struct tuner_ask_tell_args *)ptr;
	ccs_result_t                err;
	for (int i = 0; i < args->count; i++) {
		ccs_search_configuration_t config;
		ccs_evaluation_t           eval;
		ccs_datum_t                values[2];
		ccs_datum_t                res;
		err = ccs_tuner_ask(args->tuner, NULL, 1, &config, NULL);
		assert(err == CCS_RESULT_SUCCESS);
		err = ccs_binding_get_values(
			(ccs_binding_t)config, 2, values, NULL);
		assert(err == CCS_RESULT_SUCCESS);
		res = ccs_float(
			(values[0].value.f - 1) * (values[0].value.f - 1) +
			(values[1].value.f - 2) * (values[1].value.f - 2));
		err = ccs_create_evaluation(
			args->ospace, config, CCS_RESULT_SUCCESS, 1, &res,
			&eval);
		assert(err == CCS_RESULT_SUCCESS);
		err = ccs_tuner_tell(args->tuner, 1, &eval);
		assert(err == CCS_RESULT_SUCCESS);
		err = ccs_release_object(config);
		assert(err == CCS_RESULT_SUCCESS);
		err = ccs_release_object(eval);
		assert(err == CCS_RESULT_SUCCESS);
	}
	return NULL;
}

void
test_parallel_tuner(void)
{
	ccs_configuration_space_t  cspace;
	ccs_objective_space_t      ospace;
	ccs_tuner_t                tuner;
	ccs_result_t               err;
	pthread_t                  threads[NUM_THREADS];
	struct tuner_ask_tell_args args[NUM_THREADS];
	size_t                     count;
	int                        ret;

	cspace = create_2d_plane(NULL);
	ospace = create_height_objective(cspace);

	err    = ccs_create_random_tuner("problem", ospace, &tuner);
	assert(err == CCS_RESULT_SUCCESS);

	for (int i = 0; i < NUM_THREADS; i++) {
		args[i].tuner  = tuner;
		args[i].ospace = ospace;
		args[i].count  = SAMPLES_PER_THR;
		ret            = pthread_create(
                        &threads[i], NULL, tuner_ask_tell_thread, &args[i]);
		assert(ret == 0);
	}
	for (int i = 0; i < NUM_THREADS; i++) {
		ret = pthread_join(threads[i], NULL);
		assert(ret == 0);
	}

	err = ccs_tuner_get_history(tuner, NULL, 0, NULL, &count);
	assert(err == CCS_RESULT_SUCCESS);
	assert(count == NUM_THREADS * SAMPLES_PER_THR);

	ccs_release_object(tuner);
	ccs_release_object(ospace);
	ccs_release_object(cspace);
}

struct map_args {
	ccs_map_t map;
	int       offset;
	int       count;
};

static void *
map_thread(void *ptr)
{
	struct map_args *args = (struct map_args *)ptr;
	ccs_result_t     err;
	for (int i = 0; i < args->count; i++) {
		int key = args->offset + i;
		err     = ccs_map_set(
                        args->map, ccs_int(key), ccs_float((double)key));
		assert(err == CCS_RESULT_SUCCESS);
	}
	for (int i = 0; i < args->count; i++) {
		int         key = args->offset + i;
		ccs_datum_t value;
		err = ccs_map_get(args->map, ccs_int(key), &value);
		assert(err == CCS_RESULT_SUCCESS);
		assert(value.type == CCS_DATA_TYPE_FLOAT);
		assert(value.value.f == (double)key);
	}
	return NULL;
}

void
test_parallel_map(void)
{
	ccs_map_t       map;
	ccs_result_t    err;
	pthread_t       threads[NUM_THREADS];
	struct map_args args[NUM_THREADS];
	size_t          count;
	int             ret;

	err = ccs_create_map(&map);
	assert(err == CCS_RESULT_SUCCESS);

	for (int i = 0; i < NUM_THREADS; i++) {
		args[i].map    = map;
		args[i].offset = i * SAMPLES_PER_THR;
		args[i].count  = SAMPLES_PER_THR;
		ret = pthread_create(&threads[i], NULL, map_thread, &args[i]);
		assert(ret == 0);
	}
	for (int i = 0; i < NUM_THREADS; i++) {
		ret = pthread_join(threads[i], NULL);
		assert(ret == 0);
	}

	err = ccs_map_get_keys(map, 0, NULL, &count);
	assert(err == CCS_RESULT_SUCCESS);
	assert(count == NUM_THREADS * SAMPLES_PER_THR);

	ccs_release_object(map);
}

int
main(void)
{
	ccs_init();
	test_parallel_retain_release();
	test_parallel_sampling();
	test_parallel_tuner();
	test_parallel_map();
	ccs_clear_thread_error();
	ccs_fini();
	return 0;
}
