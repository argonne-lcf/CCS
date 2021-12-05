#ifndef _HYPERPARAMETER_INTERNAL_H
#define _HYPERPARAMETER_INTERNAL_H

#define CCS_CHECK_HYPERPARAMETER(o, t) do { \
	CCS_CHECK_OBJ(o, CCS_HYPERPARAMETER); \
	if (CCS_UNLIKELY(((_ccs_hyperparameter_common_data_t*)(hyperparameter->data))->type != (t))) \
		return -CCS_INVALID_HYPERPARAMETER; \
} while (0)

struct _ccs_hyperparameter_data_s;
typedef struct _ccs_hyperparameter_data_s _ccs_hyperparameter_data_t;

struct _ccs_hyperparameter_ops_s {
	_ccs_object_ops_t obj_ops;

	ccs_result_t (*check_values)(
		_ccs_hyperparameter_data_t *data,
		size_t                      num_values,
		const ccs_datum_t          *values,
		ccs_datum_t                *values_ret,
		ccs_bool_t                 *results);

        ccs_result_t (*samples)(
		_ccs_hyperparameter_data_t *data,
		ccs_distribution_t          distribution,
		ccs_rng_t                   rng,
		size_t                      num_values,
		ccs_datum_t                *values);

	ccs_result_t (*get_default_distribution)(
		_ccs_hyperparameter_data_t *data,
		ccs_distribution_t         *distribution);

	ccs_result_t (*convert_samples)(
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
	void                      *user_data;
	ccs_datum_t                default_value;
	ccs_interval_t             interval;
};

typedef struct _ccs_hyperparameter_common_data_s _ccs_hyperparameter_common_data_t;
#endif //_HYPERPARAMETER_INTERNAL_H
