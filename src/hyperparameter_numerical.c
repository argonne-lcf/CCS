#include "cconfigspace_internal.h"
#include "hyperparameter_internal.h"

struct _ccs_hyperparameter_numerical_data_s {
	_ccs_hyperparameter_common_data_t common_data;
	ccs_data_type_t                   data_type;
	ccs_value_t                       lower;
	ccs_value_t                       upper;
	ccs_value_t                       quantization;
};
typedef struct _ccs_hyperparameter_numerical_data_s _ccs_hyperparameter_numerical_data_t;

static ccs_error_t
_ccs_hyperparameter_numerical_del(ccs_object_t o) {
	ccs_hyperparameter_t d = o.hyperparameter;
	_ccs_hyperparameter_numerical_data_t *data = (_ccs_hyperparameter_numerical_data_t *)(d->data);
	return ccs_release_object(data->common_data.distribution);
}

static ccs_error_t
_ccs_hyperparameter_numerical_samples(_ccs_hyperparameter_data_t *data,
                                      ccs_rng_t                   rng,
                                      size_t                      num_values,
                                      ccs_datum_t                *values) {
	return CCS_SUCCESS;
}

_ccs_hyperparameter_ops_t _ccs_hyperparameter_numerical_ops = {
	{ &_ccs_hyperparameter_numerical_del },
	&_ccs_hyperparameter_numerical_samples
};

ccs_error_t
_ccs_create_numerical_hyperparameter(const char           *name,
                                     ccs_data_type_t       data_type,
                                     ccs_value_t           lower,
                                     ccs_value_t           upper,
                                     ccs_value_t           quantization,
                                     ccs_value_t           default_value,
                                     ccs_distribution_t    distribution,
                                     void                 *user_data,
                                     ccs_hyperparameter_t *hyperparameter_ret) {
	if (!hyperparameter_ret)
		return -CCS_INVALID_VALUE;
	if (data_type != CCS_FLOAT && data_type != CCS_INTEGER)
		return -CCS_INVALID_TYPE;
	if (data_type == CCS_INTEGER && (
		lower.i >= upper.i ||
		quantization.i < 0 ||
		quantization.i > upper.i - lower.i ||
		default_value.i < lower.i ||
		default_value.i >= upper.i ) )
		return -CCS_INVALID_VALUE;
	if (data_type == CCS_FLOAT && (
		lower.f >= upper.f ||
		quantization.f < 0.0 ||
		quantization.f > upper.f - lower.f ||
		default_value.f < lower.f ||
		default_value.f >= upper.f ) )
		return -CCS_INVALID_VALUE;
	uintptr_t mem = (uintptr_t)calloc(1, sizeof(struct _ccs_hyperparameter_s) + sizeof(_ccs_hyperparameter_numerical_data_t));

	if (!mem)
		return -CCS_ENOMEM;
	if (!distribution) {
		ccs_error_t err;
		err = ccs_create_uniform_distribution(data_type, lower, upper,
                                                      CCS_LINEAR, quantization,
                                                      &distribution);
		if (err) {
			free((void *)mem);
			return err;
		}
	} 
	ccs_hyperparameter_t hyperparam = (ccs_hyperparameter_t)mem;
	_ccs_object_init(&(hyperparam->obj), CCS_HYPERPARAMETER, (_ccs_object_ops_t *)&_ccs_hyperparameter_numerical_ops);
	_ccs_hyperparameter_numerical_data_t *hyperparam_data = (_ccs_hyperparameter_numerical_data_t *)(mem + sizeof(struct _ccs_hyperparameter_s));
	hyperparam_data->common_data.type = CCS_NUMERICAL;
	hyperparam_data->common_data.name = name;
	hyperparam_data->common_data.user_data = user_data;
	hyperparam_data->common_data.distribution = distribution;
	hyperparam_data->common_data.default_value.type = data_type;
	hyperparam_data->common_data.default_value.value = default_value;
	hyperparam_data->data_type = data_type;
	hyperparam_data->lower = lower;
	hyperparam_data->upper = upper;
	hyperparam_data->quantization = quantization;
	hyperparam->data = (_ccs_hyperparameter_data_t *)hyperparam_data;
	*hyperparameter_ret = hyperparam;
	return CCS_SUCCESS;
}
