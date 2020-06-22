#ifndef _TUNER_INTERNAL_H
#define _TUNER_INTERNAL_H

struct _ccs_tuner_data_s;
typedef struct _ccs_tuner_data_s _ccs_tuner_data_t;

struct _ccs_tuner_ops_s {
	_ccs_object_ops_t obj_ops;

	ccs_result_t (*ask)(
		_ccs_tuner_data_t   *data,
		size_t               num_configurations,
		ccs_configuration_t *configurations,
		size_t              *num_configurations_ret);

	ccs_result_t (*tell)(
		_ccs_tuner_data_t *data,
		size_t             num_evaluations,
		ccs_evaluation_t  *evaluations);

	ccs_result_t (*get_optimums)(
		_ccs_tuner_data_t *data,
		size_t             num_evaluations,
		ccs_evaluation_t  *evaluations,
		size_t            *num_evaluations_ret);

	ccs_result_t (*get_history)(
		_ccs_tuner_data_t *data,
		size_t             num_evaluations,
		ccs_evaluation_t  *evaluations,
		size_t            *num_evaluations_ret);
};
typedef struct _ccs_tuner_ops_s _ccs_tuner_ops_t;

struct _ccs_tuner_s {
	_ccs_object_internal_t  obj;
	_ccs_tuner_data_t      *data;
};

#endif //_TUNER_INTERNAL_H
