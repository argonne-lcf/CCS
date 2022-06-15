#ifndef _CCS_ERROR_STACK_H
#define _CCS_ERROR_STACK_H

/**
 * @file error_stack.h
 * CCS rich error reporting.
 */

#ifdef __cplusplus
extern "C" {
#endif

extern ccs_error_stack_t
ccs_get_thread_error();

extern void
ccs_set_thread_error(ccs_error_stack_t error_stack);

extern void
ccs_clear_thread_error();

struct ccs_error_stack_elem_s {
	const char *file;
	int         line;
	const char *func;
};
typedef struct ccs_error_stack_elem_s ccs_error_stack_elem_t;

extern ccs_result_t
ccs_create_error_stack(
	ccs_error_stack_t *error_stack_ret,
	ccs_error_t        error_code,
	const char        *msg,
	...);

extern ccs_result_t
ccs_error_stack_push(
	ccs_error_stack_t  error_stack,
	const char        *file,
	int                line,
	const char        *func);

extern ccs_result_t
ccs_error_stack_get_elems(
	ccs_error_stack_t        error_stack,
	size_t                  *num_elems_ret,
	ccs_error_stack_elem_t **elems);

#ifdef __cplusplus
}
#endif

#endif //_CCS_ERROR_STACK_H
