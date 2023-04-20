#ifndef _TREE_TUNER_INTERNAL_H
#define _TREE_TUNER_INTERNAL_H
#include "cconfigspace_internal.h"
#include "tree_space_internal.h"
#include "objective_space_internal.h"

struct _ccs_tree_tuner_data_s;
typedef struct _ccs_tree_tuner_data_s _ccs_tree_tuner_data_t;

struct _ccs_tree_tuner_ops_s {
	_ccs_object_ops_t obj_ops;

	ccs_result_t (*ask)(
		ccs_tree_tuner_t          tree,
		size_t                    num_configurations,
		ccs_tree_configuration_t *configurations,
		size_t                   *num_configurations_ret);

	ccs_result_t (*tell)(
		ccs_tree_tuner_t       tree,
		size_t                 num_evaluations,
		ccs_tree_evaluation_t *evaluations);

	ccs_result_t (*get_optimums)(
		ccs_tree_tuner_t       tree,
		size_t                 num_evaluations,
		ccs_tree_evaluation_t *evaluations,
		size_t                *num_evaluations_ret);

	ccs_result_t (*get_history)(
		ccs_tree_tuner_t       tree,
		size_t                 num_evaluations,
		ccs_tree_evaluation_t *evaluations,
		size_t                *num_evaluations_ret);

	ccs_result_t (*suggest)(
		ccs_tree_tuner_t          tree,
		ccs_tree_configuration_t *configuration);
};
typedef struct _ccs_tree_tuner_ops_s _ccs_tree_tuner_ops_t;

struct _ccs_tree_tuner_s {
	_ccs_object_internal_t  obj;
	_ccs_tree_tuner_data_t *data;
};

struct _ccs_tree_tuner_common_data_s {
	ccs_tree_tuner_type_t type;
	const char           *name;
	ccs_tree_space_t      tree_space;
	ccs_objective_space_t objective_space;
};
typedef struct _ccs_tree_tuner_common_data_s _ccs_tree_tuner_common_data_t;

static inline ccs_result_t
_ccs_serialize_bin_size_ccs_tree_tuner_common_data(
	_ccs_tree_tuner_common_data_t   *data,
	size_t                          *cum_size,
	_ccs_object_serialize_options_t *opts)
{
	*cum_size += _ccs_serialize_bin_size_ccs_tree_tuner_type(data->type);
	*cum_size += _ccs_serialize_bin_size_string(data->name);
	CCS_VALIDATE(data->tree_space->obj.ops->serialize_size(
		data->tree_space, CCS_SERIALIZE_FORMAT_BINARY, cum_size, opts));
	CCS_VALIDATE(data->objective_space->obj.ops->serialize_size(
		data->objective_space, CCS_SERIALIZE_FORMAT_BINARY, cum_size,
		opts));
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_serialize_bin_ccs_tree_tuner_common_data(
	_ccs_tree_tuner_common_data_t   *data,
	size_t                          *buffer_size,
	char                           **buffer,
	_ccs_object_serialize_options_t *opts)
{
	CCS_VALIDATE(_ccs_serialize_bin_ccs_tree_tuner_type(
		data->type, buffer_size, buffer));
	CCS_VALIDATE(
		_ccs_serialize_bin_string(data->name, buffer_size, buffer));
	CCS_VALIDATE(data->tree_space->obj.ops->serialize(
		data->tree_space, CCS_SERIALIZE_FORMAT_BINARY, buffer_size,
		buffer, opts));
	CCS_VALIDATE(data->objective_space->obj.ops->serialize(
		data->objective_space, CCS_SERIALIZE_FORMAT_BINARY, buffer_size,
		buffer, opts));
	return CCS_RESULT_SUCCESS;
}

#endif //_TREE_TUNER_INTERNAL_H
