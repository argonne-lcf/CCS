#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <cconfigspace.h>
#include <gsl/gsl_rng.h>
#include "test_utils.h"

static void
test_rng_create_with_type(void)
{
	const gsl_rng_type **t1, **t2, *t;
	size_t               type_count = 0;
	int32_t              selected, refcount;
	ccs_rng_t            rng = NULL;
	ccs_result_t         err = CCS_RESULT_SUCCESS;
	ccs_object_type_t    otype;

	t1 = t2 = gsl_rng_types_setup();
	while (*t1++)
		type_count++;
	selected = rand() % type_count;
	err      = ccs_create_rng_with_type(t2[selected], NULL);
	assert(err == CCS_RESULT_ERROR_INVALID_VALUE);
	err = ccs_create_rng_with_type(NULL, &rng);
	assert(err == CCS_RESULT_ERROR_INVALID_VALUE);
	err = ccs_create_rng_with_type(t2[selected], &rng);
	assert(err == CCS_RESULT_SUCCESS);
	assert(rng);
	err = ccs_rng_get_type(rng, &t);
	assert(err == CCS_RESULT_SUCCESS);
	assert(t == t2[selected]);
	err = ccs_object_get_type(rng, &otype);
	assert(err == CCS_RESULT_SUCCESS);
	assert(otype == CCS_OBJECT_TYPE_RNG);
	err = ccs_object_get_refcount(rng, &refcount);
	assert(err == CCS_RESULT_SUCCESS);
	assert(refcount == 1);
	err = ccs_release_object(rng);
	assert(err == CCS_RESULT_SUCCESS);
}

static void
test_rng_create_serialize(ccs_serialize_format_t format)
{
	ccs_rng_t           rng = NULL, rng2 = NULL;
	ccs_result_t        err = CCS_RESULT_SUCCESS;
	const gsl_rng_type *t, *t2;
	char               *buff;
	size_t              buff_size;
	unsigned long int   i = 0, i2 = 0;

	err = ccs_create_rng(NULL);
	assert(err == CCS_RESULT_ERROR_INVALID_VALUE);
	err = ccs_create_rng(&rng);
	assert(err == CCS_RESULT_SUCCESS);
	assert(rng);
	err = ccs_rng_get_type(rng, &t);
	assert(err == CCS_RESULT_SUCCESS);
	assert(t == gsl_rng_default);

	err = ccs_rng_get(rng, &i);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_object_serialize(
		rng, format, CCS_SERIALIZE_OPERATION_SIZE, &buff_size,
		CCS_SERIALIZE_OPTION_END);
	assert(err == CCS_RESULT_SUCCESS);

	buff = (char *)malloc(buff_size);
	assert(buff);

	err = ccs_object_serialize(
		rng, format, CCS_SERIALIZE_OPERATION_MEMORY, buff_size, buff,
		CCS_SERIALIZE_OPTION_END);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_object_deserialize(
		(ccs_object_t *)&rng2, format, CCS_DESERIALIZE_OPERATION_MEMORY,
		buff_size, buff, CCS_DESERIALIZE_OPTION_END);
	assert(err == CCS_RESULT_SUCCESS);
	free(buff);

	err = ccs_rng_get_type(rng2, &t2);
	assert(err == CCS_RESULT_SUCCESS);
	assert(t == t2);

	err = ccs_rng_get(rng, &i);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_rng_get(rng2, &i2);
	assert(err == CCS_RESULT_SUCCESS);
	assert(i == i2);

	err = ccs_release_object(rng);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(rng2);
	assert(err == CCS_RESULT_SUCCESS);
}

static void
test_rng_min_max(void)
{
	ccs_rng_t         rng  = NULL;
	ccs_result_t      err  = CCS_RESULT_SUCCESS;
	unsigned long int imin = 0;
	unsigned long int imax = 0;
	err                    = ccs_create_rng(&rng);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_rng_min(NULL, &imin);
	assert(err == CCS_RESULT_ERROR_INVALID_OBJECT);
	err = ccs_rng_min(rng, NULL);
	assert(err == CCS_RESULT_ERROR_INVALID_VALUE);
	err = ccs_rng_min(rng, &imin);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_rng_max(NULL, &imax);
	assert(err == CCS_RESULT_ERROR_INVALID_OBJECT);
	err = ccs_rng_max(rng, NULL);
	assert(err == CCS_RESULT_ERROR_INVALID_VALUE);
	err = ccs_rng_max(rng, &imax);
	assert(err == CCS_RESULT_SUCCESS);
	assert(imin < imax);
	err = ccs_release_object(rng);
	assert(err == CCS_RESULT_SUCCESS);
}

static void
test_rng_get(void)
{
	ccs_rng_t         rng  = NULL;
	ccs_result_t      err  = CCS_RESULT_SUCCESS;
	unsigned long int i    = 0;
	unsigned long int imin = 0;
	unsigned long int imax = 0;

	err                    = ccs_create_rng(&rng);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_rng_min(rng, &imin);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_rng_max(rng, &imax);
	assert(err == CCS_RESULT_SUCCESS);
	for (int j = 0; j < 100; j++) {
		err = ccs_rng_get(rng, &i);
		assert(err == CCS_RESULT_SUCCESS);
		assert(i >= imin);
		assert(i <= imax);
	}
	err = ccs_release_object(rng);
	assert(err == CCS_RESULT_SUCCESS);
}

static void
test_rng_uniform(void)
{
	ccs_rng_t    rng = NULL;
	ccs_result_t err = CCS_RESULT_SUCCESS;
	double       d   = -1.0;

	err              = ccs_create_rng(&rng);
	assert(err == CCS_RESULT_SUCCESS);
	for (int j = 0; j < 100; j++) {
		err = ccs_rng_uniform(rng, &d);
		assert(err == CCS_RESULT_SUCCESS);
		assert(d >= 0.0);
		assert(d < 1.0);
	}
	err = ccs_release_object(rng);
	assert(err == CCS_RESULT_SUCCESS);
}

static void
test_rng_file_serialize(ccs_serialize_format_t format)
{
	ccs_rng_t           rng = NULL, rng2 = NULL;
	ccs_object_t        obj;
	ccs_result_t        err;
	const gsl_rng_type *t, *t2;
	unsigned long int   i = 0, i2 = 0;
	char                tmppath[] = "/tmp/ccs_test_XXXXXX";
	int                 fd;

	err = ccs_create_rng(&rng);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_rng_get(rng, &i);
	assert(err == CCS_RESULT_SUCCESS);

	/* Create temp file */
	fd = mkstemp(tmppath);
	assert(fd != -1);
	close(fd);

	/* Serialize to file */
	err = ccs_object_serialize(
		rng, format, CCS_SERIALIZE_OPERATION_FILE, tmppath,
		CCS_SERIALIZE_OPTION_END);
	assert(err == CCS_RESULT_SUCCESS);

	/* Deserialize from file */
	err = ccs_object_deserialize(
		(ccs_object_t *)&rng2, format, CCS_DESERIALIZE_OPERATION_FILE,
		tmppath, CCS_DESERIALIZE_OPTION_END);
	assert(err == CCS_RESULT_SUCCESS);

	/* Verify roundtrip */
	err = ccs_rng_get_type(rng, &t);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_rng_get_type(rng2, &t2);
	assert(err == CCS_RESULT_SUCCESS);
	assert(t == t2);

	err = ccs_rng_get(rng, &i);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_rng_get(rng2, &i2);
	assert(err == CCS_RESULT_SUCCESS);
	assert(i == i2);

	/* Bad file path */
	err = ccs_object_deserialize(
		&obj, format, CCS_DESERIALIZE_OPERATION_FILE,
		"/nonexistent/path/ccs_test.bin", CCS_DESERIALIZE_OPTION_END);
	assert(err == CCS_RESULT_ERROR_INVALID_FILE_PATH);
	ccs_clear_thread_error();

	unlink(tmppath);
	ccs_release_object(rng2);
	ccs_release_object(rng);
}

static void
test_rng_fd_serialize_blocking(ccs_serialize_format_t format)
{
	ccs_rng_t           rng = NULL, rng2 = NULL;
	ccs_result_t        err;
	const gsl_rng_type *t, *t2;
	unsigned long int   i = 0, i2 = 0;
	int                 pipefd[2];
	int                 ret;

	err = ccs_create_rng(&rng);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_rng_get(rng, &i);
	assert(err == CCS_RESULT_SUCCESS);

	ret = pipe(pipefd);
	assert(ret == 0);

	/* Blocking serialize to pipe write end */
	err = ccs_object_serialize(
		rng, format, CCS_SERIALIZE_OPERATION_FILE_DESCRIPTOR, pipefd[1],
		CCS_SERIALIZE_OPTION_END);
	assert(err == CCS_RESULT_SUCCESS);
	close(pipefd[1]);

	/* Blocking deserialize from pipe read end */
	err = ccs_object_deserialize(
		(ccs_object_t *)&rng2, format,
		CCS_DESERIALIZE_OPERATION_FILE_DESCRIPTOR, pipefd[0],
		CCS_DESERIALIZE_OPTION_END);
	assert(err == CCS_RESULT_SUCCESS);
	close(pipefd[0]);

	/* Verify roundtrip */
	err = ccs_rng_get_type(rng, &t);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_rng_get_type(rng2, &t2);
	assert(err == CCS_RESULT_SUCCESS);
	assert(t == t2);

	err = ccs_rng_get(rng, &i);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_rng_get(rng2, &i2);
	assert(err == CCS_RESULT_SUCCESS);
	assert(i == i2);

	ccs_release_object(rng2);
	ccs_release_object(rng);
}

struct _nb_write_args {
	ccs_rng_t rng;
	int       fd;
};

static void *
_nb_write_thread(void *arg)
{
	struct _nb_write_args *wa          = (struct _nb_write_args *)arg;
	void                  *write_state = NULL;
	ccs_result_t           err;
	do {
		err = ccs_object_serialize(
			wa->rng, CCS_SERIALIZE_FORMAT_BINARY,
			CCS_SERIALIZE_OPERATION_FILE_DESCRIPTOR, wa->fd,
			CCS_SERIALIZE_OPTION_NON_BLOCKING, &write_state,
			CCS_SERIALIZE_OPTION_END);
	} while (err == CCS_RESULT_AGAIN);
	assert(err == CCS_RESULT_SUCCESS);
	assert(write_state == NULL);
	close(wa->fd);
	return NULL;
}

static void
test_rng_fd_serialize_non_blocking(void)
{
	ccs_rng_t             rng = NULL, rng2 = NULL;
	ccs_result_t          err;
	const gsl_rng_type   *t, *t2;
	unsigned long int     i = 0, i2 = 0;
	int                   pipefd[2];
	int                   ret;
	int                   flags;
	void                 *read_state = NULL;
	pthread_t             writer;
	struct _nb_write_args wa;

	err = ccs_create_rng(&rng);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_rng_get(rng, &i);
	assert(err == CCS_RESULT_SUCCESS);

	ret = pipe(pipefd);
	assert(ret == 0);

	/* Set both ends to non-blocking */
	flags = fcntl(pipefd[0], F_GETFL, 0);
	fcntl(pipefd[0], F_SETFL, flags | O_NONBLOCK);
	flags = fcntl(pipefd[1], F_GETFL, 0);
	fcntl(pipefd[1], F_SETFL, flags | O_NONBLOCK);

	/* Write in a separate thread so that if the pipe buffer fills,
	 * the read thread can drain it. */
	wa.rng = rng;
	wa.fd  = pipefd[1];
	ret    = pthread_create(&writer, NULL, _nb_write_thread, &wa);
	assert(ret == 0);

	/* Non-blocking deserialize in the main thread */
	do {
		err = ccs_object_deserialize(
			(ccs_object_t *)&rng2, CCS_SERIALIZE_FORMAT_BINARY,
			CCS_DESERIALIZE_OPERATION_FILE_DESCRIPTOR, pipefd[0],
			CCS_DESERIALIZE_OPTION_NON_BLOCKING, &read_state,
			CCS_DESERIALIZE_OPTION_END);
	} while (err == CCS_RESULT_AGAIN);
	assert(err == CCS_RESULT_SUCCESS);
	assert(read_state == NULL);
	close(pipefd[0]);

	ret = pthread_join(writer, NULL);
	assert(ret == 0);

	/* Verify roundtrip */
	err = ccs_rng_get_type(rng, &t);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_rng_get_type(rng2, &t2);
	assert(err == CCS_RESULT_SUCCESS);
	assert(t == t2);

	err = ccs_rng_get(rng, &i);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_rng_get(rng2, &i2);
	assert(err == CCS_RESULT_SUCCESS);
	assert(i == i2);

	ccs_release_object(rng2);
	ccs_release_object(rng);
}

int
main(void)
{
	ccs_init();
	test_rng_create_with_type();
	test_rng_create_serialize(CCS_SERIALIZE_FORMAT_BINARY);
	test_rng_create_serialize(CCS_SERIALIZE_FORMAT_JSON);
	test_rng_min_max();
	test_rng_get();
	test_rng_uniform();
	test_rng_file_serialize(CCS_SERIALIZE_FORMAT_BINARY);
	test_rng_file_serialize(CCS_SERIALIZE_FORMAT_JSON);
	test_rng_fd_serialize_blocking(CCS_SERIALIZE_FORMAT_BINARY);
	test_rng_fd_serialize_blocking(CCS_SERIALIZE_FORMAT_JSON);
	test_rng_fd_serialize_non_blocking();
	ccs_clear_thread_error();
	ccs_fini();
	return 0;
}
