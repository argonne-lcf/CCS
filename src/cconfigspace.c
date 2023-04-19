#include "cconfigspace_internal.h"
#include <stdlib.h>
#include <gsl/gsl_rng.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

const ccs_datum_t   ccs_none     = CCS_NONE_VAL;
const ccs_datum_t   ccs_inactive = CCS_INACTIVE_VAL;
const ccs_datum_t   ccs_true     = CCS_TRUE_VAL;
const ccs_datum_t   ccs_false    = CCS_FALSE_VAL;
const ccs_version_t ccs_version  = {0, 1, 0, 0};

ccs_error_t
ccs_init()
{
	gsl_rng_env_setup();
	return CCS_SUCCESS;
}

ccs_error_t
ccs_fini()
{
	return CCS_SUCCESS;
}

ccs_version_t
ccs_get_version()
{
	return ccs_version;
}

ccs_error_t
ccs_retain_object(ccs_object_t object)
{
	_ccs_object_internal_t *obj = (_ccs_object_internal_t *)object;
	CCS_REFUTE(!obj || obj->refcount <= 0, CCS_INVALID_OBJECT);
	obj->refcount += 1;
	return CCS_SUCCESS;
}

ccs_error_t
ccs_release_object(ccs_object_t object)
{
	_ccs_object_internal_t *obj = (_ccs_object_internal_t *)object;
	CCS_REFUTE(!obj || obj->refcount <= 0, CCS_INVALID_OBJECT);
	obj->refcount -= 1;
	if (obj->refcount == 0) {
		if (obj->callbacks) {
			_ccs_object_callback_t *cb = NULL;
			while ((cb = (_ccs_object_callback_t *)utarray_prev(
					obj->callbacks, cb))) {
				cb->callback(object, cb->user_data);
			}
			utarray_free(obj->callbacks);
		}
		CCS_VALIDATE(obj->ops->del(object));
		free(object);
	}
	return CCS_SUCCESS;
}

ccs_error_t
ccs_object_get_type(ccs_object_t object, ccs_object_type_t *type_ret)
{
	CCS_REFUTE(!object, CCS_INVALID_OBJECT);
	CCS_CHECK_PTR(type_ret);
	_ccs_object_internal_t *obj = (_ccs_object_internal_t *)object;
	*type_ret                   = obj->type;
	return CCS_SUCCESS;
}

ccs_error_t
ccs_object_get_refcount(ccs_object_t object, int32_t *refcount_ret)
{
	CCS_REFUTE(!object, CCS_INVALID_OBJECT);
	CCS_CHECK_PTR(refcount_ret);
	_ccs_object_internal_t *obj = (_ccs_object_internal_t *)object;
	*refcount_ret               = obj->refcount;
	return CCS_SUCCESS;
}

static const UT_icd _object_callback_icd = {
	sizeof(_ccs_object_callback_t), NULL, NULL, NULL};

ccs_error_t
ccs_object_set_destroy_callback(
	ccs_object_t object,
	void (*callback)(ccs_object_t object, void *user_data),
	void *user_data)
{
	CCS_REFUTE(!object, CCS_INVALID_OBJECT);
	CCS_CHECK_PTR(callback);
	_ccs_object_internal_t *obj = (_ccs_object_internal_t *)object;
	if (!obj->callbacks)
		utarray_new(obj->callbacks, &_object_callback_icd);
	_ccs_object_callback_t cb = {callback, user_data};
	utarray_push_back(obj->callbacks, &cb);
	return CCS_SUCCESS;
}

ccs_error_t
ccs_object_set_user_data(ccs_object_t object, void *user_data)
{
	CCS_REFUTE(!object, CCS_INVALID_OBJECT);
	_ccs_object_internal_t *obj = (_ccs_object_internal_t *)object;
	obj->user_data              = user_data;
	return CCS_SUCCESS;
}

ccs_error_t
ccs_object_get_user_data(ccs_object_t object, void **user_data_ret)
{
	CCS_REFUTE(!object, CCS_INVALID_OBJECT);
	CCS_CHECK_PTR(user_data_ret);
	_ccs_object_internal_t *obj = (_ccs_object_internal_t *)object;
	*user_data_ret              = obj->user_data;
	return CCS_SUCCESS;
}

ccs_error_t
ccs_object_set_serialize_callback(
	ccs_object_t                    object,
	ccs_object_serialize_callback_t callback,
	void                           *user_data)
{
	CCS_REFUTE(!object, CCS_INVALID_OBJECT);
	_ccs_object_internal_t *obj = (_ccs_object_internal_t *)object;
	obj->serialize_callback     = callback;
	obj->serialize_user_data    = user_data;
	return CCS_SUCCESS;
}

static size_t
_ccs_serialize_header_size(ccs_serialize_format_t format)
{
	switch (format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		/* MAGIC + size */
		return _ccs_serialize_bin_size_magic_tag(_ccs_magic_tag) +
		       _ccs_serialize_bin_size_uncompressed_uint64(0) +
		       CCS_SERIALIZATION_API_VERSION_SERIALIZE_SIZE_BIN(
			       CCS_SERIALIZATION_API_VERSION);
		break;
	default:
		return 0;
	}
}

static ccs_error_t
_ccs_serialize_header(
	ccs_serialize_format_t format,
	size_t                *buffer_size,
	char                 **buffer,
	size_t                 size)
{
	switch (format) {
	case CCS_SERIALIZE_FORMAT_BINARY: {
		CCS_VALIDATE(_ccs_serialize_bin_magic_tag(
			_ccs_magic_tag, buffer_size, buffer));
		CCS_VALIDATE(_ccs_serialize_bin_uncompressed_uint64(
			size, buffer_size, buffer));
		CCS_VALIDATE(CCS_SERIALIZATION_API_VERSION_SERIALIZE_BIN(
			CCS_SERIALIZATION_API_VERSION, buffer_size, buffer));
	} break;
	default:
		CCS_RAISE(
			CCS_INVALID_VALUE,
			"Unsupported serialization format: %d", format);
	}
	return CCS_SUCCESS;
}

static ccs_error_t
_ccs_deserialize_header(
	ccs_serialize_format_t format,
	size_t                *buffer_size,
	const char           **buffer,
	size_t                *size,
	uint32_t              *version)
{
	switch (format) {
	case CCS_SERIALIZE_FORMAT_BINARY: {
		char     tag[4];
		uint64_t sz;
		CCS_VALIDATE(_ccs_deserialize_bin_magic_tag(
			tag, buffer_size, buffer));
		CCS_REFUTE(memcmp(tag, _ccs_magic_tag, 4), CCS_INVALID_VALUE);
		CCS_VALIDATE(_ccs_deserialize_bin_uncompressed_uint64(
			&sz, buffer_size, buffer));
		*size = sz;
		CCS_VALIDATE(CCS_SERIALIZATION_API_VERSION_DESERIALIZE_BIN(
			version, buffer_size, buffer));
		CCS_REFUTE(
			*version > CCS_SERIALIZATION_API_VERSION,
			CCS_INVALID_VALUE);
	} break;
	default:
		CCS_RAISE(
			CCS_INVALID_VALUE,
			"Unsupported serialization format: %d", format);
	}
	return CCS_SUCCESS;
}

static inline ccs_error_t
_ccs_object_serialize_options(
	ccs_serialize_format_t           format,
	ccs_serialize_operation_t        operation,
	va_list                          args,
	_ccs_object_serialize_options_t *opts)
{
	(void)format;
	ccs_serialize_option_t opt =
		(ccs_serialize_option_t)va_arg(args, int32_t);
	while (opt != CCS_SERIALIZE_OPTION_END) {
		switch (opt) {
		case CCS_SERIALIZE_OPTION_NON_BLOCKING:
			CCS_REFUTE(
				operation !=
					CCS_SERIALIZE_OPERATION_FILE_DESCRIPTOR,
				CCS_INVALID_VALUE);
			opts->ppfd_state =
				va_arg(args, _ccs_file_descriptor_state_t **);
			CCS_CHECK_PTR(opts->ppfd_state);
			break;
		case CCS_SERIALIZE_OPTION_CALLBACK:
			opts->serialize_callback =
				va_arg(args, ccs_object_serialize_callback_t);
			CCS_CHECK_PTR(opts->serialize_callback);
			opts->serialize_user_data = va_arg(args, void *);
			break;
		default:
			CCS_RAISE(
				CCS_INVALID_VALUE,
				"Unsupported serialization option: %d", opt);
		}
		opt = (ccs_serialize_option_t)va_arg(args, int32_t);
	}
	return CCS_SUCCESS;
}

static inline ccs_error_t
_ccs_object_serialize_size_with_opts(
	ccs_object_t                     object,
	ccs_serialize_format_t           format,
	size_t                          *buffer_size,
	_ccs_object_serialize_options_t *opts)
{
	_ccs_object_internal_t *obj = (_ccs_object_internal_t *)object;
	*buffer_size                = _ccs_serialize_header_size(format);
	CCS_VALIDATE(
		obj->ops->serialize_size(object, format, buffer_size, opts));
	return CCS_SUCCESS;
}

static inline ccs_error_t
_ccs_object_serialize_size(
	ccs_object_t           object,
	ccs_serialize_format_t format,
	va_list                args)
{
	size_t *p_buffer_size                = NULL;
	p_buffer_size                        = va_arg(args, size_t *);
	_ccs_object_serialize_options_t opts = {NULL, NULL, NULL};
	CCS_VALIDATE(_ccs_object_serialize_options(
		format, CCS_SERIALIZE_OPERATION_SIZE, args, &opts));
	CCS_CHECK_PTR(p_buffer_size);
	CCS_VALIDATE(_ccs_object_serialize_size_with_opts(
		object, format, p_buffer_size, &opts));
	return CCS_SUCCESS;
}

static inline ccs_error_t
_ccs_object_serialize_memory_with_opts(
	ccs_object_t                     object,
	ccs_serialize_format_t           format,
	size_t                           buffer_size,
	char                            *buffer,
	_ccs_object_serialize_options_t *opts)
{
	_ccs_object_internal_t *obj          = (_ccs_object_internal_t *)object;
	size_t                  total_size   = buffer_size;
	char                   *buffer_start = buffer;
	CCS_VALIDATE(_ccs_serialize_header(format, &buffer_size, &buffer, 0));
	CCS_VALIDATE(obj->ops->serialize(
		object, format, &buffer_size, &buffer, opts));
	CCS_VALIDATE(_ccs_serialize_header(
		format, &total_size, &buffer_start, total_size - buffer_size));
	return CCS_SUCCESS;
}

static inline ccs_error_t
_ccs_object_serialize_memory(
	ccs_object_t           object,
	ccs_serialize_format_t format,
	va_list                args)
{
	char                           *buffer      = NULL;
	size_t                          buffer_size = 0;
	_ccs_object_serialize_options_t opts        = {NULL, NULL, NULL};
	buffer_size                                 = va_arg(args, size_t);
	buffer                                      = va_arg(args, char *);
	CCS_CHECK_PTR(buffer);
	CCS_VALIDATE(_ccs_object_serialize_options(
		format, CCS_SERIALIZE_OPERATION_MEMORY, args, &opts));
	CCS_VALIDATE(_ccs_object_serialize_memory_with_opts(
		object, format, buffer_size, buffer, &opts));
	return CCS_SUCCESS;
}

static inline ccs_error_t
_ccs_object_serialize_file(
	ccs_object_t           object,
	ccs_serialize_format_t format,
	va_list                args)
{
	char                           *buffer      = NULL;
	size_t                          buffer_size = 0;
	const char                     *path;
	int                             fd;
	ccs_error_t                     res;
	_ccs_object_serialize_options_t opts = {NULL, NULL, NULL};
	path                                 = va_arg(args, const char *);
	CCS_CHECK_PTR(path);
	CCS_VALIDATE(_ccs_object_serialize_options(
		format, CCS_SERIALIZE_OPERATION_FILE, args, &opts));
	fd =
		open(path, O_CREAT | O_TRUNC | O_RDWR,
		     S_IRUSR | S_IWUSR | S_IWGRP | S_IRGRP | S_IROTH); // 664
	CCS_REFUTE(fd == -1, CCS_INVALID_FILE_PATH);
	CCS_VALIDATE_ERR_GOTO(
		res,
		_ccs_object_serialize_size_with_opts(
			object, format, &buffer_size, &opts),
		err_file_fd);
	buffer = (char *)mmap(
		0, buffer_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (CCS_UNLIKELY(buffer == MAP_FAILED)) {
		switch (errno) {
		case ENOMEM:
			CCS_RAISE_ERR_GOTO(
				res, CCS_OUT_OF_MEMORY, err_file_fd,
				"mmap failed: out of memory");
			break;
		case EACCES:
			CCS_RAISE_ERR_GOTO(
				res, CCS_INVALID_FILE_PATH, err_file_fd,
				"mmap failed: invalid file");
			break;
		default:
			CCS_RAISE_ERR_GOTO(
				res, CCS_SYSTEM_ERROR, err_file_fd,
				"mmap failed: unexpected system error");
		}
	}
	CCS_REFUTE_ERR_GOTO(
		res, ftruncate(fd, buffer_size) == -1, CCS_SYSTEM_ERROR,
		err_file_map);
	CCS_VALIDATE_ERR_GOTO(
		res,
		_ccs_object_serialize_memory_with_opts(
			object, format, buffer_size, buffer, &opts),
		err_file_map);
	CCS_REFUTE_ERR_GOTO(
		res, msync(buffer, buffer_size, MS_SYNC) == -1,
		CCS_SYSTEM_ERROR, err_file_map);
err_file_map:
	munmap(buffer, buffer_size);
err_file_fd:
	close(fd);
	return res;
}

static inline ccs_error_t
_ccs_object_serialize_file_descriptor(
	ccs_object_t           object,
	ccs_serialize_format_t format,
	va_list                args)
{
	int                             fd;
	ccs_error_t                     res;
	_ccs_object_serialize_options_t opts   = {NULL, NULL, NULL};
	_ccs_file_descriptor_state_t    state  = {NULL, 0, NULL, 0, -1, 0};
	_ccs_file_descriptor_state_t   *pstate = NULL;
	fd                                     = va_arg(args, int);
	CCS_VALIDATE(_ccs_object_serialize_options(
		format, CCS_SERIALIZE_OPERATION_FILE_DESCRIPTOR, args, &opts));
	/* non blocking */
	if (opts.ppfd_state) {
		/* restart */
		if (*(opts.ppfd_state)) {
			/* check coherency */
			CCS_REFUTE(
				(*(opts.ppfd_state))->fd != fd,
				CCS_INVALID_VALUE);
			pstate = *(opts.ppfd_state);
		}
	} else
		pstate = &state;
	/* if non blocking start or blocking */
	if (!pstate || !pstate->base) {
		size_t object_size;
		CCS_VALIDATE(_ccs_object_serialize_size_with_opts(
			object, format, &object_size, &opts));
		/* initialize user_state */
		if (!pstate) {
			char *mem = (char *)malloc(
				sizeof(_ccs_file_descriptor_state_t) +
				object_size);
			CCS_REFUTE(!mem, CCS_OUT_OF_MEMORY);
			*(opts.ppfd_state) = pstate =
				(_ccs_file_descriptor_state_t *)mem;
			pstate->base_size =
				sizeof(_ccs_file_descriptor_state_t) +
				object_size;
			pstate->buffer =
				mem + sizeof(_ccs_file_descriptor_state_t);
		} else {
			pstate->base = (char *)malloc(object_size);
			CCS_REFUTE(!pstate->base, CCS_OUT_OF_MEMORY);
			pstate->base_size = object_size;
			pstate->buffer    = pstate->base;
		}
		pstate->buffer_size = object_size;
		pstate->fd          = fd;
		CCS_VALIDATE_ERR_GOTO(
			res,
			_ccs_object_serialize_memory_with_opts(
				object, format, pstate->buffer_size,
				pstate->buffer, &opts),
			err_fd_buffer);
	}
	do {
		ssize_t count;
		count = write(fd, pstate->buffer, pstate->buffer_size);
		if (count == -1) {
			CCS_REFUTE_ERR_GOTO(
				res, errno != EAGAIN && errno != EINTR,
				CCS_SYSTEM_ERROR, err_fd_buffer);
			if (errno == EAGAIN && opts.ppfd_state)
				return CCS_AGAIN;
		} else {
			pstate->buffer_size -= count;
			pstate->buffer += count;
		}
	} while (pstate->buffer_size);
err_fd_buffer:
	free(pstate->base);
	if (opts.ppfd_state)
		*(opts.ppfd_state) = NULL;
	return res;
}

ccs_error_t
ccs_object_serialize(
	ccs_object_t              object,
	ccs_serialize_format_t    format,
	ccs_serialize_operation_t operation,
	...)
{
	_ccs_object_internal_t *obj = (_ccs_object_internal_t *)object;
	ccs_error_t             res;
	va_list                 args;

	CCS_REFUTE(!obj || !obj->ops, CCS_INVALID_OBJECT);
	CCS_REFUTE(!obj->ops->serialize, CCS_UNSUPPORTED_OPERATION);

	va_start(args, operation);
	switch (operation) {
	case CCS_SERIALIZE_OPERATION_SIZE:
		CCS_VALIDATE_ERR_GOTO(
			res, _ccs_object_serialize_size(object, format, args),
			end);
		break;
	case CCS_SERIALIZE_OPERATION_MEMORY:
		CCS_VALIDATE_ERR_GOTO(
			res, _ccs_object_serialize_memory(object, format, args),
			end);
		break;
	case CCS_SERIALIZE_OPERATION_FILE:
		CCS_VALIDATE_ERR_GOTO(
			res, _ccs_object_serialize_file(object, format, args),
			end);
		break;
	case CCS_SERIALIZE_OPERATION_FILE_DESCRIPTOR:
		CCS_VALIDATE_ERR_GOTO(
			res,
			_ccs_object_serialize_file_descriptor(
				object, format, args),
			end);
		break;
	default:
		CCS_RAISE_ERR_GOTO(
			res, CCS_INVALID_VALUE, end,
			"Unsupported serialize operation: %d", operation);
	}
end:
	va_end(args);
	return res;
}

#include "rng_deserialize.h"
#include "distribution_deserialize.h"
#include "parameter_deserialize.h"
#include "expression_deserialize.h"
#include "features_space_deserialize.h"
#include "configuration_space_deserialize.h"
#include "objective_space_deserialize.h"
#include "configuration_deserialize.h"
#include "evaluation_deserialize.h"
#include "features_deserialize.h"
#include "features_evaluation_deserialize.h"
#include "tuner_deserialize.h"
#include "features_tuner_deserialize.h"
#include "map_deserialize.h"
#include "tree_deserialize.h"
#include "tree_space_deserialize.h"
#include "tree_configuration_deserialize.h"
#include "tree_evaluation_deserialize.h"
#include "tree_tuner_deserialize.h"

static inline ccs_error_t
_ccs_object_deserialize_options(
	ccs_serialize_format_t             format,
	ccs_serialize_operation_t          operation,
	va_list                            args,
	_ccs_object_deserialize_options_t *opts)
{
	(void)format;
	ccs_deserialize_option_t opt =
		(ccs_deserialize_option_t)va_arg(args, int32_t);
	while (opt != CCS_DESERIALIZE_OPTION_END) {
		switch (opt) {
		case CCS_DESERIALIZE_OPTION_HANDLE_MAP:
			opts->handle_map = va_arg(args, ccs_map_t);
			CCS_CHECK_OBJ(opts->handle_map, CCS_MAP);
			break;
		case CCS_DESERIALIZE_OPTION_VECTOR:
			opts->vector = va_arg(args, void *);
			CCS_CHECK_PTR(opts->vector);
			break;
		case CCS_DESERIALIZE_OPTION_DATA:
			opts->data = va_arg(args, void *);
			break;
		case CCS_DESERIALIZE_OPTION_NON_BLOCKING:
			CCS_REFUTE(
				operation !=
					CCS_SERIALIZE_OPERATION_FILE_DESCRIPTOR,
				CCS_INVALID_VALUE);
			opts->ppfd_state =
				va_arg(args, _ccs_file_descriptor_state_t **);
			CCS_CHECK_PTR(opts->ppfd_state);
			break;
		case CCS_DESERIALIZE_CALLBACK:
			opts->deserialize_callback =
				va_arg(args, ccs_object_deserialize_callback_t);
			CCS_CHECK_PTR(opts->deserialize_callback);
			opts->deserialize_user_data = va_arg(args, void *);
			break;
		default:
			CCS_RAISE(
				CCS_INVALID_VALUE,
				"Unsupported deserialization option: %d", opt);
		}
		opt = (ccs_deserialize_option_t)va_arg(args, int32_t);
	}
	return CCS_SUCCESS;
}

static inline ccs_error_t
_ccs_object_deserialize_with_opts(
	ccs_object_t                      *object_ret,
	ccs_serialize_format_t             format,
	uint32_t                           version,
	size_t                            *buffer_size,
	const char                       **buffer,
	_ccs_object_deserialize_options_t *opts)
{
	switch (format) {
	case CCS_SERIALIZE_FORMAT_BINARY: {
		ccs_object_type_t otype;
		CCS_VALIDATE(_ccs_peek_bin_ccs_object_type(
			&otype, buffer_size, buffer));
		switch (otype) {
		case CCS_RNG:
			CCS_VALIDATE(_ccs_rng_deserialize(
				(ccs_rng_t *)object_ret, format, version,
				buffer_size, buffer, opts));
			break;
		case CCS_DISTRIBUTION:
			CCS_VALIDATE(_ccs_distribution_deserialize(
				(ccs_distribution_t *)object_ret, format,
				version, buffer_size, buffer, opts));
			break;
		case CCS_PARAMETER:
			CCS_VALIDATE(_ccs_parameter_deserialize(
				(ccs_parameter_t *)object_ret, format, version,
				buffer_size, buffer, opts));
			break;
		case CCS_EXPRESSION:
			CCS_VALIDATE(_ccs_expression_deserialize(
				(ccs_expression_t *)object_ret, format, version,
				buffer_size, buffer, opts));
			break;
		case CCS_FEATURES_SPACE:
			CCS_VALIDATE(_ccs_features_space_deserialize(
				(ccs_features_space_t *)object_ret, format,
				version, buffer_size, buffer, opts));
			break;
		case CCS_CONFIGURATION_SPACE:
			CCS_VALIDATE(_ccs_configuration_space_deserialize(
				(ccs_configuration_space_t *)object_ret, format,
				version, buffer_size, buffer, opts));
			break;
		case CCS_OBJECTIVE_SPACE:
			CCS_VALIDATE(_ccs_objective_space_deserialize(
				(ccs_objective_space_t *)object_ret, format,
				version, buffer_size, buffer, opts));
			break;
		case CCS_CONFIGURATION:
			CCS_VALIDATE(_ccs_configuration_deserialize(
				(ccs_configuration_t *)object_ret, format,
				version, buffer_size, buffer, opts));
			break;
		case CCS_EVALUATION:
			CCS_VALIDATE(_ccs_evaluation_deserialize(
				(ccs_evaluation_t *)object_ret, format, version,
				buffer_size, buffer, opts));
			break;
		case CCS_FEATURES:
			CCS_VALIDATE(_ccs_features_deserialize(
				(ccs_features_t *)object_ret, format, version,
				buffer_size, buffer, opts));
			break;
		case CCS_FEATURES_EVALUATION:
			CCS_VALIDATE(_ccs_features_evaluation_deserialize(
				(ccs_features_evaluation_t *)object_ret, format,
				version, buffer_size, buffer, opts));
			break;
		case CCS_TUNER:
			CCS_VALIDATE(_ccs_tuner_deserialize(
				(ccs_tuner_t *)object_ret, format, version,
				buffer_size, buffer, opts));
			break;
		case CCS_FEATURES_TUNER:
			CCS_VALIDATE(_ccs_features_tuner_deserialize(
				(ccs_features_tuner_t *)object_ret, format,
				version, buffer_size, buffer, opts));
			break;
		case CCS_MAP:
			CCS_VALIDATE(_ccs_map_deserialize(
				(ccs_map_t *)object_ret, format, version,
				buffer_size, buffer, opts));
			break;
		case CCS_TREE:
			CCS_VALIDATE(_ccs_tree_deserialize(
				(ccs_tree_t *)object_ret, format, version,
				buffer_size, buffer, opts));
			break;
		case CCS_TREE_SPACE:
			CCS_VALIDATE(_ccs_tree_space_deserialize(
				(ccs_tree_space_t *)object_ret, format, version,
				buffer_size, buffer, opts));
			break;
		case CCS_TREE_CONFIGURATION:
			CCS_VALIDATE(_ccs_tree_configuration_deserialize(
				(ccs_tree_configuration_t *)object_ret, format,
				version, buffer_size, buffer, opts));
			break;
		case CCS_TREE_EVALUATION:
			CCS_VALIDATE(_ccs_tree_evaluation_deserialize(
				(ccs_tree_evaluation_t *)object_ret, format,
				version, buffer_size, buffer, opts));
			break;
		case CCS_TREE_TUNER:
			CCS_VALIDATE(_ccs_tree_tuner_deserialize(
				(ccs_tree_tuner_t *)object_ret, format, version,
				buffer_size, buffer, opts));
			break;
		default:
			CCS_RAISE(
				CCS_UNSUPPORTED_OPERATION,
				"Unsupported object type: %d", otype);
		}
		break;
	}
	default:
		CCS_RAISE(
			CCS_INVALID_VALUE,
			"Unsupported serialization format: %d", format);
	}
	return CCS_SUCCESS;
}

static inline ccs_error_t
_ccs_object_deserialize(
	ccs_object_t             *object_ret,
	ccs_serialize_format_t    format,
	ccs_serialize_operation_t operation,
	size_t                   *buffer_size,
	const char              **buffer,
	va_list                   args)
{
	uint32_t                          version;
	size_t                            size;
	_ccs_object_deserialize_options_t opts = {NULL, CCS_TRUE, NULL, NULL,
						  NULL, NULL,     NULL};
	CCS_VALIDATE(_ccs_object_deserialize_options(
		format, operation, args, &opts));
	CCS_VALIDATE(_ccs_deserialize_header(
		format, buffer_size, buffer, &size, &version));
	CCS_VALIDATE(_ccs_object_deserialize_with_opts(
		object_ret, format, version, buffer_size, buffer, &opts));
	return CCS_SUCCESS;
}

static inline ccs_error_t
_ccs_object_deserialize_memory(
	ccs_object_t          *object_ret,
	ccs_serialize_format_t format,
	va_list                args)
{
	size_t      buffer_size = va_arg(args, size_t);
	const char *buffer      = va_arg(args, const char *);

	CCS_CHECK_PTR(buffer);
	CCS_VALIDATE(_ccs_object_deserialize(
		object_ret, format, CCS_SERIALIZE_OPERATION_MEMORY,
		&buffer_size, &buffer, args));
	return CCS_SUCCESS;
}

static inline ccs_error_t
_ccs_object_deserialize_file(
	ccs_object_t          *object_ret,
	ccs_serialize_format_t format,
	va_list                args)
{
	ccs_error_t res         = CCS_SUCCESS;
	size_t      buffer_size = 0;
	const char *buffer      = NULL;
	int         fd;
	struct stat stat_buffer;
	const char *path = va_arg(args, const char *);
	CCS_CHECK_PTR(path);
	fd = open(path, O_RDONLY);
	CCS_REFUTE(fd == -1, CCS_INVALID_FILE_PATH);
	CCS_REFUTE_ERR_GOTO(
		res, fstat(fd, &stat_buffer) == -1, CCS_SYSTEM_ERROR,
		err_file_fd);
	buffer_size = stat_buffer.st_size;
	buffer      = (const char *)mmap(
                0, buffer_size, PROT_READ, MAP_PRIVATE, fd, 0);
	if (CCS_UNLIKELY(buffer == MAP_FAILED)) {
		switch (errno) {
		case ENOMEM:
			CCS_RAISE_ERR_GOTO(
				res, CCS_OUT_OF_MEMORY, err_file_fd,
				"mmap failed: out of memory");
			break;
		case EACCES:
			CCS_RAISE_ERR_GOTO(
				res, CCS_INVALID_FILE_PATH, err_file_fd,
				"mmap failed: invalid file");
			break;
		default:
			CCS_RAISE_ERR_GOTO(
				res, CCS_SYSTEM_ERROR, err_file_fd,
				"mmap failed: unexpected system error");
		}
	}
	{
		const char *b  = buffer;
		size_t      bs = buffer_size;
		CCS_VALIDATE_ERR_GOTO(
			res,
			_ccs_object_deserialize(
				object_ret, format,
				CCS_SERIALIZE_OPERATION_FILE, &bs, &b, args),
			err_file_map);
	}
err_file_map:
	munmap((void *)buffer, buffer_size);
err_file_fd:
	close(fd);
	return res;
}

static inline ccs_error_t
_ccs_object_deserialize_file_descriptor_read_loop(
	_ccs_file_descriptor_state_t *pstate,
	int                           non_blocking)
{
	do {
		ssize_t count;
		count = read(pstate->fd, pstate->buffer, pstate->buffer_size);
		if (count == -1) {
			CCS_REFUTE(
				errno != EAGAIN && errno != EINTR,
				CCS_SYSTEM_ERROR);
			/* if non blocking and try again */
			if (errno == EAGAIN && non_blocking)
				return CCS_AGAIN;
		} else {
			pstate->buffer_size -= count;
			pstate->buffer += count;
		}
	} while (pstate->buffer_size);
	return CCS_SUCCESS;
}

#define FD_READ_LOOP(pstate, non_blocking)                                     \
	do {                                                                   \
		CCS_VALIDATE_ERR_GOTO(                                         \
			res,                                                   \
			_ccs_object_deserialize_file_descriptor_read_loop(     \
				pstate, non_blocking),                         \
			err_fd_buffer);                                        \
		if (res == CCS_AGAIN)                                          \
			return res;                                            \
	} while (0)

static inline ccs_error_t
_ccs_object_deserialize_file_descriptor(
	ccs_object_t          *object_ret,
	ccs_serialize_format_t format,
	va_list                args)
{
	ccs_error_t                       res = CCS_SUCCESS;
	int                               fd;
	int                               non_blocking;
	size_t                            header_size;
	ssize_t                           offset;
	_ccs_object_deserialize_options_t opts   = {NULL, CCS_TRUE, NULL, NULL,
						    NULL, NULL,     NULL};
	_ccs_file_descriptor_state_t      state  = {NULL, 0, NULL, 0, -1, 0};
	_ccs_file_descriptor_state_t     *pstate = NULL;
	fd                                       = va_arg(args, int);
	CCS_VALIDATE(_ccs_object_deserialize_options(
		format, CCS_SERIALIZE_OPERATION_FILE_DESCRIPTOR, args, &opts));
	non_blocking = !!(opts.ppfd_state);
	header_size  = _ccs_serialize_header_size(format);
	/* non blocking */
	if (non_blocking) {
		/* restart */
		if (*(opts.ppfd_state)) {
			/* check coherency */
			CCS_REFUTE(
				(*(opts.ppfd_state))->fd != fd,
				CCS_INVALID_VALUE);
			pstate = *(opts.ppfd_state);
		}
	} else
		pstate = &state;
	/* if non blocking start or blocking, allocate buffer for header */
	if (!pstate || !pstate->base) {
		offset = 0;
		if (non_blocking)
			offset += sizeof(_ccs_file_descriptor_state_t);
		char *mem = (char *)malloc(offset + header_size);
		CCS_REFUTE(!mem, CCS_OUT_OF_MEMORY);
		if (non_blocking) {
			*(opts.ppfd_state) = pstate =
				(_ccs_file_descriptor_state_t *)mem;
			pstate->base_size =
				0; /* Use zero as header phase marker */
		} else
			pstate->base_size = header_size;
		pstate->base        = mem;
		pstate->buffer      = mem + offset;
		pstate->buffer_size = header_size;
		pstate->fd          = fd;
	}
	/* if blocking or in first phase non blocking, read header to query total read size */
	if (!non_blocking || !pstate->base_size) {
		size_t object_size;
		char  *new_buffer;
		FD_READ_LOOP(pstate, non_blocking);
		/* rewind */
		pstate->buffer_size += header_size;
		pstate->buffer -= header_size;
		/* decode header */
		CCS_VALIDATE_ERR_GOTO(
			res,
			_ccs_deserialize_header(
				format, &pstate->buffer_size,
				(const char **)&pstate->buffer, &object_size,
				&pstate->version),
			err_fd_buffer);
		/* reallocate buffer to account for whole size */
		if (non_blocking)
			pstate->base_size =
				sizeof(_ccs_file_descriptor_state_t) +
				object_size;
		else
			pstate->base_size = object_size;
		new_buffer = (char *)realloc(pstate->base, pstate->base_size);
		CCS_REFUTE_ERR_GOTO(
			res, !new_buffer, CCS_OUT_OF_MEMORY, err_fd_buffer);
		pstate->base = new_buffer;
		/* seek to after the header */
		offset       = header_size;
		if (non_blocking)
			offset += sizeof(_ccs_file_descriptor_state_t);
		pstate->buffer_size = pstate->base_size - offset;
		pstate->buffer      = pstate->base + offset;
	}
	/* read rest of object */
	FD_READ_LOOP(pstate, non_blocking);
	/* rewind */
	offset = header_size;
	if (non_blocking)
		offset += sizeof(_ccs_file_descriptor_state_t);
	pstate->buffer_size = pstate->base_size - offset;
	pstate->buffer      = pstate->base + offset;
	/* decode object */
	CCS_VALIDATE_ERR_GOTO(
		res,
		_ccs_object_deserialize_with_opts(
			object_ret, format, pstate->version,
			&pstate->buffer_size, (const char **)&pstate->buffer,
			&opts),
		err_fd_buffer);
err_fd_buffer:
	free(pstate->base);
	if (opts.ppfd_state)
		*(opts.ppfd_state) = NULL;
	return res;
}

ccs_error_t
ccs_object_deserialize(
	ccs_object_t             *object_ret,
	ccs_serialize_format_t    format,
	ccs_serialize_operation_t operation,
	...)
{
	ccs_error_t res;
	va_list     args;

	CCS_CHECK_PTR(object_ret);

	va_start(args, operation);
	switch (operation) {
	case CCS_SERIALIZE_OPERATION_MEMORY:
		CCS_VALIDATE_ERR_GOTO(
			res,
			_ccs_object_deserialize_memory(object_ret, format, args),
			end);
		break;
	case CCS_SERIALIZE_OPERATION_FILE:
		CCS_VALIDATE_ERR_GOTO(
			res,
			_ccs_object_deserialize_file(object_ret, format, args),
			end);
		break;
	case CCS_SERIALIZE_OPERATION_FILE_DESCRIPTOR:
		CCS_VALIDATE_ERR_GOTO(
			res,
			_ccs_object_deserialize_file_descriptor(
				object_ret, format, args),
			end);
		break;
	default:
		CCS_RAISE_ERR_GOTO(
			res, CCS_INVALID_VALUE, end,
			"Unsupported deserialize operation: %d", operation);
	}
end:
	va_end(args);
	return res;
}

#define ETOCASE(value)                                                         \
	case value:                                                            \
		*name = #value;                                                \
		break

ccs_error_t
ccs_get_error_name(ccs_error_t error, const char **name)
{
	switch (error) {
		ETOCASE(CCS_AGAIN);
		ETOCASE(CCS_SUCCESS);
		ETOCASE(CCS_INVALID_OBJECT);
		ETOCASE(CCS_INVALID_VALUE);
		ETOCASE(CCS_INVALID_TYPE);
		ETOCASE(CCS_INVALID_SCALE);
		ETOCASE(CCS_INVALID_DISTRIBUTION);
		ETOCASE(CCS_INVALID_EXPRESSION);
		ETOCASE(CCS_INVALID_PARAMETER);
		ETOCASE(CCS_INVALID_CONFIGURATION);
		ETOCASE(CCS_INVALID_NAME);
		ETOCASE(CCS_INVALID_CONDITION);
		ETOCASE(CCS_INVALID_TUNER);
		ETOCASE(CCS_INVALID_GRAPH);
		ETOCASE(CCS_TYPE_NOT_COMPARABLE);
		ETOCASE(CCS_INVALID_BOUNDS);
		ETOCASE(CCS_OUT_OF_BOUNDS);
		ETOCASE(CCS_SAMPLING_UNSUCCESSFUL);
		ETOCASE(CCS_OUT_OF_MEMORY);
		ETOCASE(CCS_UNSUPPORTED_OPERATION);
		ETOCASE(CCS_INVALID_EVALUATION);
		ETOCASE(CCS_INVALID_FEATURES);
		ETOCASE(CCS_INVALID_FEATURES_TUNER);
		ETOCASE(CCS_INVALID_FILE_PATH);
		ETOCASE(CCS_NOT_ENOUGH_DATA);
		ETOCASE(CCS_HANDLE_DUPLICATE);
		ETOCASE(CCS_INVALID_HANDLE);
		ETOCASE(CCS_SYSTEM_ERROR);
		ETOCASE(CCS_EXTERNAL_ERROR);
		ETOCASE(CCS_INVALID_TREE);
		ETOCASE(CCS_INVALID_TREE_SPACE);
		ETOCASE(CCS_INVALID_TREE_TUNER);
	default:
		*name = NULL;
		CCS_RAISE(
			CCS_INVALID_VALUE, "Unsupported error code: %d", error);
	}
	return CCS_SUCCESS;
}
