#include "cconfigspace_internal.h"
#include <stdlib.h>
#include <gsl/gsl_rng.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include "version.h"

const ccs_datum_t   ccs_none     = CCS_NONE_VAL;
const ccs_datum_t   ccs_inactive = CCS_INACTIVE_VAL;
const ccs_datum_t   ccs_true     = CCS_TRUE_VAL;
const ccs_datum_t   ccs_false    = CCS_FALSE_VAL;
const ccs_version_t ccs_version  = {
        CCS_VERSION_REVISION, CCS_VERSION_PATCH, CCS_VERSION_MINOR,
        CCS_VERSION_MAJOR};

#if CCS_THREAD_SAFE
static pthread_mutex_t _ccs_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif
static int32_t _ccs_refcount = 0;

ccs_result_t
ccs_init(void)
{
	ccs_result_t err = CCS_RESULT_SUCCESS;
	CCS_MUTEX_LOCK(_ccs_mutex);

	CCS_REFUTE_ERR_GOTO(
		err, _ccs_refcount < 0 || _ccs_refcount == INT32_MAX,
		CCS_RESULT_ERROR_INVALID_VALUE, end);
	if (_ccs_refcount == 0)
		gsl_rng_env_setup();
	_ccs_refcount += 1;
end:
	CCS_MUTEX_UNLOCK(_ccs_mutex);
	return err;
}

ccs_result_t
ccs_fini(void)
{
	ccs_result_t err = CCS_RESULT_SUCCESS;
	CCS_MUTEX_LOCK(_ccs_mutex);

	CCS_REFUTE_ERR_GOTO(
		err, _ccs_refcount < 1, CCS_RESULT_ERROR_INVALID_VALUE, end);
	_ccs_refcount -= 1;
end:
	CCS_MUTEX_UNLOCK(_ccs_mutex);
	return err;
}

ccs_version_t
ccs_get_version(void)
{
	return ccs_version;
}

const char *
ccs_get_version_string(void)
{
	return CCS_VERSION_STRING;
}

static inline int32_t
_ccs_inc_ref(_ccs_object_internal_t *obj)
{
	return CCS_ATOMIC_FETCH_ADD(obj->refcount);
}

ccs_result_t
ccs_retain_object(ccs_object_t object)
{
	CCS_REFUTE(!object, CCS_RESULT_ERROR_INVALID_OBJECT);
	_ccs_object_internal_t *obj      = (_ccs_object_internal_t *)object;
	int32_t                 refcount = _ccs_inc_ref(obj);
	CCS_REFUTE(refcount <= 0, CCS_RESULT_ERROR_INVALID_OBJECT);
	return CCS_RESULT_SUCCESS;
}

static inline int32_t
_ccs_dec_ref(_ccs_object_internal_t *obj)
{
	return CCS_ATOMIC_SUB_FETCH(obj->refcount);
}

ccs_result_t
ccs_release_object(ccs_object_t object)
{
	CCS_REFUTE(!object, CCS_RESULT_ERROR_INVALID_OBJECT);
	_ccs_object_internal_t *obj      = (_ccs_object_internal_t *)object;
	int32_t                 refcount = _ccs_dec_ref(obj);
	CCS_REFUTE(refcount < 0, CCS_RESULT_ERROR_INVALID_OBJECT);
	if (refcount == 0) {
		if (obj->callbacks) {
			_ccs_object_callback_t *cb = NULL;
			while ((cb = (_ccs_object_callback_t *)utarray_prev(
					obj->callbacks, cb))) {
				cb->callback(object, cb->user_data);
			}
			utarray_free(obj->callbacks);
		}
		CCS_VALIDATE(obj->ops->del(object));
		_ccs_object_deinit(obj);
		free(object);
	}
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
ccs_object_get_type(ccs_object_t object, ccs_object_type_t *type_ret)
{
	CCS_CHECK_BASE_OBJ(object);
	CCS_CHECK_PTR(type_ret);
	_ccs_object_internal_t *obj = (_ccs_object_internal_t *)object;
	*type_ret                   = obj->type;
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
ccs_object_get_refcount(ccs_object_t object, int32_t *refcount_ret)
{
	CCS_CHECK_BASE_OBJ(object);
	CCS_CHECK_PTR(refcount_ret);
	_ccs_object_internal_t *obj = (_ccs_object_internal_t *)object;
	*refcount_ret               = CCS_ATOMIC_LOAD(obj->refcount);
	return CCS_RESULT_SUCCESS;
}

static const UT_icd _object_callback_icd = {
	sizeof(_ccs_object_callback_t), NULL, NULL, NULL};

#undef utarray_oom
#define utarray_oom()                                                          \
	{                                                                      \
		CCS_RAISE_ERR_GOTO(                                            \
			err, CCS_RESULT_ERROR_OUT_OF_MEMORY, end,              \
			"Not enough memory to allocate array");                \
	}
ccs_result_t
ccs_object_set_destroy_callback(
	ccs_object_t                  object,
	ccs_object_destroy_callback_t callback,
	void                         *user_data)
{
	CCS_CHECK_BASE_OBJ(object);
	CCS_CHECK_PTR(callback);
	ccs_result_t            err = CCS_RESULT_SUCCESS;
	_ccs_object_internal_t *obj = (_ccs_object_internal_t *)object;
	_ccs_object_callback_t  cb  = {callback, user_data};
	CCS_MUTEX_LOCK(obj->mutex);
	if (!obj->callbacks)
		utarray_new(obj->callbacks, &_object_callback_icd);
	utarray_push_back(obj->callbacks, &cb);
end:
	CCS_MUTEX_UNLOCK(obj->mutex);
	return err;
}
#undef utarray_oom
#define utarray_oom() exit(-1)

ccs_result_t
ccs_object_set_user_data(ccs_object_t object, void *user_data)
{
	CCS_CHECK_BASE_OBJ(object);
	_ccs_object_internal_t *obj = (_ccs_object_internal_t *)object;
	CCS_RWLOCK_WRLOCK(obj->lock);
	obj->user_data = user_data;
	CCS_RWLOCK_UNLOCK(obj->lock);
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
ccs_object_get_user_data(ccs_object_t object, void **user_data_ret)
{
	CCS_CHECK_BASE_OBJ(object);
	CCS_CHECK_PTR(user_data_ret);
	_ccs_object_internal_t *obj = (_ccs_object_internal_t *)object;
	CCS_RWLOCK_RDLOCK(obj->lock);
	*user_data_ret = obj->user_data;
	CCS_RWLOCK_UNLOCK(obj->lock);
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
ccs_object_set_serialize_callback(
	ccs_object_t                    object,
	ccs_object_serialize_callback_t callback,
	void                           *user_data)
{
	CCS_CHECK_BASE_OBJ(object);
	_ccs_object_internal_t *obj = (_ccs_object_internal_t *)object;
	CCS_RWLOCK_WRLOCK(obj->lock);
	obj->serialize_callback  = callback;
	obj->serialize_user_data = user_data;
	CCS_RWLOCK_UNLOCK(obj->lock);
	return CCS_RESULT_SUCCESS;
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

static ccs_result_t
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
	case CCS_SERIALIZE_FORMAT_JSON: {
		cJSON *header = *(cJSON **)buffer;
		CCS_VALIDATE(_ccs_json_add_int(
			header, "version", CCS_SERIALIZATION_API_VERSION));
		char hex[sizeof(size_t) * 2 + 1];
		_ccs_json_hex_encode_buf(&size, sizeof(size_t), hex);
		CCS_REFUTE(
			!cJSON_AddStringToObject(header, "size", hex),
			CCS_RESULT_ERROR_OUT_OF_MEMORY);
	} break;
	default:
		CCS_RAISE(
			CCS_RESULT_ERROR_INVALID_VALUE,
			"Unsupported serialization format: %d", format);
	}
	return CCS_RESULT_SUCCESS;
}

static ccs_result_t
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
		CCS_REFUTE(
			memcmp(tag, _ccs_magic_tag, 4),
			CCS_RESULT_ERROR_INVALID_VALUE);
		CCS_VALIDATE(_ccs_deserialize_bin_uncompressed_uint64(
			&sz, buffer_size, buffer));
		CCS_REFUTE(sz > SIZE_MAX, CCS_RESULT_ERROR_INVALID_VALUE);
		*size = sz;
		CCS_VALIDATE(CCS_SERIALIZATION_API_VERSION_DESERIALIZE_BIN(
			version, buffer_size, buffer));
		CCS_REFUTE(
			*version > CCS_SERIALIZATION_API_VERSION,
			CCS_RESULT_ERROR_INVALID_VALUE);
	} break;
	case CCS_SERIALIZE_FORMAT_JSON: {
		cJSON *j_header = *(cJSON **)buffer;
		cJSON *j_version =
			cJSON_GetObjectItemCaseSensitive(j_header, "version");
		cJSON *j_size =
			cJSON_GetObjectItemCaseSensitive(j_header, "size");
		ccs_int_t version_val;
		CCS_REFUTE(!j_version, CCS_RESULT_ERROR_INVALID_VALUE);
		CCS_VALIDATE(_ccs_json_get_int(j_version, &version_val));
		*version = (uint32_t)version_val;
		CCS_REFUTE(
			*version > CCS_SERIALIZATION_API_VERSION,
			CCS_RESULT_ERROR_INVALID_VALUE);
		CCS_REFUTE(
			!j_size || !cJSON_IsString(j_size),
			CCS_RESULT_ERROR_INVALID_VALUE);
		CCS_REFUTE(
			strlen(j_size->valuestring) != sizeof(size_t) * 2,
			CCS_RESULT_ERROR_INVALID_VALUE);
		CCS_REFUTE(
			_ccs_json_hex_decode_buf(
				j_size->valuestring, sizeof(size_t) * 2, size),
			CCS_RESULT_ERROR_INVALID_VALUE);
	} break;
	default:
		CCS_RAISE(
			CCS_RESULT_ERROR_INVALID_VALUE,
			"Unsupported serialization format: %d", format);
	}
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
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
				CCS_RESULT_ERROR_INVALID_VALUE);
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
				CCS_RESULT_ERROR_INVALID_VALUE,
				"Unsupported serialization option: %d", opt);
		}
		opt = (ccs_serialize_option_t)va_arg(args, int32_t);
	}
	return CCS_RESULT_SUCCESS;
}

/* Forward declaration — defined after _ccs_object_serialize_with_opts */
static inline ccs_result_t
_ccs_object_serialize_json_alloc(
	ccs_object_t                     object,
	char                           **buffer_ret,
	size_t                          *buffer_size_ret,
	_ccs_object_serialize_options_t *opts);

static inline ccs_result_t
_ccs_object_header_serialize_size_with_opts(
	ccs_object_t                     object,
	ccs_serialize_format_t           format,
	size_t                          *buffer_size,
	_ccs_object_serialize_options_t *opts)
{
	switch (format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		*buffer_size = _ccs_serialize_header_size(format);
		CCS_VALIDATE(_ccs_object_serialize_size_with_opts(
			object, format, buffer_size, opts));
		break;
	case CCS_SERIALIZE_FORMAT_JSON: {
		char  *str = NULL;
		size_t sz  = 0;
		CCS_VALIDATE(_ccs_object_serialize_json_alloc(
			object, &str, &sz, opts));
		*buffer_size = sz;
		free(str);
	} break;
	default:
		CCS_RAISE(
			CCS_RESULT_ERROR_INVALID_VALUE,
			"Unsupported serialization format: %d", format);
	}
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
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
	CCS_VALIDATE(_ccs_object_header_serialize_size_with_opts(
		object, format, p_buffer_size, &opts));
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_object_serialize_memory_with_opts(
	ccs_object_t                     object,
	ccs_serialize_format_t           format,
	size_t                           buffer_size,
	char                            *buffer,
	_ccs_object_serialize_options_t *opts)
{
	switch (format) {
	case CCS_SERIALIZE_FORMAT_BINARY: {
		size_t total_size   = buffer_size;
		char  *buffer_start = buffer;
		CCS_VALIDATE(_ccs_serialize_header(
			format, &buffer_size, &buffer, 0));
		CCS_VALIDATE(_ccs_object_serialize_with_opts(
			object, format, &buffer_size, &buffer, opts));
		CCS_VALIDATE(_ccs_serialize_header(
			format, &total_size, &buffer_start,
			total_size - buffer_size));
	} break;
	case CCS_SERIALIZE_FORMAT_JSON: {
		char  *str = NULL;
		size_t sz  = 0;
		CCS_VALIDATE(_ccs_object_serialize_json_alloc(
			object, &str, &sz, opts));
		if (sz > buffer_size) {
			free(str);
			CCS_RAISE(
				CCS_RESULT_ERROR_NOT_ENOUGH_DATA,
				"Buffer too small for JSON");
		}
		memcpy(buffer, str, sz);
		free(str);
	} break;
	default:
		CCS_RAISE(
			CCS_RESULT_ERROR_INVALID_VALUE,
			"Unsupported serialization format: %d", format);
	}
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
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
	return CCS_RESULT_SUCCESS;
}

/* Build a JSON buffer in a single pass: cJSON tree -> print -> patch size.
 * Caller receives the malloc'd string and must free it. */
static inline ccs_result_t
_ccs_object_serialize_json_alloc(
	ccs_object_t                     object,
	char                           **buffer_ret,
	size_t                          *buffer_size_ret,
	_ccs_object_serialize_options_t *opts)
{
	ccs_result_t err      = CCS_RESULT_SUCCESS;
	cJSON       *root     = NULL;
	cJSON       *hdr_node = NULL;
	cJSON       *obj_node = NULL;
	char        *bp       = NULL;
	char        *str      = NULL;
	char        *size_loc = NULL;
	size_t       dummy    = 0;
	size_t       sz       = 0;
	size_t       hex_len  = sizeof(size_t) * 2;
	root                  = cJSON_CreateObject();
	CCS_REFUTE(!root, CCS_RESULT_ERROR_OUT_OF_MEMORY);
	hdr_node = cJSON_AddObjectToObject(root, "header");
	CCS_REFUTE_ERR_GOTO(
		err, !hdr_node, CCS_RESULT_ERROR_OUT_OF_MEMORY, err_json_alloc);
	bp = (char *)hdr_node;
	CCS_VALIDATE_ERR_GOTO(
		err,
		_ccs_serialize_header(CCS_SERIALIZE_FORMAT_JSON, NULL, &bp, 0),
		err_json_alloc);
	obj_node = cJSON_AddObjectToObject(root, "object");
	CCS_REFUTE_ERR_GOTO(
		err, !obj_node, CCS_RESULT_ERROR_OUT_OF_MEMORY, err_json_alloc);
	bp = (char *)obj_node;
	CCS_VALIDATE_ERR_GOTO(
		err,
		_ccs_object_serialize_with_opts(
			object, CCS_SERIALIZE_FORMAT_JSON, &dummy, &bp, opts),
		err_json_alloc);
	str = cJSON_PrintUnformatted(root);
	cJSON_Delete(root);
	root = NULL;
	CCS_REFUTE(!str, CCS_RESULT_ERROR_OUT_OF_MEMORY);
	sz = strlen(str) + 1;
	{
		char size_hex_buf[sizeof(size_t) * 2 + 1];
		_ccs_json_hex_encode_buf(&sz, sizeof(size_t), size_hex_buf);
		size_loc = strstr(str, "\"size\":\"");
		if (size_loc) {
			size_loc += strlen("\"size\":\"");
			memcpy(size_loc, size_hex_buf, hex_len);
		}
	}
	*buffer_ret      = str;
	*buffer_size_ret = sz;
	return CCS_RESULT_SUCCESS;
err_json_alloc:
	if (root)
		cJSON_Delete(root);
	return err;
}

static inline ccs_result_t
_ccs_object_serialize_buffer(
	ccs_object_t           object,
	ccs_serialize_format_t format,
	va_list                args)
{
	char                          **buffer_ret      = NULL;
	size_t                         *buffer_size_ret = NULL;
	_ccs_object_serialize_options_t opts            = {NULL, NULL, NULL};
	buffer_ret                                      = va_arg(args, char **);
	buffer_size_ret = va_arg(args, size_t *);
	CCS_CHECK_PTR(buffer_ret);
	CCS_CHECK_PTR(buffer_size_ret);
	CCS_VALIDATE(_ccs_object_serialize_options(
		format, CCS_SERIALIZE_OPERATION_BUFFER, args, &opts));

	switch (format) {
	case CCS_SERIALIZE_FORMAT_BINARY: {
		ccs_result_t err = CCS_RESULT_SUCCESS;
		size_t       sz  = 0;
		char        *buf = NULL;
		CCS_VALIDATE(_ccs_object_header_serialize_size_with_opts(
			object, format, &sz, &opts));
		buf = (char *)malloc(sz);
		CCS_REFUTE(!buf, CCS_RESULT_ERROR_OUT_OF_MEMORY);
		CCS_VALIDATE_ERR_GOTO(
			err,
			_ccs_object_serialize_memory_with_opts(
				object, format, sz, buf, &opts),
			err_bin_buf);
		*buffer_ret      = buf;
		*buffer_size_ret = sz;
		break;
	err_bin_buf:
		free(buf);
		return err;
	}
	case CCS_SERIALIZE_FORMAT_JSON:
		CCS_VALIDATE(_ccs_object_serialize_json_alloc(
			object, buffer_ret, buffer_size_ret, &opts));
		break;
	default:
		CCS_RAISE(
			CCS_RESULT_ERROR_INVALID_VALUE,
			"Unsupported serialization format: %d", format);
	}
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_object_serialize_file(
	ccs_object_t           object,
	ccs_serialize_format_t format,
	va_list                args)
{
	char                           *buffer      = NULL;
	size_t                          buffer_size = 0;
	const char                     *path;
	int                             fd;
	ccs_result_t                    res;
	_ccs_object_serialize_options_t opts = {NULL, NULL, NULL};
	path                                 = va_arg(args, const char *);
	CCS_CHECK_PTR(path);
	CCS_VALIDATE(_ccs_object_serialize_options(
		format, CCS_SERIALIZE_OPERATION_FILE, args, &opts));
	switch (format) {
	case CCS_SERIALIZE_FORMAT_JSON: {
		FILE  *fp;
		size_t written;
		CCS_VALIDATE(_ccs_object_serialize_json_alloc(
			object, &buffer, &buffer_size, &opts));
		fp = fopen(path, "wb");
		if (!fp) {
			free(buffer);
			CCS_RAISE(
				CCS_RESULT_ERROR_INVALID_FILE_PATH,
				"Could not open file: %s", path);
		}
		written = fwrite(buffer, 1, buffer_size, fp);
		free(buffer);
		if (fclose(fp) != 0 || written != buffer_size)
			CCS_RAISE(
				CCS_RESULT_ERROR_SYSTEM,
				"Failed to write JSON to file");
		return CCS_RESULT_SUCCESS;
	}
	case CCS_SERIALIZE_FORMAT_BINARY:
		fd =
			open(path, O_CREAT | O_TRUNC | O_RDWR,
			     S_IRUSR | S_IWUSR | S_IWGRP | S_IRGRP | S_IROTH);
		CCS_REFUTE(fd == -1, CCS_RESULT_ERROR_INVALID_FILE_PATH);
		CCS_VALIDATE_ERR_GOTO(
			res,
			_ccs_object_header_serialize_size_with_opts(
				object, format, &buffer_size, &opts),
			err_file_fd);
		/* ftruncate must come before mmap so the file has the
		 * correct size before mapping. */
		CCS_REFUTE_ERR_GOTO(
			res, ftruncate(fd, buffer_size) == -1,
			CCS_RESULT_ERROR_SYSTEM, err_file_fd);
		buffer = (char *)mmap(
			0, buffer_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd,
			0);
		if (CCS_UNLIKELY(buffer == MAP_FAILED)) {
			switch (errno) {
			case ENOMEM:
				CCS_RAISE_ERR_GOTO(
					res, CCS_RESULT_ERROR_OUT_OF_MEMORY,
					err_file_truncated,
					"mmap failed: out of memory");
				break;
			case EACCES:
				CCS_RAISE_ERR_GOTO(
					res, CCS_RESULT_ERROR_INVALID_FILE_PATH,
					err_file_truncated,
					"mmap failed: invalid file");
				break;
			default:
				CCS_RAISE_ERR_GOTO(
					res, CCS_RESULT_ERROR_SYSTEM,
					err_file_truncated,
					"mmap failed: unexpected error");
			}
		}
		CCS_VALIDATE_ERR_GOTO(
			res,
			_ccs_object_serialize_memory_with_opts(
				object, format, buffer_size, buffer, &opts),
			err_file_map);
		CCS_REFUTE_ERR_GOTO(
			res, msync(buffer, buffer_size, MS_SYNC) == -1,
			CCS_RESULT_ERROR_SYSTEM, err_file_map);
	err_file_map:
		if (munmap(buffer, buffer_size) == -1) {
		}
	err_file_truncated:
		if (CCS_UNLIKELY(res < CCS_RESULT_SUCCESS))
			if (ftruncate(fd, 0) == -1) {
			}
	err_file_fd:
		if (close(fd) == -1) {
		}
		return res;
	default:
		CCS_RAISE(
			CCS_RESULT_ERROR_INVALID_VALUE,
			"Unsupported serialization format: %d", format);
	}
}

static inline ccs_result_t
_ccs_object_serialize_file_descriptor(
	ccs_object_t           object,
	ccs_serialize_format_t format,
	va_list                args)
{
	int                             fd;
	ccs_result_t                    res    = CCS_RESULT_SUCCESS;
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
				CCS_RESULT_ERROR_INVALID_VALUE);
			pstate = *(opts.ppfd_state);
		}
	} else
		pstate = &state;
	/* if non blocking start or blocking, allocate and fill buffer */
	if (!pstate || !pstate->base) {
		size_t object_size = 0;
		/* get size (binary) or full buffer (json) */
		CCS_VALIDATE(_ccs_object_header_serialize_size_with_opts(
			object, format, &object_size, &opts));
		/* initialize user_state */
		if (!pstate) {
			char *mem = (char *)malloc(
				sizeof(_ccs_file_descriptor_state_t) +
				object_size);
			CCS_REFUTE(!mem, CCS_RESULT_ERROR_OUT_OF_MEMORY);
			*(opts.ppfd_state) = pstate =
				(_ccs_file_descriptor_state_t *)mem;
			pstate->base = mem;
			pstate->base_size =
				sizeof(_ccs_file_descriptor_state_t) +
				object_size;
			pstate->buffer =
				mem + sizeof(_ccs_file_descriptor_state_t);
		} else {
			pstate->base = (char *)malloc(object_size);
			CCS_REFUTE(
				!pstate->base, CCS_RESULT_ERROR_OUT_OF_MEMORY);
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
				CCS_RESULT_ERROR_SYSTEM, err_fd_buffer);
			if (errno == EAGAIN && opts.ppfd_state)
				return CCS_RESULT_AGAIN;
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

ccs_result_t
ccs_object_serialize(
	ccs_object_t              object,
	ccs_serialize_format_t    format,
	ccs_serialize_operation_t operation,
	...)
{
	_ccs_object_internal_t *obj = (_ccs_object_internal_t *)object;
	ccs_result_t            res;
	va_list                 args;

	CCS_REFUTE(!obj || !obj->ops, CCS_RESULT_ERROR_INVALID_OBJECT);
	CCS_REFUTE(
		!obj->ops->serialize, CCS_RESULT_ERROR_UNSUPPORTED_OPERATION);

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
	case CCS_SERIALIZE_OPERATION_BUFFER:
		CCS_VALIDATE_ERR_GOTO(
			res, _ccs_object_serialize_buffer(object, format, args),
			end);
		break;
	default:
		CCS_RAISE_ERR_GOTO(
			res, CCS_RESULT_ERROR_INVALID_VALUE, end,
			"Unsupported serialize operation: %d", operation);
	}
end:
	va_end(args);
	return res;
}

#include "cconfigspace_deserialize.h"

static inline ccs_result_t
_ccs_object_deserialize(
	ccs_object_t               *object_ret,
	ccs_serialize_format_t      format,
	ccs_deserialize_operation_t operation,
	size_t                     *buffer_size,
	const char                **buffer,
	va_list                     args)
{
	uint32_t                          version;
	size_t                            size;
	ccs_map_checkpoint_t              map_checkpoint;
	ccs_result_t                      err  = CCS_RESULT_SUCCESS;
	_ccs_object_deserialize_options_t opts = {NULL, CCS_FALSE, NULL, NULL,
						  NULL, NULL,      NULL};
	CCS_VALIDATE(_ccs_object_deserialize_options(
		format, operation, args, &opts));
	switch (format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE(_ccs_deserialize_header(
			format, buffer_size, buffer, &size, &version));
		if (opts.map_values)
			CCS_VALIDATE(_ccs_map_get_checkpoint(
				opts.handle_map, &map_checkpoint));
		CCS_VALIDATE_ERR_GOTO(
			err,
			_ccs_object_deserialize_with_opts(
				object_ret, format, version, buffer_size,
				buffer, &opts),
			error);
		return CCS_RESULT_SUCCESS;
	error:
		if (opts.map_values)
			_ccs_map_rewind(opts.handle_map, map_checkpoint);
		return err;
	case CCS_SERIALIZE_FORMAT_JSON: {
		cJSON      *root     = NULL;
		cJSON      *j_header = NULL;
		cJSON      *j_obj    = NULL;
		const char *hdr_buf  = NULL;
		const char *obj_buf  = NULL;
		size_t      dummy    = 0;
		root = cJSON_ParseWithLength(*buffer, *buffer_size);
		CCS_REFUTE(!root, CCS_RESULT_ERROR_INVALID_VALUE);
		j_header = cJSON_GetObjectItemCaseSensitive(root, "header");
		CCS_REFUTE_ERR_GOTO(
			err, !j_header || !cJSON_IsObject(j_header),
			CCS_RESULT_ERROR_INVALID_VALUE, err_json);
		j_obj = cJSON_GetObjectItemCaseSensitive(root, "object");
		CCS_REFUTE_ERR_GOTO(
			err, !j_obj || !cJSON_IsObject(j_obj),
			CCS_RESULT_ERROR_INVALID_VALUE, err_json);
		hdr_buf = (const char *)j_header;
		CCS_VALIDATE_ERR_GOTO(
			err,
			_ccs_deserialize_header(
				format, &dummy, &hdr_buf, &size, &version),
			err_json);
		if (opts.map_values)
			CCS_VALIDATE_ERR_GOTO(
				err,
				_ccs_map_get_checkpoint(
					opts.handle_map, &map_checkpoint),
				err_json);
		obj_buf = (const char *)j_obj;
		CCS_VALIDATE_ERR_GOTO(
			err,
			_ccs_object_deserialize_with_opts(
				object_ret, format, version, &dummy, &obj_buf,
				&opts),
			err_json_map);
		cJSON_Delete(root);
		return CCS_RESULT_SUCCESS;
	err_json_map:
		if (opts.map_values)
			_ccs_map_rewind(opts.handle_map, map_checkpoint);
	err_json:
		cJSON_Delete(root);
		return err;
	}
	default:
		CCS_RAISE(
			CCS_RESULT_ERROR_INVALID_VALUE,
			"Unsupported serialization format: %d", format);
	}
}

static inline ccs_result_t
_ccs_object_deserialize_memory(
	ccs_object_t          *object_ret,
	ccs_serialize_format_t format,
	va_list                args)
{
	size_t      buffer_size = va_arg(args, size_t);
	const char *buffer      = va_arg(args, const char *);

	CCS_CHECK_PTR(buffer);
	CCS_VALIDATE(_ccs_object_deserialize(
		object_ret, format, CCS_DESERIALIZE_OPERATION_MEMORY,
		&buffer_size, &buffer, args));
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_object_deserialize_file(
	ccs_object_t          *object_ret,
	ccs_serialize_format_t format,
	va_list                args)
{
	ccs_result_t res         = CCS_RESULT_SUCCESS;
	size_t       buffer_size = 0;
	const char  *buffer      = NULL;
	int          fd;
	struct stat  stat_buffer;
	const char  *path = va_arg(args, const char *);
	CCS_CHECK_PTR(path);
	fd = open(path, O_RDONLY);
	CCS_REFUTE(fd == -1, CCS_RESULT_ERROR_INVALID_FILE_PATH);
	CCS_REFUTE_ERR_GOTO(
		res, fstat(fd, &stat_buffer) == -1, CCS_RESULT_ERROR_SYSTEM,
		err_file_fd);
	buffer_size = stat_buffer.st_size;
	buffer      = (const char *)mmap(
                0, buffer_size, PROT_READ, MAP_PRIVATE, fd, 0);
	if (CCS_UNLIKELY(buffer == MAP_FAILED)) {
		switch (errno) {
		case ENOMEM:
			CCS_RAISE_ERR_GOTO(
				res, CCS_RESULT_ERROR_OUT_OF_MEMORY,
				err_file_fd, "mmap failed: out of memory");
			break;
		case EACCES:
			CCS_RAISE_ERR_GOTO(
				res, CCS_RESULT_ERROR_INVALID_FILE_PATH,
				err_file_fd, "mmap failed: invalid file");
			break;
		default:
			CCS_RAISE_ERR_GOTO(
				res, CCS_RESULT_ERROR_SYSTEM, err_file_fd,
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
				CCS_DESERIALIZE_OPERATION_FILE, &bs, &b, args),
			err_file_map);
	}
err_file_map:
	if (munmap((void *)buffer, buffer_size) == -1) { /* best-effort */
	}
err_file_fd:
	if (close(fd) == -1) { /* best-effort */
	}
	return res;
}

static inline ccs_result_t
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
				CCS_RESULT_ERROR_SYSTEM);
			/* if non blocking and try again */
			if (errno == EAGAIN && non_blocking)
				return CCS_RESULT_AGAIN;
		} else {
			pstate->buffer_size -= count;
			pstate->buffer += count;
		}
	} while (pstate->buffer_size);
	return CCS_RESULT_SUCCESS;
}

#define FD_READ_LOOP(pstate, non_blocking)                                     \
	do {                                                                   \
		CCS_VALIDATE_ERR_GOTO(                                         \
			res,                                                   \
			_ccs_object_deserialize_file_descriptor_read_loop(     \
				pstate, non_blocking),                         \
			err_fd_buffer);                                        \
		if (res == CCS_RESULT_AGAIN)                                   \
			return res;                                            \
	} while (0)

/* Allocate header buffer and read header bytes from fd.
 * May return CCS_RESULT_AGAIN for non-blocking fds. */
static inline ccs_result_t
_ccs_object_deserialize_file_descriptor_header_read(
	ccs_serialize_format_t             format,
	int                                non_blocking,
	int                                fd,
	size_t                            *header_size_ptr,
	_ccs_file_descriptor_state_t     **pstate_ptr,
	_ccs_object_deserialize_options_t *opts)
{
	ccs_result_t                  res    = CCS_RESULT_SUCCESS;
	_ccs_file_descriptor_state_t *pstate = *pstate_ptr;
	ssize_t                       offset;
	switch (format) {
	case CCS_SERIALIZE_FORMAT_BINARY: {
		size_t header_size = *header_size_ptr;
		if (!pstate || !pstate->base) {
			offset = 0;
			if (non_blocking)
				offset += sizeof(_ccs_file_descriptor_state_t);
			char *mem = (char *)malloc(offset + header_size);
			CCS_REFUTE(!mem, CCS_RESULT_ERROR_OUT_OF_MEMORY);
			if (non_blocking) {
				*(opts->ppfd_state) = pstate =
					(_ccs_file_descriptor_state_t *)mem;
				pstate->base_size = 0;
			} else
				pstate->base_size = header_size;
			pstate->base        = mem;
			pstate->buffer      = mem + offset;
			pstate->buffer_size = header_size;
			pstate->fd          = fd;
		}
		if (!non_blocking || !pstate->base_size) {
			FD_READ_LOOP(pstate, non_blocking);
			/* rewind to start of header */
			pstate->buffer_size += header_size;
			pstate->buffer -= header_size;
		}
	} break;
	case CCS_SERIALIZE_FORMAT_JSON: {
		size_t len;
		offset = 0;
		if (non_blocking)
			offset += sizeof(_ccs_file_descriptor_state_t);
		if (!pstate || !pstate->base) {
			char *mem = (char *)malloc(offset + 256);
			CCS_REFUTE(!mem, CCS_RESULT_ERROR_OUT_OF_MEMORY);
			if (non_blocking) {
				*(opts->ppfd_state) = pstate =
					(_ccs_file_descriptor_state_t *)mem;
			}
			pstate->base        = mem;
			pstate->base_size   = 0; /* header phase */
			pstate->buffer      = mem + offset;
			pstate->buffer_size = 1;
			pstate->fd          = fd;
		}
		if (!pstate->base_size) {
			for (;;) {
				FD_READ_LOOP(pstate, non_blocking);
				if (*(pstate->buffer - 1) == '}')
					break;
				len = pstate->buffer - (pstate->base + offset);
				CCS_REFUTE_ERR_GOTO(
					res, len >= 256,
					CCS_RESULT_ERROR_INVALID_VALUE,
					err_fd_buffer);
				pstate->buffer_size = 1;
			}
			len = pstate->buffer - (pstate->base + offset);
			*header_size_ptr    = len;
			pstate->buffer      = pstate->base + offset;
			pstate->buffer_size = len;
		}
	} break;
	default:
		CCS_RAISE(
			CCS_RESULT_ERROR_INVALID_VALUE,
			"Unsupported serialization format: %d", format);
	}
	*pstate_ptr = pstate;
	return CCS_RESULT_SUCCESS;
err_fd_buffer:
	free(pstate->base);
	if (opts->ppfd_state)
		*(opts->ppfd_state) = NULL;
	return res;
}

/* Parse the header bytes in pstate, extract object_size and version,
 * then reallocate the buffer to hold the full object.
 * Sets pstate->buffer/buffer_size to the region after the header. */
static inline ccs_result_t
_ccs_object_deserialize_file_descriptor_header_decode(
	ccs_serialize_format_t             format,
	int                                non_blocking,
	size_t                             header_size,
	_ccs_file_descriptor_state_t     **pstate_ptr,
	_ccs_object_deserialize_options_t *opts)
{
	ccs_result_t                  res    = CCS_RESULT_SUCCESS;
	_ccs_file_descriptor_state_t *pstate = *pstate_ptr;
	size_t                        object_size;
	char                         *new_buffer;
	ssize_t                       offset;
	switch (format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE_ERR_GOTO(
			res,
			_ccs_deserialize_header(
				format, &pstate->buffer_size,
				(const char **)&pstate->buffer, &object_size,
				&pstate->version),
			err_fd_buffer);
		break;
	case CCS_SERIALIZE_FORMAT_JSON: {
		char       *first_brace  = NULL;
		char       *second_brace = NULL;
		size_t      remaining    = 0;
		size_t      inner_len    = 0;
		cJSON      *j_header     = NULL;
		const char *hdr_buf      = NULL;
		size_t      dummy        = 0;
		first_brace              = (char *)memchr(
                        pstate->buffer, '{', pstate->buffer_size);
		CCS_REFUTE_ERR_GOTO(
			res, !first_brace, CCS_RESULT_ERROR_INVALID_VALUE,
			err_fd_buffer);
		remaining = pstate->buffer_size -
			    (first_brace + 1 - pstate->buffer);
		second_brace = (char *)memchr(first_brace + 1, '{', remaining);
		CCS_REFUTE_ERR_GOTO(
			res, !second_brace, CCS_RESULT_ERROR_INVALID_VALUE,
			err_fd_buffer);
		inner_len =
			pstate->buffer_size - (second_brace - pstate->buffer);
		j_header = cJSON_ParseWithLength(second_brace, inner_len);
		CCS_REFUTE_ERR_GOTO(
			res, !j_header, CCS_RESULT_ERROR_INVALID_VALUE,
			err_fd_buffer);
		hdr_buf = (const char *)j_header;
		CCS_VALIDATE_ERR_GOTO(
			res,
			_ccs_deserialize_header(
				format, &dummy, &hdr_buf, &object_size,
				&pstate->version),
			err_fd_json_header);
		cJSON_Delete(j_header);
		break;
	err_fd_json_header:
		cJSON_Delete(j_header);
		goto err_fd_buffer;
	}
	default:
		CCS_RAISE_ERR_GOTO(
			res, CCS_RESULT_ERROR_INVALID_VALUE, err_fd_buffer,
			"Unsupported serialization format: %d", format);
	}
	if (non_blocking) {
		size_t new_size =
			sizeof(_ccs_file_descriptor_state_t) + object_size;
		new_buffer = (char *)realloc(pstate->base, new_size);
		CCS_REFUTE_ERR_GOTO(
			res, !new_buffer, CCS_RESULT_ERROR_OUT_OF_MEMORY,
			err_fd_buffer);
		pstate = (_ccs_file_descriptor_state_t *)new_buffer;
		*(opts->ppfd_state) = pstate;
		pstate->base        = new_buffer;
		pstate->base_size   = new_size;
	} else {
		pstate->base_size = object_size;
		new_buffer = (char *)realloc(pstate->base, pstate->base_size);
		CCS_REFUTE_ERR_GOTO(
			res, !new_buffer, CCS_RESULT_ERROR_OUT_OF_MEMORY,
			err_fd_buffer);
		pstate->base = new_buffer;
	}
	offset = header_size;
	if (non_blocking)
		offset += sizeof(_ccs_file_descriptor_state_t);
	pstate->buffer_size = pstate->base_size - offset;
	pstate->buffer      = pstate->base + offset;
	*pstate_ptr         = pstate;
	return CCS_RESULT_SUCCESS;
err_fd_buffer:
	free(pstate->base);
	if (opts->ppfd_state)
		*(opts->ppfd_state) = NULL;
	return res;
}

static inline ccs_result_t
_ccs_object_deserialize_file_descriptor_header(
	ccs_serialize_format_t             format,
	int                                non_blocking,
	int                                fd,
	size_t                            *header_size_ptr,
	_ccs_file_descriptor_state_t     **pstate_ptr,
	_ccs_object_deserialize_options_t *opts)
{
	ccs_result_t res = CCS_RESULT_SUCCESS;
	CCS_VALIDATE_ERR(
		res, _ccs_object_deserialize_file_descriptor_header_read(
			     format, non_blocking, fd, header_size_ptr,
			     pstate_ptr, opts));
	if (res == CCS_RESULT_AGAIN)
		return res;
	return _ccs_object_deserialize_file_descriptor_header_decode(
		format, non_blocking, *header_size_ptr, pstate_ptr, opts);
}

static inline ccs_result_t
_ccs_object_deserialize_file_descriptor(
	ccs_object_t          *object_ret,
	ccs_serialize_format_t format,
	va_list                args)
{
	ccs_result_t                      res = CCS_RESULT_SUCCESS;
	int                               non_blocking;
	size_t                            header_size;
	ssize_t                           offset;
	_ccs_object_deserialize_options_t opts   = {NULL, CCS_FALSE, NULL, NULL,
						    NULL, NULL,      NULL};
	_ccs_file_descriptor_state_t      state  = {NULL, 0, NULL, 0, -1, 0};
	_ccs_file_descriptor_state_t     *pstate = NULL;
	va_list                           args_copy;
	int                               fd = va_arg(args, int);
	va_copy(args_copy, args);
	CCS_VALIDATE(_ccs_object_deserialize_options(
		format, CCS_DESERIALIZE_OPERATION_FILE_DESCRIPTOR, args,
		&opts));
	non_blocking = !!(opts.ppfd_state);
	header_size  = _ccs_serialize_header_size(format);
	/* non blocking */
	if (non_blocking) {
		if (*(opts.ppfd_state)) {
			CCS_REFUTE(
				(*(opts.ppfd_state))->fd != fd,
				CCS_RESULT_ERROR_INVALID_VALUE);
			pstate = *(opts.ppfd_state);
		}
	} else
		pstate = &state;
	CCS_VALIDATE_ERR(
		res, _ccs_object_deserialize_file_descriptor_header(
			     format, non_blocking, fd, &header_size, &pstate,
			     &opts));
	if (res == CCS_RESULT_AGAIN)
		return res;
	/* read rest of object */
	FD_READ_LOOP(pstate, non_blocking);
	/* rewind to start of buffer (including header) */
	offset = 0;
	if (non_blocking)
		offset += sizeof(_ccs_file_descriptor_state_t);
	pstate->buffer_size = pstate->base_size - offset;
	pstate->buffer      = pstate->base + offset;
	/* decode via _ccs_object_deserialize (header + object) */
	CCS_VALIDATE_ERR_GOTO(
		res,
		_ccs_object_deserialize(
			object_ret, format,
			CCS_DESERIALIZE_OPERATION_FILE_DESCRIPTOR,
			&pstate->buffer_size, (const char **)&pstate->buffer,
			args_copy),
		err_fd_buffer);
err_fd_buffer:
	va_end(args_copy);
	free(pstate->base);
	if (opts.ppfd_state)
		*(opts.ppfd_state) = NULL;
	return res;
}

ccs_result_t
ccs_object_deserialize(
	ccs_object_t               *object_ret,
	ccs_serialize_format_t      format,
	ccs_deserialize_operation_t operation,
	...)
{
	ccs_result_t res;
	va_list      args;

	CCS_CHECK_PTR(object_ret);

	va_start(args, operation);
	switch (operation) {
	case CCS_DESERIALIZE_OPERATION_MEMORY:
		CCS_VALIDATE_ERR_GOTO(
			res,
			_ccs_object_deserialize_memory(object_ret, format, args),
			end);
		break;
	case CCS_DESERIALIZE_OPERATION_FILE:
		CCS_VALIDATE_ERR_GOTO(
			res,
			_ccs_object_deserialize_file(object_ret, format, args),
			end);
		break;
	case CCS_DESERIALIZE_OPERATION_FILE_DESCRIPTOR:
		CCS_VALIDATE_ERR_GOTO(
			res,
			_ccs_object_deserialize_file_descriptor(
				object_ret, format, args),
			end);
		break;
	default:
		CCS_RAISE_ERR_GOTO(
			res, CCS_RESULT_ERROR_INVALID_VALUE, end,
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

ccs_result_t
ccs_release_buffer(void *buffer)
{
	free(buffer);
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
ccs_get_result_name(ccs_result_t result, const char **name)
{
	switch (result) {
		ETOCASE(CCS_RESULT_AGAIN);
		ETOCASE(CCS_RESULT_SUCCESS);
		ETOCASE(CCS_RESULT_ERROR_INVALID_OBJECT);
		ETOCASE(CCS_RESULT_ERROR_INVALID_VALUE);
		ETOCASE(CCS_RESULT_ERROR_INVALID_TYPE);
		ETOCASE(CCS_RESULT_ERROR_INVALID_SCALE);
		ETOCASE(CCS_RESULT_ERROR_INVALID_DISTRIBUTION);
		ETOCASE(CCS_RESULT_ERROR_INVALID_EXPRESSION);
		ETOCASE(CCS_RESULT_ERROR_INVALID_PARAMETER);
		ETOCASE(CCS_RESULT_ERROR_INVALID_CONFIGURATION);
		ETOCASE(CCS_RESULT_ERROR_INVALID_NAME);
		ETOCASE(CCS_RESULT_ERROR_INVALID_CONDITION);
		ETOCASE(CCS_RESULT_ERROR_INVALID_TUNER);
		ETOCASE(CCS_RESULT_ERROR_INVALID_GRAPH);
		ETOCASE(CCS_RESULT_ERROR_TYPE_NOT_COMPARABLE);
		ETOCASE(CCS_RESULT_ERROR_INVALID_BOUNDS);
		ETOCASE(CCS_RESULT_ERROR_OUT_OF_BOUNDS);
		ETOCASE(CCS_RESULT_ERROR_SAMPLING_UNSUCCESSFUL);
		ETOCASE(CCS_RESULT_ERROR_OUT_OF_MEMORY);
		ETOCASE(CCS_RESULT_ERROR_UNSUPPORTED_OPERATION);
		ETOCASE(CCS_RESULT_ERROR_INVALID_EVALUATION);
		ETOCASE(CCS_RESULT_ERROR_INVALID_FEATURES);
		ETOCASE(CCS_RESULT_ERROR_INVALID_FILE_PATH);
		ETOCASE(CCS_RESULT_ERROR_NOT_ENOUGH_DATA);
		ETOCASE(CCS_RESULT_ERROR_DUPLICATE_HANDLE);
		ETOCASE(CCS_RESULT_ERROR_INVALID_HANDLE);
		ETOCASE(CCS_RESULT_ERROR_SYSTEM);
		ETOCASE(CCS_RESULT_ERROR_EXTERNAL);
		ETOCASE(CCS_RESULT_ERROR_INVALID_TREE);
		ETOCASE(CCS_RESULT_ERROR_INVALID_TREE_SPACE);
		ETOCASE(CCS_RESULT_ERROR_INVALID_DISTRIBUTION_SPACE);
	default:
		*name = NULL;
		CCS_RAISE(
			CCS_RESULT_ERROR_INVALID_VALUE,
			"Unsupported result code: %d", result);
	}
	return CCS_RESULT_SUCCESS;
}
