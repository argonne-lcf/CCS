#ifndef _CONFIGURATION_SPACE_DESERIALIZE_H
#define _CONFIGURATION_SPACE_DESERIALIZE_H
#include "context_deserialize.h"
#include "expression_deserialize.h"
#include "distribution_deserialize.h"
#include "rng_deserialize.h"

struct _ccs_configuration_space_data_mock_s {
	const char           *name;
	size_t                num_hyperparameters;
	size_t                num_conditions;
	size_t                num_distributions;
	size_t                num_forbidden_clauses;
	ccs_rng_t             rng;
	ccs_hyperparameter_t *hyperparameters;
	size_t               *cond_hyperparameter_indices;
	ccs_expression_t     *conditions;
	ccs_distribution_t   *distributions;
	size_t               *dimensions;
	size_t               *distrib_hyperparameter_indices;
	ccs_expression_t     *forbidden_clauses;
};
typedef struct _ccs_configuration_space_data_mock_s _ccs_configuration_space_data_mock_t;

static inline ccs_result_t
_ccs_deserialize_bin_ccs_configuration_space_data(
		_ccs_configuration_space_data_mock_t  *data,
		uint32_t                               version,
		size_t                                *buffer_size,
		const char                           **buffer,
		_ccs_object_deserialize_options_t     *opts) {
	uint64_t num;
	uintptr_t mem;

	CCS_VALIDATE(_ccs_deserialize_bin_string(
		&data->name, buffer_size, buffer));
	CCS_VALIDATE(_ccs_deserialize_bin_uint64(
		&num, buffer_size, buffer));
	data->num_hyperparameters = num;
	CCS_VALIDATE(_ccs_deserialize_bin_uint64(
		&num, buffer_size, buffer));
	data->num_conditions = num;
	CCS_VALIDATE(_ccs_deserialize_bin_uint64(
		&num, buffer_size, buffer));
	data->num_distributions = num;
	CCS_VALIDATE(_ccs_deserialize_bin_uint64(
		&num, buffer_size, buffer));
	data->num_forbidden_clauses = num;
	CCS_VALIDATE(_ccs_rng_deserialize(
		&data->rng, CCS_SERIALIZE_FORMAT_BINARY, version, buffer_size, buffer, opts));

	if (!(data->num_hyperparameters + data->num_conditions + data->num_distributions + data->num_forbidden_clauses))
		return CCS_SUCCESS;
	mem = (uintptr_t)calloc(
		data->num_hyperparameters * (sizeof(ccs_hyperparameter_t) + sizeof(size_t)) +
		data->num_conditions * (sizeof(ccs_expression_t) + sizeof(size_t)) +
		data->num_distributions * (sizeof(ccs_distribution_t) + sizeof(size_t)) +
		data->num_forbidden_clauses * sizeof(ccs_expression_t), 1);
	CCS_REFUTE(!mem, CCS_OUT_OF_MEMORY);

	data->hyperparameters = (ccs_hyperparameter_t *)mem;
	mem += data->num_hyperparameters * sizeof(ccs_hyperparameter_t);
	data->cond_hyperparameter_indices = (size_t *)mem;
	mem += data->num_conditions * sizeof(size_t);
	data->conditions = (ccs_expression_t *)mem;
	mem += data->num_conditions * sizeof(ccs_expression_t);
	data->distributions = (ccs_distribution_t *)mem;
	mem += data->num_distributions * sizeof(ccs_distribution_t);
	data->dimensions = (size_t *)mem;
	mem += data->num_distributions * sizeof(size_t);
	data->distrib_hyperparameter_indices = (size_t *)mem;
	mem += data->num_hyperparameters * sizeof(size_t);
	data->forbidden_clauses = (ccs_expression_t *)mem;

	for (size_t i = 0; i < data->num_hyperparameters; i++)
		CCS_VALIDATE(_ccs_hyperparameter_deserialize(
			data->hyperparameters + i, CCS_SERIALIZE_FORMAT_BINARY, version, buffer_size, buffer, opts));

	for (size_t i = 0; i < data->num_conditions; i++) {
		uint64_t index;
		CCS_VALIDATE(_ccs_deserialize_bin_uint64(
			&index, buffer_size, buffer));
		data->cond_hyperparameter_indices[i] = index;
		CCS_VALIDATE(_ccs_expression_deserialize(
			data->conditions + i, CCS_SERIALIZE_FORMAT_BINARY, version, buffer_size, buffer, opts));
	}

	size_t *indices;
	indices = data->distrib_hyperparameter_indices;
	for (size_t i = 0; i < data->num_distributions; i++) {
		uint64_t dimension;
		CCS_VALIDATE(_ccs_distribution_deserialize(
			data->distributions + i, CCS_SERIALIZE_FORMAT_BINARY, version, buffer_size, buffer, opts));
		CCS_VALIDATE(_ccs_deserialize_bin_uint64(
			&dimension, buffer_size, buffer));
		data->dimensions[i] = dimension;
		for (size_t j = 0; j < dimension; j++) {
			uint64_t index;
			CCS_VALIDATE(_ccs_deserialize_bin_uint64(
				&index, buffer_size, buffer));
			*indices++ = index;
		}
	}

	for (size_t i = 0; i < data->num_forbidden_clauses; i++) {
		CCS_VALIDATE(_ccs_expression_deserialize(
			data->forbidden_clauses + i, CCS_SERIALIZE_FORMAT_BINARY, version, buffer_size, buffer, opts));
	}

	return CCS_SUCCESS;
}

static inline ccs_result_t
_ccs_deserialize_bin_configuration_space(
		ccs_configuration_space_t          *configuration_space_ret,
		uint32_t                            version,
		size_t                             *buffer_size,
		const char                        **buffer,
		_ccs_object_deserialize_options_t  *opts) {
	_ccs_object_deserialize_options_t new_opts = *opts;
	_ccs_object_internal_t obj;
	ccs_object_t handle;
	ccs_result_t res = CCS_SUCCESS;
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_object_internal(
		&obj, buffer_size, buffer, &handle));
	CCS_REFUTE(obj.type != CCS_CONFIGURATION_SPACE, CCS_INVALID_TYPE);

	new_opts.map_values = CCS_TRUE;
	CCS_VALIDATE(ccs_create_map(&new_opts.handle_map));

	_ccs_configuration_space_data_mock_t data = {NULL, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
	CCS_VALIDATE_ERR_GOTO(res, _ccs_deserialize_bin_ccs_configuration_space_data(
		&data, version, buffer_size, buffer, &new_opts), end);
	CCS_VALIDATE_ERR_GOTO(res, ccs_create_configuration_space(
		data.name, configuration_space_ret), end);
	CCS_VALIDATE_ERR_GOTO(res, ccs_configuration_space_set_rng(
		*configuration_space_ret, data.rng), end);
	CCS_VALIDATE_ERR_GOTO(res, ccs_configuration_space_add_hyperparameters(
		*configuration_space_ret, data.num_hyperparameters, data.hyperparameters, NULL),
		err_configuration_space);
	for (size_t i = 0; i < data.num_conditions; i++)
		CCS_VALIDATE_ERR_GOTO(res, ccs_configuration_space_set_condition(
			*configuration_space_ret, data.cond_hyperparameter_indices[i],
			data.conditions[i]), err_configuration_space);
	size_t *indices;
	indices = data.distrib_hyperparameter_indices;
	for (size_t i = 0; i < data.num_distributions; i++) {
		CCS_VALIDATE_ERR_GOTO(res, ccs_configuration_space_set_distribution(
			*configuration_space_ret, data.distributions[i], indices),
			err_configuration_space);
		indices += data.dimensions[i];
	}
	CCS_VALIDATE_ERR_GOTO(res, ccs_configuration_space_add_forbidden_clauses(
		*configuration_space_ret, data.num_forbidden_clauses, data.forbidden_clauses),
		err_configuration_space);
	if (opts && opts->map_values && opts->handle_map)
		CCS_VALIDATE_ERR_GOTO(res,
			_ccs_object_handle_check_add(
				opts->handle_map, handle,
				(ccs_object_t)*configuration_space_ret),
			err_configuration_space);
	goto end;

err_configuration_space:
	ccs_release_object(*configuration_space_ret);
	*configuration_space_ret = NULL;
end:
	if (data.rng)
		ccs_release_object(data.rng);
	if (data.hyperparameters)
		for (size_t i = 0; i < data.num_hyperparameters; i++)
			if(data.hyperparameters[i])
				ccs_release_object(data.hyperparameters[i]);
	if (data.conditions)
		for (size_t i = 0; i < data.num_conditions; i++)
			if(data.conditions[i])
				ccs_release_object(data.conditions[i]);
	if (data.distributions)
		for (size_t i = 0; i < data.num_distributions; i++)
			if(data.distributions[i])
				ccs_release_object(data.distributions[i]);
	if (data.forbidden_clauses)
		for (size_t i = 0; i < data.num_forbidden_clauses; i++)
			if(data.forbidden_clauses[i])
				ccs_release_object(data.forbidden_clauses[i]);
	if (data.hyperparameters)
		free(data.hyperparameters);
	ccs_release_object(new_opts.handle_map);
	return res;
}

static ccs_result_t
_ccs_configuration_space_deserialize(
		ccs_configuration_space_t          *configuration_space_ret,
		ccs_serialize_format_t              format,
		uint32_t                            version,
		size_t                             *buffer_size,
		const char                        **buffer,
		_ccs_object_deserialize_options_t  *opts) {
	switch(format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE(_ccs_deserialize_bin_configuration_space(
			configuration_space_ret, version, buffer_size, buffer, opts));
		break;
	default:
		CCS_RAISE(CCS_INVALID_VALUE, "Unsupported serialization format: %d", format);
	}
	CCS_VALIDATE(_ccs_object_deserialize_user_data(
		(ccs_object_t)*configuration_space_ret, format, version, buffer_size, buffer, opts));
	return CCS_SUCCESS;
}

#endif //_CONFIGURATION_SPACE_DESERIALIZE_H
