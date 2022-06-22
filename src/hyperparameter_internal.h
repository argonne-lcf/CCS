#ifndef _HYPERPARAMETER_INTERNAL_H
#define _HYPERPARAMETER_INTERNAL_H

#define CCS_CHECK_HYPERPARAMETER(o, t) do { \
	CCS_CHECK_OBJ(o, CCS_HYPERPARAMETER); \
	CCS_REFUTE(((_ccs_hyperparameter_common_data_t*)(hyperparameter->data))->type != (t), CCS_INVALID_HYPERPARAMETER); \
} while (0)

struct _ccs_hyperparameter_data_s;
typedef struct _ccs_hyperparameter_data_s _ccs_hyperparameter_data_t;

struct _ccs_hyperparameter_ops_s {
	_ccs_object_ops_t obj_ops;

	ccs_error_t (*check_values)(
		_ccs_hyperparameter_data_t *data,
		size_t                      num_values,
		const ccs_datum_t          *values,
		ccs_datum_t                *values_ret,
		ccs_bool_t                 *results);

        ccs_error_t (*samples)(
		_ccs_hyperparameter_data_t *data,
		ccs_distribution_t          distribution,
		ccs_rng_t                   rng,
		size_t                      num_values,
		ccs_datum_t                *values);

	ccs_error_t (*get_default_distribution)(
		_ccs_hyperparameter_data_t *data,
		ccs_distribution_t         *distribution);

	ccs_error_t (*convert_samples)(
		_ccs_hyperparameter_data_t *data,
		ccs_bool_t                  oversampling,
		size_t                      num_values,
		const ccs_numeric_t        *values,
		ccs_datum_t                *results);
};
typedef struct _ccs_hyperparameter_ops_s _ccs_hyperparameter_ops_t;

struct _ccs_hyperparameter_s {
	_ccs_object_internal_t      obj;
	_ccs_hyperparameter_data_t *data;
};

struct _ccs_hyperparameter_common_data_s {
	ccs_hyperparameter_type_t  type;
	const char                *name;
	ccs_datum_t                default_value;
	ccs_interval_t             interval;
};

typedef struct _ccs_hyperparameter_common_data_s _ccs_hyperparameter_common_data_t;

static inline size_t
_ccs_serialize_bin_size_ccs_hyperparameter_common_data(
		_ccs_hyperparameter_common_data_t *data) {
	return _ccs_serialize_bin_size_ccs_hyperparameter_type(data->type) +
	       _ccs_serialize_bin_size_string(data->name) +
	       _ccs_serialize_bin_size_ccs_datum(data->default_value) +
	       _ccs_serialize_bin_size_ccs_interval(&data->interval);
}

static inline ccs_error_t
_ccs_serialize_bin_ccs_hyperparameter_common_data(
		_ccs_hyperparameter_common_data_t  *data,
		size_t                             *buffer_size,
		char                              **buffer) {
	CCS_VALIDATE(_ccs_serialize_bin_ccs_hyperparameter_type(data->type, buffer_size, buffer));
	CCS_VALIDATE(_ccs_serialize_bin_string(data->name, buffer_size, buffer));
	CCS_VALIDATE(_ccs_serialize_bin_ccs_datum(data->default_value, buffer_size, buffer));
	CCS_VALIDATE(_ccs_serialize_bin_ccs_interval(&data->interval, buffer_size, buffer));
	return CCS_SUCCESS;
}

static inline ccs_error_t
_ccs_deserialize_bin_ccs_hyperparameter_common_data(
		_ccs_hyperparameter_common_data_t  *data,
		size_t                             *buffer_size,
		const char                        **buffer) {
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_hyperparameter_type(&data->type, buffer_size, buffer));
	CCS_VALIDATE(_ccs_deserialize_bin_string(&data->name, buffer_size, buffer));
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_datum(&data->default_value, buffer_size, buffer));
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_interval(&data->interval, buffer_size, buffer));
	return CCS_SUCCESS;
}

struct _ccs_hyperparameter_numerical_data_s {
	_ccs_hyperparameter_common_data_t common_data;
	ccs_numeric_t                     quantization;
};
typedef struct _ccs_hyperparameter_numerical_data_s _ccs_hyperparameter_numerical_data_t;

static inline size_t
_ccs_serialize_bin_size_ccs_hyperparameter_numerical_data(
		_ccs_hyperparameter_numerical_data_t *data) {
	return _ccs_serialize_bin_size_ccs_hyperparameter_common_data(&data->common_data) +
	       (data->common_data.interval.type == CCS_NUM_FLOAT ?
			_ccs_serialize_bin_size_ccs_float(data->quantization.f) :
			_ccs_serialize_bin_size_ccs_int(data->quantization.i));
}

static inline ccs_error_t
_ccs_serialize_bin_ccs_hyperparameter_numerical_data(
		_ccs_hyperparameter_numerical_data_t  *data,
		size_t                                *buffer_size,
		char                                 **buffer) {
	CCS_VALIDATE(_ccs_serialize_bin_ccs_hyperparameter_common_data(
		&data->common_data, buffer_size, buffer));
	if (data->common_data.interval.type == CCS_NUM_FLOAT)
		CCS_VALIDATE(_ccs_serialize_bin_ccs_float(
			data->quantization.f, buffer_size, buffer));
	else
		CCS_VALIDATE(_ccs_serialize_bin_ccs_int(
			data->quantization.i, buffer_size, buffer));
	return CCS_SUCCESS;
}

static inline ccs_error_t
_ccs_deserialize_bin_ccs_hyperparameter_numerical_data(
		_ccs_hyperparameter_numerical_data_t  *data,
		size_t                                *buffer_size,
		const char                           **buffer) {
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_hyperparameter_common_data(
		&data->common_data, buffer_size, buffer));
	if (data->common_data.interval.type == CCS_NUM_FLOAT)
		CCS_VALIDATE(_ccs_deserialize_bin_ccs_float(
			&data->quantization.f, buffer_size, buffer));
	else
		CCS_VALIDATE(_ccs_deserialize_bin_ccs_int(
			&data->quantization.i, buffer_size, buffer));
	return CCS_SUCCESS;
}

#endif //_HYPERPARAMETER_INTERNAL_H
