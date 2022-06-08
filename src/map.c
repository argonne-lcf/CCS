#include "cconfigspace_internal.h"
#include "map_internal.h"
#include "datum_uthash.h"
#include "datum_hash.h"
#include <string.h>


struct _ccs_map_datum_s {
	ccs_datum_t key;
	ccs_datum_t value;
	UT_hash_handle hh;
};
typedef struct _ccs_map_datum_s _ccs_map_datum_t;

struct _ccs_map_data_s {
	_ccs_map_datum_t  *map;
};
typedef struct _ccs_map_data_s _ccs_map_data_t;


static inline ccs_result_t
_ccs_map_check_object_and_release(ccs_datum_t d) {
	if (d.type == CCS_OBJECT && !(d.flags & CCS_FLAG_ID))
		return ccs_release_object(d.value.o);
	return CCS_SUCCESS;
}

static inline void
_ccs_map_remove(_ccs_map_data_t  *data,
                _ccs_map_datum_t *entry) {
	HASH_DEL(data->map, entry);
	_ccs_map_check_object_and_release(entry->key);
	_ccs_map_check_object_and_release(entry->value);
	free(entry);
}

static ccs_result_t
_ccs_map_del(ccs_object_t o) {
	ccs_map_t m = (ccs_map_t)o;
	_ccs_map_data_t *data = (_ccs_map_data_t *)(m->data);
	_ccs_map_datum_t *current = NULL, *tmp;
	HASH_ITER(hh, data->map, current, tmp)
		_ccs_map_remove(data, current);
	return CCS_SUCCESS;
}

static inline size_t
_ccs_serialize_bin_size_ccs_map_data(
		_ccs_map_data_t *data) {
	size_t sz = 0;
	_ccs_map_datum_t *current = NULL, *tmp;
	sz += _ccs_serialize_bin_size_uint64(HASH_COUNT(data->map));
	HASH_ITER(hh, data->map, current, tmp) {
		sz += _ccs_serialize_bin_size_ccs_datum(current->key);
		sz += _ccs_serialize_bin_size_ccs_datum(current->value);
	}
	return sz;
}

static inline ccs_result_t
_ccs_serialize_bin_ccs_map_data(
		_ccs_map_data_t  *data,
		size_t           *buffer_size,
		char            **buffer) {
	_ccs_map_datum_t *current = NULL, *tmp;
	CCS_VALIDATE(_ccs_serialize_bin_uint64(
		HASH_COUNT(data->map), buffer_size, buffer));
	HASH_ITER(hh, data->map, current, tmp) {
		CCS_VALIDATE(_ccs_serialize_bin_ccs_datum(
			current->key, buffer_size, buffer));
		CCS_VALIDATE(_ccs_serialize_bin_ccs_datum(
			current->value, buffer_size, buffer));
	}
	return CCS_SUCCESS;
}

static inline size_t
_ccs_serialize_bin_size_ccs_map(ccs_map_t map) {
	_ccs_map_data_t *data = (_ccs_map_data_t *)(map->data);
	return _ccs_serialize_bin_size_ccs_object_internal(
			(_ccs_object_internal_t *)map) +
	       _ccs_serialize_bin_size_ccs_map_data(data);
}

static inline ccs_result_t
_ccs_serialize_bin_ccs_map(
		ccs_map_t   map,
		size_t     *buffer_size,
		char      **buffer) {
	_ccs_map_data_t *data = (_ccs_map_data_t *)(map->data);
	CCS_VALIDATE(_ccs_serialize_bin_ccs_object_internal(
		(_ccs_object_internal_t *)map, buffer_size, buffer));
	CCS_VALIDATE(_ccs_serialize_bin_ccs_map_data(
		data, buffer_size, buffer));
	return CCS_SUCCESS;
}

static ccs_result_t
_ccs_map_serialize_size(
		ccs_object_t            object,
		ccs_serialize_format_t  format,
		size_t                 *cum_size) {
	switch(format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		*cum_size += _ccs_serialize_bin_size_ccs_map((ccs_map_t)object);
		break;
	default:
		return -CCS_INVALID_VALUE;
	}
	return CCS_SUCCESS;
}

static ccs_result_t
_ccs_map_serialize(
		ccs_object_t             object,
		ccs_serialize_format_t   format,
		size_t                  *buffer_size,
		char                   **buffer) {
	switch(format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE(_ccs_serialize_bin_ccs_map(
			(ccs_map_t)object, buffer_size, buffer));
		break;
	default:
		return -CCS_INVALID_VALUE;
	}
	return CCS_SUCCESS;
}

static _ccs_map_ops_t _ccs_map_ops = {
	{ &_ccs_map_del,
	  &_ccs_map_serialize_size,
	  &_ccs_map_serialize }
};

ccs_result_t
ccs_create_map(ccs_map_t *map_ret) {
	CCS_CHECK_PTR(map_ret);
	uintptr_t mem = (uintptr_t)calloc(1, sizeof(struct _ccs_map_s) + sizeof(_ccs_map_data_t));
	if (!mem)
		return -CCS_OUT_OF_MEMORY;
	ccs_map_t map = (ccs_map_t)mem;
	_ccs_object_init(&(map->obj), CCS_MAP, (_ccs_object_ops_t *)&_ccs_map_ops);
	map->data = (_ccs_map_data_t *)(mem + sizeof(struct _ccs_map_s));
	*map_ret = map;
	return CCS_SUCCESS;
}

#undef uthash_nonfatal_oom
#define uthash_nonfatal_oom(elt) { \
	res = -CCS_OUT_OF_MEMORY;  \
	goto err_mem;              \
}

static inline ccs_result_t
_ccs_map_check_datum(ccs_datum_t d, size_t *cum_sz, size_t *sz) {
	if (d.type == CCS_STRING && (d.flags & CCS_FLAG_TRANSIENT) && d.value.s)
		*cum_sz += *sz = strlen(d.value.s) + 1;
	else if (d.type == CCS_OBJECT && !(d.flags & CCS_FLAG_ID))
		return ccs_retain_object(d.value.o);
	return CCS_SUCCESS;
}

static inline void
_ccs_map_set_string(ccs_datum_t *d, size_t sz, uintptr_t *mem) {
	if (d->type == CCS_STRING) {
		d->flags = CCS_FLAG_DEFAULT;
		if (sz) {
			char *p = (char *)*mem;
			memcpy(p, d->value.s, sz);
			d->value.s = p;
			*mem += sz;
		}
	}
}

ccs_result_t
ccs_map_set(ccs_map_t   map,
            ccs_datum_t key,
            ccs_datum_t value) {
	CCS_CHECK_OBJ(map, CCS_MAP);
	ccs_result_t res = CCS_SUCCESS;
	_ccs_map_data_t *d = map->data;
	size_t sz = sizeof(_ccs_map_datum_t);
	size_t sz1 = 0;
	uintptr_t mem = 0;
	size_t sz2 = 0;
	_ccs_map_datum_t *old_entry = NULL;
	_ccs_map_datum_t *entry = NULL;
	HASH_FIND(hh, d->map, &key, sizeof(ccs_datum_t), old_entry);
	if (old_entry)
		if (!_datum_cmp(&value, &old_entry->value))
			return CCS_SUCCESS;

	CCS_VALIDATE_ERR_GOTO(res, _ccs_map_check_datum(key, &sz, &sz1), err);
	CCS_VALIDATE_ERR_GOTO(res, _ccs_map_check_datum(value, &sz, &sz2), err_o1);

	mem = (uintptr_t)calloc(1, sz);
	if (!mem) {
		res = -CCS_OUT_OF_MEMORY;
		goto err_o2;
	}
	entry = (_ccs_map_datum_t *)mem;
	mem += sizeof(_ccs_map_datum_t);
	entry->key = key;
	entry->value = value;
	_ccs_map_set_string(&entry->key, sz1, &mem);
	_ccs_map_set_string(&entry->value, sz2, &mem);

	if (old_entry)
		_ccs_map_remove(d, old_entry);
	HASH_ADD(hh, d->map, key, sizeof(ccs_datum_t), entry);
	return CCS_SUCCESS;
err_mem:
	free(entry);
err_o2:
	if (value.type == CCS_OBJECT && !(value.flags & CCS_FLAG_ID))
		ccs_release_object(value.value.o);
err_o1:
	if (key.type == CCS_OBJECT && !(key.flags & CCS_FLAG_ID))
		ccs_release_object(key.value.o);
err:
	return res;
}

ccs_result_t
ccs_map_exist(ccs_map_t    map,
              ccs_datum_t  key,
              ccs_bool_t  *exist) {
	CCS_CHECK_OBJ(map, CCS_MAP);
	CCS_CHECK_PTR(exist);
	_ccs_map_datum_t *entry;
	HASH_FIND(hh, map->data->map, &key, sizeof(ccs_datum_t), entry);
	*exist = (entry != NULL);
	return CCS_SUCCESS;
}

ccs_result_t
ccs_map_get(ccs_map_t    map,
            ccs_datum_t  key,
            ccs_datum_t *value_ret) {
	CCS_CHECK_OBJ(map, CCS_MAP);
	CCS_CHECK_PTR(value_ret);
	_ccs_map_datum_t *entry;
	HASH_FIND(hh, map->data->map, &key, sizeof(ccs_datum_t), entry);
	if (entry)
		*value_ret = entry->value;
	else
		*value_ret = ccs_none;
	return CCS_SUCCESS;
}

ccs_result_t
ccs_map_del(ccs_map_t    map,
            ccs_datum_t  key) {
	CCS_CHECK_OBJ(map, CCS_MAP);
	_ccs_map_datum_t *entry;
	HASH_FIND(hh, map->data->map, &key, sizeof(ccs_datum_t), entry);
	if (entry)
		_ccs_map_remove(map->data, entry);
	else
		return -CCS_INVALID_VALUE;
	return CCS_SUCCESS;
}

ccs_result_t
ccs_map_get_keys(ccs_map_t    map,
                 size_t       num_keys,
                 ccs_datum_t *keys,
                 size_t      *num_keys_ret) {
	CCS_CHECK_OBJ(map, CCS_MAP);
	CCS_CHECK_ARY(num_keys, keys);
	if (CCS_UNLIKELY(!keys && !num_keys_ret))
		return -CCS_INVALID_VALUE;
	size_t count = HASH_COUNT(map->data->map);
	if (keys) {
		if (num_keys < count)
			return -CCS_INVALID_VALUE;
		_ccs_map_datum_t *current = NULL, *tmp;
		size_t i = 0;
		HASH_ITER(hh, map->data->map, current, tmp)
			keys[i++] = current->value;
		for (i = count; i <num_keys; i++)
			keys[i] = ccs_none;
	}
	if (num_keys_ret)
		*num_keys_ret = count;
	return CCS_SUCCESS;
}

ccs_result_t
ccs_map_get_values(ccs_map_t    map,
                   size_t       num_values,
                   ccs_datum_t *values,
                   size_t      *num_values_ret) {
	CCS_CHECK_OBJ(map, CCS_MAP);
	CCS_CHECK_ARY(num_values, values);
	if (CCS_UNLIKELY(!values && !num_values_ret))
		return -CCS_INVALID_VALUE;
	size_t count = HASH_COUNT(map->data->map);
	if (values) {
		if (num_values < count)
			return -CCS_INVALID_VALUE;
		_ccs_map_datum_t *current = NULL, *tmp;
		size_t i = 0;
		HASH_ITER(hh, map->data->map, current, tmp)
			values[i++] = current->value;
		for (i = count; i < num_values; i++)
			values[i] = ccs_none;
	}
	if (num_values_ret)
		*num_values_ret = count;
	return CCS_SUCCESS;
}

ccs_result_t
ccs_map_get_pairs(ccs_map_t    map,
                  size_t       num_pairs,
                  ccs_datum_t *keys,
                  ccs_datum_t *values,
                  size_t      *num_pairs_ret) {
	CCS_CHECK_OBJ(map, CCS_MAP);
	CCS_CHECK_ARY(num_pairs, keys);
	CCS_CHECK_ARY(num_pairs, values);
	if (CCS_UNLIKELY(!keys && !num_pairs_ret))
		return -CCS_INVALID_VALUE;
	if (CCS_UNLIKELY(keys && !values))
		return -CCS_INVALID_VALUE;
	size_t count = HASH_COUNT(map->data->map);
	if (keys) {
		if (num_pairs < count)
			return -CCS_INVALID_VALUE;
		_ccs_map_datum_t *current = NULL, *tmp;
		size_t i = 0;
		HASH_ITER(hh, map->data->map, current, tmp) {
			keys[i] = current->key;
			values[i++] = current->value;
		}
		for (i = count; i < num_pairs; i++) {
			keys[i] =  ccs_none;
			values[i] = ccs_none;
		}
	}
	if (num_pairs_ret)
		*num_pairs_ret = count;
	return CCS_SUCCESS;
}

ccs_result_t
ccs_map_clear(ccs_map_t    map) {
	CCS_CHECK_OBJ(map, CCS_MAP);
	_ccs_map_datum_t *current = NULL, *tmp;
	HASH_ITER(hh, map->data->map, current, tmp)
		_ccs_map_remove(map->data, current);
	return CCS_SUCCESS;
}
