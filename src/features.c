#include "cconfigspace_internal.h"
#include "features_internal.h"
#include "feature_space_internal.h"
#include "datum_hash.h"
#include <string.h>

static ccs_result_t
_ccs_features_del(ccs_object_t object)
{
	ccs_features_t features = (ccs_features_t)object;
	ccs_release_object(features->data->feature_space);
	return CCS_RESULT_SUCCESS;
}

static ccs_result_t
_ccs_features_serialize_size(
	ccs_object_t                     object,
	ccs_serialize_format_t           format,
	size_t                          *cum_size,
	_ccs_object_serialize_options_t *opts)
{
	switch (format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE(_ccs_serialize_bin_size_ccs_binding(
			(ccs_binding_t)object, cum_size));
		break;
	default:
		CCS_RAISE(
			CCS_RESULT_ERROR_INVALID_VALUE,
			"Unsupported serialization format: %d", format);
	}
	CCS_VALIDATE(_ccs_object_serialize_user_data_size(
		object, format, cum_size, opts));
	return CCS_RESULT_SUCCESS;
}

static ccs_result_t
_ccs_features_serialize(
	ccs_object_t                     object,
	ccs_serialize_format_t           format,
	size_t                          *buffer_size,
	char                           **buffer,
	_ccs_object_serialize_options_t *opts)
{
	switch (format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE(_ccs_serialize_bin_ccs_binding(
			(ccs_binding_t)object, buffer_size, buffer));
		break;
	default:
		CCS_RAISE(
			CCS_RESULT_ERROR_INVALID_VALUE,
			"Unsupported serialization format: %d", format);
	}
	CCS_VALIDATE(_ccs_object_serialize_user_data(
		object, format, buffer_size, buffer, opts));
	return CCS_RESULT_SUCCESS;
}

static ccs_result_t
_ccs_features_hash(ccs_features_t features, ccs_hash_t *hash_ret)
{
	CCS_VALIDATE(_ccs_binding_hash((ccs_binding_t)features, hash_ret));
	return CCS_RESULT_SUCCESS;
}

static ccs_result_t
_ccs_features_cmp(ccs_features_t features, ccs_features_t other, int *cmp_ret)
{
	CCS_VALIDATE(_ccs_binding_cmp(
		(ccs_binding_t)features, (ccs_binding_t)other, cmp_ret));
	return CCS_RESULT_SUCCESS;
}

static _ccs_features_ops_t _features_ops = {
	{&_ccs_features_del, &_ccs_features_serialize_size,
	 &_ccs_features_serialize},
	&_ccs_features_hash,
	&_ccs_features_cmp};

ccs_result_t
ccs_create_features(
	ccs_feature_space_t feature_space,
	size_t              num_values,
	ccs_datum_t        *values,
	ccs_features_t     *features_ret)
{
	CCS_CHECK_OBJ(feature_space, CCS_OBJECT_TYPE_FEATURE_SPACE);
	CCS_CHECK_PTR(features_ret);
	CCS_CHECK_ARY(num_values, values);
	ccs_result_t err;
	size_t       num_parameters = feature_space->data->num_parameters;
	CCS_REFUTE(
		values && num_parameters != num_values,
		CCS_RESULT_ERROR_INVALID_VALUE);
	uintptr_t mem = (uintptr_t)calloc(
		1, sizeof(struct _ccs_features_s) +
			   sizeof(struct _ccs_features_data_s) +
			   num_parameters * sizeof(ccs_datum_t));
	CCS_REFUTE(!mem, CCS_RESULT_ERROR_OUT_OF_MEMORY);
	CCS_VALIDATE_ERR_GOTO(err, ccs_retain_object(feature_space), errmem);
	ccs_features_t feat;
	feat = (ccs_features_t)mem;
	_ccs_object_init(
		&(feat->obj), CCS_OBJECT_TYPE_FEATURES,
		(_ccs_object_ops_t *)&_features_ops);
	feat->data                = (struct _ccs_features_data_s
                              *)(mem + sizeof(struct _ccs_features_s));
	feat->data->num_values    = num_parameters;
	feat->data->feature_space = feature_space;
	feat->data->values =
		(ccs_datum_t
			 *)(mem + sizeof(struct _ccs_features_s) + sizeof(struct _ccs_features_data_s));
	if (values) {
		memcpy(feat->data->values, values,
		       num_parameters * sizeof(ccs_datum_t));
		for (size_t i = 0; i < num_values; i++)
			if (values[i].flags & CCS_DATUM_FLAG_TRANSIENT)
				CCS_VALIDATE_ERR_GOTO(
					err,
					ccs_context_validate_value(
						(ccs_context_t)feature_space, i,
						values[i],
						feat->data->values + i),
					errfs);
	}
	*features_ret = feat;
	return CCS_RESULT_SUCCESS;
errfs:
	_ccs_object_deinit(&(feat->obj));
	ccs_release_object(feature_space);
errmem:
	free((void *)mem);
	return err;
}

ccs_result_t
ccs_features_get_feature_space(
	ccs_features_t       features,
	ccs_feature_space_t *feature_space_ret)
{
	CCS_CHECK_OBJ(features, CCS_OBJECT_TYPE_FEATURES);
	CCS_VALIDATE(_ccs_binding_get_context(
		(ccs_binding_t)features, (ccs_context_t *)feature_space_ret));
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
ccs_features_check(ccs_features_t features, ccs_bool_t *is_valid_ret)
{
	CCS_CHECK_OBJ(features, CCS_OBJECT_TYPE_FEATURES);
	CCS_VALIDATE(ccs_feature_space_check_features(
		features->data->feature_space, features, is_valid_ret));
	return CCS_RESULT_SUCCESS;
}
