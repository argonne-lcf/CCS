#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <float.h>
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
	buff    = serialize(expression, &size);
	offset  = find_bytes(buff, size, pattern, sizeof(pattern));

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
	ccs_object_t              object     = NULL;
	size_t                    indices[2] = {2, 0};
	size_t                    size, offset;
	char                     *buff;
	/* compressed: num_parameters 3, num_distributions 2 */
	const char                pattern[] = {0x03, 0x02};

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

/* A binding (here a configuration) serializes its number of values, then the
 * values. That count was not bounded by the remaining input: 2^62 values made
 * calloc's size computation overflow instead of being rejected as corrupt. */
static void
test_binding_huge_num_values(void)
{
	ccs_result_t              err;
	ccs_parameter_t           parameters[2];
	ccs_configuration_space_t cspace;
	ccs_configuration_t       configuration;
	ccs_datum_t               values[2] = {ccs_int(77), ccs_int(66)};
	ccs_map_t                 map;
	ccs_datum_t               key;
	ccs_object_t              object = NULL;
	size_t                    size, offset;
	char                     *buff, *corrupt;
	/* compressed: num_values 2, INT 77 (zigzag varint 0x9a 0x01) */
	const char pattern[] = {0x02, CCS_DATA_TYPE_INT, (char)0x9a, 0x01};

	err = ccs_create_int_numerical_parameter("i", 0, 100, 0, 0, parameters);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_create_int_numerical_parameter(
		"j", 0, 100, 0, 0, parameters + 1);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_create_configuration_space(
		"space", 2, parameters, NULL, 0, NULL, NULL, NULL, &cspace);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_create_configuration(cspace, NULL, 2, values, &configuration);
	assert(err == CCS_RESULT_SUCCESS);

	buff   = serialize(configuration, &size);
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
	object  = NULL;

	/* replace num_values (1 byte) by the 9-byte varint encoding of 2^62 */
	corrupt = (char *)malloc(size + 8);
	assert(corrupt);
	memcpy(corrupt, buff, offset);
	memset(corrupt + offset, 0x80, 8);
	corrupt[offset + 8] = 0x40;
	memcpy(corrupt + offset + 9, buff + offset + 1, size - offset - 1);

	err = ccs_object_deserialize(
		&object, CCS_SERIALIZE_FORMAT_BINARY,
		CCS_DESERIALIZE_OPERATION_MEMORY, size + 8, corrupt,
		CCS_DESERIALIZE_OPTION_HANDLE_MAP, map,
		CCS_DESERIALIZE_OPTION_END);
	assert(err == CCS_RESULT_ERROR_INVALID_VALUE);
	assert(object == NULL);

	free(corrupt);
	free(buff);
	ccs_release_object(map);
	ccs_release_object(configuration);
	ccs_release_object(cspace);
	ccs_release_object(parameters[0]);
	ccs_release_object(parameters[1]);
}

/* Strings serialize as their length including the terminating NUL, then the
 * bytes. A string whose last byte is not NUL used to be accepted, and every
 * later strlen read past it, up to the end of the input buffer and beyond. */
static void
test_unterminated_string(void)
{
	ccs_result_t    err;
	ccs_parameter_t parameter;
	ccs_datum_t  possible_values[2] = {ccs_string("ab"), ccs_string("cd")};
	ccs_object_t object             = NULL;
	size_t       size, offset;
	char        *buff;
	/* compressed: name length 4, "cat\0" */
	const char   pattern[] = {0x04, 'c', 'a', 't', 0x00};

	err                    = ccs_create_categorical_parameter(
                "cat", 2, possible_values, 0, &parameter);
	assert(err == CCS_RESULT_SUCCESS);
	buff   = serialize(parameter, &size);
	offset = find_bytes(buff, size, pattern, sizeof(pattern));

	/* sanity check: the untouched buffer round-trips */
	err    = ccs_object_deserialize(
                &object, CCS_SERIALIZE_FORMAT_BINARY,
                CCS_DESERIALIZE_OPERATION_MEMORY, size, buff,
                CCS_DESERIALIZE_OPTION_END);
	assert(err == CCS_RESULT_SUCCESS);
	ccs_release_object(object);
	object           = NULL;

	buff[offset + 4] = 'x'; /* drop the name's terminating NUL */
	err              = ccs_object_deserialize(
                &object, CCS_SERIALIZE_FORMAT_BINARY,
                CCS_DESERIALIZE_OPERATION_MEMORY, size, buff,
                CCS_DESERIALIZE_OPTION_END);
	assert(err == CCS_RESULT_ERROR_INVALID_VALUE);
	assert(object == NULL);

	free(buff);
	ccs_release_object(parameter);
}

/* A tuner serializes its type, then its name. An unknown type is rejected,
 * but used to leak the handle map allocated beforehand: only a leak checker
 * (ASan, valgrind) can tell. */
static void
test_tuner_unknown_type(void)
{
	ccs_result_t              err;
	ccs_configuration_space_t cspace;
	ccs_objective_space_t     ospace;
	ccs_tuner_t               tuner;
	ccs_object_t              object = NULL;
	size_t                    size, offset;
	char                     *buff;
	/* compressed: name length 10, then "malformed" and its NUL (implicit in
	 * the literal); the tuner type byte precedes them */
	const char                pattern[] = "\x0amalformed";

	cspace                              = create_2d_plane(NULL);
	ospace                              = create_height_objective(cspace);
	err = ccs_create_random_tuner("malformed", ospace, &tuner);
	assert(err == CCS_RESULT_SUCCESS);
	buff   = serialize(tuner, &size);
	offset = find_bytes(buff, size, pattern, sizeof(pattern));

	/* sanity check: the untouched buffer round-trips */
	err    = ccs_object_deserialize(
                &object, CCS_SERIALIZE_FORMAT_BINARY,
                CCS_DESERIALIZE_OPERATION_MEMORY, size, buff,
                CCS_DESERIALIZE_OPTION_END);
	assert(err == CCS_RESULT_SUCCESS);
	ccs_release_object(object);
	object = NULL;

	assert(offset > 0 && buff[offset - 1] == CCS_TUNER_TYPE_RANDOM);
	buff[offset - 1] = CCS_TUNER_TYPE_MAX; /* not a valid tuner type */
	err              = ccs_object_deserialize(
                &object, CCS_SERIALIZE_FORMAT_BINARY,
                CCS_DESERIALIZE_OPERATION_MEMORY, size, buff,
                CCS_DESERIALIZE_OPTION_END);
	assert(err == CCS_RESULT_ERROR_INVALID_TYPE);
	assert(object == NULL);

	free(buff);
	ccs_release_object(tuner);
	ccs_release_object(ospace);
	ccs_release_object(cspace);
}

/* An integer range whose width upper - lower does not fit in a ccs_int_t
 * cannot be represented internally; computing it was signed overflow
 * (undefined behavior, reported by UBSan). Deserialized parameters and
 * distributions reach these constructors with arbitrary bounds. */
static void
test_int_range_overflow(void)
{
	ccs_result_t       err;
	ccs_parameter_t    parameter    = NULL;
	ccs_distribution_t distribution = NULL;

	err                             = ccs_create_int_numerical_parameter(
                "p", CCS_INT_MIN, 0, 0, CCS_INT_MIN, &parameter);
	assert(err == CCS_RESULT_ERROR_INVALID_VALUE);
	assert(parameter == NULL);
	err = ccs_create_int_numerical_parameter(
		"p", -1, CCS_INT_MAX, 0, 0, &parameter);
	assert(err == CCS_RESULT_ERROR_INVALID_VALUE);
	assert(parameter == NULL);

	err = ccs_create_uniform_int_distribution(
		CCS_INT_MIN, 0, CCS_SCALE_TYPE_LINEAR, 0, &distribution);
	assert(err == CCS_RESULT_ERROR_INVALID_VALUE);
	assert(distribution == NULL);

	/* the widest representable range is still accepted */
	err = ccs_create_int_numerical_parameter(
		"p", 0, CCS_INT_MAX, 0, 0, &parameter);
	assert(err == CCS_RESULT_SUCCESS);
	ccs_release_object(parameter);
	err = ccs_create_uniform_int_distribution(
		-1, CCS_INT_MAX - 1, CCS_SCALE_TYPE_LINEAR, 0, &distribution);
	assert(err == CCS_RESULT_SUCCESS);
	ccs_release_object(distribution);
}

/* Configuration space conditions serialize as {index, expression} pairs.
 * A repeated index used to deserialize a second expression over the first
 * one, leaking it. Checked for both formats. */
static void
test_duplicate_condition_index(void)
{
	ccs_result_t              err;
	ccs_parameter_t           parameters[3];
	ccs_expression_t          conditions[3] = {NULL, NULL, NULL};
	ccs_configuration_space_t cspace;
	ccs_object_t              object = NULL;
	size_t                    size, expr_size, offset;
	char                     *buff, *expr_buff, *index_str;

	for (int i = 0; i < 3; i++) {
		const char *names[] = {"p", "q", "r"};
		err                 = ccs_create_int_numerical_parameter(
                        names[i], 0, 100, 0, 0, parameters + i);
		assert(err == CCS_RESULT_SUCCESS);
	}
	for (int i = 1; i < 3; i++) {
		err = ccs_create_binary_expression(
			CCS_EXPRESSION_TYPE_LESS, ccs_object(parameters[0]),
			ccs_int(50), conditions + i);
		assert(err == CCS_RESULT_SUCCESS);
	}
	err = ccs_create_configuration_space(
		"space", 3, parameters, conditions, 0, NULL, NULL, NULL,
		&cspace);
	assert(err == CCS_RESULT_SUCCESS);

	/* binary: the condition on parameter 2 is its index followed by the
	 * expression body, i.e. the standalone serialization of the
	 * expression without its 13-byte file header (magic, size, version) */
	buff      = serialize(cspace, &size);
	expr_buff = serialize(conditions[2], &expr_size);
	assert(expr_size > 13);
	offset = find_bytes(buff, size, expr_buff + 13, expr_size - 13);
	assert(offset > 0 && buff[offset - 1] == 2);

	err = ccs_object_deserialize(
		&object, CCS_SERIALIZE_FORMAT_BINARY,
		CCS_DESERIALIZE_OPERATION_MEMORY, size, buff,
		CCS_DESERIALIZE_OPTION_END);
	assert(err == CCS_RESULT_SUCCESS);
	ccs_release_object(object);
	object           = NULL;

	buff[offset - 1] = 1;
	err              = ccs_object_deserialize(
                &object, CCS_SERIALIZE_FORMAT_BINARY,
                CCS_DESERIALIZE_OPERATION_MEMORY, size, buff,
                CCS_DESERIALIZE_OPTION_END);
	assert(err == CCS_RESULT_ERROR_INVALID_VALUE);
	assert(object == NULL);
	free(buff);
	free(expr_buff);

	/* JSON: same corruption, same length */
	err = ccs_object_serialize(
		cspace, CCS_SERIALIZE_FORMAT_JSON, CCS_SERIALIZE_OPERATION_SIZE,
		&size, CCS_SERIALIZE_OPTION_END);
	assert(err == CCS_RESULT_SUCCESS);
	buff = (char *)malloc(size);
	assert(buff);
	err = ccs_object_serialize(
		cspace, CCS_SERIALIZE_FORMAT_JSON,
		CCS_SERIALIZE_OPERATION_MEMORY, size, buff,
		CCS_SERIALIZE_OPTION_END);
	assert(err == CCS_RESULT_SUCCESS);
	index_str = strstr(buff, "\"index\":2");
	assert(index_str && !strstr(index_str + 1, "\"index\":2"));
	index_str[8] = '1';
	err          = ccs_object_deserialize(
                &object, CCS_SERIALIZE_FORMAT_JSON,
                CCS_DESERIALIZE_OPERATION_MEMORY, size, buff,
                CCS_DESERIALIZE_OPTION_END);
	assert(err == CCS_RESULT_ERROR_INVALID_VALUE);
	assert(object == NULL);
	free(buff);

	ccs_release_object(cspace);
	for (int i = 0; i < 3; i++)
		ccs_release_object(parameters[i]);
	for (int i = 1; i < 3; i++)
		ccs_release_object(conditions[i]);
}

/* Roulette areas and mixture weights are normalized by the inverse of their
 * sum. An infinite area made that inverse 0 and every normalized area NaN,
 * which then failed to deserialize after a round trip; a NaN area slipped
 * through the negativity check. */
static void
test_non_finite_weights(void)
{
	ccs_result_t       err;
	ccs_distribution_t distribution = NULL, distributions[2];
	ccs_float_t        bad[][2]     = {
                {1.0, INFINITY}, {NAN, 1.0}, {DBL_MAX, DBL_MAX}};

	for (int i = 0; i < 2; i++) {
		err = ccs_create_uniform_float_distribution(
			0.0, 1.0, CCS_SCALE_TYPE_LINEAR, 0.0,
			distributions + i);
		assert(err == CCS_RESULT_SUCCESS);
	}
	for (size_t i = 0; i < sizeof(bad) / sizeof(bad[0]); i++) {
		err = ccs_create_roulette_distribution(
			2, bad[i], &distribution);
		assert(err == CCS_RESULT_ERROR_INVALID_VALUE);
		assert(distribution == NULL);
		err = ccs_create_mixture_distribution(
			2, distributions, bad[i], &distribution);
		assert(err == CCS_RESULT_ERROR_INVALID_VALUE);
		assert(distribution == NULL);
	}
	for (int i = 0; i < 2; i++)
		ccs_release_object(distributions[i]);
}

/* With these weights, rounding makes the cumulative weight before the last
 * (zero) one exceed 1.0, which the mixture constructor rejects. That error
 * path used to skip releasing a temporary buffer. */
static void
test_mixture_rounding_error_path(void)
{
	ccs_result_t       err;
	ccs_distribution_t distribution = NULL, distributions[6];
	ccs_float_t        weights[6]   = {1.0, 3.0, 1e-17, 0.1, 1.0, 0.0};

	for (int i = 0; i < 6; i++) {
		err = ccs_create_uniform_float_distribution(
			0.0, 1.0, CCS_SCALE_TYPE_LINEAR, 0.0,
			distributions + i);
		assert(err == CCS_RESULT_SUCCESS);
	}
	err = ccs_create_mixture_distribution(
		6, distributions, weights, &distribution);
	assert(err == CCS_RESULT_ERROR_INVALID_VALUE);
	assert(distribution == NULL);
	for (int i = 0; i < 6; i++)
		ccs_release_object(distributions[i]);
}

int
main(void)
{
	ccs_init();
	test_leaf_expression_with_nodes();
	test_huge_count();
	test_distribution_space_dimension_overflow();
	test_binding_huge_num_values();
	test_unterminated_string();
	test_tuner_unknown_type();
	test_int_range_overflow();
	test_duplicate_condition_index();
	test_non_finite_weights();
	test_mixture_rounding_error_path();
	ccs_clear_thread_error();
	ccs_fini();
	return 0;
}
