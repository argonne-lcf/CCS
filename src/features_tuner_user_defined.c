#include "cconfigspace_internal.h"
#include "features_tuner_internal.h"
#include "string.h"

struct _ccs_user_defined_features_tuner_data_s {
	_ccs_features_tuner_common_data_t         common_data;
	ccs_features_tuner_t                      selfref;
	ccs_user_defined_features_tuner_vector_t  vector;
	void                                     *tuner_data;
};
typedef struct _ccs_user_defined_features_tuner_data_s _ccs_user_defined_features_tuner_data_t;

static ccs_result_t
_ccs_features_tuner_user_defined_del(ccs_object_t o) {
	_ccs_user_defined_features_tuner_data_t *d =
	    (_ccs_user_defined_features_tuner_data_t *)((ccs_features_tuner_t)o)->data;
	ccs_result_t err;
	err = d->vector.del((ccs_features_tuner_t)o);
	ccs_release_object(d->common_data.configuration_space);
	ccs_release_object(d->common_data.objective_space);
	ccs_release_object(d->common_data.features_space);
	return err;
}

static ccs_result_t
_ccs_features_tuner_user_defined_ask(_ccs_features_tuner_data_t *data,
                                     ccs_features_t              features,
                                     size_t                      num_configurations,
                                     ccs_configuration_t        *configurations,
                                     size_t                     *num_configurations_ret) {
	_ccs_user_defined_features_tuner_data_t *d =
		(_ccs_user_defined_features_tuner_data_t *)data;
	return d->vector.ask(d->selfref, features, num_configurations, configurations, num_configurations_ret);
}

static ccs_result_t
_ccs_features_tuner_user_defined_tell(_ccs_features_tuner_data_t *data,
                                      size_t                      num_evaluations,
                                      ccs_features_evaluation_t  *evaluations) {
	_ccs_user_defined_features_tuner_data_t *d =
		(_ccs_user_defined_features_tuner_data_t *)data;
	return d->vector.tell(d->selfref, num_evaluations, evaluations);
}

static ccs_result_t
_ccs_features_tuner_user_defined_get_optimums(
		_ccs_features_tuner_data_t *data,
		size_t                      num_evaluations,
		ccs_features_evaluation_t  *evaluations,
		size_t                     *num_evaluations_ret) {
	_ccs_user_defined_features_tuner_data_t *d =
		(_ccs_user_defined_features_tuner_data_t *)data;
	return d->vector.get_optimums(d->selfref, num_evaluations, evaluations, num_evaluations_ret);
}

static ccs_result_t
_ccs_features_tuner_user_defined_get_history(
		_ccs_features_tuner_data_t *data,
		size_t                      num_evaluations,
		ccs_features_evaluation_t  *evaluations,
		size_t                     *num_evaluations_ret) {
	_ccs_user_defined_features_tuner_data_t *d =
		(_ccs_user_defined_features_tuner_data_t *)data;
	return d->vector.get_history(d->selfref, num_evaluations, evaluations, num_evaluations_ret);
}

static ccs_result_t
_ccs_features_tuner_user_defined_suggest(_ccs_features_tuner_data_t *data,
                                         ccs_features_t              features,
                                         ccs_configuration_t        *configuration_ret) {
	_ccs_user_defined_features_tuner_data_t *d =
		(_ccs_user_defined_features_tuner_data_t *)data;
	return d->vector.suggest(d->selfref, features, configuration_ret);
}

static _ccs_features_tuner_ops_t _ccs_features_tuner_user_defined_ops = {
	{ &_ccs_features_tuner_user_defined_del },
	&_ccs_features_tuner_user_defined_ask,
	&_ccs_features_tuner_user_defined_tell,
	&_ccs_features_tuner_user_defined_get_optimums,
	&_ccs_features_tuner_user_defined_get_history,
	&_ccs_features_tuner_user_defined_suggest
};

ccs_result_t
ccs_create_user_defined_features_tuner(
		const char                               *name,
		ccs_configuration_space_t                 configuration_space,
		ccs_features_space_t                      features_space,
		ccs_objective_space_t                     objective_space,
		void                                     *user_data,
		ccs_user_defined_features_tuner_vector_t *vector,
		void                                     *tuner_data,
		ccs_features_tuner_t                     *tuner_ret) {
	CCS_CHECK_PTR(name);
	CCS_CHECK_OBJ(configuration_space, CCS_CONFIGURATION_SPACE);
	CCS_CHECK_OBJ(features_space, CCS_FEATURES_SPACE);
	CCS_CHECK_OBJ(objective_space, CCS_OBJECTIVE_SPACE);
	CCS_CHECK_PTR(tuner_ret);
	CCS_CHECK_PTR(vector);
	CCS_CHECK_PTR(vector->del);
	CCS_CHECK_PTR(vector->ask);
	CCS_CHECK_PTR(vector->tell);
	CCS_CHECK_PTR(vector->get_optimums);
	CCS_CHECK_PTR(vector->get_history);

	uintptr_t mem = (uintptr_t)calloc(1,
		sizeof(struct _ccs_features_tuner_s) +
		sizeof(struct _ccs_user_defined_features_tuner_data_s) +
		strlen(name) + 1);
	if (!mem)
		return -CCS_OUT_OF_MEMORY;
	ccs_features_tuner_t tun;
	_ccs_user_defined_features_tuner_data_t * data;
	ccs_result_t err;
	err = ccs_retain_object(configuration_space);
	if (err)
		goto errmem;
	err = ccs_retain_object(objective_space);
	if (err)
		goto errcs;
	err = ccs_retain_object(features_space);
	if (err)
		goto erros;

	tun = (ccs_features_tuner_t)mem;
	_ccs_object_init(&(tun->obj), CCS_TUNER, (_ccs_object_ops_t *)&_ccs_features_tuner_user_defined_ops);
	tun->data = (struct _ccs_features_tuner_data_s *)(mem + sizeof(struct _ccs_features_tuner_s));
	data = (_ccs_user_defined_features_tuner_data_t *)tun->data;
	data->common_data.type = CCS_TUNER_USER_DEFINED;
	data->common_data.name = (const char *)(mem +
		sizeof(struct _ccs_features_tuner_s) +
		sizeof(struct _ccs_user_defined_features_tuner_data_s));
	data->common_data.user_data = user_data;
	data->common_data.configuration_space = configuration_space;
	data->common_data.objective_space = objective_space;
	data->common_data.features_space = features_space;
        data->selfref = tun;
	data->vector = *vector;
	data->tuner_data = tuner_data;
	strcpy((char*)data->common_data.name, name);
	*tuner_ret = tun;
	return CCS_SUCCESS;
erros:
	ccs_release_object(objective_space);
errcs:
	ccs_release_object(configuration_space);
errmem:
	free((void *)mem);
	return err;
}

ccs_result_t
ccs_user_defined_features_tuner_get_tuner_data(ccs_features_tuner_t   tuner,
                                               void        **tuner_data_ret) {
	CCS_CHECK_OBJ(tuner, CCS_FEATURES_TUNER);
	CCS_CHECK_PTR(tuner_data_ret);
	_ccs_user_defined_features_tuner_data_t *d =
		(_ccs_user_defined_features_tuner_data_t *)tuner->data;
        if (d->common_data.type != CCS_FEATURES_TUNER_USER_DEFINED)
		return -CCS_INVALID_TUNER;
	*tuner_data_ret = d->tuner_data;
	return CCS_SUCCESS;
}

