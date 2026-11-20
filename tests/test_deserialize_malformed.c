#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <cconfigspace.h>
#include "test_utils.h"

/* Regression tests for binary deserialization of malformed input. Each test
 * takes a valid serialization, corrupts one field, and checks that
 * deserialization fails cleanly instead of crashing or corrupting memory.
 * Run under valgrind (make check-valgrind-memcheck) or ASan to catch silent
 * heap corruption. */

static char *
serialize(ccs_object_t object, size_t *size)
{
	ccs_result_t err;
	char        *buff;
	err = ccs_object_serialize(
		object, CCS_SERIALIZE_FORMAT_BINARY,
		CCS_SERIALIZE_OPERATION_SIZE, size, CCS_SERIALIZE_OPTION_END);
	assert(err == CCS_RESULT_SUCCESS);
	buff = (char *)malloc(*size);
	assert(buff);
	err = ccs_object_serialize(
		object, CCS_SERIALIZE_FORMAT_BINARY,
		CCS_SERIALIZE_OPERATION_MEMORY, *size, buff,
		CCS_SERIALIZE_OPTION_END);
	assert(err == CCS_RESULT_SUCCESS);
	return buff;
}

static size_t
find_bytes(
	const char *buff,
	size_t      size,
	const char *pattern,
	size_t      pattern_size)
{
	size_t found = size;
	for (size_t i = 0; i + pattern_size <= size; i++)
		if (!memcmp(buff + i, pattern, pattern_size)) {
			/* the pattern must be unambiguous */
			assert(found == size);
			found = i;
		}
	assert(found != size);
	return found;
}

/* A literal expression serializes as:
 *   ... type(LITERAL) num_nodes(0) datum ...
 * Leaf expressions are deserialized without options; claiming child nodes
 * used to dereference a NULL options pointer. */
static void
test_leaf_expression_with_nodes(void)
{
	ccs_result_t     err;
	ccs_expression_t expression;
	ccs_object_t     object = NULL;
	size_t           size;
	char            *buff;
	/* compressed: LITERAL type, num_nodes 0, datum type INT, value 77 */
	const char       pattern[] = {
                CCS_EXPRESSION_TYPE_LITERAL, 0x00, CCS_DATA_TYPE_INT};
	size_t offset;

	err = ccs_create_literal(ccs_int(77), &expression);
	assert(err == CCS_RESULT_SUCCESS);
	buff   = serialize(expression, &size);
	offset = find_bytes(buff, size, pattern, sizeof(pattern));

	/* sanity check: the untouched buffer round-trips */
	err    = ccs_object_deserialize(
                &object, CCS_SERIALIZE_FORMAT_BINARY,
                CCS_DESERIALIZE_OPERATION_MEMORY, size, buff,
                CCS_DESERIALIZE_OPTION_END);
	assert(err == CCS_RESULT_SUCCESS);
	ccs_release_object(object);
	object           = NULL;

	buff[offset + 1] = 1; /* num_nodes = 1 */
	err              = ccs_object_deserialize(
                &object, CCS_SERIALIZE_FORMAT_BINARY,
                CCS_DESERIALIZE_OPERATION_MEMORY, size, buff,
                CCS_DESERIALIZE_OPTION_END);
	assert(err == CCS_RESULT_ERROR_INVALID_VALUE);
	assert(object == NULL);

	free(buff);
	ccs_release_object(expression);
}

/* An element count larger than the remaining input must be rejected before
 * anything is allocated from it. */
static void
test_huge_count(void)
{
	ccs_result_t     err;
	ccs_expression_t l1, l2, expression;
	ccs_object_t     object = NULL;
	size_t           size;
	char            *buff, *corrupt;
	/* compressed: ADD type, num_nodes 2, then the first child object */
	const char       pattern[] = {CCS_EXPRESSION_TYPE_ADD, 0x02};
	size_t           offset;

	err = ccs_create_literal(ccs_int(1), &l1);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_create_literal(ccs_int(2), &l2);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_create_binary_expression(
		CCS_EXPRESSION_TYPE_ADD, ccs_object(l1), ccs_object(l2),
		&expression);
	assert(err == CCS_RESULT_SUCCESS);
	buff   = serialize(expression, &size);
	offset = find_bytes(buff, size, pattern, sizeof(pattern));

	/* replace num_nodes (1 byte) by a 10-byte varint encoding of
	 * UINT64_MAX */
	corrupt = (char *)malloc(size + 9);
	assert(corrupt);
	memcpy(corrupt, buff, offset + 1);
	memset(corrupt + offset + 1, 0xff, 9);
	corrupt[offset + 10] = 0x01;
	memcpy(corrupt + offset + 11, buff + offset + 2, size - offset - 2);

	err = ccs_object_deserialize(
		&object, CCS_SERIALIZE_FORMAT_BINARY,
		CCS_DESERIALIZE_OPERATION_MEMORY, size + 9, corrupt,
		CCS_DESERIALIZE_OPTION_END);
	assert(err == CCS_RESULT_ERROR_INVALID_VALUE);
	assert(object == NULL);

	free(corrupt);
	free(buff);
	ccs_release_object(expression);
	ccs_release_object(l1);
	ccs_release_object(l2);
}

/* A distribution space serializes the total number of parameter indices,
 * then for each distribution its dimension followed by its indices. A
 * dimension exceeding the declared total used to overflow the index array. */
static void
test_distribution_space_dimension_overflow(void)
{
	ccs_result_t              err;
	ccs_parameter_t           parameters[3];
	ccs_configuration_space_t cspace;
	ccs_distribution_space_t  dspace;
	ccs_distribution_t        distribs[2], distrib;
	ccs_map_t                 map;
	ccs_datum_t               key;
	ccs_object_t              object  = NULL;
	size_t                    indices[2] = {2, 0};
	size_t                    size, offset;
	char                     *buff;
	/* compressed: num_parameters 3, num_distributions 2 */
	const char                pattern[]  = {0x03, 0x02};

	for (size_t i = 0; i < 3; i++) {
		char name[2] = {(char)('a' + i), 0};
		err          = ccs_create_numerical_parameter(
                        name, CCS_NUMERIC_TYPE_FLOAT, CCSF(-5.0), CCSF(5.0),
                        CCSF(0.0), CCSF(0.0), parameters + i);
		assert(err == CCS_RESULT_SUCCESS);
	}
	err = ccs_create_configuration_space(
		"space", 3, parameters, NULL, 0, NULL, NULL, NULL, &cspace);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_create_distribution_space(cspace, &dspace);
	assert(err == CCS_RESULT_SUCCESS);
	for (size_t i = 0; i < 2; i++) {
		err = ccs_create_uniform_float_distribution(
			-4.0, 4.0, CCS_SCALE_TYPE_LINEAR, 0.0, distribs + i);
		assert(err == CCS_RESULT_SUCCESS);
	}
	err = ccs_create_multivariate_distribution(2, distribs, &distrib);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_distribution_space_set_distribution(dspace, distrib, indices);
	assert(err == CCS_RESULT_SUCCESS);

	buff   = serialize(dspace, &size);
	offset = find_bytes(buff, size, pattern, sizeof(pattern));

	err    = ccs_create_map(&map);
	assert(err == CCS_RESULT_SUCCESS);
	key = ccs_object(cspace);
	key.flags |= CCS_DATUM_FLAG_ID;
	err = ccs_map_set(map, key, ccs_object(cspace));
	assert(err == CCS_RESULT_SUCCESS);

	/* sanity check: the untouched buffer round-trips */
	err = ccs_object_deserialize(
		&object, CCS_SERIALIZE_FORMAT_BINARY,
		CCS_DESERIALIZE_OPERATION_MEMORY, size, buff,
		CCS_DESERIALIZE_OPTION_HANDLE_MAP, map,
		CCS_DESERIALIZE_OPTION_END);
	assert(err == CCS_RESULT_SUCCESS);
	ccs_release_object(object);
	object       = NULL;

	buff[offset] = 1; /* num_parameters = 1 < 2 + 1 indices that follow */
	err          = ccs_object_deserialize(
                &object, CCS_SERIALIZE_FORMAT_BINARY,
                CCS_DESERIALIZE_OPERATION_MEMORY, size, buff,
                CCS_DESERIALIZE_OPTION_HANDLE_MAP, map,
                CCS_DESERIALIZE_OPTION_END);
	assert(err == CCS_RESULT_ERROR_INVALID_VALUE);
	assert(object == NULL);

	free(buff);
	ccs_release_object(map);
	ccs_release_object(distrib);
	for (size_t i = 0; i < 2; i++)
		ccs_release_object(distribs[i]);
	ccs_release_object(dspace);
	ccs_release_object(cspace);
	for (size_t i = 0; i < 3; i++)
		ccs_release_object(parameters[i]);
}

int
main(void)
{
	ccs_init();
	test_leaf_expression_with_nodes();
	test_huge_count();
	test_distribution_space_dimension_overflow();
	ccs_clear_thread_error();
	ccs_fini();
	return 0;
}
