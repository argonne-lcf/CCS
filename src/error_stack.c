#include "cconfigspace_internal.h"
#include "error_stack_internal.h"
#include <stdarg.h>

#define CCS_CHECK_ERROR_STACK(s) do { \
	if (CCS_UNLIKELY(!(s) || \
	    !((_ccs_object_template_t *)(s))->data || \
	    ((_ccs_object_template_t *)(s))->obj.type != CCS_ERROR_STACK)) \
		return -CCS_INVALID_OBJECT; \
} while (0)

static __thread ccs_error_stack_t ccs_error_stack = NULL;

ccs_error_stack_t
ccs_get_thread_error() {
	ccs_error_stack_t tmp = ccs_error_stack;
	ccs_error_stack = NULL;
	return tmp;
}

void
ccs_clear_thread_error() {
	if (ccs_error_stack)
		ccs_release_object(ccs_error_stack);
	ccs_error_stack = NULL;
}

ccs_result_t
ccs_set_thread_error(ccs_error_stack_t error_stack) {
	CCS_CHECK_ERROR_STACK(error_stack);
	if (ccs_error_stack)
		ccs_release_object(ccs_error_stack);
	ccs_error_stack = error_stack;
	return CCS_SUCCESS;
}


static ccs_result_t
_ccs_error_stack_del(ccs_object_t object) {
	ccs_error_stack_t error_stack = (ccs_error_stack_t)object;
	utarray_free(error_stack->data->elems);
	return CCS_SUCCESS;
}

static struct _ccs_error_stack_ops_s _error_stack_ops =
    { { &_ccs_error_stack_del, NULL, NULL } };

static const UT_icd _error_stack_elem_icd = {
	sizeof(ccs_error_stack_elem_t),
	NULL,
	NULL,
	NULL,
};

#undef  utarray_oom
#define utarray_oom() { \
	err = -CCS_OUT_OF_MEMORY; \
	goto arrays; \
}

ccs_result_t
ccs_create_error_stack(
		ccs_error_stack_t *error_stack_ret,
		ccs_error_t        error_code,
		const char        *msg,
		...) {
	ccs_result_t err;
	if (CCS_UNLIKELY(!error_stack_ret))
		return -CCS_INVALID_VALUE;
	size_t msg_size = 1;
	va_list args;
	if (msg) {
		va_start(args, msg);
		msg_size = vsnprintf(NULL, 0, msg, args) + 1;
		va_end(args);
	}
	uintptr_t mem = (uintptr_t)calloc(1, sizeof(struct _ccs_error_stack_s) + sizeof(struct _ccs_error_stack_data_s) + msg_size);
	if (!mem)
		return -CCS_OUT_OF_MEMORY;
	ccs_error_stack_t error_stack = (ccs_error_stack_t)mem;
	_ccs_object_init(&(error_stack->obj), CCS_ERROR_STACK, (_ccs_object_ops_t *)&_error_stack_ops);
	error_stack->data = (struct _ccs_error_stack_data_s *)(mem +
		 sizeof(struct _ccs_error_stack_s));
	error_stack->data->msg = (const char *)(mem +
		sizeof(struct _ccs_error_stack_s) +
		sizeof(struct _ccs_error_stack_data_s));
	utarray_new(error_stack->data->elems, &_error_stack_elem_icd);
	error_stack->data->error = error_code;
	if (msg) {
		va_start(args, msg);
		vsnprintf((char *)(error_stack->data->msg), msg_size, msg, args);
		va_end(args);
	}
	*error_stack_ret = error_stack;
	return CCS_SUCCESS;
arrays:
	free((void *)mem);
	return err;
}

#undef  utarray_oom
#define utarray_oom() { \
	return -CCS_OUT_OF_MEMORY; \
}
ccs_result_t
ccs_error_stack_push(
		ccs_error_stack_t  error_stack,
		const char        *file,
		int                line,
		const char        *func) {
	CCS_CHECK_ERROR_STACK(error_stack);
	ccs_error_stack_elem_t elem = {file, line, func};
	utarray_push_back(error_stack->data->elems, &elem);
	return CCS_SUCCESS;
}

ccs_result_t
ccs_error_stack_get_elems(
		ccs_error_stack_t        error_stack,
		size_t                  *num_elems_ret,
		ccs_error_stack_elem_t **elems) {
	CCS_CHECK_ERROR_STACK(error_stack);
	if (CCS_UNLIKELY(!num_elems_ret || !elems))
		return -CCS_INVALID_VALUE;
	*num_elems_ret = utarray_len(error_stack->data->elems);
	if (*num_elems_ret)
		*elems = (ccs_error_stack_elem_t *)utarray_eltptr(error_stack->data->elems, 0);
	else
		*elems = NULL;
	return CCS_SUCCESS;
}

ccs_result_t
ccs_error_stack_get_message(
		ccs_error_stack_t   error_stack,
		const char        **message_ret) {
	CCS_CHECK_ERROR_STACK(error_stack);
	if (CCS_UNLIKELY(!message_ret))
		return -CCS_INVALID_VALUE;
	*message_ret = error_stack->data->msg;
	return CCS_SUCCESS;
}

ccs_result_t
ccs_error_stack_get_code(
		ccs_error_stack_t  error_stack,
		ccs_error_t       *error_code_ret) {
	CCS_CHECK_ERROR_STACK(error_stack);
	if (CCS_UNLIKELY(!error_code_ret))
		return -CCS_INVALID_VALUE;
	*error_code_ret = error_stack->data->error;
	return CCS_SUCCESS;
}
