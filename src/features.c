#include "cconfigspace_internal.h"
#include "features_internal.h"
#include "datum_hash.h"
#include <string.h>

static inline _ccs_features_ops_t *
ccs_features_get_ops(ccs_features_t features) {
	return (_ccs_features_ops_t *)features->obj.ops;
}

static ccs_result_t
_ccs_features_del(ccs_object_t object) {
	ccs_features_t features = (ccs_features_t)object;
	ccs_release_object(features->data->features_space);
	return CCS_SUCCESS;
}

static _ccs_features_ops_t _features_ops =
    { {&_ccs_features_del} };

ccs_result_t
ccs_create_features(ccs_features_space_t  features_space,
                    size_t                num_values,
                    ccs_datum_t          *values,
                    void                 *user_data,
                    ccs_features_t       *features_ret) {
	CCS_CHECK_OBJ(features_space, CCS_FEATURES_SPACE);
	CCS_CHECK_PTR(features_ret);
	CCS_CHECK_ARY(num_values, values);
	ccs_result_t err;
	size_t num;
	err = ccs_features_space_get_num_hyperparameters(features_space, &num);
	if (err)
		return err;
	if (values && num != num_values)
		return -CCS_INVALID_VALUE;
	uintptr_t mem = (uintptr_t)calloc(1, sizeof(struct _ccs_features_s) +
	                                     sizeof(struct _ccs_features_data_s) +
	                                     num * sizeof(ccs_datum_t));
	if (!mem)
		return -CCS_OUT_OF_MEMORY;
	err = ccs_retain_object(features_space);
	if (err) {
		free((void*)mem);
		return err;
	}
	ccs_features_t config = (ccs_features_t)mem;
	_ccs_object_init(&(config->obj), CCS_FEATURES, (_ccs_object_ops_t*)&_features_ops);
	config->data = (struct _ccs_features_data_s*)(mem + sizeof(struct _ccs_features_s));
	config->data->user_data = user_data;
	config->data->num_values = num;
	config->data->features_space = features_space;
	config->data->values = (ccs_datum_t *)(mem + sizeof(struct _ccs_features_s) + sizeof(struct _ccs_features_data_s));
	if (values)
		memcpy(config->data->values, values, num*sizeof(ccs_datum_t));
	*features_ret = config;
	return CCS_SUCCESS;
}

ccs_result_t
ccs_features_get_features_space(ccs_features_t        features,
                                ccs_features_space_t *features_space_ret) {
	CCS_CHECK_OBJ(features, CCS_FEATURES);
	CCS_CHECK_PTR(features_space_ret);
	*features_space_ret = features->data->features_space;
	return CCS_SUCCESS;
}

ccs_result_t
ccs_features_get_user_data(ccs_features_t   features,
                           void           **user_data_ret) {
	CCS_CHECK_OBJ(features, CCS_FEATURES);
	CCS_CHECK_PTR(user_data_ret);
	*user_data_ret = features->data->user_data;
	return CCS_SUCCESS;
}

ccs_result_t
ccs_features_get_value(ccs_features_t  features,
                       size_t          index,
                       ccs_datum_t    *value_ret) {
	CCS_CHECK_OBJ(features, CCS_FEATURES);
	CCS_CHECK_PTR(value_ret);
	if (index >= features->data->num_values)
		return -CCS_OUT_OF_BOUNDS;
	*value_ret = features->data->values[index];
	return CCS_SUCCESS;
}

ccs_result_t
ccs_features_set_value(ccs_features_t features,
                       size_t         index,
                       ccs_datum_t    value) {
	CCS_CHECK_OBJ(features, CCS_FEATURES);
	if (index >= features->data->num_values)
		return -CCS_OUT_OF_BOUNDS;
	features->data->values[index] = value;
	return CCS_SUCCESS;
}

ccs_result_t
ccs_features_get_values(ccs_features_t  features,
                        size_t          num_values,
                        ccs_datum_t    *values,
                        size_t         *num_values_ret) {
	CCS_CHECK_OBJ(features, CCS_FEATURES);
	CCS_CHECK_ARY(num_values, values);
	if (!num_values_ret && !values)
		return -CCS_INVALID_VALUE;
	size_t num = features->data->num_values;
	if (values) {
		if (num_values < num)
			return -CCS_INVALID_VALUE;
		memcpy(values, features->data->values, num*sizeof(ccs_datum_t));
		for (size_t i = num; i < num_values; i++) {
			values[i].type = CCS_NONE;
			values[i].value.i = 0;
		}
	}
	if (num_values_ret)
		*num_values_ret = num;
	return CCS_SUCCESS;
}

ccs_result_t
ccs_features_get_value_by_name(ccs_features_t  features,
                               const char     *name,
                               ccs_datum_t    *value_ret) {
	CCS_CHECK_OBJ(features, CCS_FEATURES);
	CCS_CHECK_PTR(name);
	size_t index;
	ccs_result_t err;
	err = ccs_features_space_get_hyperparameter_index_by_name(
		features->data->features_space, name, &index);
	if (err)
		return err;
	*value_ret = features->data->values[index];
	return CCS_SUCCESS;
}

ccs_result_t
ccs_features_check(ccs_features_t features) {
	CCS_CHECK_OBJ(features, CCS_FEATURES);
	return ccs_features_space_check_features(
		features->data->features_space, features);
}

ccs_result_t
ccs_features_hash(ccs_features_t  features,
                  ccs_hash_t     *hash_ret) {
	CCS_CHECK_OBJ(features, CCS_FEATURES);
	CCS_CHECK_PTR(hash_ret);
	_ccs_features_data_t *data = features->data;
	ccs_hash_t h, ht;
	HASH_JEN(&(data->features_space), sizeof(data->features_space), h);
	HASH_JEN(&(data->num_values), sizeof(data->num_values), ht);
	h = _hash_combine(h, ht);
	for (size_t i = 0; i < data->num_values; i++) {
		ht = _hash_datum(data->values + i);
		h = _hash_combine(h, ht);
	}
	*hash_ret = h;
	return CCS_SUCCESS;
}

ccs_result_t
ccs_features_cmp(ccs_features_t  features,
                 ccs_features_t  other_features,
                 int            *cmp_ret) {
	CCS_CHECK_OBJ(features, CCS_FEATURES);
	CCS_CHECK_OBJ(other_features, CCS_FEATURES);
	CCS_CHECK_PTR(cmp_ret);
	if (features == other_features) {
		*cmp_ret = 0;
		return CCS_SUCCESS;
	}
	_ccs_features_data_t *data = features->data;
	_ccs_features_data_t *other_data = other_features->data;
	*cmp_ret = data->features_space < other_data->features_space ? -1 :
	           data->features_space > other_data->features_space ?  1 : 0;
	if (*cmp_ret)
		return CCS_SUCCESS;
	*cmp_ret = data->num_values < other_data->num_values ? -1 :
	           data->num_values > other_data->num_values ?  1 : 0;
	if (*cmp_ret)
		return CCS_SUCCESS;
	for (size_t i = 0; i < data->num_values; i++) {
		if ( (*cmp_ret = _datum_cmp(data->values + i, other_data->values + i)) )
			return CCS_SUCCESS;
	}
	return CCS_SUCCESS;
}

