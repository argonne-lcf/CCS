#include "cconfigspace_internal.h"
#include "configuration_internal.h"
#include <string.h>

static inline _ccs_configuration_ops_t *
ccs_configuration_get_ops(ccs_configuration_t configuration) {
	return (_ccs_configuration_ops_t *)configuration->obj.ops;
}

static ccs_result_t
_ccs_configuration_del(ccs_object_t object) {
	ccs_configuration_t configuration = (ccs_configuration_t)object;
	ccs_release_object(configuration->data->configuration_space);
	return CCS_SUCCESS;
}

static ccs_result_t
_ccs_configuration_hash(_ccs_configuration_data_t *data,
                        ccs_hash_t                *hash_ret) {
	return _ccs_binding_hash((_ccs_binding_data_t *)data, hash_ret);
}

static ccs_result_t
_ccs_configuration_cmp(_ccs_configuration_data_t *data,
                       ccs_configuration_t        other,
                       int                       *cmp_ret) {
	return _ccs_binding_cmp((_ccs_binding_data_t *)data, (ccs_binding_t)other, cmp_ret);
}

static _ccs_configuration_ops_t _configuration_ops =
    { {&_ccs_configuration_del},
      &_ccs_configuration_hash,
      &_ccs_configuration_cmp };

ccs_result_t
ccs_create_configuration(ccs_configuration_space_t configuration_space,
                         size_t                    num_values,
                         ccs_datum_t              *values,
                         void                     *user_data,
                         ccs_configuration_t      *configuration_ret) {
	CCS_CHECK_PTR(configuration_ret);
	CCS_CHECK_ARY(num_values, values);
	ccs_result_t err;
	size_t num;
	err = ccs_configuration_space_get_num_hyperparameters(configuration_space, &num);
	if (err)
		return err;
	if (values && num != num_values)
		return -CCS_INVALID_VALUE;
	uintptr_t mem = (uintptr_t)calloc(1, sizeof(struct _ccs_configuration_s) +
	                                     sizeof(struct _ccs_configuration_data_s) +
	                                     num * sizeof(ccs_datum_t));
	if (!mem)
		return -CCS_OUT_OF_MEMORY;
	err = ccs_retain_object(configuration_space);
	if (err) {
		free((void*)mem);
		return err;
	}
	ccs_configuration_t config = (ccs_configuration_t)mem;
	_ccs_object_init(&(config->obj), CCS_CONFIGURATION, (_ccs_object_ops_t*)&_configuration_ops);
	config->data = (struct _ccs_configuration_data_s*)(mem + sizeof(struct _ccs_configuration_s));
	config->data->user_data = user_data;
	config->data->num_values = num;
	config->data->configuration_space = configuration_space;
	config->data->values = (ccs_datum_t *)(mem + sizeof(struct _ccs_configuration_s) + sizeof(struct _ccs_configuration_data_s));
	if (values) {
		memcpy(config->data->values, values, num*sizeof(ccs_datum_t));
		for (size_t i = 0; i < num_values; i++) {
			if (values[i].flags & CCS_FLAG_TRANSIENT) {
				err = ccs_configuration_space_validate_value(
					configuration_space, i, values[i],
					 config->data->values + i);
				if (unlikely(err)) {
					free((void*)mem);
					return err;
				}
			}
		}
	}
	*configuration_ret = config;
	return CCS_SUCCESS;
}

ccs_result_t
ccs_configuration_get_configuration_space(ccs_configuration_t        configuration,
                                          ccs_configuration_space_t *configuration_space_ret) {
	CCS_CHECK_OBJ(configuration, CCS_CONFIGURATION);
	return _ccs_binding_get_context(
		(ccs_binding_t)configuration, (ccs_context_t *)configuration_space_ret);
}

ccs_result_t
ccs_configuration_get_user_data(ccs_configuration_t   configuration,
                                void                **user_data_ret) {
	CCS_CHECK_OBJ(configuration, CCS_CONFIGURATION);
	return _ccs_binding_get_user_data(
		(ccs_binding_t)configuration, user_data_ret);
}

ccs_result_t
ccs_configuration_get_value(ccs_configuration_t  configuration,
                            size_t               index,
                            ccs_datum_t         *value_ret) {
	CCS_CHECK_OBJ(configuration, CCS_CONFIGURATION);
	return _ccs_binding_get_value(
		(ccs_binding_t)configuration, index, value_ret);
}

ccs_result_t
ccs_configuration_set_value(ccs_configuration_t configuration,
                            size_t              index,
                            ccs_datum_t         value) {
	CCS_CHECK_OBJ(configuration, CCS_CONFIGURATION);
	return _ccs_binding_set_value(
		(ccs_binding_t)configuration, index, value);
}

ccs_result_t
ccs_configuration_get_values(ccs_configuration_t  configuration,
                             size_t               num_values,
                             ccs_datum_t         *values,
                             size_t              *num_values_ret) {
	CCS_CHECK_OBJ(configuration, CCS_CONFIGURATION);
	return _ccs_binding_get_values(
		(ccs_binding_t)configuration, num_values, values, num_values_ret);
}

ccs_result_t
ccs_configuration_get_value_by_name(ccs_configuration_t  configuration,
                                    const char          *name,
                                    ccs_datum_t         *value_ret) {
	CCS_CHECK_OBJ(configuration, CCS_CONFIGURATION);
	return _ccs_binding_get_value_by_name(
		(ccs_binding_t)configuration, name, value_ret);
}

ccs_result_t
ccs_configuration_check(ccs_configuration_t configuration) {
	CCS_CHECK_OBJ(configuration, CCS_CONFIGURATION);
	return ccs_configuration_space_check_configuration(
		configuration->data->configuration_space, configuration);
}

ccs_result_t
ccs_configuration_hash(ccs_configuration_t  configuration,
                       ccs_hash_t          *hash_ret) {
	CCS_CHECK_OBJ(configuration, CCS_CONFIGURATION);
	_ccs_configuration_ops_t *ops = ccs_configuration_get_ops(configuration);
	return ops->hash(configuration->data, hash_ret);
}

ccs_result_t
ccs_configuration_cmp(ccs_configuration_t  configuration,
                      ccs_configuration_t  other_configuration,
                      int                 *cmp_ret) {
	CCS_CHECK_OBJ(configuration, CCS_CONFIGURATION);
	CCS_CHECK_OBJ(other_configuration, CCS_CONFIGURATION);
	_ccs_configuration_ops_t *ops = ccs_configuration_get_ops(configuration);
	return ops->cmp(configuration->data, other_configuration, cmp_ret);
}

