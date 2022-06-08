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

static ccs_result_t
_ccs_features_serialize_size(
		ccs_object_t            object,
		ccs_serialize_format_t  format,
		size_t                 *cum_size) {
	switch(format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE(_ccs_serialize_bin_size_ccs_binding(
			(ccs_binding_t)object, cum_size));
		break;
	default:
		return -CCS_INVALID_VALUE;
	}
	return CCS_SUCCESS;
}

static ccs_result_t
_ccs_features_serialize(
		ccs_object_t             object,
		ccs_serialize_format_t   format,
		size_t                  *buffer_size,
		char                   **buffer) {
	switch(format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE(_ccs_serialize_bin_ccs_binding(
			(ccs_binding_t)object, buffer_size, buffer));
		break;
	default:
		return -CCS_INVALID_VALUE;
	}
	return CCS_SUCCESS;
}

static ccs_result_t
_ccs_features_hash(_ccs_features_data_t *data,
                   ccs_hash_t           *hash_ret) {
	return _ccs_binding_hash((_ccs_binding_data_t *)data, hash_ret);
}

static ccs_result_t
_ccs_features_cmp(_ccs_features_data_t *data,
                  ccs_features_t        other,
                  int                       *cmp_ret) {
	return _ccs_binding_cmp((_ccs_binding_data_t *)data, (ccs_binding_t)other, cmp_ret);
}

static _ccs_features_ops_t _features_ops =
    { { &_ccs_features_del,
        &_ccs_features_serialize_size,
        &_ccs_features_serialize },
      &_ccs_features_hash,
      &_ccs_features_cmp };

ccs_result_t
ccs_create_features(ccs_features_space_t  features_space,
                    size_t                num_values,
                    ccs_datum_t          *values,
                    ccs_features_t       *features_ret) {
	CCS_CHECK_OBJ(features_space, CCS_FEATURES_SPACE);
	CCS_CHECK_PTR(features_ret);
	CCS_CHECK_ARY(num_values, values);
	ccs_result_t err;
	size_t num;
	CCS_VALIDATE(ccs_features_space_get_num_hyperparameters(features_space, &num));
	if (values && num != num_values)
		return -CCS_INVALID_VALUE;
	uintptr_t mem = (uintptr_t)calloc(1, sizeof(struct _ccs_features_s) +
	                                     sizeof(struct _ccs_features_data_s) +
	                                     num * sizeof(ccs_datum_t));
	if (!mem)
		return -CCS_OUT_OF_MEMORY;
	CCS_VALIDATE_ERR_GOTO(err, ccs_retain_object(features_space), errmem);
	ccs_features_t feat;
	feat = (ccs_features_t)mem;
	_ccs_object_init(&(feat->obj), CCS_FEATURES, (_ccs_object_ops_t*)&_features_ops);
	feat->data = (struct _ccs_features_data_s*)(mem + sizeof(struct _ccs_features_s));
	feat->data->num_values = num;
	feat->data->features_space = features_space;
	feat->data->values = (ccs_datum_t *)(mem + sizeof(struct _ccs_features_s) + sizeof(struct _ccs_features_data_s));
	if (values) {
		memcpy(feat->data->values, values, num*sizeof(ccs_datum_t));
		for (size_t i = 0; i < num_values; i++)
			if (values[i].flags & CCS_FLAG_TRANSIENT)
				CCS_VALIDATE_ERR_GOTO(err, ccs_features_space_validate_value(
					features_space, i, values[i], feat->data->values + i), errfs);
	}
	*features_ret = feat;
	return CCS_SUCCESS;
errfs:
	ccs_release_object(features_space);
errmem:
	free((void*)mem);
	return err;
}

ccs_result_t
ccs_features_get_features_space(ccs_features_t        features,
                                ccs_features_space_t *features_space_ret) {
	CCS_CHECK_OBJ(features, CCS_FEATURES);
	return _ccs_binding_get_context(
		(ccs_binding_t)features, (ccs_context_t *)features_space_ret);
}

ccs_result_t
ccs_features_get_value(ccs_features_t  features,
                       size_t          index,
                       ccs_datum_t    *value_ret) {
	CCS_CHECK_OBJ(features, CCS_FEATURES);
	return _ccs_binding_get_value(
		(ccs_binding_t)features, index, value_ret);
}

ccs_result_t
ccs_features_set_value(ccs_features_t features,
                       size_t         index,
                       ccs_datum_t    value) {
	CCS_CHECK_OBJ(features, CCS_FEATURES);
	return _ccs_binding_set_value(
		(ccs_binding_t)features, index, value);
}

ccs_result_t
ccs_features_get_values(ccs_features_t  features,
                        size_t          num_values,
                        ccs_datum_t    *values,
                        size_t         *num_values_ret) {
	CCS_CHECK_OBJ(features, CCS_FEATURES);
	return _ccs_binding_get_values(
		(ccs_binding_t)features, num_values, values, num_values_ret);
}

ccs_result_t
ccs_features_get_value_by_name(ccs_features_t  features,
                               const char     *name,
                               ccs_datum_t    *value_ret) {
	CCS_CHECK_OBJ(features, CCS_FEATURES);
	return _ccs_binding_get_value_by_name(
		(ccs_binding_t)features, name, value_ret);
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
	_ccs_features_ops_t *ops = ccs_features_get_ops(features);
	return ops->hash(features->data, hash_ret);
}

ccs_result_t
ccs_features_cmp(ccs_features_t  features,
                 ccs_features_t  other_features,
                 int            *cmp_ret) {
	CCS_CHECK_OBJ(features, CCS_FEATURES);
	CCS_CHECK_OBJ(other_features, CCS_FEATURES);
	_ccs_features_ops_t *ops = ccs_features_get_ops(features);
	return ops->cmp(features->data, other_features, cmp_ret);
}

