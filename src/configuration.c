#include "cconfigspace_internal.h"
#include "configuration_internal.h"
#include "configuration_space_internal.h"
#include <string.h>

static ccs_result_t
_ccs_configuration_del(ccs_object_t object)
{
	ccs_configuration_t configuration = (ccs_configuration_t)object;
	ccs_release_object(configuration->data->configuration_space);
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
_ccs_configuration_serialize(
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
_ccs_configuration_hash(ccs_configuration_t configuration, ccs_hash_t *hash_ret)
{
	return _ccs_binding_hash((ccs_binding_t)configuration, hash_ret);
}

static ccs_result_t
_ccs_configuration_cmp(
	ccs_configuration_t configuration,
	ccs_configuration_t other,
	int                *cmp_ret)
{
	return _ccs_binding_cmp(
		(ccs_binding_t)configuration, (ccs_binding_t)other, cmp_ret);
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
	ccs_configuration_t      *configuration_ret)
{
	ccs_result_t err;
	size_t       num_parameters = configuration_space->data->num_parameters;
	uintptr_t    mem            = (uintptr_t)calloc(
                1, sizeof(struct _ccs_configuration_s) +
                           sizeof(struct _ccs_configuration_data_s) +
                           num_parameters * sizeof(ccs_datum_t));
	CCS_REFUTE(!mem, CCS_RESULT_ERROR_OUT_OF_MEMORY);
	CCS_VALIDATE_ERR_GOTO(
		err, ccs_retain_object(configuration_space), errmem);
	ccs_configuration_t config;
	config = (ccs_configuration_t)mem;
	_ccs_object_init(
		&(config->obj), CCS_OBJECT_TYPE_CONFIGURATION,
		(_ccs_object_ops_t *)&_configuration_ops);
	config->data                      = (struct _ccs_configuration_data_s
                                *)(mem + sizeof(struct _ccs_configuration_s));
	config->data->num_values          = num_parameters;
	config->data->configuration_space = configuration_space;
	config->data->values =
		(ccs_datum_t
			 *)(mem + sizeof(struct _ccs_configuration_s) + sizeof(struct _ccs_configuration_data_s));
	if (values) {
		memcpy(config->data->values, values,
		       num_parameters * sizeof(ccs_datum_t));
		for (size_t i = 0; i < num_values; i++)
			if (values[i].flags & CCS_DATUM_FLAG_TRANSIENT)
				CCS_VALIDATE_ERR_GOTO(
					err,
					ccs_context_validate_value(
						(ccs_context_t)
							configuration_space,
						i, values[i],
						config->data->values + i),
					errinit);
	}
	*configuration_ret = config;
	return CCS_RESULT_SUCCESS;
errinit:
	_ccs_object_deinit(&(config->obj));
errmem:
	free((void *)mem);
	return err;
}

ccs_result_t
ccs_create_configuration(
	ccs_configuration_space_t configuration_space,
	size_t                    num_values,
	ccs_datum_t              *values,
	ccs_configuration_t      *configuration_ret)
{
	CCS_CHECK_OBJ(configuration_space, CCS_OBJECT_TYPE_CONFIGURATION_SPACE);
	CCS_CHECK_PTR(configuration_ret);
	CCS_CHECK_ARY(num_values, values);
	size_t num_parameters = configuration_space->data->num_parameters;
	CCS_REFUTE(
		num_parameters != num_values, CCS_RESULT_ERROR_INVALID_VALUE);
	CCS_VALIDATE(_ccs_create_configuration(
		configuration_space, num_values, values, configuration_ret));
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
