#ifndef _CCS_ERROR_STACK_H
#define _CCS_ERROR_STACK_H

/**
 * @file error_stack.h
 * CCS rich error reporting.
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Transfers ownership of thread error stack from CCS to the user.
 * @returns the thread specific error stack or NULL if none exist.
 */
extern ccs_error_stack_t
ccs_get_thread_error();

/**
 * Transfers ownership of error stack from the user to CCS.
 * If a previous error was owned by the thread it will be released.
 * @param[in] error_stack the error stack to transfer
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p error_stack is not a valid CCS
 *                              error stack
 */
extern ccs_result_t
ccs_set_thread_error(ccs_error_stack_t error_stack);

/**
 * Clears the error stack of the calling thread, releasing the current
 * error if it existed.
 */
extern void
ccs_clear_thread_error();

/**
 * An element of stack.
 */
struct ccs_error_stack_elem_s {
	/** The file name */
	const char *file;
	/** The line number */
	int         line;
	/** The function name */
	const char *func;
};

/**
 * A commodity type to represent a CCS stack element.
 */
typedef struct ccs_error_stack_elem_s ccs_error_stack_elem_t;

/**
 * Creates a new error stack and sets it for the current thread, potentially
 * replacing the previous one.
 * @param[in] error_code the CCS error code
 * @param[in] msg a format string for the error message
 * @param[in] ... a list of optional values to fill the format string
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_OUT_OF_MEMORY if there was not enough memory to allocate the
 *                             error stack
 */
extern ccs_result_t
ccs_create_thread_error(ccs_result_t error_code, const char *msg, ...);

/**
 * Pushes a stack trace element on the thread error stack. A call to this
 * function may invalidate pointers obtained previously by
 * #ccs_error_stack_get_elems for this error stack.
 * @param[in] file the name of the file
 * @param[in] line the line number
 * @param[in] func the function name
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if the thread has no associated error stack
 * @return #CCS_RESULT_ERROR_OUT_OF_MEMORY if there was not enough memory to allocate
 *                             new stack elements
 */
extern ccs_result_t
ccs_thread_error_stack_push(const char *file, int line, const char *func);

/**
 * Creates a new error stack object.
 * @param[out] error_stack_ret a pointer to the variable that will contain
 *                             the newly created error stack.
 * @param[in] error_code the CCS error code
 * @param[in] msg a format string for the error message
 * @param[in] ... a list of optional values to fill the format string
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p error_stack_ret is NULL
 * @return #CCS_RESULT_ERROR_OUT_OF_MEMORY if there was not enough memory to allocate the
 *                             error stack
 */
extern ccs_result_t
ccs_create_error_stack(
	ccs_error_stack_t *error_stack_ret,
	ccs_result_t       error_code,
	const char        *msg,
	...);

/**
 * Pushes a stack trace element in an error stack. A call to this function
 * may invalidate pointers obtained previously by #ccs_error_stack_get_elems
 * for this error stack.
 * @param[in,out] error_stack
 * @param[in] file the name of the file
 * @param[in] line the line number
 * @param[in] func the function name
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p error_stack is not a valid CCS error
 *                              stack
 * @return #CCS_RESULT_ERROR_OUT_OF_MEMORY if there was not enough memory to allocate
 *                             new stack elements
 */
extern ccs_result_t
ccs_error_stack_push(
	ccs_error_stack_t error_stack,
	const char       *file,
	int               line,
	const char       *func);

/**
 * Retrieves the message from an error stack.
 * @param[in] error_stack
 * @param[out] message_ret a pointer to a variable that will contain the message.
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p error_stack is not a valid CCS error
 *                              stack
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p message_ret is NULL
 */
extern ccs_result_t
ccs_error_stack_get_message(
	ccs_error_stack_t error_stack,
	const char      **message_ret);

/**
 * Retrieves the error code from an error stack.
 * @param[in] error_stack
 * @param[out] error_code_ret a pointer to a variable that will contain the error code.
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p error_stack is not a valid CCS error
 *                              stack
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p error_code_ret is NULL
 */
extern ccs_result_t
ccs_error_stack_get_code(
	ccs_error_stack_t error_stack,
	ccs_result_t     *error_code_ret);

/**
 * Retrieves the stack elements from an error stack.
 * @param[in] error_stack
 * @param[out] num_elems_ret a pointer to a variable that will contain the number
 *                           of stack elements.
 * @param[out] elems a pointer to a variable that will point to the start of the
 *                   array of `*num_elems_ret` elements.
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p error_stack is not a valid CCS error
 *                              stack
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p num_elems_ret is NULL; or if \p elems is NULL
 */
extern ccs_result_t
ccs_error_stack_get_elems(
	ccs_error_stack_t        error_stack,
	size_t                  *num_elems_ret,
	ccs_error_stack_elem_t **elems);

#ifdef __cplusplus
}
#endif

#endif //_CCS_ERROR_STACK_H
