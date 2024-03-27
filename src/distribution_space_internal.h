#ifndef _DISTRIBUTION_SPACE_INTERNAL_H
#define _DISTRIBUTION_SPACE_INTERNAL_H
#include "configuration_space_internal.h"
#include "utlist.h"

struct _ccs_distribution_wrapper_s {
	ccs_distribution_t           distribution;
	size_t                       dimension;
	size_t                      *parameter_indexes;
	_ccs_distribution_wrapper_t *prev;
	_ccs_distribution_wrapper_t *next;
};
typedef struct _ccs_distribution_wrapper_s _ccs_distribution_wrapper_t;

struct _ccs_parameter_distribution_s {
	size_t                       distribution_index;
	_ccs_distribution_wrapper_t *distribution;
};
typedef struct _ccs_parameter_distribution_s _ccs_parameter_distribution_t;

struct _ccs_distribution_space_ops_s {
	_ccs_object_ops_t obj_ops;
};
typedef struct _ccs_distribution_space_ops_s _ccs_distribution_space_ops_t;

struct _ccs_distribution_space_data_s;
typedef struct _ccs_distribution_space_data_s _ccs_distribution_space_data_t;

struct _ccs_distribution_space_s {
	_ccs_object_internal_t          obj;
	_ccs_distribution_space_data_t *data;
};

struct _ccs_distribution_space_data_s {
	ccs_configuration_space_t      configuration_space;
	size_t                         num_parameters;
	_ccs_parameter_distribution_t *parameter_distributions;
	_ccs_distribution_wrapper_t   *distribution_list;
};

static inline void
_ccs_distribution_space_del_no_release(
	ccs_distribution_space_t distribution_space)
{
	_ccs_distribution_wrapper_t *dw, *tmp;
	DL_FOREACH_SAFE(distribution_space->data->distribution_list, dw, tmp)
	{
		DL_DELETE(distribution_space->data->distribution_list, dw);
		ccs_release_object(dw->distribution);
		free(dw);
	}
}

static inline ccs_result_t
_ccs_get_distribution_wrapper(
	ccs_configuration_space_t     configuration_space,
	size_t                        index,
	_ccs_distribution_wrapper_t **distrib_wrapper_ret)
{
	ccs_parameter_t              parameter;
	ccs_distribution_t           distribution;
	ccs_result_t                 err = CCS_RESULT_SUCCESS;
	_ccs_distribution_wrapper_t *distrib_wrapper;
	CCS_VALIDATE(_ccs_context_get_parameter(
		(ccs_context_t)configuration_space, index, &parameter));
	CCS_VALIDATE(ccs_parameter_get_default_distribution(
		parameter, &distribution));

	uintptr_t dmem = (uintptr_t)malloc(
		sizeof(_ccs_distribution_wrapper_t) + sizeof(size_t));

	CCS_REFUTE_ERR_GOTO(
		err, !dmem, CCS_RESULT_ERROR_OUT_OF_MEMORY, err_distrib);
	distrib_wrapper               = (_ccs_distribution_wrapper_t *)dmem;
	distrib_wrapper->distribution = distribution;
	distrib_wrapper->dimension    = 1;
	distrib_wrapper->parameter_indexes =
		(size_t *)(dmem + sizeof(_ccs_distribution_wrapper_t));
	distrib_wrapper->parameter_indexes[0] = index;
	*distrib_wrapper_ret                  = distrib_wrapper;
	return CCS_RESULT_SUCCESS;
err_distrib:
	ccs_release_object(distribution);
	return err;
}

static inline ccs_result_t
_ccs_create_distribution_space_no_retain(
	ccs_configuration_space_t      configuration_space,
	_ccs_distribution_space_ops_t *ops,
	ccs_distribution_space_t      *distribution_space_ret)
{
	size_t                       num_parameters;
	_ccs_distribution_wrapper_t *dw, *tmp;
	ccs_result_t                 err = CCS_RESULT_SUCCESS;
	CCS_VALIDATE(_ccs_context_get_num_parameters(
		(ccs_context_t)configuration_space, &num_parameters));
	uintptr_t mem = (uintptr_t)calloc(
		1, sizeof(struct _ccs_distribution_space_s) +
			   sizeof(struct _ccs_distribution_space_data_s) +
			   sizeof(struct _ccs_parameter_distribution_s) *
				   num_parameters);
	CCS_REFUTE(!mem, CCS_RESULT_ERROR_OUT_OF_MEMORY);

	ccs_distribution_space_t distrib_space;
	distrib_space = (ccs_distribution_space_t)mem;
	if (ops) {
		_ccs_object_init(
			&(distrib_space->obj),
			CCS_OBJECT_TYPE_DISTRIBUTION_SPACE,
			(_ccs_object_ops_t *)ops);
	}
	distrib_space->data =
		(struct _ccs_distribution_space_data_s
			 *)(mem + sizeof(struct _ccs_distribution_space_s));
	distrib_space->data->num_parameters = num_parameters;
	distrib_space->data->parameter_distributions =
		(struct _ccs_parameter_distribution_s
			 *)(mem + sizeof(struct _ccs_distribution_space_s) + sizeof(struct _ccs_distribution_space_data_s));
	distrib_space->data->configuration_space = configuration_space;
	for (size_t i = 0; i < num_parameters; i++) {
		_ccs_distribution_wrapper_t   *distrib_wrapper;
		_ccs_parameter_distribution_t *pdist;
		CCS_VALIDATE_ERR_GOTO(
			err,
			_ccs_get_distribution_wrapper(
				configuration_space, i, &distrib_wrapper),
			err_dis);
		DL_APPEND(
			distrib_space->data->distribution_list,
			distrib_wrapper);
		pdist = distrib_space->data->parameter_distributions + i;
		pdist->distribution_index = 0;
		pdist->distribution       = distrib_wrapper;
	}
	*distribution_space_ret = distrib_space;
	return CCS_RESULT_SUCCESS;
err_dis:
	DL_FOREACH_SAFE(distrib_space->data->distribution_list, dw, tmp)
	{
		DL_DELETE(distrib_space->data->distribution_list, dw);
		ccs_release_object(dw->distribution);
		free(dw);
	}
	free((void *)mem);
	return err;
}

#endif //_DISTRIBUTION_SPACE_INTERNAL_H
