#include "cconfigspace_internal.h"
#include "features_space_internal.h"
#include "features_internal.h"
#include "utlist.h"

static ccs_result_t
_ccs_features_space_del(ccs_object_t object)
{
	ccs_features_space_t features_space = (ccs_features_space_t)object;
	size_t           num_parameters = features_space->data->num_parameters;
	ccs_parameter_t *parameters     = features_space->data->parameters;

	for (size_t i = 0; i < num_parameters; i++)
		if (parameters[i])
			ccs_release_object(parameters[i]);

	HASH_CLEAR(hh_name, features_space->data->name_hash);
	HASH_CLEAR(hh_handle, features_space->data->handle_hash);
	return CCS_RESULT_SUCCESS;
}

static ccs_result_t
_ccs_features_space_serialize_size(
	ccs_object_t                     object,
	ccs_serialize_format_t           format,
	size_t                          *cum_size,
	_ccs_object_serialize_options_t *opts)
{
	switch (format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE(_ccs_serialize_bin_size_ccs_context(
			(ccs_context_t)object, cum_size, opts));
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
_ccs_features_space_serialize(
	ccs_object_t                     object,
	ccs_serialize_format_t           format,
	size_t                          *buffer_size,
	char                           **buffer,
	_ccs_object_serialize_options_t *opts)
{
	switch (format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE(_ccs_serialize_bin_ccs_context(
			(ccs_context_t)object, buffer_size, buffer, opts));
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

static _ccs_features_space_ops_t _features_space_ops = {
	{{&_ccs_features_space_del, &_ccs_features_space_serialize_size,
	  &_ccs_features_space_serialize}}};

static ccs_result_t
_ccs_features_space_add_parameters(
	ccs_features_space_t features_space,
	size_t               num_parameters,
	ccs_parameter_t     *parameters)
{
	for (size_t i = 0; i < num_parameters; i++)
		CCS_VALIDATE(_ccs_context_add_parameter(
			(ccs_context_t)features_space, parameters[i], i));
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
ccs_create_features_space(
	const char           *name,
	size_t                num_parameters,
	ccs_parameter_t      *parameters,
	ccs_features_space_t *features_space_ret)
{
	ccs_result_t err;
	CCS_CHECK_PTR(name);
	CCS_CHECK_PTR(features_space_ret);
	CCS_CHECK_ARY(num_parameters, parameters);
	for (size_t i = 0; i < num_parameters; i++)
		CCS_CHECK_OBJ(parameters[i], CCS_OBJECT_TYPE_PARAMETER);

	uintptr_t mem = (uintptr_t)calloc(
		1,
		sizeof(struct _ccs_features_space_s) +
			sizeof(struct _ccs_features_space_data_s) +
			sizeof(ccs_parameter_t) * num_parameters +
			sizeof(_ccs_parameter_index_hash_t) * num_parameters +
			strlen(name) + 1);
	CCS_REFUTE(!mem, CCS_RESULT_ERROR_OUT_OF_MEMORY);
	uintptr_t            mem_orig   = mem;

	ccs_features_space_t feat_space = (ccs_features_space_t)mem;
	mem += sizeof(struct _ccs_features_space_s);
	_ccs_object_init(
		&(feat_space->obj), CCS_OBJECT_TYPE_FEATURES_SPACE,
		(_ccs_object_ops_t *)&_features_space_ops);
	feat_space->data = (struct _ccs_features_space_data_s *)mem;
	mem += sizeof(struct _ccs_features_space_data_s);
	feat_space->data->parameters = (ccs_parameter_t *)mem;
	mem += sizeof(ccs_parameter_t) * num_parameters;
	feat_space->data->hash_elems = (_ccs_parameter_index_hash_t *)mem;
	mem += sizeof(_ccs_parameter_index_hash_t) * num_parameters;
	feat_space->data->name           = (const char *)mem;
	feat_space->data->num_parameters = num_parameters;
	strcpy((char *)(feat_space->data->name), name);
	CCS_VALIDATE_ERR_GOTO(
		err,
		_ccs_features_space_add_parameters(
			feat_space, num_parameters, parameters),
		errparams);
	*features_space_ret = feat_space;
	return CCS_RESULT_SUCCESS;
errparams:
	_ccs_features_space_del(feat_space);
	_ccs_object_deinit(&(feat_space->obj));
	free((void *)mem_orig);
	return err;
}

ccs_result_t
ccs_features_space_get_name(
	ccs_features_space_t features_space,
	const char         **name_ret)
{
	CCS_CHECK_OBJ(features_space, CCS_OBJECT_TYPE_FEATURES_SPACE);
	CCS_VALIDATE(
		_ccs_context_get_name((ccs_context_t)features_space, name_ret));
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
ccs_features_space_get_num_parameters(
	ccs_features_space_t features_space,
	size_t              *num_parameters_ret)
{
	CCS_CHECK_OBJ(features_space, CCS_OBJECT_TYPE_FEATURES_SPACE);
	CCS_VALIDATE(_ccs_context_get_num_parameters(
		(ccs_context_t)features_space, num_parameters_ret));
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
ccs_features_space_get_parameter(
	ccs_features_space_t features_space,
	size_t               index,
	ccs_parameter_t     *parameter_ret)
{
	CCS_CHECK_OBJ(features_space, CCS_OBJECT_TYPE_FEATURES_SPACE);
	CCS_VALIDATE(_ccs_context_get_parameter(
		(ccs_context_t)features_space, index, parameter_ret));
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
ccs_features_space_get_parameter_by_name(
	ccs_features_space_t features_space,
	const char          *name,
	ccs_parameter_t     *parameter_ret)
{
	CCS_CHECK_OBJ(features_space, CCS_OBJECT_TYPE_FEATURES_SPACE);
	CCS_VALIDATE(_ccs_context_get_parameter_by_name(
		(ccs_context_t)features_space, name, parameter_ret));
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
ccs_features_space_get_parameter_index_by_name(
	ccs_features_space_t features_space,
	const char          *name,
	size_t              *index_ret)
{
	CCS_CHECK_OBJ(features_space, CCS_OBJECT_TYPE_FEATURES_SPACE);
	CCS_VALIDATE(_ccs_context_get_parameter_index_by_name(
		(ccs_context_t)features_space, name, index_ret));
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
ccs_features_space_get_parameter_index(
	ccs_features_space_t features_space,
	ccs_parameter_t      parameter,
	ccs_bool_t          *found_ret,
	size_t              *index_ret)
{
	CCS_CHECK_OBJ(features_space, CCS_OBJECT_TYPE_FEATURES_SPACE);
	CCS_CHECK_OBJ(parameter, CCS_OBJECT_TYPE_PARAMETER);
	CCS_VALIDATE(_ccs_context_get_parameter_index(
		(ccs_context_t)(features_space), parameter, found_ret,
		index_ret));
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
ccs_features_space_get_parameter_indexes(
	ccs_features_space_t features_space,
	size_t               num_parameters,
	ccs_parameter_t     *parameters,
	ccs_bool_t          *found,
	size_t              *indexes)
{
	CCS_CHECK_OBJ(features_space, CCS_OBJECT_TYPE_FEATURES_SPACE);
	CCS_VALIDATE(_ccs_context_get_parameter_indexes(
		(ccs_context_t)features_space, num_parameters, parameters,
		found, indexes));
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
ccs_features_space_get_parameters(
	ccs_features_space_t features_space,
	size_t               num_parameters,
	ccs_parameter_t     *parameters,
	size_t              *num_parameters_ret)
{
	CCS_CHECK_OBJ(features_space, CCS_OBJECT_TYPE_FEATURES_SPACE);
	CCS_VALIDATE(_ccs_context_get_parameters(
		(ccs_context_t)features_space, num_parameters, parameters,
		num_parameters_ret));
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
ccs_features_space_validate_value(
	ccs_features_space_t features_space,
	size_t               index,
	ccs_datum_t          value,
	ccs_datum_t         *value_ret)
{
	CCS_CHECK_OBJ(features_space, CCS_OBJECT_TYPE_FEATURES_SPACE);
	CCS_VALIDATE(_ccs_context_validate_value(
		(ccs_context_t)features_space, index, value, value_ret));
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_check_features(
	ccs_features_space_t features_space,
	ccs_features_t       features,
	ccs_bool_t          *is_valid_ret)
{
	ccs_parameter_t *parameters     = features_space->data->parameters;
	size_t           num_parameters = features_space->data->num_parameters;
	ccs_datum_t     *values         = features->data->values;
	*is_valid_ret                   = CCS_TRUE;
	for (size_t i = 0; i < num_parameters; i++) {
		CCS_VALIDATE(ccs_parameter_check_value(
			parameters[i], values[i], is_valid_ret));
		if (*is_valid_ret == CCS_FALSE)
			return CCS_RESULT_SUCCESS;
	}
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
ccs_features_space_check_features(
	ccs_features_space_t features_space,
	ccs_features_t       features,
	ccs_bool_t          *is_valid_ret)
{
	CCS_CHECK_OBJ(features_space, CCS_OBJECT_TYPE_FEATURES_SPACE);
	CCS_CHECK_OBJ(features, CCS_OBJECT_TYPE_FEATURES);
	CCS_REFUTE(
		features->data->features_space != features_space,
		CCS_RESULT_ERROR_INVALID_FEATURES);
	CCS_VALIDATE(_check_features(features_space, features, is_valid_ret));
	return CCS_RESULT_SUCCESS;
}
