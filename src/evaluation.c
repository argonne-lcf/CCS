#include "cconfigspace_internal.h"
#include "evaluation_internal.h"
#include <string.h>

static inline _ccs_evaluation_ops_t *
ccs_evaluation_get_ops(ccs_evaluation_t evaluation) {
	return (_ccs_evaluation_ops_t *)evaluation->obj.ops;
}

static ccs_error_t
_ccs_evaluation_del(ccs_object_t object) {
	ccs_evaluation_t evaluation = (ccs_evaluation_t)object;
	ccs_release_object(evaluation->data->objective_space);
	ccs_release_object(evaluation->data->configuration);
	return CCS_SUCCESS;
}

static _ccs_evaluation_ops_t _evaluation_ops =
    { {&_ccs_evaluation_del} };

ccs_error_t
ccs_create_evaluation(ccs_objective_space_t  objective_space,
                      ccs_configuration_t    configuration,
                      ccs_error_t            error,
                      size_t                 num_values,
                      ccs_datum_t           *values,
                      void                  *user_data,
                      ccs_evaluation_t      *evaluation_ret) {
	if (!evaluation_ret)
		return -CCS_INVALID_VALUE;
	if (num_values && !values)
		return -CCS_INVALID_VALUE;
	if (!num_values && values)
		return -CCS_INVALID_VALUE;
	ccs_error_t err;
	size_t num;
	err = ccs_objective_space_get_num_hyperparameters(objective_space, &num);
	if (err)
		return err;
	if (values && num != num_values)
		return -CCS_INVALID_VALUE;
	uintptr_t mem = (uintptr_t)calloc(1, sizeof(struct _ccs_evaluation_s) +
	                                     sizeof(struct _ccs_evaluation_data_s) +
	                                     num * sizeof(ccs_datum_t));
	if (!mem)
		return -CCS_ENOMEM;
	err = ccs_retain_object(objective_space);
	if (err) {
		free((void*)mem);
		return err;
	}
	err = ccs_retain_object(configuration);
	if (err) {
		free((void*)mem);
		return err;
	}
	ccs_evaluation_t eval = (ccs_evaluation_t)mem;
	_ccs_object_init(&(eval->obj), CCS_EVALUATION, (_ccs_object_ops_t*)&_evaluation_ops);
	eval->data = (struct _ccs_evaluation_data_s*)(mem + sizeof(struct _ccs_evaluation_s));
	eval->data->user_data = user_data;
	eval->data->num_values = num;
	eval->data->objective_space = objective_space;
	eval->data->configuration = configuration;
	eval->data->error = error;
	eval->data->values = (ccs_datum_t *)(mem + sizeof(struct _ccs_evaluation_s) + sizeof(struct _ccs_evaluation_data_s));
	if (values)
		memcpy(eval->data->values, values, num*sizeof(ccs_datum_t));
	*evaluation_ret = eval;
	return CCS_SUCCESS;
}

ccs_error_t
ccs_evaluation_get_objective_space(ccs_evaluation_t       evaluation,
                                   ccs_objective_space_t *objective_space_ret) {
	if (!evaluation || !evaluation->data)
		return -CCS_INVALID_OBJECT;
	if (!objective_space_ret)
		return -CCS_INVALID_VALUE;
	*objective_space_ret = evaluation->data->objective_space;
	return CCS_SUCCESS;
}

ccs_error_t
ccs_evaluation_get_configuration(ccs_evaluation_t     evaluation,
                                 ccs_configuration_t *configuration_ret) {
	if (!evaluation || !evaluation->data)
		return -CCS_INVALID_OBJECT;
	if (!configuration_ret)
		return -CCS_INVALID_VALUE;
	*configuration_ret = evaluation->data->configuration;
	return CCS_SUCCESS;
}

ccs_error_t
ccs_evaluation_get_user_data(ccs_evaluation_t   evaluation,
                             void             **user_data_ret) {
	if (!evaluation || !evaluation->data)
		return -CCS_INVALID_OBJECT;
	if (!user_data_ret)
		return -CCS_INVALID_VALUE;
	*user_data_ret = evaluation->data->user_data;
	return CCS_SUCCESS;
}

ccs_error_t
ccs_evaluation_get_error(ccs_evaluation_t   evaluation,
                         ccs_error_t      *error_ret) {
	if (!evaluation || !evaluation->data)
		return -CCS_INVALID_OBJECT;
	if (!error_ret)
		return -CCS_INVALID_VALUE;
	*error_ret = evaluation->data->error;
	return CCS_SUCCESS;
}

ccs_error_t
ccs_evaluation_set_error(ccs_evaluation_t evaluation,
                         ccs_error_t      error) {
	if (!evaluation || !evaluation->data)
		return -CCS_INVALID_OBJECT;
	evaluation->data->error = error;
	return CCS_SUCCESS;
}

ccs_error_t
ccs_evaluation_get_value(ccs_evaluation_t  evaluation,
                         size_t            index,
                         ccs_datum_t      *value_ret) {
	if (!evaluation || !evaluation->data)
		return -CCS_INVALID_OBJECT;
	if (!value_ret)
		return -CCS_INVALID_VALUE;
	if (index >= evaluation->data->num_values)
		return -CCS_OUT_OF_BOUNDS;
	*value_ret = evaluation->data->values[index];
	return CCS_SUCCESS;
}

ccs_error_t
ccs_evaluation_set_value(ccs_evaluation_t evaluation,
                         size_t           index,
                         ccs_datum_t      value) {
	if (!evaluation || !evaluation->data)
		return -CCS_INVALID_OBJECT;
	if (index >= evaluation->data->num_values)
		return -CCS_OUT_OF_BOUNDS;
	evaluation->data->values[index] = value;
	return CCS_SUCCESS;
}

ccs_error_t
ccs_evaluation_get_values(ccs_evaluation_t  evaluation,
                          size_t            num_values,
                          ccs_datum_t      *values,
                          size_t           *num_values_ret) {
	if (!evaluation || !evaluation->data)
		return -CCS_INVALID_OBJECT;
	if (!num_values_ret && !values)
		return -CCS_INVALID_VALUE;
	if (num_values && !values)
		return -CCS_INVALID_VALUE;
	if (!num_values && values)
		return -CCS_INVALID_VALUE;
	size_t num = evaluation->data->num_values;
	if (values) {
		if (num_values < num)
			return -CCS_INVALID_VALUE;
		memcpy(values, evaluation->data->values, num*sizeof(ccs_datum_t));
		for (size_t i = num; i < num_values; i++) {
			values[i].type = CCS_NONE;
			values[i].value.i = 0;
		}
	}
	if (num_values_ret)
		*num_values_ret = num;
	return CCS_SUCCESS;
}

ccs_error_t
ccs_evaluation_get_value_by_name(ccs_evaluation_t  evaluation,
                                 const char       *name,
                                 ccs_datum_t      *value_ret) {
	if (!evaluation || !evaluation->data)
		return -CCS_INVALID_OBJECT;
	if (!name)
		return -CCS_INVALID_VALUE;
	size_t index;
	ccs_error_t err;
	err = ccs_objective_space_get_hyperparameter_index_by_name(
		evaluation->data->objective_space, name, &index);
	if (err)
		return err;
	*value_ret = evaluation->data->values[index];
	return CCS_SUCCESS;
}
