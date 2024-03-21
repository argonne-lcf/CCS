#include "cconfigspace_internal.h"
#include "configuration_internal.h"
#include "configuration_space_internal.h"
#include <string.h>

static inline _ccs_configuration_ops_t *
ccs_configuration_get_ops(ccs_configuration_t configuration)
{
	return (_ccs_configuration_ops_t *)configuration->obj.ops;
}

static ccs_result_t
_ccs_configuration_del(ccs_object_t object)
{
	ccs_configuration_t configuration = (ccs_configuration_t)object;
	_ccs_configuration_data_t *data = configuration->data;
	if (data->configuration_space)
		ccs_release_object(data->configuration_space);
	for (size_t i = 0; i < data->num_bindings; i++)
		if (data->bindings[i])
			ccs_release_object(data->bindings[i]);
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_serialize_bin_size_ccs_configuration_data(
	_ccs_configuration_data_t       *data,
	size_t                          *cum_size,
	_ccs_object_serialize_options_t *opts)
{
	(void)opts;
	*cum_size += _ccs_serialize_bin_size_ccs_object(data->configuration_space);
	*cum_size += _ccs_serialize_bin_size_size(data->num_values);
	*cum_size += _ccs_serialize_bin_size_size(data->num_bindings);
	for (size_t i = 0; i < data->num_values; i++)
		*cum_size += _ccs_serialize_bin_size_ccs_datum(data->values[i]);
	for (size_t i = 0; i < data->num_bindings; i++)
		CCS_VALIDATE(data->bindings[i]->obj.ops->serialize_size(
			data->bindings[i], CCS_SERIALIZE_FORMAT_BINARY,
			cum_size, opts));

	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_serialize_bin_ccs_configuration_data(
	_ccs_configuration_data_t       *data,
	size_t                          *buffer_size,
	char                           **buffer,
	_ccs_object_serialize_options_t *opts)
{
	(void)opts;
	CCS_VALIDATE(_ccs_serialize_bin_ccs_object(
		data->configuration_space, buffer_size, buffer));
	CCS_VALIDATE(_ccs_serialize_bin_size(
		data->num_values, buffer_size, buffer));
	CCS_VALIDATE(_ccs_serialize_bin_size(
		data->num_bindings, buffer_size, buffer));
	for (size_t i = 0; i < data->num_values; i++)
		CCS_VALIDATE(_ccs_serialize_bin_ccs_datum(
			data->values[i], buffer_size, buffer));
	for (size_t i = 0; i < data->num_bindings; i++)
		CCS_VALIDATE(_ccs_serialize_bin_ccs_object(
			data->bindings[i], buffer_size, buffer));
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_serialize_bin_size_ccs_configuration(
	ccs_configuration_t              configuration,
	size_t                          *cum_size,
	_ccs_object_serialize_options_t *opts)
{
	*cum_size += _ccs_serialize_bin_size_ccs_object_internal(
		(_ccs_object_internal_t *)configuration);
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
	CCS_VALIDATE(_ccs_serialize_bin_ccs_object_internal(
		(_ccs_object_internal_t *)configuration, buffer_size, buffer));
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
	CCS_VALIDATE(_ccs_object_serialize_user_data_size(
		object, format, cum_size, opts));
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
			(ccs_configuration_t)object, buffer_size, buffer, opts));
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
_ccs_configuration_hash(_ccs_configuration_data_t *data, ccs_hash_t *hash_ret)
{
	return _ccs_binding_hash((_ccs_binding_data_t *)data, hash_ret);
}

static ccs_result_t
_ccs_configuration_cmp(
	_ccs_configuration_data_t *data,
	ccs_configuration_t        other,
	int                       *cmp_ret)
{
	return _ccs_binding_cmp(
		(_ccs_binding_data_t *)data, (ccs_binding_t)other, cmp_ret);
}

static _ccs_configuration_ops_t _configuration_ops = {
	{&_ccs_configuration_del, &_ccs_configuration_serialize_size,
	 &_ccs_configuration_serialize},
	&_ccs_configuration_hash,
	&_ccs_configuration_cmp};

ccs_result_t
_ccs_create_configuration(
	ccs_configuration_space_t configuration_space,
	size_t                    num_values,
	ccs_datum_t              *values,
	size_t                    num_bindings,
        ccs_binding_t            *bindings,
	ccs_configuration_t      *configuration_ret)
{
	size_t num_parameters =	configuration_space->data->num_parameters;
        size_t num_contexts   = configuration_space->data->num_contexts;
	ccs_result_t err = CCS_RESULT_SUCCESS;
	uintptr_t    mem = (uintptr_t)calloc(
		1, sizeof(struct _ccs_configuration_s) +
			sizeof(struct _ccs_configuration_data_s) +
			num_parameters * sizeof(ccs_datum_t) +
			num_contexts * sizeof(ccs_binding_t));
	CCS_REFUTE(!mem, CCS_RESULT_ERROR_OUT_OF_MEMORY);
	uintptr_t mem_orig = mem;
	ccs_configuration_t config;
	config = (ccs_configuration_t)mem;
	mem += sizeof(struct _ccs_configuration_s);
	_ccs_object_init(
		&(config->obj), CCS_OBJECT_TYPE_CONFIGURATION,
		(_ccs_object_ops_t *)&_configuration_ops);
	config->data                      = (struct _ccs_configuration_data_s *)mem;
	mem += sizeof(struct _ccs_configuration_data_s);
	config->data->num_values   = num_parameters;
	config->data->num_bindings = num_contexts;
	config->data->values       = (ccs_datum_t *)mem;
	mem += sizeof(ccs_datum_t) * num_parameters;
	config->data->bindings     = (ccs_binding_t *)mem;
	mem += sizeof(ccs_binding_t) * num_contexts;
	CCS_VALIDATE_ERR_GOTO(
		err, ccs_retain_object(configuration_space), errinit);
	config->data->configuration_space = configuration_space;
        if (bindings) {
		for (size_t i = 0; i < num_bindings; i++) {
			CCS_VALIDATE_ERR_GOTO(
				err, ccs_retain_object(bindings[i]), errinit);
			config->data->bindings[i] = bindings[i];
		}
	}

	if (values) {
		memcpy(config->data->values, values,
		       num_parameters * sizeof(ccs_datum_t));
		for (size_t i = 0; i < num_values; i++)
			if (values[i].flags & CCS_DATUM_FLAG_TRANSIENT)
				CCS_VALIDATE_ERR_GOTO(
					err,
					ccs_configuration_space_validate_value(
						configuration_space, i,
						values[i],
						config->data->values + i),
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
	size_t                    num_values,
	ccs_datum_t              *values,
	size_t                    num_bindings,
        ccs_binding_t            *bindings,
	ccs_configuration_t      *configuration_ret)
{
	CCS_CHECK_OBJ(configuration_space, CCS_OBJECT_TYPE_CONFIGURATION_SPACE);
	CCS_CHECK_PTR(configuration_ret);
	CCS_CHECK_ARY(num_values, values);
	CCS_CHECK_ARY(num_bindings, bindings);
	for (size_t i = 0; i < num_bindings; i++)
		CCS_CHECK_BINDING(bindings[i]);
	size_t num_parameters = configuration_space->data->num_parameters;
        size_t num_contexts   = configuration_space->data->num_contexts;
	CCS_REFUTE(
		num_parameters != num_values,
		CCS_RESULT_ERROR_INVALID_VALUE);
	CCS_REFUTE(
		num_contexts != num_bindings,
		CCS_RESULT_ERROR_INVALID_VALUE);
	for (size_t i = 0; i < num_bindings; i++)
		CCS_REFUTE(
			bindings[i]->data->context !=
				configuration_space->data->contexts[i],
			CCS_RESULT_ERROR_INVALID_VALUE);
	CCS_VALIDATE(_ccs_create_configuration(
		configuration_space, num_values, values,
		num_bindings, bindings, configuration_ret));
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
ccs_configuration_get_configuration_space(
	ccs_configuration_t        configuration,
	ccs_configuration_space_t *configuration_space_ret)
{
	CCS_CHECK_OBJ(configuration, CCS_OBJECT_TYPE_CONFIGURATION);
	CCS_VALIDATE(_ccs_binding_get_context(
		(ccs_binding_t)configuration,
		(ccs_context_t *)configuration_space_ret));
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
ccs_configuration_get_value(
	ccs_configuration_t configuration,
	size_t              index,
	ccs_datum_t        *value_ret)
{
	CCS_CHECK_OBJ(configuration, CCS_OBJECT_TYPE_CONFIGURATION);
	CCS_VALIDATE(_ccs_binding_get_value(
		(ccs_binding_t)configuration, index, value_ret));
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
ccs_configuration_get_values(
	ccs_configuration_t configuration,
	size_t              num_values,
	ccs_datum_t        *values,
	size_t             *num_values_ret)
{
	CCS_CHECK_OBJ(configuration, CCS_OBJECT_TYPE_CONFIGURATION);
	CCS_VALIDATE(_ccs_binding_get_values(
		(ccs_binding_t)configuration, num_values, values,
		num_values_ret));
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
ccs_configuration_get_value_by_name(
	ccs_configuration_t configuration,
	const char         *name,
	ccs_datum_t        *value_ret)
{
	CCS_CHECK_OBJ(configuration, CCS_OBJECT_TYPE_CONFIGURATION);
	CCS_VALIDATE(_ccs_binding_get_value_by_name(
		(ccs_binding_t)configuration, name, value_ret));
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
ccs_configuration_check(
	ccs_configuration_t configuration,
	ccs_bool_t         *is_valid_ret)
{
	CCS_CHECK_OBJ(configuration, CCS_OBJECT_TYPE_CONFIGURATION);
	CCS_VALIDATE(ccs_configuration_space_check_configuration(
		configuration->data->configuration_space, configuration,
		is_valid_ret));
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
ccs_configuration_hash(ccs_configuration_t configuration, ccs_hash_t *hash_ret)
{
	CCS_CHECK_OBJ(configuration, CCS_OBJECT_TYPE_CONFIGURATION);
	_ccs_configuration_ops_t *ops =
		ccs_configuration_get_ops(configuration);
	CCS_VALIDATE(ops->hash(configuration->data, hash_ret));
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
ccs_configuration_cmp(
	ccs_configuration_t configuration,
	ccs_configuration_t other_configuration,
	int                *cmp_ret)
{
	CCS_CHECK_OBJ(configuration, CCS_OBJECT_TYPE_CONFIGURATION);
	CCS_CHECK_OBJ(other_configuration, CCS_OBJECT_TYPE_CONFIGURATION);
	_ccs_configuration_ops_t *ops =
		ccs_configuration_get_ops(configuration);
	CCS_VALIDATE(
		ops->cmp(configuration->data, other_configuration, cmp_ret));
	return CCS_RESULT_SUCCESS;
}
