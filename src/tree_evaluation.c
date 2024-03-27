#include "cconfigspace_internal.h"
#include "evaluation_binding_internal.h"
#include "tree_evaluation_internal.h"
#include "tree_configuration_internal.h"
#include "objective_space_internal.h"
#include <string.h>

static ccs_result_t
_ccs_tree_evaluation_del(ccs_object_t object)
{
	ccs_tree_evaluation_t evaluation = (ccs_tree_evaluation_t)object;
	ccs_release_object(evaluation->data->objective_space);
	ccs_release_object(evaluation->data->configuration);
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_serialize_bin_size_ccs_tree_evaluation_data(
	_ccs_tree_evaluation_data_t     *data,
	size_t                          *cum_size,
	_ccs_object_serialize_options_t *opts)
{
	*cum_size += _ccs_serialize_bin_size_ccs_binding_data(
		(_ccs_binding_data_t *)data);
	CCS_VALIDATE(data->configuration->obj.ops->serialize_size(
		data->configuration, CCS_SERIALIZE_FORMAT_BINARY, cum_size,
		opts));
	*cum_size +=
		_ccs_serialize_bin_size_ccs_evaluation_result(data->result);
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_serialize_bin_ccs_tree_evaluation_data(
	_ccs_tree_evaluation_data_t     *data,
	size_t                          *buffer_size,
	char                           **buffer,
	_ccs_object_serialize_options_t *opts)
{
	CCS_VALIDATE(_ccs_serialize_bin_ccs_binding_data(
		(_ccs_binding_data_t *)data, buffer_size, buffer));
	CCS_VALIDATE(data->configuration->obj.ops->serialize(
		data->configuration, CCS_SERIALIZE_FORMAT_BINARY, buffer_size,
		buffer, opts));
	CCS_VALIDATE(_ccs_serialize_bin_ccs_evaluation_result(
		data->result, buffer_size, buffer));
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_serialize_bin_size_ccs_tree_evaluation(
	ccs_tree_evaluation_t            evaluation,
	size_t                          *cum_size,
	_ccs_object_serialize_options_t *opts)
{
	*cum_size += _ccs_serialize_bin_size_ccs_object_internal(
		(_ccs_object_internal_t *)evaluation);
	CCS_VALIDATE(_ccs_serialize_bin_size_ccs_tree_evaluation_data(
		evaluation->data, cum_size, opts));
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_serialize_bin_ccs_tree_evaluation(
	ccs_tree_evaluation_t            evaluation,
	size_t                          *buffer_size,
	char                           **buffer,
	_ccs_object_serialize_options_t *opts)
{
	CCS_VALIDATE(_ccs_serialize_bin_ccs_object_internal(
		(_ccs_object_internal_t *)evaluation, buffer_size, buffer));
	CCS_VALIDATE(_ccs_serialize_bin_ccs_tree_evaluation_data(
		evaluation->data, buffer_size, buffer, opts));
	return CCS_RESULT_SUCCESS;
}

static ccs_result_t
_ccs_tree_evaluation_serialize_size(
	ccs_object_t                     object,
	ccs_serialize_format_t           format,
	size_t                          *cum_size,
	_ccs_object_serialize_options_t *opts)
{
	switch (format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE(_ccs_serialize_bin_size_ccs_tree_evaluation(
			(ccs_tree_evaluation_t)object, cum_size, opts));
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
_ccs_tree_evaluation_serialize(
	ccs_object_t                     object,
	ccs_serialize_format_t           format,
	size_t                          *buffer_size,
	char                           **buffer,
	_ccs_object_serialize_options_t *opts)
{
	switch (format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE(_ccs_serialize_bin_ccs_tree_evaluation(
			(ccs_tree_evaluation_t)object, buffer_size, buffer,
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
_ccs_tree_evaluation_hash(
	ccs_tree_evaluation_t tree_evaluation,
	ccs_hash_t           *hash_ret)
{
	_ccs_tree_evaluation_data_t *data = tree_evaluation->data;
	ccs_hash_t                   h, ht;
	CCS_VALIDATE(_ccs_binding_hash((ccs_binding_t)tree_evaluation, &h));
	CCS_VALIDATE(ccs_tree_configuration_hash(data->configuration, &ht));
	h = _hash_combine(h, ht);
	HASH_JEN(&(data->result), sizeof(data->result), ht);
	h         = _hash_combine(h, ht);
	*hash_ret = h;
	return CCS_RESULT_SUCCESS;
}

static ccs_result_t
_ccs_tree_evaluation_cmp(
	ccs_tree_evaluation_t tree_evaluation,
	ccs_tree_evaluation_t other,
	int                  *cmp_ret)
{
	CCS_VALIDATE(_ccs_binding_cmp(
		(ccs_binding_t)tree_evaluation, (ccs_binding_t)other, cmp_ret));
	if (*cmp_ret)
		return CCS_RESULT_SUCCESS;
	_ccs_tree_evaluation_data_t *data       = tree_evaluation->data;
	_ccs_tree_evaluation_data_t *other_data = other->data;
	*cmp_ret = data->result < other_data->result ? -1 :
		   data->result > other_data->result ? 1 :
						       0;
	if (*cmp_ret)
		return CCS_RESULT_SUCCESS;
	CCS_VALIDATE(ccs_tree_configuration_cmp(
		data->configuration, other_data->configuration, cmp_ret));
	return CCS_RESULT_SUCCESS;
}

static ccs_result_t
_ccs_tree_evaluation_compare(
	ccs_tree_evaluation_t evaluation,
	ccs_tree_evaluation_t other,
	ccs_comparison_t     *result_ret)
{
	CCS_VALIDATE(_ccs_evaluation_binding_compare(
		(ccs_evaluation_binding_t)evaluation,
		(ccs_evaluation_binding_t)other, result_ret));
	return CCS_RESULT_SUCCESS;
}

static _ccs_tree_evaluation_ops_t _evaluation_ops = {
	{&_ccs_tree_evaluation_del, &_ccs_tree_evaluation_serialize_size,
	 &_ccs_tree_evaluation_serialize},
	&_ccs_tree_evaluation_hash,
	&_ccs_tree_evaluation_cmp,
	&_ccs_tree_evaluation_compare};

ccs_result_t
ccs_create_tree_evaluation(
	ccs_objective_space_t    objective_space,
	ccs_tree_configuration_t configuration,
	ccs_evaluation_result_t  result,
	size_t                   num_values,
	ccs_datum_t             *values,
	ccs_tree_evaluation_t   *evaluation_ret)
{
	CCS_CHECK_OBJ(objective_space, CCS_OBJECT_TYPE_OBJECTIVE_SPACE);
	CCS_CHECK_OBJ(configuration, CCS_OBJECT_TYPE_TREE_CONFIGURATION);
	CCS_CHECK_PTR(evaluation_ret);
	CCS_CHECK_ARY(num_values, values);
	ccs_result_t err;
	size_t       num_parameters = objective_space->data->num_parameters;
	CCS_REFUTE(
		values && num_parameters != num_values,
		CCS_RESULT_ERROR_INVALID_VALUE);
	uintptr_t mem = (uintptr_t)calloc(
		1, sizeof(struct _ccs_tree_evaluation_s) +
			   sizeof(struct _ccs_tree_evaluation_data_s) +
			   num_parameters * sizeof(ccs_datum_t));
	CCS_REFUTE(!mem, CCS_RESULT_ERROR_OUT_OF_MEMORY);
	CCS_VALIDATE_ERR_GOTO(err, ccs_retain_object(objective_space), errmem);
	CCS_VALIDATE_ERR_GOTO(err, ccs_retain_object(configuration), erros);
	ccs_tree_evaluation_t eval;
	eval = (ccs_tree_evaluation_t)mem;
	_ccs_object_init(
		&(eval->obj), CCS_OBJECT_TYPE_TREE_EVALUATION,
		(_ccs_object_ops_t *)&_evaluation_ops);
	eval->data                  = (struct _ccs_tree_evaluation_data_s
                              *)(mem + sizeof(struct _ccs_tree_evaluation_s));
	eval->data->num_values      = num_parameters;
	eval->data->objective_space = objective_space;
	eval->data->configuration   = configuration;
	eval->data->result          = result;
	eval->data->values =
		(ccs_datum_t
			 *)(mem + sizeof(struct _ccs_tree_evaluation_s) + sizeof(struct _ccs_tree_evaluation_data_s));
	if (values) {
		memcpy(eval->data->values, values,
		       num_parameters * sizeof(ccs_datum_t));
		for (size_t i = 0; i < num_values; i++)
			if (values[i].flags & CCS_DATUM_FLAG_TRANSIENT)
				CCS_VALIDATE_ERR_GOTO(
					err,
					ccs_context_validate_value(
						(ccs_context_t)objective_space,
						i, values[i],
						eval->data->values + i),
					errc);
	}
	*evaluation_ret = eval;
	return CCS_RESULT_SUCCESS;
errc:
	_ccs_object_deinit(&(eval->obj));
	ccs_release_object(configuration);
erros:
	ccs_release_object(objective_space);
errmem:
	free((void *)mem);
	return err;
}

ccs_result_t
ccs_tree_evaluation_get_configuration(
	ccs_tree_evaluation_t     evaluation,
	ccs_tree_configuration_t *configuration_ret)
{
	CCS_CHECK_OBJ(evaluation, CCS_OBJECT_TYPE_TREE_EVALUATION);
	CCS_CHECK_PTR(configuration_ret);
	*configuration_ret = evaluation->data->configuration;
	return CCS_RESULT_SUCCESS;
}
