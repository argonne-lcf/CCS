#include "cconfigspace_internal.h"
#include "map_internal.h"
#include "datum_uthash.h"
#include "datum_hash.h"
#include <string.h>

struct _ccs_map_datum_s {
	ccs_datum_t    key;
	ccs_datum_t    value;
	UT_hash_handle hh;
};
typedef struct _ccs_map_datum_s _ccs_map_datum_t;

struct _ccs_map_data_s {
	_ccs_map_datum_t *map;
};
typedef struct _ccs_map_data_s _ccs_map_data_t;

static inline ccs_result_t
_ccs_map_check_object_and_release(ccs_datum_t d)
{
	if (d.type == CCS_DATA_TYPE_OBJECT && !(d.flags & CCS_DATUM_FLAG_ID))
		return ccs_release_object(d.value.o);
	return CCS_RESULT_SUCCESS;
}

static inline void
_ccs_map_remove(_ccs_map_data_t *data, _ccs_map_datum_t *entry)
{
	HASH_DEL(data->map, entry);
	_ccs_map_check_object_and_release(entry->key);
	_ccs_map_check_object_and_release(entry->value);
	free(entry);
}

static ccs_result_t
_ccs_map_del(ccs_object_t o)
{
	ccs_map_t         m    = (ccs_map_t)o;
	_ccs_map_data_t  *data = (_ccs_map_data_t *)(m->data);
	_ccs_map_datum_t *current, *tmp;
	HASH_ITER(hh, data->map, current, tmp)
	{
		_ccs_map_remove(data, current);
	}
	return CCS_RESULT_SUCCESS;
}

static inline size_t
_ccs_serialize_bin_size_ccs_map_data(_ccs_map_data_t *data)
{
	size_t            sz = 0;
	_ccs_map_datum_t *current, *tmp;
	sz += _ccs_serialize_bin_size_size(HASH_COUNT(data->map));
	HASH_ITER(hh, data->map, current, tmp)
	{
		sz += _ccs_serialize_bin_size_ccs_datum(current->key);
		sz += _ccs_serialize_bin_size_ccs_datum(current->value);
	}
	return sz;
}

static inline ccs_result_t
_ccs_serialize_bin_ccs_map_data(
	_ccs_map_data_t *data,
	size_t          *buffer_size,
	char           **buffer)
{
	_ccs_map_datum_t *current, *tmp;
	CCS_VALIDATE(_ccs_serialize_bin_size(
		HASH_COUNT(data->map), buffer_size, buffer));
	HASH_ITER(hh, data->map, current, tmp)
	{
		CCS_VALIDATE(_ccs_serialize_bin_ccs_datum(
			current->key, buffer_size, buffer));
		CCS_VALIDATE(_ccs_serialize_bin_ccs_datum(
			current->value, buffer_size, buffer));
	}
	return CCS_RESULT_SUCCESS;
}

static inline size_t
_ccs_serialize_bin_size_ccs_map(ccs_map_t map)
{
	_ccs_map_data_t *data = (_ccs_map_data_t *)(map->data);
	return _ccs_serialize_bin_size_ccs_object_internal(
		       (_ccs_object_internal_t *)map) +
	       _ccs_serialize_bin_size_ccs_map_data(data);
}

static inline ccs_result_t
_ccs_serialize_bin_ccs_map(ccs_map_t map, size_t *buffer_size, char **buffer)
{
	_ccs_map_data_t *data = (_ccs_map_data_t *)(map->data);
	CCS_VALIDATE(_ccs_serialize_bin_ccs_object_internal(
		(_ccs_object_internal_t *)map, buffer_size, buffer));
	CCS_VALIDATE(
		_ccs_serialize_bin_ccs_map_data(data, buffer_size, buffer));
	return CCS_RESULT_SUCCESS;
}

static ccs_result_t
_ccs_map_serialize_size(
	ccs_object_t                     object,
	ccs_serialize_format_t           format,
	size_t                          *cum_size,
	_ccs_object_serialize_options_t *opts)
{
	ccs_result_t err = CCS_RESULT_SUCCESS;
	CCS_OBJ_RDLOCK(object);
	switch (format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		*cum_size += _ccs_serialize_bin_size_ccs_map((ccs_map_t)object);
		break;
	default:
		CCS_RAISE_ERR_GOTO(
			err, CCS_RESULT_ERROR_INVALID_VALUE, end,
			"Unsupported serialization format: %d", format);
	}
	CCS_VALIDATE_ERR_GOTO(
		err,
		_ccs_object_serialize_user_data_size(
			object, format, cum_size, opts),
		end);
end:
	CCS_OBJ_UNLOCK(object);
	return err;
}

static ccs_result_t
_ccs_map_serialize(
	ccs_object_t                     object,
	ccs_serialize_format_t           format,
	size_t                          *buffer_size,
	char                           **buffer,
	_ccs_object_serialize_options_t *opts)
{
	ccs_result_t err = CCS_RESULT_SUCCESS;
	CCS_OBJ_RDLOCK(object);
	switch (format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE_ERR_GOTO(
			err,
			_ccs_serialize_bin_ccs_map(
				(ccs_map_t)object, buffer_size, buffer),
			end);
		break;
	default:
		CCS_RAISE_ERR_GOTO(
			err, CCS_RESULT_ERROR_INVALID_VALUE, end,
			"Unsupported serialization format: %d", format);
	}
	CCS_VALIDATE_ERR_GOTO(
		err,
		_ccs_object_serialize_user_data(
			object, format, buffer_size, buffer, opts),
		end);
end:
	CCS_OBJ_UNLOCK(object);
	return err;
}

static _ccs_map_ops_t _ccs_map_ops = {
	{&_ccs_map_del, &_ccs_map_serialize_size, &_ccs_map_serialize}};

ccs_result_t
ccs_create_map(ccs_map_t *map_ret)
{
	CCS_CHECK_PTR(map_ret);
	uintptr_t mem = (uintptr_t)calloc(
		1, sizeof(struct _ccs_map_s) + sizeof(_ccs_map_data_t));
	CCS_REFUTE(!mem, CCS_RESULT_ERROR_OUT_OF_MEMORY);
	ccs_map_t map = (ccs_map_t)mem;
	_ccs_object_init(
		&(map->obj), CCS_OBJECT_TYPE_MAP,
		(_ccs_object_ops_t *)&_ccs_map_ops);
	map->data = (_ccs_map_data_t *)(mem + sizeof(struct _ccs_map_s));
	*map_ret  = map;
	return CCS_RESULT_SUCCESS;
}

#undef uthash_nonfatal_oom
#define uthash_nonfatal_oom(elt)                                               \
	{                                                                      \
		CCS_RAISE_ERR_GOTO(                                            \
			res, CCS_RESULT_ERROR_OUT_OF_MEMORY, err_mem,          \
			"Not enough memory to allocate Hash");                 \
	}

static inline ccs_result_t
_ccs_map_check_datum(ccs_datum_t d, size_t *cum_sz, size_t *sz)
{
	if (d.type == CCS_DATA_TYPE_STRING &&
	    (d.flags & CCS_DATUM_FLAG_TRANSIENT) && d.value.s)
		*cum_sz += *sz = strlen(d.value.s) + 1;
	else if (d.type == CCS_DATA_TYPE_OBJECT && !(d.flags & CCS_DATUM_FLAG_ID))
		CCS_VALIDATE(ccs_retain_object(d.value.o));
	return CCS_RESULT_SUCCESS;
}

static inline void
_ccs_map_set_string(ccs_datum_t *d, size_t sz, uintptr_t *mem)
{
	if (d->type == CCS_DATA_TYPE_STRING) {
		d->flags = CCS_DATUM_FLAG_DEFAULT;
		if (sz) {
			char *p = (char *)*mem;
			memcpy(p, d->value.s, sz);
			d->value.s = p;
			*mem += sz;
		}
	}
}

ccs_result_t
ccs_map_set(ccs_map_t map, ccs_datum_t key, ccs_datum_t value)
{
	CCS_CHECK_OBJ(map, CCS_OBJECT_TYPE_MAP);
	ccs_result_t      res       = CCS_RESULT_SUCCESS;
	_ccs_map_data_t  *d         = map->data;
	size_t            sz        = sizeof(_ccs_map_datum_t);
	size_t            sz1       = 0;
	uintptr_t         mem       = 0;
	size_t            sz2       = 0;
	_ccs_map_datum_t *old_entry = NULL;
	_ccs_map_datum_t *entry     = NULL;
	CCS_OBJ_WRLOCK(map);
	HASH_FIND(hh, d->map, &key, sizeof(ccs_datum_t), old_entry);
	if (old_entry)
		if (!_datum_cmp(&value, &old_entry->value))
			goto end;

	CCS_VALIDATE_ERR_GOTO(res, _ccs_map_check_datum(key, &sz, &sz1), end);
	CCS_VALIDATE_ERR_GOTO(
		res, _ccs_map_check_datum(value, &sz, &sz2), err_o1);

	mem = (uintptr_t)calloc(1, sz);
	CCS_REFUTE_ERR_GOTO(res, !mem, CCS_RESULT_ERROR_OUT_OF_MEMORY, err_o2);
	entry = (_ccs_map_datum_t *)mem;
	mem += sizeof(_ccs_map_datum_t);
	entry->key   = key;
	entry->value = value;
	_ccs_map_set_string(&entry->key, sz1, &mem);
	_ccs_map_set_string(&entry->value, sz2, &mem);

	if (old_entry)
		_ccs_map_remove(d, old_entry);
	HASH_ADD(hh, d->map, key, sizeof(ccs_datum_t), entry);
	goto end;
err_mem:
	free(entry);
err_o2:
	if (value.type == CCS_DATA_TYPE_OBJECT &&
	    !(value.flags & CCS_DATUM_FLAG_ID))
		ccs_release_object(value.value.o);
err_o1:
	if (key.type == CCS_DATA_TYPE_OBJECT &&
	    !(key.flags & CCS_DATUM_FLAG_ID))
		ccs_release_object(key.value.o);
end:
	CCS_OBJ_UNLOCK(map);
	return res;
}

ccs_result_t
ccs_map_exist(ccs_map_t map, ccs_datum_t key, ccs_bool_t *exist)
{
	CCS_CHECK_OBJ(map, CCS_OBJECT_TYPE_MAP);
	CCS_CHECK_PTR(exist);
	_ccs_map_datum_t *entry;
	CCS_OBJ_RDLOCK(map);
	HASH_FIND(hh, map->data->map, &key, sizeof(ccs_datum_t), entry);
	*exist = (entry != NULL) ? CCS_TRUE : CCS_FALSE;
	CCS_OBJ_UNLOCK(map);
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
ccs_map_get(ccs_map_t map, ccs_datum_t key, ccs_datum_t *value_ret)
{
	CCS_CHECK_OBJ(map, CCS_OBJECT_TYPE_MAP);
	CCS_CHECK_PTR(value_ret);
	_ccs_map_datum_t *entry;
	CCS_OBJ_RDLOCK(map);
	HASH_FIND(hh, map->data->map, &key, sizeof(ccs_datum_t), entry);
	if (entry)
		*value_ret = entry->value;
	else
		*value_ret = ccs_none;
	CCS_OBJ_UNLOCK(map);
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
ccs_map_del(ccs_map_t map, ccs_datum_t key)
{
	CCS_CHECK_OBJ(map, CCS_OBJECT_TYPE_MAP);
	_ccs_map_datum_t *entry;
	ccs_result_t      err = CCS_RESULT_SUCCESS;
	CCS_OBJ_WRLOCK(map);
	HASH_FIND(hh, map->data->map, &key, sizeof(ccs_datum_t), entry);
	CCS_REFUTE_ERR_GOTO(err, !entry, CCS_RESULT_ERROR_INVALID_VALUE, end);
	_ccs_map_remove(map->data, entry);
end:
	CCS_OBJ_UNLOCK(map);
	return err;
}

ccs_result_t
ccs_map_get_keys(
	ccs_map_t    map,
	size_t       num_keys,
	ccs_datum_t *keys,
	size_t      *num_keys_ret)
{
	CCS_CHECK_OBJ(map, CCS_OBJECT_TYPE_MAP);
	CCS_CHECK_ARY(num_keys, keys);
	CCS_REFUTE(!keys && !num_keys_ret, CCS_RESULT_ERROR_INVALID_VALUE);
	CCS_OBJ_RDLOCK(map);
	ccs_result_t err         = CCS_RESULT_SUCCESS;
	size_t       num_entries = HASH_COUNT(map->data->map);
	if (keys) {
		CCS_REFUTE_ERR_GOTO(
			err, num_keys < num_entries,
			CCS_RESULT_ERROR_INVALID_VALUE, end);
		_ccs_map_datum_t *current, *tmp;
		size_t            i = 0;
		HASH_ITER(hh, map->data->map, current, tmp)
		{
			keys[i++] = current->value;
		}
		for (i = num_entries; i < num_keys; i++)
			keys[i] = ccs_none;
	}
	if (num_keys_ret)
		*num_keys_ret = num_entries;
end:
	CCS_OBJ_UNLOCK(map);
	return err;
}

ccs_result_t
ccs_map_get_values(
	ccs_map_t    map,
	size_t       num_values,
	ccs_datum_t *values,
	size_t      *num_values_ret)
{
	CCS_CHECK_OBJ(map, CCS_OBJECT_TYPE_MAP);
	CCS_CHECK_ARY(num_values, values);
	CCS_REFUTE(!values && !num_values_ret, CCS_RESULT_ERROR_INVALID_VALUE);
	CCS_OBJ_RDLOCK(map);
	ccs_result_t err         = CCS_RESULT_SUCCESS;
	size_t       num_entries = HASH_COUNT(map->data->map);
	if (values) {
		CCS_REFUTE_ERR_GOTO(
			err, num_values < num_entries,
			CCS_RESULT_ERROR_INVALID_VALUE, end);
		_ccs_map_datum_t *current, *tmp;
		size_t            i = 0;
		HASH_ITER(hh, map->data->map, current, tmp)
		{
			values[i++] = current->value;
		}
		for (i = num_entries; i < num_values; i++)
			values[i] = ccs_none;
	}
	if (num_values_ret)
		*num_values_ret = num_entries;
end:
	CCS_OBJ_UNLOCK(map);
	return err;
}

ccs_result_t
ccs_map_get_pairs(
	ccs_map_t    map,
	size_t       num_pairs,
	ccs_datum_t *keys,
	ccs_datum_t *values,
	size_t      *num_pairs_ret)
{
	CCS_CHECK_OBJ(map, CCS_OBJECT_TYPE_MAP);
	CCS_CHECK_ARY(num_pairs, keys);
	CCS_CHECK_ARY(num_pairs, values);
	CCS_REFUTE(!keys && !num_pairs_ret, CCS_RESULT_ERROR_INVALID_VALUE);
	CCS_OBJ_RDLOCK(map);
	ccs_result_t err         = CCS_RESULT_SUCCESS;
	size_t       num_entries = HASH_COUNT(map->data->map);
	if (keys) {
		CCS_REFUTE_ERR_GOTO(
			err, num_pairs < num_entries,
			CCS_RESULT_ERROR_INVALID_VALUE, end);
		_ccs_map_datum_t *current, *tmp;
		size_t            i = 0;
		HASH_ITER(hh, map->data->map, current, tmp)
		{
			keys[i]     = current->key;
			values[i++] = current->value;
		}
		for (i = num_entries; i < num_pairs; i++) {
			keys[i]   = ccs_none;
			values[i] = ccs_none;
		}
	}
	if (num_pairs_ret)
		*num_pairs_ret = num_entries;
end:
	CCS_OBJ_UNLOCK(map);
	return err;
}

ccs_result_t
ccs_map_clear(ccs_map_t map)
{
	CCS_CHECK_OBJ(map, CCS_OBJECT_TYPE_MAP);
	_ccs_map_datum_t *current, *tmp;
	CCS_OBJ_WRLOCK(map);
	HASH_ITER(hh, map->data->map, current, tmp)
	{
		_ccs_map_remove(map->data, current);
	}
	CCS_OBJ_UNLOCK(map);
	return CCS_RESULT_SUCCESS;
}
