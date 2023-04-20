#include <stdlib.h>
#include <assert.h>
#include <cconfigspace.h>
#include <string.h>

static void
compare_parameter(ccs_parameter_t parameter)
{
	ccs_result_t         err;
	ccs_parameter_type_t type;
	ccs_datum_t          default_value;
	const char          *name;
	ccs_distribution_t   distribution;
	ccs_interval_t       interval;
	ccs_bool_t           check;

	err = ccs_parameter_get_type(parameter, &type);
	assert(err == CCS_SUCCESS);
	assert(type == CCS_PARAMETER_TYPE_STRING);

	err = ccs_parameter_get_default_value(parameter, &default_value);
	assert(err == CCS_SUCCESS);
	assert(default_value.type == CCS_DATA_TYPE_NONE);

	err = ccs_parameter_get_name(parameter, &name);
	assert(err == CCS_SUCCESS);
	assert(strcmp(name, "my_param") == 0);

	err = ccs_parameter_get_default_distribution(parameter, &distribution);
	assert(err == CCS_UNSUPPORTED_OPERATION);

	err = ccs_parameter_sampling_interval(parameter, &interval);
	assert(err == CCS_SUCCESS);
	err = ccs_interval_empty(&interval, &check);
	assert(err == CCS_SUCCESS);
	assert(CCS_TRUE == check);
}

void
test_create()
{
	ccs_parameter_t parameter;
	ccs_result_t    err;
	char           *buff;
	size_t          buff_size;

	err = ccs_create_string_parameter("my_param", &parameter);
	assert(err == CCS_SUCCESS);

	compare_parameter(parameter);

	err = ccs_object_serialize(
		parameter, CCS_SERIALIZE_FORMAT_BINARY,
		CCS_SERIALIZE_OPERATION_SIZE, &buff_size,
		CCS_SERIALIZE_OPTION_END);
	assert(err == CCS_SUCCESS);

	buff = (char *)malloc(buff_size);
	assert(buff);

	err = ccs_object_serialize(
		parameter, CCS_SERIALIZE_FORMAT_BINARY,
		CCS_SERIALIZE_OPERATION_MEMORY, buff_size, buff,
		CCS_SERIALIZE_OPTION_END);
	assert(err == CCS_SUCCESS);

	err = ccs_release_object(parameter);
	assert(err == CCS_SUCCESS);

	err = ccs_object_deserialize(
		(ccs_object_t *)&parameter, CCS_SERIALIZE_FORMAT_BINARY,
		CCS_SERIALIZE_OPERATION_MEMORY, buff_size, buff,
		CCS_DESERIALIZE_OPTION_END);
	assert(err == CCS_SUCCESS);
	free(buff);

	compare_parameter(parameter);

	ccs_release_object(parameter);
}

void
test_string_memoization()
{
	ccs_parameter_t parameter;
	ccs_result_t    err;
	ccs_datum_t     din, dout, dout2;
	ccs_bool_t      check;
	char           *str;

	err = ccs_create_string_parameter("my_param", &parameter);
	assert(err == CCS_SUCCESS);

	str       = strdup("my string");
	din       = ccs_string(str);
	din.flags = CCS_DATUM_FLAG_TRANSIENT;

	err       = ccs_parameter_validate_value(parameter, din, &dout, &check);
	assert(err == CCS_SUCCESS);
	assert(check == CCS_TRUE);
	assert(dout.type == CCS_DATA_TYPE_STRING);
	assert(dout.flags == 0);
	assert(strcmp(dout.value.s, "my string") == 0);
	err = ccs_parameter_validate_value(parameter, din, &dout2, NULL);
	assert(err == CCS_SUCCESS);
	assert(dout2.value.s == dout.value.s);

	strcpy(str, "nope");
	assert(strcmp(dout.value.s, "my string") == 0);

	err = ccs_parameter_validate_value(parameter, din, &dout, &check);
	assert(err == CCS_SUCCESS);
	assert(check == CCS_TRUE);
	assert(dout.type == CCS_DATA_TYPE_STRING);
	assert(dout.flags == 0);
	assert(strcmp(dout.value.s, "nope") == 0);

	din = ccs_int(3);
	err = ccs_parameter_validate_value(parameter, din, &dout, &check);
	assert(err == CCS_SUCCESS);
	assert(check == CCS_FALSE);
	assert(dout.type == CCS_DATA_TYPE_INACTIVE);

	ccs_release_object(parameter);
}

int
main()
{
	ccs_init();
	test_create();
	ccs_clear_thread_error();
	ccs_fini();
	return 0;
}
