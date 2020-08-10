#ifndef _DISTRIBUTION_INTERNAL_H
#define _DISTRIBUTION_INTERNAL_H

struct _ccs_distribution_data_s;
typedef struct _ccs_distribution_data_s _ccs_distribution_data_t;

struct _ccs_distribution_ops_s {
	_ccs_object_ops_t obj_ops;

	ccs_result_t (*samples)(
		_ccs_distribution_data_t *distribution,
		ccs_rng_t                 rng,
		size_t                    num_values,
		ccs_numeric_t            *values);

	ccs_result_t (*get_bounds)(
		_ccs_distribution_data_t *distribution,
		ccs_interval_t           *interval_ret);

	ccs_result_t (*strided_samples)(
		_ccs_distribution_data_t *distribution,
		ccs_rng_t                 rng,
		size_t                    num_values,
		size_t                    stride,
		ccs_numeric_t            *values);

	ccs_result_t (*soa_samples)(
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
#endif //_DISTRIBUTION_INTERNAL_H
