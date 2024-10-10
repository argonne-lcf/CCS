#include "cconfigspace_internal.h"
#include "feature_space_internal.h"
#include "features_internal.h"
#include "utlist.h"

static ccs_result_t
_ccs_feature_space_del(ccs_object_t object)
{
	ccs_feature_space_t feature_space = (ccs_feature_space_t)object;
	size_t           num_parameters   = feature_space->data->num_parameters;
	ccs_parameter_t *parameters       = feature_space->data->parameters;

	for (size_t i = 0; i < num_parameters; i++)
		if (parameters[i]) {
			_ccs_parameter_release_ownership(
				parameters[i], (ccs_context_t)feature_space);
			ccs_release_object(parameters[i]);
		}

	HASH_CLEAR(hh_name, feature_space->data->name_hash);
	HASH_CLEAR(hh_handle, feature_space->data->handle_hash);
	return CCS_RESULT_SUCCESS;
}

static ccs_result_t
_ccs_feature_space_serialize_size(
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
	return CCS_RESULT_SUCCESS;
}

static ccs_result_t
_ccs_feature_space_serialize(
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
	return CCS_RESULT_SUCCESS;
}

static _ccs_feature_space_ops_t _feature_space_ops = {
	{{&_ccs_feature_space_del, &_ccs_feature_space_serialize_size,
	  &_ccs_feature_space_serialize}}};

static ccs_result_t
_ccs_feature_space_add_parameters(
	ccs_feature_space_t feature_space,
	size_t              num_parameters,
	ccs_parameter_t    *parameters)
{
	for (size_t i = 0; i < num_parameters; i++)
		CCS_VALIDATE(_ccs_context_add_parameter(
			(ccs_context_t)feature_space, parameters[i], i));
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
ccs_create_feature_space(
	const char          *name,
	size_t               num_parameters,
	ccs_parameter_t     *parameters,
	ccs_feature_space_t *feature_space_ret)
{
	ccs_result_t err;
	CCS_CHECK_PTR(name);
	CCS_CHECK_PTR(feature_space_ret);
	CCS_CHECK_ARY(num_parameters, parameters);
	for (size_t i = 0; i < num_parameters; i++)
		CCS_CHECK_OBJ(parameters[i], CCS_OBJECT_TYPE_PARAMETER);

	uintptr_t mem = (uintptr_t)calloc(
		1,
		sizeof(struct _ccs_feature_space_s) +
			sizeof(struct _ccs_feature_space_data_s) +
			sizeof(ccs_parameter_t) * num_parameters +
			sizeof(_ccs_parameter_index_hash_t) * num_parameters +
			strlen(name) + 1);
	CCS_REFUTE(!mem, CCS_RESULT_ERROR_OUT_OF_MEMORY);
	uintptr_t           mem_orig   = mem;

	ccs_feature_space_t feat_space = (ccs_feature_space_t)mem;
	mem += sizeof(struct _ccs_feature_space_s);
	_ccs_object_init(
		&(feat_space->obj), CCS_OBJECT_TYPE_FEATURE_SPACE,
		(_ccs_object_ops_t *)&_feature_space_ops);
	feat_space->data = (struct _ccs_feature_space_data_s *)mem;
	mem += sizeof(struct _ccs_feature_space_data_s);
	feat_space->data->parameters = (ccs_parameter_t *)mem;
	mem += sizeof(ccs_parameter_t) * num_parameters;
	feat_space->data->hash_elems = (_ccs_parameter_index_hash_t *)mem;
	mem += sizeof(_ccs_parameter_index_hash_t) * num_parameters;
	feat_space->data->name           = (const char *)mem;
	feat_space->data->num_parameters = num_parameters;
	strcpy((char *)(feat_space->data->name), name);
	CCS_VALIDATE_ERR_GOTO(
		err,
		_ccs_feature_space_add_parameters(
			feat_space, num_parameters, parameters),
		errparams);
	*feature_space_ret = feat_space;
	return CCS_RESULT_SUCCESS;
errparams:
	_ccs_feature_space_del(feat_space);
	_ccs_object_deinit(&(feat_space->obj));
	free((void *)mem_orig);
	return err;
}

ccs_result_t
ccs_feature_space_get_default_features(
	ccs_feature_space_t feature_space,
	ccs_features_t     *features_ret)
{
	CCS_CHECK_OBJ(feature_space, CCS_OBJECT_TYPE_FEATURE_SPACE);
	CCS_CHECK_PTR(features_ret);
	ccs_result_t   err;
	ccs_features_t features;
	CCS_VALIDATE(_ccs_create_features(feature_space, 0, NULL, &features));
	ccs_parameter_t *parameters = feature_space->data->parameters;
	ccs_datum_t     *values     = features->data->values;
	for (size_t i = 0; i < feature_space->data->num_parameters; i++)
		CCS_VALIDATE_ERR_GOTO(
			err,
			ccs_parameter_get_default_value(
				parameters[i], values + i),
			errc);
	*features_ret = features;
	return CCS_RESULT_SUCCESS;
errc:
	ccs_release_object(features);
	return err;
}
