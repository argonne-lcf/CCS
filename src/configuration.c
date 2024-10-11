#include "cconfigspace_internal.h"
#include "configuration_internal.h"
#include "configuration_space_internal.h"
#include "features_internal.h"
#include <string.h>

static ccs_result_t
_ccs_configuration_del(ccs_object_t object)
{
	ccs_configuration_t configuration = (ccs_configuration_t)object;
	if (configuration->data->configuration_space)
		ccs_release_object(configuration->data->configuration_space);
	if (configuration->data->features)
		ccs_release_object(configuration->data->features);
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_serialize_bin_size_ccs_configuration_data(
	_ccs_configuration_data_t       *data,
	size_t                          *cum_size,
	_ccs_object_serialize_options_t *opts)
{
	*cum_size += _ccs_serialize_bin_size_ccs_binding_data(
		(_ccs_binding_data_t *)data);
	*cum_size += _ccs_serialize_bin_size_ccs_bool(data->features != NULL);
	if (data->features)
		CCS_VALIDATE(_ccs_object_serialize_size_with_opts(
			data->features, CCS_SERIALIZE_FORMAT_BINARY, cum_size,
			opts));
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_serialize_bin_ccs_configuration_data(
	_ccs_configuration_data_t       *data,
	size_t                          *buffer_size,
	char                           **buffer,
	_ccs_object_serialize_options_t *opts)
{
	CCS_VALIDATE(_ccs_serialize_bin_ccs_binding_data(
		(_ccs_binding_data_t *)data, buffer_size, buffer));
	CCS_VALIDATE(_ccs_serialize_bin_ccs_bool(
		data->features != NULL, buffer_size, buffer));
	if (data->features)
		CCS_VALIDATE(_ccs_object_serialize_with_opts(
			data->features, CCS_SERIALIZE_FORMAT_BINARY,
			buffer_size, buffer, opts));
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_serialize_bin_size_ccs_configuration(
	ccs_configuration_t              configuration,
	size_t                          *cum_size,
	_ccs_object_serialize_options_t *opts)
{
	CCS_VALIDATE(_ccs_serialize_bin_size_ccs_configuration_data(
		configuration->data, cum_size, opts));
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_serialize_bin_ccs_configuration(
	ccs_configuration_t              configuration,
	size_t                          *buffer_size,
	char                           **buffer,
	_ccs_object_serialize_options_t *opts)
{
	CCS_VALIDATE(_ccs_serialize_bin_ccs_configuration_data(
		configuration->data, buffer_size, buffer, opts));
	return CCS_RESULT_SUCCESS;
}

static ccs_result_t
_ccs_configuration_serialize_size(
	ccs_object_t                     object,
	ccs_serialize_format_t           format,
	size_t                          *cum_size,
	_ccs_object_serialize_options_t *opts)
{
	switch (format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE(_ccs_serialize_bin_size_ccs_configuration(
			(ccs_configuration_t)object, cum_size, opts));
		break;
	default:
		CCS_RAISE(
			CCS_RESULT_ERROR_INVALID_VALUE,
			"Unsupported serialization format: %d", format);
	}
	return CCS_RESULT_SUCCESS;
}

static ccs_result_t
_ccs_configuration_serialize(
	ccs_object_t                     object,
	ccs_serialize_format_t           format,
	size_t                          *buffer_size,
	char                           **buffer,
	_ccs_object_serialize_options_t *opts)
{
	switch (format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE(_ccs_serialize_bin_ccs_configuration(
			(ccs_configuration_t)object, buffer_size, buffer,
			opts));
		break;
	default:
		CCS_RAISE(
			CCS_RESULT_ERROR_INVALID_VALUE,
			"Unsupported serialization format: %d", format);
	}
	return CCS_RESULT_SUCCESS;
}

static ccs_result_t
_ccs_configuration_hash(ccs_configuration_t configuration, ccs_hash_t *hash_ret)
{
	ccs_hash_t h, ht;
	CCS_VALIDATE(_ccs_binding_hash(
		(ccs_binding_t)configuration->data->features, &h));
	CCS_VALIDATE(_ccs_binding_hash((ccs_binding_t)configuration, &ht));
	h         = _hash_combine(h, ht);
	*hash_ret = h;
	return CCS_RESULT_SUCCESS;
}

static ccs_result_t
_ccs_configuration_cmp(
	ccs_configuration_t configuration,
	ccs_configuration_t other,
	int                *cmp_ret)
{
	if (configuration->data->features) {
		CCS_VALIDATE(_ccs_binding_cmp(
			(ccs_binding_t)configuration->data->features,
			(ccs_binding_t)other->data->features, cmp_ret));
		if (*cmp_ret)
			return CCS_RESULT_SUCCESS;
	}
	CCS_VALIDATE(_ccs_binding_cmp(
		(ccs_binding_t)configuration, (ccs_binding_t)other, cmp_ret));
	return CCS_RESULT_SUCCESS;
}

static _ccs_configuration_ops_t _configuration_ops = {
	{&_ccs_configuration_del, &_ccs_configuration_serialize_size,
	 &_ccs_configuration_serialize},
	&_ccs_configuration_hash,
	&_ccs_configuration_cmp};

ccs_result_t
_ccs_create_configuration(
	ccs_configuration_space_t configuration_space,
	ccs_features_t            features,
	size_t                    num_values,
	ccs_datum_t              *values,
	ccs_configuration_t      *configuration_ret)
{
	ccs_result_t err;
	size_t       num_parameters = configuration_space->data->num_parameters;
	uintptr_t    mem            = (uintptr_t)calloc(
                1, sizeof(struct _ccs_configuration_s) +
                           sizeof(struct _ccs_configuration_data_s) +
                           num_parameters * sizeof(ccs_datum_t));
	CCS_REFUTE(!mem, CCS_RESULT_ERROR_OUT_OF_MEMORY);
	uintptr_t           mem_orig = mem;
	ccs_configuration_t config;
	config = (ccs_configuration_t)mem;
	mem += sizeof(struct _ccs_configuration_s);
	_ccs_object_init(
		&(config->obj), CCS_OBJECT_TYPE_CONFIGURATION,
		(_ccs_object_ops_t *)&_configuration_ops);
	config->data = (struct _ccs_configuration_data_s *)mem;
	mem += sizeof(struct _ccs_configuration_data_s);
	config->data->num_values = num_parameters;
	config->data->values     = (ccs_datum_t *)mem;
	mem += sizeof(ccs_datum_t) * num_parameters;
	CCS_VALIDATE_ERR_GOTO(
		err, ccs_retain_object(configuration_space), errinit);
	config->data->configuration_space = configuration_space;
	config->data->bindings[0]         = (ccs_binding_t)config;
	config->data->num_bindings        = 1;
	if (features)
		CCS_VALIDATE_ERR_GOTO(
			err, ccs_retain_object(features), errinit);
	else if (configuration_space->data->feature_space)
		CCS_VALIDATE_ERR_GOTO(
			err,
			ccs_feature_space_get_default_features(
				configuration_space->data->feature_space,
				&features),
			errinit);
	config->data->features = features;
	if (features) {
		config->data->bindings[config->data->num_bindings] =
			(ccs_binding_t)features;
		config->data->num_bindings++;
	}
	if (values) {
		ccs_bool_t is_valid;
		for (size_t i = 0; i < num_values; i++)
			CCS_VALIDATE_ERR_GOTO(
				err,
				ccs_context_validate_value(
					(ccs_context_t)configuration_space, i,
					values[i], config->data->values + i),
				errinit);
		CCS_VALIDATE_ERR_GOTO(
			err,
			_check_configuration(
				configuration_space, config, &is_valid),
			errinit);
		CCS_REFUTE_ERR_GOTO(
			err, !is_valid, CCS_RESULT_ERROR_INVALID_VALUE,
			errinit);
	}
	*configuration_ret = config;
	return CCS_RESULT_SUCCESS;
errinit:
	_ccs_configuration_del(config);
	_ccs_object_deinit(&(config->obj));
	free((void *)mem_orig);
	return err;
}

ccs_result_t
ccs_create_configuration(
	ccs_configuration_space_t configuration_space,
	ccs_features_t            features,
	size_t                    num_values,
	ccs_datum_t              *values,
	ccs_configuration_t      *configuration_ret)
{
	CCS_CHECK_OBJ(configuration_space, CCS_OBJECT_TYPE_CONFIGURATION_SPACE);
	if (features) {
		CCS_CHECK_OBJ(features, CCS_OBJECT_TYPE_FEATURES);
		CCS_REFUTE(
			features->data->feature_space !=
				configuration_space->data->feature_space,
			CCS_RESULT_ERROR_INVALID_FEATURES);
	}
	CCS_CHECK_PTR(configuration_ret);
	CCS_CHECK_ARY(num_values, values);
	size_t num_parameters = configuration_space->data->num_parameters;
	CCS_REFUTE(
		num_parameters != num_values, CCS_RESULT_ERROR_INVALID_VALUE);
	CCS_VALIDATE(_ccs_create_configuration(
		configuration_space, features, num_values, values,
		configuration_ret));
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
ccs_configuration_get_configuration_space(
	ccs_configuration_t        configuration,
	ccs_configuration_space_t *configuration_space_ret)
{
	CCS_CHECK_OBJ(configuration, CCS_OBJECT_TYPE_CONFIGURATION);
	CCS_CHECK_PTR(configuration_space_ret);
	*configuration_space_ret = configuration->data->configuration_space;
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
ccs_configuration_get_features(
	ccs_configuration_t configuration,
	ccs_features_t     *features_ret)
{
	CCS_CHECK_OBJ(configuration, CCS_OBJECT_TYPE_CONFIGURATION);
	CCS_CHECK_PTR(features_ret);
	*features_ret = configuration->data->features;
	return CCS_RESULT_SUCCESS;
}
