#include "cconfigspace_internal.h"
#include "features_space_internal.h"
#include "features_internal.h"
#include "utlist.h"

static ccs_error_t
_ccs_features_space_del(ccs_object_t object)
{
	ccs_features_space_t      features_space = (ccs_features_space_t)object;
	UT_array                 *array   = features_space->data->parameters;
	_ccs_parameter_wrapper_t *wrapper = NULL;
	while ((wrapper = (_ccs_parameter_wrapper_t *)utarray_next(
			array, wrapper))) {
		ccs_release_object(wrapper->parameter);
	}
	HASH_CLEAR(hh_name, features_space->data->name_hash);
	_ccs_parameter_index_hash_t *elem, *tmpelem;
	HASH_ITER(hh_handle, features_space->data->handle_hash, elem, tmpelem)
	{
		HASH_DELETE(hh_handle, features_space->data->handle_hash, elem);
		free(elem);
	}
	utarray_free(features_space->data->parameters);
	return CCS_SUCCESS;
}

static ccs_error_t
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
			CCS_INVALID_VALUE,
			"Unsupported serialization format: %d",
			format);
	}
	CCS_VALIDATE(_ccs_object_serialize_user_data_size(
		object, format, cum_size, opts));
	return CCS_SUCCESS;
}

static ccs_error_t
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
			CCS_INVALID_VALUE,
			"Unsupported serialization format: %d",
			format);
	}
	CCS_VALIDATE(_ccs_object_serialize_user_data(
		object, format, buffer_size, buffer, opts));
	return CCS_SUCCESS;
}

static _ccs_features_space_ops_t _features_space_ops = {
	{{&_ccs_features_space_del,
	  &_ccs_features_space_serialize_size,
	  &_ccs_features_space_serialize}}};

static const UT_icd _parameter_wrapper_icd = {
	sizeof(_ccs_parameter_wrapper_t),
	NULL,
	NULL,
	NULL,
};

#undef utarray_oom
#define utarray_oom()                                                          \
	{                                                                      \
		CCS_RAISE_ERR_GOTO(                                            \
			err,                                                   \
			CCS_OUT_OF_MEMORY,                                     \
			arrays,                                                \
			"Not enough memory to allocate array");                \
	}
ccs_error_t
ccs_create_features_space(
	const char           *name,
	ccs_features_space_t *features_space_ret)
{
	ccs_error_t err;
	CCS_CHECK_PTR(name);
	CCS_CHECK_PTR(features_space_ret);
	uintptr_t mem = (uintptr_t)calloc(
		1,
		sizeof(struct _ccs_features_space_s) +
			sizeof(struct _ccs_features_space_data_s) +
			strlen(name) + 1);
	CCS_REFUTE(!mem, CCS_OUT_OF_MEMORY);

	ccs_features_space_t feat_space = (ccs_features_space_t)mem;
	_ccs_object_init(
		&(feat_space->obj),
		CCS_FEATURES_SPACE,
		(_ccs_object_ops_t *)&_features_space_ops);
	feat_space->data =
		(struct _ccs_features_space_data_s
			 *)(mem + sizeof(struct _ccs_features_space_s));
	feat_space->data->name =
		(const char
			 *)(mem + sizeof(struct _ccs_features_space_s) + sizeof(struct _ccs_features_space_data_s));
	utarray_new(feat_space->data->parameters, &_parameter_wrapper_icd);
	strcpy((char *)(feat_space->data->name), name);
	*features_space_ret = feat_space;
	return CCS_SUCCESS;
arrays:
	if (feat_space->data->parameters)
		utarray_free(feat_space->data->parameters);
	free((void *)mem);
	return err;
}

ccs_error_t
ccs_features_space_get_name(
	ccs_features_space_t features_space,
	const char         **name_ret)
{
	CCS_CHECK_OBJ(features_space, CCS_FEATURES_SPACE);
	CCS_VALIDATE(
		_ccs_context_get_name((ccs_context_t)features_space, name_ret));
	return CCS_SUCCESS;
}

#undef utarray_oom
#define utarray_oom()                                                          \
	{                                                                      \
		CCS_RAISE_ERR_GOTO(                                            \
			err,                                                   \
			CCS_OUT_OF_MEMORY,                                     \
			errormem,                                              \
			"Not enough memory to allocate array");                \
	}
#undef uthash_nonfatal_oom
#define uthash_nonfatal_oom(elt)                                               \
	{                                                                      \
		CCS_RAISE_ERR_GOTO(                                            \
			err,                                                   \
			CCS_OUT_OF_MEMORY,                                     \
			errorutarray,                                          \
			"Not enough memory to allocate hash");                 \
	}
ccs_error_t
ccs_features_space_add_parameter(
	ccs_features_space_t features_space,
	ccs_parameter_t      parameter)
{
	CCS_CHECK_OBJ(features_space, CCS_FEATURES_SPACE);
	CCS_CHECK_OBJ(parameter, CCS_PARAMETER);
	ccs_error_t                  err;
	const char                  *name;
	size_t                       sz_name;
	_ccs_parameter_index_hash_t *parameter_hash;
	CCS_VALIDATE(ccs_parameter_get_name(parameter, &name));
	sz_name = strlen(name);
	HASH_FIND(
		hh_name,
		features_space->data->name_hash,
		name,
		sz_name,
		parameter_hash);
	CCS_REFUTE_MSG(
		parameter_hash,
		CCS_INVALID_PARAMETER,
		"An parameter with name '%s' already exists in the feature "
		"space",
		name);
	UT_array *parameters;
	CCS_VALIDATE(ccs_retain_object(parameter));
	_ccs_parameter_wrapper_t parameter_wrapper;
	parameter_wrapper.parameter = parameter;

	parameters                  = features_space->data->parameters;
	parameter_hash              = (_ccs_parameter_index_hash_t *)malloc(
                sizeof(_ccs_parameter_index_hash_t));
	CCS_REFUTE_ERR_GOTO(
		err, !parameter_hash, CCS_OUT_OF_MEMORY, errorparameter);
	parameter_hash->parameter = parameter;
	parameter_hash->name      = name;
	parameter_hash->index     = utarray_len(parameters);

	utarray_push_back(parameters, &parameter_wrapper);

	HASH_ADD_KEYPTR(
		hh_name,
		features_space->data->name_hash,
		parameter_hash->name,
		sz_name,
		parameter_hash);
	HASH_ADD(
		hh_handle,
		features_space->data->handle_hash,
		parameter,
		sizeof(ccs_parameter_t),
		parameter_hash);

	return CCS_SUCCESS;
errorutarray:
	utarray_pop_back(parameters);
errormem:
	free(parameter_hash);
errorparameter:
	ccs_release_object(parameter);
	return err;
}

#undef utarray_oom
#define utarray_oom() exit(-1)

ccs_error_t
ccs_features_space_add_parameters(
	ccs_features_space_t features_space,
	size_t               num_parameters,
	ccs_parameter_t     *parameters)
{
	CCS_CHECK_OBJ(features_space, CCS_FEATURES_SPACE);
	CCS_CHECK_ARY(num_parameters, parameters);
	for (size_t i = 0; i < num_parameters; i++)
		CCS_VALIDATE(ccs_features_space_add_parameter(
			features_space, parameters[i]));
	return CCS_SUCCESS;
}

ccs_error_t
ccs_features_space_get_num_parameters(
	ccs_features_space_t features_space,
	size_t              *num_parameters_ret)
{
	CCS_CHECK_OBJ(features_space, CCS_FEATURES_SPACE);
	CCS_VALIDATE(_ccs_context_get_num_parameters(
		(ccs_context_t)features_space, num_parameters_ret));
	return CCS_SUCCESS;
}

ccs_error_t
ccs_features_space_get_parameter(
	ccs_features_space_t features_space,
	size_t               index,
	ccs_parameter_t     *parameter_ret)
{
	CCS_CHECK_OBJ(features_space, CCS_FEATURES_SPACE);
	CCS_VALIDATE(_ccs_context_get_parameter(
		(ccs_context_t)features_space, index, parameter_ret));
	return CCS_SUCCESS;
}

ccs_error_t
ccs_features_space_get_parameter_by_name(
	ccs_features_space_t features_space,
	const char          *name,
	ccs_parameter_t     *parameter_ret)
{
	CCS_CHECK_OBJ(features_space, CCS_FEATURES_SPACE);
	CCS_VALIDATE(_ccs_context_get_parameter_by_name(
		(ccs_context_t)features_space, name, parameter_ret));
	return CCS_SUCCESS;
}

ccs_error_t
ccs_features_space_get_parameter_index_by_name(
	ccs_features_space_t features_space,
	const char          *name,
	size_t              *index_ret)
{
	CCS_CHECK_OBJ(features_space, CCS_FEATURES_SPACE);
	CCS_VALIDATE(_ccs_context_get_parameter_index_by_name(
		(ccs_context_t)features_space, name, index_ret));
	return CCS_SUCCESS;
}

ccs_error_t
ccs_features_space_get_parameter_index(
	ccs_features_space_t features_space,
	ccs_parameter_t      parameter,
	size_t              *index_ret)
{
	CCS_CHECK_OBJ(features_space, CCS_FEATURES_SPACE);
	CCS_CHECK_OBJ(parameter, CCS_PARAMETER);
	CCS_VALIDATE(_ccs_context_get_parameter_index(
		(ccs_context_t)(features_space), parameter, index_ret));
	return CCS_SUCCESS;
}

ccs_error_t
ccs_features_space_get_parameter_indexes(
	ccs_features_space_t features_space,
	size_t               num_parameters,
	ccs_parameter_t     *parameters,
	size_t              *indexes)
{
	CCS_CHECK_OBJ(features_space, CCS_FEATURES_SPACE);
	CCS_VALIDATE(_ccs_context_get_parameter_indexes(
		(ccs_context_t)features_space,
		num_parameters,
		parameters,
		indexes));
	return CCS_SUCCESS;
}

ccs_error_t
ccs_features_space_get_parameters(
	ccs_features_space_t features_space,
	size_t               num_parameters,
	ccs_parameter_t     *parameters,
	size_t              *num_parameters_ret)
{
	CCS_CHECK_OBJ(features_space, CCS_FEATURES_SPACE);
	CCS_VALIDATE(_ccs_context_get_parameters(
		(ccs_context_t)features_space,
		num_parameters,
		parameters,
		num_parameters_ret));
	return CCS_SUCCESS;
}

ccs_error_t
ccs_features_space_validate_value(
	ccs_features_space_t features_space,
	size_t               index,
	ccs_datum_t          value,
	ccs_datum_t         *value_ret)
{
	CCS_CHECK_OBJ(features_space, CCS_FEATURES_SPACE);
	CCS_VALIDATE(_ccs_context_validate_value(
		(ccs_context_t)features_space, index, value, value_ret));
	return CCS_SUCCESS;
}

static inline ccs_error_t
_check_features(
	ccs_features_space_t features_space,
	size_t               num_values,
	ccs_datum_t         *values,
	ccs_bool_t          *is_valid_ret)
{
	UT_array *array          = features_space->data->parameters;
	size_t    num_parameters = utarray_len(array);
	CCS_REFUTE(num_values != num_parameters, CCS_INVALID_FEATURES);
	*is_valid_ret = CCS_TRUE;
	for (size_t i = 0; i < num_values; i++) {
		_ccs_parameter_wrapper_t *wrapper =
			(_ccs_parameter_wrapper_t *)utarray_eltptr(array, i);
		CCS_VALIDATE(ccs_parameter_check_value(
			wrapper->parameter, values[i], is_valid_ret));
		if (*is_valid_ret == CCS_FALSE)
			return CCS_SUCCESS;
	}
	return CCS_SUCCESS;
}

ccs_error_t
ccs_features_space_check_features(
	ccs_features_space_t features_space,
	ccs_features_t       features,
	ccs_bool_t          *is_valid_ret)
{
	CCS_CHECK_OBJ(features_space, CCS_FEATURES_SPACE);
	CCS_CHECK_OBJ(features, CCS_FEATURES);
	CCS_REFUTE(
		features->data->features_space != features_space,
		CCS_INVALID_FEATURES);
	CCS_VALIDATE(_check_features(
		features_space,
		features->data->num_values,
		features->data->values,
		is_valid_ret));
	return CCS_SUCCESS;
}

ccs_error_t
ccs_features_space_check_features_values(
	ccs_features_space_t features_space,
	size_t               num_values,
	ccs_datum_t         *values,
	ccs_bool_t          *is_valid_ret)
{
	CCS_CHECK_OBJ(features_space, CCS_CONFIGURATION_SPACE);
	CCS_CHECK_ARY(num_values, values);
	CCS_VALIDATE(_check_features(
		features_space, num_values, values, is_valid_ret));
	return CCS_SUCCESS;
}
