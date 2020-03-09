#ifndef _DISTRIBUTION_INTERNAL_H
#define _DISTRIBUTION_INTERNAL_H

struct _ccs_distribution_data_s;
typedef struct _ccs_distribution_data_s _ccs_distribution_data_t;

struct _ccs_distribution_ops_s {
	_ccs_object_ops_t obj_ops;

	ccs_error_t (*get_num_parameters)(
		_ccs_distribution_data_t *distribution,
		size_t                   *num_parameters_ret);

	ccs_error_t (*get_parameters)(
		_ccs_distribution_data_t *distribution,
		size_t                    num_parameters,
		ccs_datum_t              *parameters,
		size_t                   *num_parameters_ret);

	ccs_error_t (*samples)(
		_ccs_distribution_data_t *distribution,
		ccs_rng_t                 rng,
		size_t                    num_values,
		ccs_value_t              *values);
		
};
typedef struct _ccs_distribution_ops_s _ccs_distribution_ops_t;

struct _ccs_distribution_s {
	_ccs_object_internal_t    obj;
	_ccs_distribution_data_t *data;
};

struct _ccs_distribution_common_data_s {
        ccs_distribution_type_t type;
	ccs_data_type_t         data_type;
	ccs_scale_type_t        scale_type;
	ccs_value_t             quantization;
};
typedef struct _ccs_distribution_common_data_s _ccs_distribution_common_data_t;
#endif //_DISTRIBUTION_INTERNAL_H
