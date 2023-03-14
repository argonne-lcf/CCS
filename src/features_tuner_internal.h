#ifndef _FEATURES_TUNER_INTERNAL_H
#define _FEATURES_TUNER_INTERNAL_H
#include "cconfigspace_internal.h"
#include "configuration_space_internal.h"
#include "objective_space_internal.h"
#include "features_space_internal.h"

struct _ccs_features_tuner_data_s;
typedef struct _ccs_features_tuner_data_s _ccs_features_tuner_data_t;

struct _ccs_features_tuner_ops_s {
	_ccs_object_ops_t obj_ops;

	ccs_error_t (*ask)(
		ccs_features_tuner_t  tuner,
		ccs_features_t        features,
		size_t                num_configurations,
		ccs_configuration_t  *configurations,
		size_t               *num_configurations_ret);

	ccs_error_t (*tell)(
		ccs_features_tuner_t        tuner,
		size_t                      num_evaluations,
		ccs_features_evaluation_t  *evaluations);

	ccs_error_t (*get_optimums)(
		ccs_features_tuner_t        tuner,
		ccs_features_t              features,
		size_t                      num_evaluations,
		ccs_features_evaluation_t  *evaluations,
		size_t                     *num_evaluations_ret);

	ccs_error_t (*get_history)(
		ccs_features_tuner_t        tuner,
		ccs_features_t              features,
		size_t                      num_evaluations,
		ccs_features_evaluation_t  *evaluations,
		size_t                     *num_evaluations_ret);

	ccs_error_t (*suggest)(
		ccs_features_tuner_t        tuner,
		ccs_features_t              features,
		ccs_configuration_t        *configuration);
};
typedef struct _ccs_features_tuner_ops_s _ccs_features_tuner_ops_t;

struct _ccs_features_tuner_s {
	_ccs_object_internal_t      obj;
	_ccs_features_tuner_data_t *data;
};

struct _ccs_features_tuner_common_data_s {
	ccs_features_tuner_type_t  type;
	const char                *name;
	ccs_configuration_space_t  configuration_space;
	ccs_objective_space_t      objective_space;
	ccs_features_space_t       features_space;
};
typedef struct _ccs_features_tuner_common_data_s _ccs_features_tuner_common_data_t;

static inline ccs_error_t
_ccs_serialize_bin_size_ccs_features_tuner_common_data(
		_ccs_features_tuner_common_data_t *data,
		size_t                            *cum_size,
		_ccs_object_serialize_options_t   *opts) {
	*cum_size += _ccs_serialize_bin_size_ccs_features_tuner_type(data->type);
	*cum_size += _ccs_serialize_bin_size_string(data->name);
	CCS_VALIDATE(data->configuration_space->obj.ops->serialize_size(
		data->configuration_space, CCS_SERIALIZE_FORMAT_BINARY, cum_size, opts));
	CCS_VALIDATE(data->objective_space->obj.ops->serialize_size(
		data->objective_space, CCS_SERIALIZE_FORMAT_BINARY, cum_size, opts));
	CCS_VALIDATE(data->features_space->obj.ops->serialize_size(
		data->features_space, CCS_SERIALIZE_FORMAT_BINARY, cum_size, opts));
	return CCS_SUCCESS;
}

static inline ccs_error_t
_ccs_serialize_bin_ccs_features_tuner_common_data(
		_ccs_features_tuner_common_data_t  *data,
		size_t                             *buffer_size,
		char                              **buffer,
		_ccs_object_serialize_options_t    *opts) {
	CCS_VALIDATE(_ccs_serialize_bin_ccs_features_tuner_type(
		data->type, buffer_size, buffer));
	CCS_VALIDATE(_ccs_serialize_bin_string(
		data->name, buffer_size, buffer));
	CCS_VALIDATE(data->configuration_space->obj.ops->serialize(
		 data->configuration_space, CCS_SERIALIZE_FORMAT_BINARY, buffer_size, buffer, opts));
	CCS_VALIDATE(data->objective_space->obj.ops->serialize(
		data->objective_space, CCS_SERIALIZE_FORMAT_BINARY, buffer_size, buffer, opts));
	CCS_VALIDATE(data->features_space->obj.ops->serialize(
		data->features_space, CCS_SERIALIZE_FORMAT_BINARY, buffer_size, buffer, opts));
	return CCS_SUCCESS;
}

#endif //_FEATURES_TUNER_INTERNAL_H
