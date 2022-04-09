#ifndef _FEATURES_TUNER_DESERIALIZE_H
#define _FEATURES_TUNER_DESERIALIZE_H
#include "features_tuner_internal.h"
#include "configuration_space_deserialize.h"
#include "objective_space_deserialize.h"
#include "features_space_deserialize.h"
#include "features_evaluation_deserialize.h"

struct _ccs_random_features_tuner_data_mock_s {
	_ccs_features_tuner_common_data_t  common_data;
	size_t                             size_history;
	size_t                             size_optimums;
	ccs_features_evaluation_t         *history;
	ccs_features_evaluation_t         *optimums;
};
typedef struct _ccs_random_features_tuner_data_mock_s _ccs_random_features_tuner_data_mock_t;

static inline ccs_result_t
_ccs_deserialize_bin_size_ccs_random_features_tuner_data(
		_ccs_random_features_tuner_data_mock_t  *data,
		uint32_t                                 version,
		size_t                                  *buffer_size,
		const char                             **buffer,
		_ccs_object_deserialize_options_t       *opts) {
	uint64_t num;
	uintptr_t mem;

	CCS_VALIDATE(_ccs_deserialize_bin_ccs_features_tuner_type(
		&data->common_data.type, buffer_size, buffer));
	CCS_VALIDATE(_ccs_deserialize_bin_string(
		&data->common_data.name, buffer_size, buffer));
	CCS_VALIDATE(_ccs_deserialize_bin_configuration_space(
		&data->common_data.configuration_space, version, buffer_size, buffer, opts));
	CCS_VALIDATE(_ccs_deserialize_bin_objective_space(
		&data->common_data.objective_space, version, buffer_size, buffer, opts));
	CCS_VALIDATE(_ccs_deserialize_bin_features_space(
		&data->common_data.features_space, version, buffer_size, buffer, opts));
	CCS_VALIDATE(_ccs_deserialize_bin_uint64(
		&num, buffer_size, buffer));
	data->size_history = num;
	CCS_VALIDATE(_ccs_deserialize_bin_uint64(
		&num, buffer_size, buffer));
	data->size_optimums = num;

	if (!(data->size_history + data->size_optimums))
		return CCS_SUCCESS;
	mem = (uintptr_t)calloc(
		(data->size_history + data->size_optimums), sizeof(ccs_features_evaluation_t));
	if (!mem)
		return -CCS_OUT_OF_MEMORY;

	data->history = (ccs_features_evaluation_t *)mem;
	mem += data->size_history * sizeof(ccs_features_evaluation_t);
	data->optimums = (ccs_features_evaluation_t *)mem;

	for (size_t i = 0; i < data->size_history; i++)
		CCS_VALIDATE(_ccs_deserialize_bin_features_evaluation(
			data->history + i, version, buffer_size, buffer, opts));

	for (size_t i = 0; i < data->size_optimums; i++)
		CCS_VALIDATE(_ccs_deserialize_bin_ccs_object(
			(ccs_object_t *)data->optimums + i, buffer_size, buffer));
	for (size_t i = 0; i < data->size_optimums; i++) {
		ccs_datum_t d;
		CCS_VALIDATE(ccs_map_get(
			opts->handle_map, ccs_object(data->optimums[i]), &d));
		if (CCS_UNLIKELY(d.type != CCS_OBJECT))
			return -CCS_INVALID_HANDLE;
		data->optimums[i] = (ccs_features_evaluation_t)(d.value.o);
	}
	return CCS_SUCCESS;
}

struct _ccs_random_features_tuner_data_clone_s {
	_ccs_features_tuner_common_data_t  common_data;
	UT_array                          *history;
	UT_array                          *optimums;
	UT_array                          *old_optimums;
};
typedef struct _ccs_random_features_tuner_data_clone_s _ccs_random_features_tuner_data_clone_t;

#undef  utarray_oom
#define utarray_oom() { \
	res = -CCS_OUT_OF_MEMORY; \
	goto features_tuner; \
}

static inline ccs_result_t
_ccs_deserialize_bin_random_features_tuner(
		_ccs_object_internal_t             *obj,
		ccs_features_tuner_t               *features_tuner_ret,
		uint32_t                            version,
		size_t                             *buffer_size,
		const char                        **buffer,
		_ccs_object_deserialize_options_t  *opts) {
	_ccs_random_features_tuner_data_mock_t data = {{(ccs_features_tuner_type_t)0, NULL, NULL, NULL, NULL}, 0, 0, NULL, NULL};
	_ccs_random_features_tuner_data_clone_t *odata = NULL;
	ccs_result_t res;
	CCS_VALIDATE_ERR_GOTO(res, _ccs_deserialize_bin_size_ccs_random_features_tuner_data(
		&data, version, buffer_size, buffer, opts), end);
	CCS_VALIDATE_ERR_GOTO(res, ccs_create_random_features_tuner(
		data.common_data.name, data.common_data.configuration_space, data.common_data.features_space, data.common_data.objective_space,
		obj->user_data, features_tuner_ret), features_evaluations);
	odata = (_ccs_random_features_tuner_data_clone_t *)((*features_tuner_ret)->data);
	for (size_t i = 0; i < data.size_history; i++)
		utarray_push_back(odata->history, data.history + i);
	for (size_t i = 0; i < data.size_optimums; i++)
		utarray_push_back(odata->optimums, data.optimums + i);

	res =  CCS_SUCCESS;
	goto end;
features_tuner:
	ccs_release_object(*features_tuner_ret);
	*features_tuner_ret = NULL;
features_evaluations:
	for (size_t i = 0; i < data.size_history; i++)
		ccs_release_object(data.history[i]);
end:
	if (data.common_data.configuration_space)
		ccs_release_object(data.common_data.configuration_space);
	if (data.common_data.objective_space)
		ccs_release_object(data.common_data.objective_space);
	if (data.common_data.features_space)
		ccs_release_object(data.common_data.features_space);
	if (data.history)
		free(data.history);
	return res;
}

#undef  utarray_oom

static inline ccs_result_t
_ccs_deserialize_bin_features_tuner(
		ccs_features_tuner_t                        *features_tuner_ret,
		uint32_t                            version,
		size_t                             *buffer_size,
		const char                        **buffer,
		_ccs_object_deserialize_options_t  *opts) {
	_ccs_object_deserialize_options_t new_opts = *opts;
	_ccs_object_internal_t obj;
	ccs_object_t handle;
	ccs_result_t res;
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_object_internal(
		&obj, buffer_size, buffer, &handle));
	if (CCS_UNLIKELY(obj.type != CCS_FEATURES_TUNER))
		return -CCS_INVALID_TYPE;

	ccs_features_tuner_type_t ttype;
	CCS_VALIDATE(_ccs_peek_bin_ccs_features_tuner_type(
		&ttype, buffer_size, buffer));

	new_opts.map_values = CCS_TRUE;
	CCS_VALIDATE(ccs_create_map(&new_opts.handle_map));

	switch (ttype) {
	case CCS_FEATURES_TUNER_RANDOM:
		CCS_VALIDATE_ERR_GOTO(res, _ccs_deserialize_bin_random_features_tuner(
			&obj, features_tuner_ret, version, buffer_size, buffer, &new_opts), end);
		break;
	default:
		return -CCS_UNSUPPORTED_OPERATION;
	}
	if (opts->handle_map)
		CCS_VALIDATE_ERR_GOTO(res,
			_ccs_object_handle_check_add(
				opts->handle_map, handle,
				(ccs_object_t)*features_tuner_ret),
			err_features_tuner);

	res = CCS_SUCCESS;
	goto end;
err_features_tuner:
	ccs_release_object(*features_tuner_ret);
end:
	ccs_release_object(new_opts.handle_map);
	return res;
}

static ccs_result_t
_ccs_features_tuner_deserialize(
		ccs_features_tuner_t               *features_tuner_ret,
		ccs_serialize_format_t              format,
		uint32_t                            version,
		size_t                             *buffer_size,
		const char                        **buffer,
		_ccs_object_deserialize_options_t  *opts) {
	switch(format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE(_ccs_deserialize_bin_features_tuner(
			features_tuner_ret, version, buffer_size, buffer, opts));
		break;
	default:
		return -CCS_INVALID_VALUE;
	}
	return CCS_SUCCESS;
}

#endif //_FEATURES_TUNER_DESERIALIZE_H
