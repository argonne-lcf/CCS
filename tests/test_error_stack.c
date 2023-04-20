#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <cconfigspace.h>

void
test_error_stack()
{
	ccs_error_stack_t       s1, s2;
	ccs_result_t            res;
	ccs_result_t            err;
	const char             *msg, *file, *func;
	int                     line;
	size_t                  num_elem;
	ccs_error_stack_elem_t *elems;

	s2 = ccs_get_thread_error();
	assert(NULL == s2);

	res = ccs_create_error_stack(
		&s1, CCS_INVALID_VALUE,
		"An invalid value was specified for %s: %d", "param", 5);
	assert(CCS_SUCCESS == res);

	ccs_set_thread_error(s1);
	s1  = ccs_get_thread_error();

	res = ccs_error_stack_get_code(s1, &err);
	assert(CCS_SUCCESS == res);
	assert(CCS_INVALID_VALUE == err);

	res = ccs_error_stack_get_message(s1, &msg);
	assert(CCS_SUCCESS == res);
	assert(!strcmp("An invalid value was specified for param: 5", msg));

	res = ccs_error_stack_get_elems(s1, &num_elem, &elems);
	assert(CCS_SUCCESS == res);
	assert(0 == num_elem);
	assert(NULL == elems);

	file = __FILE__;
	line = __LINE__;
	func = __func__;
	res  = ccs_error_stack_push(s1, file, line, func);
	assert(CCS_SUCCESS == res);

	res = ccs_error_stack_get_elems(s1, &num_elem, &elems);
	assert(CCS_SUCCESS == res);
	assert(1 == num_elem);
	assert(elems);
	assert(!strcmp(file, elems[0].file));
	assert(!strcmp(func, elems[0].func));
	assert(line == elems[0].line);

	file = __FILE__;
	line = __LINE__;
	func = __func__;
	res  = ccs_error_stack_push(s1, file, line, func);
	assert(CCS_SUCCESS == res);

	res = ccs_error_stack_get_elems(s1, &num_elem, &elems);
	assert(CCS_SUCCESS == res);
	assert(2 == num_elem);
	assert(elems);
	assert(!strcmp(file, elems[1].file));
	assert(!strcmp(func, elems[1].func));
	assert(line == elems[1].line);

	ccs_set_thread_error(s1);

	res = ccs_create_thread_error(CCS_INVALID_OBJECT, NULL);
	assert(CCS_SUCCESS == res);
	file = __FILE__;
	line = __LINE__;
	func = __func__;
	res  = ccs_thread_error_stack_push(file, line, func);
	assert(CCS_SUCCESS == res);

	s2  = ccs_get_thread_error();
	res = ccs_error_stack_get_code(s2, &err);
	assert(CCS_SUCCESS == res);
	assert(CCS_INVALID_OBJECT == err);

	res = ccs_error_stack_get_elems(s2, &num_elem, &elems);
	assert(CCS_SUCCESS == res);
	assert(1 == num_elem);
	assert(elems);
	assert(!strcmp(file, elems[0].file));
	assert(!strcmp(func, elems[0].func));
	assert(line == elems[0].line);

	ccs_release_object(s2);
}

int
main()
{
	ccs_init();
	test_error_stack();
	ccs_fini();
	return 0;
}
