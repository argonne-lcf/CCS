#include "cconfigspace_internal.h"
#include "tuner_internal.h"

#include "utarray.h"

struct _ccs_random_tuner_data_s {
	_ccs_tuner_common_data_t  common_data;
	UT_array                 *history;
	UT_array                 *optimums;
	UT_array                 *old_optimums;
};
typedef struct _ccs_random_tuner_data_s _ccs_random_tuner_data_t;

static ccs_result_t
_ccs_tuner_random_del(ccs_object_t o) {
	_ccs_random_tuner_data_t *d = (_ccs_random_tuner_data_t *)((ccs_tuner_t)o)->data;
	ccs_release_object(d->common_data.configuration_space);
	ccs_release_object(d->common_data.objective_space);
	ccs_evaluation_t *e = NULL;
	while ( (e = (ccs_evaluation_t *)utarray_next(d->history, e)) ) {
		ccs_release_object(*e);
	}
	utarray_free(d->history);
	utarray_free(d->optimums);
	utarray_free(d->old_optimums);
	return CCS_SUCCESS;
}

static ccs_result_t
_ccs_tuner_random_ask(_ccs_tuner_data_t   *data,
                      size_t               num_configurations,
                      ccs_configuration_t *configurations,
                      size_t              *num_configurations_ret) {
	_ccs_random_tuner_data_t *d = (_ccs_random_tuner_data_t *)data;
	if (!configurations) {
		*num_configurations_ret = 1;
		return CCS_SUCCESS;
	}
	ccs_result_t err;
	err = ccs_configuration_space_samples(d->common_data.configuration_space,
	                                      num_configurations, configurations);
	if (err)
		return err;
	if (num_configurations_ret)
		*num_configurations_ret = num_configurations;
	return CCS_SUCCESS;
}
#undef  utarray_oom
#define utarray_oom() { \
	ccs_release_object(evaluations[i]); \
	return -CCS_OUT_OF_MEMORY; \
}
static ccs_result_t
_ccs_tuner_random_tell(_ccs_tuner_data_t *data,
                       size_t             num_evaluations,
                       ccs_evaluation_t  *evaluations) {
	_ccs_random_tuner_data_t *d = (_ccs_random_tuner_data_t *)data;
	UT_array *history = d->history;
	ccs_result_t err;
	for (size_t i = 0; i < num_evaluations; i++) {
		ccs_result_t error;
		err = ccs_evaluation_get_error(evaluations[i], &error);
		if (err)
			return err;
		if (!error) {
			int discard = 0;
			UT_array *tmp;
			ccs_retain_object(evaluations[i]);
			utarray_push_back(history, evaluations + i);
			tmp = d->old_optimums;
			d->old_optimums = d->optimums;
			d->optimums = tmp;
			utarray_clear(d->optimums);
			ccs_evaluation_t *eval = NULL;
#undef  utarray_oom
#define utarray_oom() { \
	d->optimums = d->old_optimums; \
	return -CCS_OUT_OF_MEMORY; \
}
			while ( (eval = (ccs_evaluation_t *)utarray_next(d->old_optimums, eval)) ) {
				if (!discard) {
					ccs_comparison_t cmp;
					err = ccs_evaluation_cmp(evaluations[i], *eval, &cmp);
					if (err)
						discard = 1;
					else switch (cmp) {
					case CCS_EQUIVALENT:
					case CCS_WORSE:
						discard = 1;
						utarray_push_back(d->optimums, eval);
						break;
					case CCS_BETTER:
						break;
					case CCS_NOT_COMPARABLE:
					default:
						utarray_push_back(d->optimums, eval);
						break;
					}
				} else {
					utarray_push_back(d->optimums, eval);
				}
			}
			if(!discard)
				utarray_push_back(d->optimums, evaluations + i);
		}
	}
	return CCS_SUCCESS;
}

static ccs_result_t
 _ccs_tuner_random_get_optimums(_ccs_tuner_data_t *data,
                                size_t             num_evaluations,
                                ccs_evaluation_t  *evaluations,
                                size_t            *num_evaluations_ret) {
	_ccs_random_tuner_data_t *d = (_ccs_random_tuner_data_t *)data;
	size_t count = utarray_len(d->optimums);
	if (evaluations) {
		if (count < num_evaluations)
			return -CCS_INVALID_VALUE;
		ccs_evaluation_t *eval = NULL;
		size_t index = 0;
		while ( (eval = (ccs_evaluation_t *)utarray_next(d->optimums, eval)) )
			evaluations[index++] = *eval;
		for (size_t i = count; i <num_evaluations; i++)
			evaluations[i] = NULL;
	}
	if (num_evaluations_ret)
		*num_evaluations_ret = count;
	return CCS_SUCCESS;
}

static ccs_result_t
 _ccs_tuner_random_get_history(_ccs_tuner_data_t *data,
                               size_t             num_evaluations,
                               ccs_evaluation_t  *evaluations,
                               size_t            *num_evaluations_ret) {
	_ccs_random_tuner_data_t *d = (_ccs_random_tuner_data_t *)data;
	size_t count = utarray_len(d->history);
	if (evaluations) {
		if (count < num_evaluations)
			return -CCS_INVALID_VALUE;
		ccs_evaluation_t *eval = NULL;
		size_t index = 0;
		while ( (eval = (ccs_evaluation_t *)utarray_next(d->history, eval)) )
			evaluations[index++] = *eval;
		for (size_t i = count; i <num_evaluations; i++)
			evaluations[i] = NULL;
	}
	if (num_evaluations_ret)
		*num_evaluations_ret = count;
	return CCS_SUCCESS;
}

static _ccs_tuner_ops_t _ccs_tuner_random_ops = {
	{ &_ccs_tuner_random_del },
	&_ccs_tuner_random_ask,
	&_ccs_tuner_random_tell,
	&_ccs_tuner_random_get_optimums,
	&_ccs_tuner_random_get_history
};

static const UT_icd _evaluation_icd = {
	sizeof(ccs_evaluation_t),
	NULL,
	NULL,
	NULL,
};

#undef  utarray_oom
#define utarray_oom() { \
	err = -CCS_OUT_OF_MEMORY; \
	goto arrays; \
}
ccs_result_t
ccs_create_random_tuner(const char                *name,
                        ccs_configuration_space_t  configuration_space,
                        ccs_objective_space_t      objective_space,
                        void                      *user_data,
                        ccs_tuner_t               *tuner_ret) {
	if (!name || !tuner_ret)
		return -CCS_INVALID_VALUE;
	if (!configuration_space || !objective_space)
		return -CCS_INVALID_OBJECT;

	uintptr_t mem = (uintptr_t)calloc(1, sizeof(struct _ccs_tuner_s) +
	                                     sizeof(struct _ccs_random_tuner_data_s) +
	                                     strlen(name) + 1);
	if (!mem)
		return -CCS_OUT_OF_MEMORY;
	ccs_tuner_t tun;
	_ccs_random_tuner_data_t * data;
	ccs_result_t err;
	err = ccs_retain_object(configuration_space);
	if (err)
		goto errmemory;
	err = ccs_retain_object(objective_space);
	if (err)
		goto errconfigs;
	tun = (ccs_tuner_t)mem;
	_ccs_object_init(&(tun->obj), CCS_TUNER, (_ccs_object_ops_t *)&_ccs_tuner_random_ops);
	tun->data = (struct _ccs_tuner_data_s *)(mem + sizeof(struct _ccs_tuner_s));
	data = (_ccs_random_tuner_data_t *)tun->data;
	data->common_data.name = (const char *)(mem + sizeof(struct _ccs_tuner_s) +
                                                      sizeof(struct _ccs_random_tuner_data_s));
	data->common_data.user_data = user_data;
	data->common_data.configuration_space = configuration_space;
	data->common_data.objective_space = objective_space;
	utarray_new(data->history, &_evaluation_icd);
	utarray_new(data->optimums, &_evaluation_icd);
	utarray_new(data->old_optimums, &_evaluation_icd);
	strcpy((char*)data->common_data.name, name);
	*tuner_ret = tun;
	return CCS_SUCCESS;

arrays:
	if (data->history)
		utarray_free(data->history);
	if (data->optimums)
		utarray_free(data->optimums);
	if (data->old_optimums)
		utarray_free(data->old_optimums);
	ccs_release_object(objective_space);
errconfigs:
	ccs_release_object(configuration_space);
errmemory:
	free((void *)mem);
	return err;
}