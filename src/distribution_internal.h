struct _ccs_distribution_ops_s {
	_ccs_object_ops_t obj_ops;

	ccs_error_t (*get_type)(
		ccs_distribution_t distribution,
		ccs_distribution_type_t *type_ret);

	ccs_error_t (*get_data_type)(
		ccs_distribution_t       distribution,
		ccs_data_type_t         *data_type_ret);

	ccs_error_t (*get_scale_type)(
		ccs_distribution_t  distribution,
		ccs_scale_type_t   *scale_type_ret);

	ccs_error_t (*get_quantization)(
		ccs_distribution_t  distribution,
		ccs_datum_t        *quantization);

	ccs_error_t (*get_num_parameters)(
		ccs_distribution_t  distribution,
		size_t             *num_parameters_ret);

	ccs_error_t (*get_parameter_types)(
		ccs_distribution_t  distribution,
		size_t              num_parameter_types,
		ccs_data_type_t    *parameter_types);

	ccs_error_t (*get_parameters)(
		ccs_distribution_t  distribution,
		size_t              num_parameters,
		ccs_value_t        *parameters);

	ccs_error_t (*sample)(
		ccs_distribution_t  distribution,
		ccs_rng_t           rng,
		ccs_datum_t        *value);

	ccs_error_t (*samples)(
		ccs_distribution_t  distribution,
		ccs_rng_t           rng,
		size_t              num_values,
		ccs_datum_t        *values);
		
};
typedef struct _ccs_distribution_ops_s _ccs_distribution_ops_t;

struct _ccs_distribution_data_s;
typedef struct _ccs_distribution_data_s _ccs_distribution_data_t;

struct _ccs_distribution_s {
	_ccs_object_internal_t    obj;
	_ccs_distribution_data_t *data;
};
