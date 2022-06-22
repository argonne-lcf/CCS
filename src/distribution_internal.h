#ifndef _DISTRIBUTION_INTERNAL_H
#define _DISTRIBUTION_INTERNAL_H

#define CCS_CHECK_DISTRIBUTION(o, t) do { \
	CCS_CHECK_OBJ(o, CCS_DISTRIBUTION); \
	CCS_REFUTE(((_ccs_distribution_common_data_t*)distribution->data)->type != (t), CCS_INVALID_DISTRIBUTION); \
} while (0)

struct _ccs_distribution_data_s;
typedef struct _ccs_distribution_data_s _ccs_distribution_data_t;

struct _ccs_distribution_ops_s {
	_ccs_object_ops_t obj_ops;

	ccs_error_t (*samples)(
		_ccs_distribution_data_t *distribution,
		ccs_rng_t                 rng,
		size_t                    num_values,
		ccs_numeric_t            *values);

	ccs_error_t (*get_bounds)(
		_ccs_distribution_data_t *distribution,
		ccs_interval_t           *interval_ret);

	ccs_error_t (*strided_samples)(
		_ccs_distribution_data_t *distribution,
		ccs_rng_t                 rng,
		size_t                    num_values,
		size_t                    stride,
		ccs_numeric_t            *values);

	ccs_error_t (*soa_samples)(
		_ccs_distribution_data_t  *distribution,
		ccs_rng_t                  rng,
		size_t                     num_values,
		ccs_numeric_t            **values);
};
typedef struct _ccs_distribution_ops_s _ccs_distribution_ops_t;

struct _ccs_distribution_s {
	_ccs_object_internal_t    obj;
	_ccs_distribution_data_t *data;
};

struct _ccs_distribution_common_data_s {
	ccs_distribution_type_t  type;
	size_t                   dimension;
	ccs_numeric_type_t      *data_types;
};
typedef struct _ccs_distribution_common_data_s _ccs_distribution_common_data_t;

static inline ccs_int_t
ccs_dichotomic_search(ccs_int_t size, ccs_float_t *values, ccs_float_t target) {
	ccs_int_t upper = size - 1;
	ccs_int_t lower = 0;
	ccs_int_t index = upper * target;
	int found = 0;
	while( !found ) {
		if (target < values[index]) {
			upper = index - 1;
			index = (lower+upper)/2;
		} else if (target >= values[index+1]) {
			lower = index + 1;
			index = (lower+upper)/2;
		} else
			found = 1;
	}
	return index;
}

static inline size_t
_ccs_serialize_bin_size_ccs_distribution_common_data(
		_ccs_distribution_common_data_t *data) {
	return _ccs_serialize_bin_size_ccs_distribution_type(data->type);
}

static inline ccs_error_t
_ccs_serialize_bin_ccs_distribution_common_data(
		_ccs_distribution_common_data_t  *data,
		size_t                             *buffer_size,
		char                              **buffer) {
	CCS_VALIDATE(_ccs_serialize_bin_ccs_distribution_type(data->type, buffer_size, buffer));
	return CCS_SUCCESS;
}

static inline ccs_error_t
_ccs_deserialize_bin_ccs_distribution_common_data(
		_ccs_distribution_common_data_t  *data,
		size_t                             *buffer_size,
		const char                        **buffer) {
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_distribution_type(&data->type, buffer_size, buffer));
	return CCS_SUCCESS;
}

#endif //_DISTRIBUTION_INTERNAL_H
