#include "cconfigspace_internal.h"
#include "features_tuner_internal.h"
#include "features_evaluation_internal.h"

#include "utarray.h"

struct _ccs_random_features_tuner_data_s {
	_ccs_features_tuner_common_data_t common_data;
	UT_array                         *history;
	UT_array                         *optima;
	UT_array                         *old_optima;
};
typedef struct _ccs_random_features_tuner_data_s
	_ccs_random_features_tuner_data_t;

static ccs_result_t
_ccs_features_tuner_random_del(ccs_object_t o)
{
	_ccs_random_features_tuner_data_t *d =
		(_ccs_random_features_tuner_data_t *)((ccs_features_tuner_t)o)
			->data;
	ccs_release_object(d->common_data.configuration_space);
	ccs_release_object(d->common_data.objective_space);
	ccs_release_object(d->common_data.features_space);
	ccs_features_evaluation_t *e = NULL;
	while ((e = (ccs_features_evaluation_t *)utarray_next(d->history, e))) {
		ccs_release_object(*e);
	}
	utarray_free(d->history);
	utarray_free(d->optima);
	utarray_free(d->old_optima);
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_serialize_bin_size_ccs_random_features_tuner_data(
	_ccs_random_features_tuner_data_t *data,
	size_t                            *cum_size,
	_ccs_object_serialize_options_t   *opts)
{
	ccs_features_evaluation_t *e = NULL;
	CCS_VALIDATE(_ccs_serialize_bin_size_ccs_features_tuner_common_data(
		&data->common_data, cum_size, opts));
	*cum_size += _ccs_serialize_bin_size_size(utarray_len(data->history));
	*cum_size += _ccs_serialize_bin_size_size(utarray_len(data->optima));
	while ((e = (ccs_features_evaluation_t *)utarray_next(data->history, e)))
		CCS_VALIDATE((*e)->obj.ops->serialize_size(
			*e, CCS_SERIALIZE_FORMAT_BINARY, cum_size, opts));
	e = NULL;
	while ((e = (ccs_features_evaluation_t *)utarray_next(data->optima, e)))
		*cum_size += _ccs_serialize_bin_size_ccs_object(*e);
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_serialize_bin_ccs_random_features_tuner_data(
	_ccs_random_features_tuner_data_t *data,
	size_t                            *buffer_size,
	char                             **buffer,
	_ccs_object_serialize_options_t   *opts)
{
	ccs_features_evaluation_t *e = NULL;
	CCS_VALIDATE(_ccs_serialize_bin_ccs_features_tuner_common_data(
		&data->common_data, buffer_size, buffer, opts));
	CCS_VALIDATE(_ccs_serialize_bin_size(
		utarray_len(data->history), buffer_size, buffer));
	CCS_VALIDATE(_ccs_serialize_bin_size(
		utarray_len(data->optima), buffer_size, buffer));
	while ((e = (ccs_features_evaluation_t *)utarray_next(data->history, e)))
		CCS_VALIDATE((*e)->obj.ops->serialize(
			*e, CCS_SERIALIZE_FORMAT_BINARY, buffer_size, buffer,
			opts));
	e = NULL;
	while ((e = (ccs_features_evaluation_t *)utarray_next(data->optima, e)))
		CCS_VALIDATE(
			_ccs_serialize_bin_ccs_object(*e, buffer_size, buffer));
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_serialize_bin_size_ccs_random_features_tuner(
	ccs_features_tuner_t             features_tuner,
	size_t                          *cum_size,
	_ccs_object_serialize_options_t *opts)
{
	_ccs_random_features_tuner_data_t *data =
		(_ccs_random_features_tuner_data_t *)(features_tuner->data);
	*cum_size += _ccs_serialize_bin_size_ccs_object_internal(
		(_ccs_object_internal_t *)features_tuner);
	CCS_VALIDATE(_ccs_serialize_bin_size_ccs_random_features_tuner_data(
		data, cum_size, opts));
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_serialize_bin_ccs_random_features_tuner(
	ccs_features_tuner_t             features_tuner,
	size_t                          *buffer_size,
	char                           **buffer,
	_ccs_object_serialize_options_t *opts)
{
	_ccs_random_features_tuner_data_t *data =
		(_ccs_random_features_tuner_data_t *)(features_tuner->data);
	CCS_VALIDATE(_ccs_serialize_bin_ccs_object_internal(
		(_ccs_object_internal_t *)features_tuner, buffer_size, buffer));
	CCS_VALIDATE(_ccs_serialize_bin_ccs_random_features_tuner_data(
		data, buffer_size, buffer, opts));
	return CCS_RESULT_SUCCESS;
}

static ccs_result_t
_ccs_random_features_tuner_serialize_size(
	ccs_object_t                     object,
	ccs_serialize_format_t           format,
	size_t                          *cum_size,
	_ccs_object_serialize_options_t *opts)
{
	switch (format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE(_ccs_serialize_bin_size_ccs_random_features_tuner(
			(ccs_features_tuner_t)object, cum_size, opts));
		break;
	default:
		CCS_RAISE(
			CCS_RESULT_ERROR_INVALID_VALUE,
			"Unsupported serialization format: %d", format);
	}
	CCS_VALIDATE(_ccs_object_serialize_user_data_size(
		object, format, cum_size, opts));
	return CCS_RESULT_SUCCESS;
}

static ccs_result_t
_ccs_random_features_tuner_serialize(
	ccs_object_t                     object,
	ccs_serialize_format_t           format,
	size_t                          *buffer_size,
	char                           **buffer,
	_ccs_object_serialize_options_t *opts)
{
	switch (format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE(_ccs_serialize_bin_ccs_random_features_tuner(
			(ccs_features_tuner_t)object, buffer_size, buffer,
			opts));
		break;
	default:
		CCS_RAISE(
			CCS_RESULT_ERROR_INVALID_VALUE,
			"Unsupported serialization format: %d", format);
	}
	CCS_VALIDATE(_ccs_object_serialize_user_data(
		object, format, buffer_size, buffer, opts));
	return CCS_RESULT_SUCCESS;
}

static ccs_result_t
_ccs_features_tuner_random_ask(
	ccs_features_tuner_t tuner,
	ccs_features_t       features,
	size_t               num_configurations,
	ccs_configuration_t *configurations,
	size_t              *num_configurations_ret)
{
	(void)features;
	_ccs_random_features_tuner_data_t *d =
		(_ccs_random_features_tuner_data_t *)tuner->data;
	if (!configurations) {
		*num_configurations_ret = 1;
		return CCS_RESULT_SUCCESS;
	}
	CCS_VALIDATE(ccs_configuration_space_samples(
		d->common_data.configuration_space, NULL, NULL,
		num_configurations, configurations));
	if (num_configurations_ret)
		*num_configurations_ret = num_configurations;
	return CCS_RESULT_SUCCESS;
}
#undef utarray_oom
#define utarray_oom()                                                          \
	{                                                                      \
		ccs_release_object(evaluations[i]);                            \
		CCS_RAISE(                                                     \
			CCS_RESULT_ERROR_OUT_OF_MEMORY,                        \
			"Not enough memory to allocate new array");            \
	}
static ccs_result_t
_ccs_features_tuner_random_tell(
	ccs_features_tuner_t       tuner,
	size_t                     num_evaluations,
	ccs_features_evaluation_t *evaluations)
{
	_ccs_random_features_tuner_data_t *d =
		(_ccs_random_features_tuner_data_t *)tuner->data;
	UT_array    *history = d->history;
	ccs_result_t err;
	for (size_t i = 0; i < num_evaluations; i++) {
		ccs_evaluation_result_t result;
		CCS_VALIDATE(ccs_features_evaluation_get_result(
			evaluations[i], &result));
		if (result == CCS_RESULT_SUCCESS) {
			int       discard = 0;
			UT_array *tmp;
			ccs_retain_object(evaluations[i]);
			utarray_push_back(history, evaluations + i);
			tmp           = d->old_optima;
			d->old_optima = d->optima;
			d->optima     = tmp;
			utarray_clear(d->optima);
			ccs_features_evaluation_t *eval = NULL;
#undef utarray_oom
#define utarray_oom()                                                          \
	{                                                                      \
		d->optima = d->old_optima;                                     \
		CCS_RAISE(                                                     \
			CCS_RESULT_ERROR_OUT_OF_MEMORY,                        \
			"Not enough memory to allocate new array");            \
	}
			while ((eval = (ccs_features_evaluation_t *)
					utarray_next(d->old_optima, eval))) {
				if (!discard) {
					ccs_comparison_t cmp;
					err = ccs_features_evaluation_compare(
						evaluations[i], *eval, &cmp);
					if (err)
						discard = 1;
					else
						switch (cmp) {
						case CCS_COMPARISON_EQUIVALENT:
						case CCS_COMPARISON_WORSE:
							discard = 1;
							utarray_push_back(
								d->optima,
								eval);
							break;
						case CCS_COMPARISON_BETTER:
							break;
						case CCS_COMPARISON_NOT_COMPARABLE:
						default:
							utarray_push_back(
								d->optima,
								eval);
							break;
						}
				} else {
					utarray_push_back(d->optima, eval);
				}
			}
			if (!discard)
				utarray_push_back(d->optima, evaluations + i);
		}
	}
	return CCS_RESULT_SUCCESS;
}

static ccs_result_t
_ccs_features_tuner_random_get_optima(
	ccs_features_tuner_t       tuner,
	ccs_features_t             features,
	size_t                     num_evaluations,
	ccs_features_evaluation_t *evaluations,
	size_t                    *num_evaluations_ret)
{
	_ccs_random_features_tuner_data_t *d =
		(_ccs_random_features_tuner_data_t *)tuner->data;
	size_t num_optima = 0;
	if (!features) {
		num_optima = utarray_len(d->optima);
		if (evaluations) {
			CCS_REFUTE(
				num_evaluations < num_optima,
				CCS_RESULT_ERROR_INVALID_VALUE);
			ccs_features_evaluation_t *eval  = NULL;
			size_t                     index = 0;
			while ((eval = (ccs_features_evaluation_t *)
					utarray_next(d->optima, eval)))
				evaluations[index++] = *eval;
			for (size_t i = num_optima; i < num_evaluations; i++)
				evaluations[i] = NULL;
		}
	} else {
		ccs_features_evaluation_t *eval  = NULL;
		size_t                     index = 0;
		ccs_features_t             feat;
		int                        cmp;
		while ((eval = (ccs_features_evaluation_t *)utarray_next(
				d->optima, eval))) {
			CCS_VALIDATE(ccs_features_evaluation_get_features(
				*eval, &feat));
			CCS_VALIDATE(ccs_binding_cmp(
				(ccs_binding_t)features, (ccs_binding_t)feat,
				&cmp));
			if (cmp == 0)
				num_optima += 1;
		}
		if (evaluations) {
			CCS_REFUTE(
				num_evaluations < num_optima,
				CCS_RESULT_ERROR_INVALID_VALUE);
			eval = NULL;
			while ((eval = (ccs_features_evaluation_t *)
					utarray_next(d->optima, eval))) {
				CCS_VALIDATE(
					ccs_features_evaluation_get_features(
						*eval, &feat));
				CCS_VALIDATE(ccs_binding_cmp(
					(ccs_binding_t)features,
					(ccs_binding_t)feat, &cmp));
				if (cmp == 0)
					evaluations[index++] = *eval;
			}
			for (size_t i = num_optima; i < num_evaluations; i++)
				evaluations[i] = NULL;
		}
	}
	if (num_evaluations_ret)
		*num_evaluations_ret = num_optima;
	return CCS_RESULT_SUCCESS;
}

static ccs_result_t
_ccs_features_tuner_random_get_history(
	ccs_features_tuner_t       tuner,
	ccs_features_t             features,
	size_t                     num_evaluations,
	ccs_features_evaluation_t *evaluations,
	size_t                    *num_evaluations_ret)
{
	_ccs_random_features_tuner_data_t *d =
		(_ccs_random_features_tuner_data_t *)tuner->data;
	size_t size_history = 0;
	if (!features) {
		size_history = utarray_len(d->history);
		if (evaluations) {
			CCS_REFUTE(
				num_evaluations < size_history,
				CCS_RESULT_ERROR_INVALID_VALUE);
			ccs_features_evaluation_t *eval  = NULL;
			size_t                     index = 0;
			while ((eval = (ccs_features_evaluation_t *)
					utarray_next(d->history, eval)))
				evaluations[index++] = *eval;
			for (size_t i = size_history; i < num_evaluations; i++)
				evaluations[i] = NULL;
		}
	} else {
		ccs_features_evaluation_t *eval = NULL;
		ccs_features_t             feat;
		int                        cmp;
		size_t                     index = 0;
		while ((eval = (ccs_features_evaluation_t *)utarray_next(
				d->history, eval))) {
			CCS_VALIDATE(ccs_features_evaluation_get_features(
				*eval, &feat));
			CCS_VALIDATE(ccs_binding_cmp(
				(ccs_binding_t)features, (ccs_binding_t)feat,
				&cmp));
			if (cmp == 0)
				size_history += 1;
		}
		if (evaluations) {
			CCS_REFUTE(
				num_evaluations < size_history,
				CCS_RESULT_ERROR_INVALID_VALUE);
			eval = NULL;
			while ((eval = (ccs_features_evaluation_t *)
					utarray_next(d->history, eval))) {
				CCS_VALIDATE(
					ccs_features_evaluation_get_features(
						*eval, &feat));
				CCS_VALIDATE(ccs_binding_cmp(
					(ccs_binding_t)features,
					(ccs_binding_t)feat, &cmp));
				if (cmp == 0)
					evaluations[index++] = *eval;
			}
			for (size_t i = size_history; i < num_evaluations; i++)
				evaluations[i] = NULL;
		}
	}
	if (num_evaluations_ret)
		*num_evaluations_ret = size_history;
	return CCS_RESULT_SUCCESS;
}

static ccs_result_t
_ccs_features_tuner_random_suggest(
	ccs_features_tuner_t tuner,
	ccs_features_t       features,
	ccs_configuration_t *configuration)
{
	_ccs_random_features_tuner_data_t *d =
		(_ccs_random_features_tuner_data_t *)tuner->data;
	size_t                     count = 0;
	ccs_features_evaluation_t *eval  = NULL;
	ccs_features_t             feat;
	int                        cmp;
	while ((eval = (ccs_features_evaluation_t *)utarray_next(
			d->optima, eval))) {
		CCS_VALIDATE(
			ccs_features_evaluation_get_features(*eval, &feat));
		CCS_VALIDATE(ccs_binding_cmp(
			(ccs_binding_t)features, (ccs_binding_t)feat, &cmp));
		if (cmp == 0)
			count += 1;
	}
	if (count > 0) {
		ccs_rng_t         rng;
		unsigned long int indx;
		CCS_VALIDATE(ccs_configuration_space_get_rng(
			d->common_data.configuration_space, &rng));
		CCS_VALIDATE(ccs_rng_get(rng, &indx));
		indx = indx % count;
		while ((eval = (ccs_features_evaluation_t *)utarray_next(
				d->optima, eval))) {
			CCS_VALIDATE(ccs_features_evaluation_get_features(
				*eval, &feat));
			CCS_VALIDATE(ccs_binding_cmp(
				(ccs_binding_t)features, (ccs_binding_t)feat,
				&cmp));
			if (cmp == 0) {
				if (indx == 0)
					break;
				else
					indx--;
			}
		}
		CCS_VALIDATE(ccs_features_evaluation_get_configuration(
			*eval, configuration));
		CCS_VALIDATE(ccs_retain_object(*configuration));
	} else
		CCS_VALIDATE(_ccs_features_tuner_random_ask(
			tuner, features, 1, configuration, NULL));
	return CCS_RESULT_SUCCESS;
}

static _ccs_features_tuner_ops_t _ccs_features_tuner_random_ops = {
	{&_ccs_features_tuner_random_del,
	 &_ccs_random_features_tuner_serialize_size,
	 &_ccs_random_features_tuner_serialize},
	&_ccs_features_tuner_random_ask,
	&_ccs_features_tuner_random_tell,
	&_ccs_features_tuner_random_get_optima,
	&_ccs_features_tuner_random_get_history,
	&_ccs_features_tuner_random_suggest};

static const UT_icd _evaluation_icd = {
	sizeof(ccs_features_evaluation_t),
	NULL,
	NULL,
	NULL,
};

#undef utarray_oom
#define utarray_oom()                                                          \
	{                                                                      \
		CCS_RAISE_ERR_GOTO(                                            \
			err, CCS_RESULT_ERROR_OUT_OF_MEMORY, arrays,           \
			"Not enough memory to allocate array");                \
	}
ccs_result_t
ccs_create_random_features_tuner(
	const char               *name,
	ccs_configuration_space_t configuration_space,
	ccs_features_space_t      features_space,
	ccs_objective_space_t     objective_space,
	ccs_features_tuner_t     *tuner_ret)
{
	CCS_CHECK_PTR(name);
	CCS_CHECK_OBJ(configuration_space, CCS_OBJECT_TYPE_CONFIGURATION_SPACE);
	CCS_CHECK_OBJ(features_space, CCS_OBJECT_TYPE_FEATURES_SPACE);
	CCS_CHECK_OBJ(objective_space, CCS_OBJECT_TYPE_OBJECTIVE_SPACE);
	CCS_CHECK_PTR(tuner_ret);

	uintptr_t mem = (uintptr_t)calloc(
		1, sizeof(struct _ccs_features_tuner_s) +
			   sizeof(struct _ccs_random_features_tuner_data_s) +
			   strlen(name) + 1);
	CCS_REFUTE(!mem, CCS_RESULT_ERROR_OUT_OF_MEMORY);
	ccs_features_tuner_t               tun;
	_ccs_random_features_tuner_data_t *data;
	ccs_result_t                       err;

	CCS_VALIDATE_ERR_GOTO(
		err, ccs_retain_object(configuration_space), errmem);
	CCS_VALIDATE_ERR_GOTO(err, ccs_retain_object(objective_space), errcs);
	CCS_VALIDATE_ERR_GOTO(err, ccs_retain_object(features_space), erros);

	tun = (ccs_features_tuner_t)mem;
	_ccs_object_init(
		&(tun->obj), CCS_OBJECT_TYPE_FEATURES_TUNER,
		(_ccs_object_ops_t *)&_ccs_features_tuner_random_ops);
	tun->data              = (struct _ccs_features_tuner_data_s
                             *)(mem + sizeof(struct _ccs_features_tuner_s));
	data                   = (_ccs_random_features_tuner_data_t *)tun->data;
	data->common_data.type = CCS_FEATURES_TUNER_TYPE_RANDOM;
	data->common_data.name =
		(const char
			 *)(mem + sizeof(struct _ccs_features_tuner_s) + sizeof(struct _ccs_random_features_tuner_data_s));
	data->common_data.configuration_space = configuration_space;
	data->common_data.objective_space     = objective_space;
	data->common_data.features_space      = features_space;
	utarray_new(data->history, &_evaluation_icd);
	utarray_new(data->optima, &_evaluation_icd);
	utarray_new(data->old_optima, &_evaluation_icd);
	strcpy((char *)data->common_data.name, name);
	*tuner_ret = tun;
	return CCS_RESULT_SUCCESS;

arrays:
	if (data->history)
		utarray_free(data->history);
	if (data->optima)
		utarray_free(data->optima);
	if (data->old_optima)
		utarray_free(data->old_optima);
	_ccs_object_deinit(&(tun->obj));
	ccs_release_object(features_space);
erros:
	ccs_release_object(objective_space);
errcs:
	ccs_release_object(configuration_space);
errmem:
	free((void *)mem);
	return err;
}
