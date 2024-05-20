#include "cconfigspace_internal.h"
#include "tree_configuration_internal.h"
#include "tree_space_internal.h"
#include "features_internal.h"
#include <string.h>

static ccs_result_t
_ccs_tree_configuration_del(ccs_object_t object)
{
	ccs_tree_configuration_t tree_configuration =
		(ccs_tree_configuration_t)object;
	if (tree_configuration->data->tree_space)
		ccs_release_object(tree_configuration->data->tree_space);
	if (tree_configuration->data->features)
		ccs_release_object(tree_configuration->data->features);
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_serialize_bin_size_ccs_tree_configuration_data(
	_ccs_tree_configuration_data_t  *data,
	size_t                          *cum_size,
	_ccs_object_serialize_options_t *opts)
{
	*cum_size += _ccs_serialize_bin_size_ccs_object(data->tree_space) +
		     _ccs_serialize_bin_size_size(data->position_size);
	for (size_t i = 0; i < data->position_size; i++)
		*cum_size += _ccs_serialize_bin_size_size(data->position[i]);
	*cum_size += _ccs_serialize_bin_size_ccs_bool(data->features != NULL);
	if (data->features)
		CCS_VALIDATE(_ccs_object_serialize_size_with_opts(
			data->features, CCS_SERIALIZE_FORMAT_BINARY, cum_size,
			opts));
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_serialize_bin_ccs_tree_configuration_data(
	_ccs_tree_configuration_data_t  *data,
	size_t                          *buffer_size,
	char                           **buffer,
	_ccs_object_serialize_options_t *opts)
{
	CCS_VALIDATE(_ccs_serialize_bin_ccs_object(
		data->tree_space, buffer_size, buffer));
	CCS_VALIDATE(_ccs_serialize_bin_size(
		data->position_size, buffer_size, buffer));
	for (size_t i = 0; i < data->position_size; i++)
		CCS_VALIDATE(_ccs_serialize_bin_size(
			data->position[i], buffer_size, buffer));
	CCS_VALIDATE(_ccs_serialize_bin_ccs_bool(
		data->features != NULL, buffer_size, buffer));
	if (data->features)
		CCS_VALIDATE(_ccs_object_serialize_with_opts(
			data->features, CCS_SERIALIZE_FORMAT_BINARY,
			buffer_size, buffer, opts));
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_serialize_bin_size_ccs_tree_configuration(
	ccs_tree_configuration_t         tree_configuration,
	size_t                          *cum_size,
	_ccs_object_serialize_options_t *opts)
{
	*cum_size += _ccs_serialize_bin_size_ccs_object_internal(
		(_ccs_object_internal_t *)tree_configuration);
	CCS_VALIDATE(_ccs_serialize_bin_size_ccs_tree_configuration_data(
		tree_configuration->data, cum_size, opts));
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_serialize_bin_ccs_tree_configuration(
	ccs_tree_configuration_t         tree_configuration,
	size_t                          *buffer_size,
	char                           **buffer,
	_ccs_object_serialize_options_t *opts)
{
	CCS_VALIDATE(_ccs_serialize_bin_ccs_object_internal(
		(_ccs_object_internal_t *)tree_configuration, buffer_size,
		buffer));
	CCS_VALIDATE(_ccs_serialize_bin_ccs_tree_configuration_data(
		tree_configuration->data, buffer_size, buffer, opts));
	return CCS_RESULT_SUCCESS;
}

static ccs_result_t
_ccs_tree_configuration_serialize_size(
	ccs_object_t                     object,
	ccs_serialize_format_t           format,
	size_t                          *cum_size,
	_ccs_object_serialize_options_t *opts)
{
	switch (format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE(_ccs_serialize_bin_size_ccs_tree_configuration(
			(ccs_tree_configuration_t)object, cum_size, opts));
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
_ccs_tree_configuration_serialize(
	ccs_object_t                     object,
	ccs_serialize_format_t           format,
	size_t                          *buffer_size,
	char                           **buffer,
	_ccs_object_serialize_options_t *opts)
{
	switch (format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE(_ccs_serialize_bin_ccs_tree_configuration(
			(ccs_tree_configuration_t)object, buffer_size, buffer,
			opts));
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
_ccs_tree_configuration_hash(
	ccs_tree_configuration_t configuration,
	ccs_hash_t              *hash_ret);

static ccs_result_t
_ccs_tree_configuration_cmp(
	ccs_tree_configuration_t configuration,
	ccs_tree_configuration_t other_configuration,
	int                     *cmp_ret);

static _ccs_tree_configuration_ops_t _tree_configuration_ops = {
	{&_ccs_tree_configuration_del, &_ccs_tree_configuration_serialize_size,
	 &_ccs_tree_configuration_serialize},
	&_ccs_tree_configuration_hash,
	&_ccs_tree_configuration_cmp};

ccs_result_t
ccs_create_tree_configuration(
	ccs_tree_space_t          tree_space,
	ccs_features_t            features,
	size_t                    position_size,
	const size_t             *position,
	ccs_tree_configuration_t *configuration_ret)
{
	CCS_CHECK_OBJ(tree_space, CCS_OBJECT_TYPE_TREE_SPACE);
	_ccs_tree_space_common_data_t *tsdata =
		(_ccs_tree_space_common_data_t *)tree_space->data;
	if (features) {
		CCS_CHECK_OBJ(features, CCS_OBJECT_TYPE_FEATURES);
		CCS_REFUTE(
			features->data->feature_space != tsdata->feature_space,
			CCS_RESULT_ERROR_INVALID_FEATURES);
	}
	CCS_CHECK_PTR(configuration_ret);
	CCS_CHECK_ARY(position_size, position);
	ccs_result_t err;
	uintptr_t    mem = (uintptr_t)calloc(
                1, sizeof(struct _ccs_tree_configuration_s) +
                           sizeof(struct _ccs_tree_configuration_data_s) +
                           position_size * sizeof(size_t));
	CCS_REFUTE(!mem, CCS_RESULT_ERROR_OUT_OF_MEMORY);
	uintptr_t                mem_orig = mem;
	ccs_tree_configuration_t config;
	config = (ccs_tree_configuration_t)mem;
	mem += sizeof(struct _ccs_tree_configuration_s);
	_ccs_object_init(
		&(config->obj), CCS_OBJECT_TYPE_TREE_CONFIGURATION,
		(_ccs_object_ops_t *)&_tree_configuration_ops);
	config->data = (struct _ccs_tree_configuration_data_s *)mem;
	mem += sizeof(struct _ccs_tree_configuration_data_s);
	config->data->position_size = position_size;
	config->data->position      = (size_t *)mem;
	mem += sizeof(size_t) * position_size;
	CCS_VALIDATE_ERR_GOTO(err, ccs_retain_object(tree_space), errinit);
	config->data->tree_space = tree_space;
	if (features)
		CCS_VALIDATE_ERR_GOTO(
			err, ccs_retain_object(features), errinit);
	else if (tsdata->feature_space)
		CCS_VALIDATE_ERR_GOTO(
			err,
			ccs_feature_space_get_default_features(
				tsdata->feature_space, &features),
			errinit);
	config->data->features = features;
	memcpy(config->data->position, position,
	       position_size * sizeof(size_t));
	*configuration_ret = config;
	return CCS_RESULT_SUCCESS;
errinit:
	_ccs_tree_configuration_del(config);
	_ccs_object_deinit(&(config->obj));
	free((void *)mem_orig);
	return err;
}

ccs_result_t
ccs_tree_configuration_get_tree_space(
	ccs_tree_configuration_t configuration,
	ccs_tree_space_t        *tree_space_ret)
{
	CCS_CHECK_OBJ(configuration, CCS_OBJECT_TYPE_TREE_CONFIGURATION);
	CCS_CHECK_PTR(tree_space_ret);
	*tree_space_ret = configuration->data->tree_space;
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
ccs_tree_configuration_get_features(
	ccs_tree_configuration_t configuration,
	ccs_features_t          *features_ret)
{
	CCS_CHECK_OBJ(configuration, CCS_OBJECT_TYPE_TREE_CONFIGURATION);
	CCS_CHECK_PTR(features_ret);
	*features_ret = configuration->data->features;
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
ccs_tree_configuration_get_position(
	ccs_tree_configuration_t configuration,
	size_t                   position_size,
	size_t                  *position,
	size_t                  *position_size_ret)
{
	CCS_CHECK_OBJ(configuration, CCS_OBJECT_TYPE_TREE_CONFIGURATION);
	CCS_CHECK_ARY(position_size, position);
	CCS_REFUTE(
		!position && !position_size_ret,
		CCS_RESULT_ERROR_INVALID_VALUE);
	size_t size = configuration->data->position_size;
	if (position) {
		CCS_REFUTE(
			position_size < size, CCS_RESULT_ERROR_INVALID_VALUE);
		memcpy(position, configuration->data->position,
		       size * sizeof(size_t));
	}
	if (position_size_ret)
		*position_size_ret = size;
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
ccs_tree_configuration_get_values(
	ccs_tree_configuration_t configuration,
	size_t                   num_values,
	ccs_datum_t             *values,
	size_t                  *num_values_ret)
{
	CCS_CHECK_OBJ(configuration, CCS_OBJECT_TYPE_TREE_CONFIGURATION);
	CCS_REFUTE(!values && !num_values_ret, CCS_RESULT_ERROR_INVALID_VALUE);
	size_t num = configuration->data->position_size + 1;
	if (values)
		CCS_VALIDATE(ccs_tree_space_get_values_at_position(
			configuration->data->tree_space,
			configuration->data->position_size,
			configuration->data->position, num_values, values));
	if (num_values_ret)
		*num_values_ret = num;
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
ccs_tree_configuration_get_node(
	ccs_tree_configuration_t configuration,
	ccs_tree_t              *tree_ret)
{
	CCS_CHECK_OBJ(configuration, CCS_OBJECT_TYPE_TREE_CONFIGURATION);
	CCS_VALIDATE(ccs_tree_space_get_node_at_position(
		configuration->data->tree_space,
		configuration->data->position_size,
		configuration->data->position, tree_ret));
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
ccs_tree_configuration_check(
	ccs_tree_configuration_t configuration,
	ccs_bool_t              *is_valid_ret)
{
	CCS_CHECK_OBJ(configuration, CCS_OBJECT_TYPE_TREE_CONFIGURATION);
	CCS_CHECK_PTR(is_valid_ret);
	CCS_VALIDATE(ccs_tree_space_check_position(
		configuration->data->tree_space,
		configuration->data->position_size,
		configuration->data->position, is_valid_ret));
	return CCS_RESULT_SUCCESS;
}

#define CCS_CMP(a, b) ((a) < (b) ? -1 : ((a) > (b) ? 1 : 0))

static ccs_result_t
_ccs_tree_configuration_cmp(
	ccs_tree_configuration_t configuration,
	ccs_tree_configuration_t other_configuration,
	int                     *cmp_ret)
{
	if (configuration == other_configuration) {
		*cmp_ret = 0;
		return CCS_RESULT_SUCCESS;
	}
	*cmp_ret =
		CCS_CMP(configuration->data->tree_space,
			other_configuration->data->tree_space);
	if (*cmp_ret)
		return CCS_RESULT_SUCCESS;
	*cmp_ret =
		CCS_CMP(configuration->data->position_size,
			other_configuration->data->position_size);
	if (*cmp_ret)
		return CCS_RESULT_SUCCESS;
	size_t  position_size  = configuration->data->position_size;
	size_t *position       = configuration->data->position;
	size_t *other_position = other_configuration->data->position;
	for (size_t i = 0; i < position_size; i++) {
		*cmp_ret = CCS_CMP(position[i], other_position[i]);
		if (*cmp_ret)
			return CCS_RESULT_SUCCESS;
	}
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
ccs_tree_configuration_cmp(
	ccs_tree_configuration_t configuration,
	ccs_tree_configuration_t other_configuration,
	int                     *cmp_ret)
{
	CCS_CHECK_OBJ(configuration, CCS_OBJECT_TYPE_TREE_CONFIGURATION);
	CCS_CHECK_OBJ(other_configuration, CCS_OBJECT_TYPE_TREE_CONFIGURATION);
	CCS_CHECK_PTR(cmp_ret);
	CCS_VALIDATE(_ccs_tree_configuration_cmp(
		configuration, other_configuration, cmp_ret));
	return CCS_RESULT_SUCCESS;
}

#include "datum_hash.h"
static ccs_result_t
_ccs_tree_configuration_hash(
	ccs_tree_configuration_t configuration,
	ccs_hash_t              *hash_ret)
{
	ccs_hash_t h, ht;
	HASH_JEN(
		&(configuration->data->tree_space),
		sizeof(configuration->data->tree_space), h);
	size_t position_size = configuration->data->position_size;
	HASH_JEN(&(position_size), sizeof(position_size), ht);
	h                = _hash_combine(h, ht);
	size_t *position = configuration->data->position;
	for (size_t i = 0; i < position_size; i++) {
		HASH_JEN(&position[i], sizeof(size_t), ht);
		h = _hash_combine(h, ht);
	}
	*hash_ret = h;
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
ccs_tree_configuration_hash(
	ccs_tree_configuration_t configuration,
	ccs_hash_t              *hash_ret)
{
	CCS_CHECK_OBJ(configuration, CCS_OBJECT_TYPE_TREE_CONFIGURATION);
	CCS_CHECK_PTR(hash_ret);
	CCS_VALIDATE(_ccs_tree_configuration_hash(configuration, hash_ret));
	return CCS_RESULT_SUCCESS;
}
