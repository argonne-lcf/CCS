#ifndef _CCS_FEATURES_TUNER_H
#define _CCS_FEATURES_TUNER_H

#ifdef __cplusplus
extern "C" {
#endif

enum ccs_features_tuner_type_e {
	CCS_FEATURES_TUNER_RANDOM,
	CCS_FEATURES_TUNER_USER_DEFINED,
	CCS_FEATURES_TUNER_TYPE_MAX,
	CCS_FEATURES_TUNER_TYPE_32BIT = INT_MAX
};
typedef enum ccs_features_tuner_type_e ccs_features_tuner_type_t;

extern ccs_result_t
ccs_features_tuner_get_type(ccs_features_tuner_t       features_tuner,
                            ccs_features_tuner_type_t *type_ret);

extern ccs_result_t
ccs_features_tuner_get_name(ccs_features_tuner_t   features_tuner,
                            const char           **name_ret);

extern ccs_result_t
ccs_features_tuner_get_user_data(ccs_features_tuner_t   features_tuner,
                                 void                 **user_data_ret);

extern ccs_result_t
ccs_features_tuner_get_configuration_space(
		ccs_features_tuner_t       features_tuner,
		ccs_configuration_space_t *configuration_space_ret);

extern ccs_result_t
ccs_features_tuner_get_objective_space(
		ccs_features_tuner_t   features_tuner,
		ccs_objective_space_t *objective_space_ret);

extern ccs_result_t
ccs_features_tuner_get_features_space(
		ccs_features_tuner_t   features_tuner,
		ccs_features_space_t  *features_space_ret);

extern ccs_result_t
ccs_features_tuner_ask(ccs_features_tuner_t  features_tuner,
                       ccs_features_t        features,
                       size_t                num_configurations,
                       ccs_configuration_t  *configurations,
                       size_t               *num_configurations_ret);

extern ccs_result_t
ccs_features_tuner_tell(ccs_features_tuner_t       features_tuner,
                        size_t                     num_evaluations,
                        ccs_features_evaluation_t *evaluations);

extern ccs_result_t
ccs_features_tuner_suggest(ccs_features_tuner_t features_tuner,
                           ccs_features_t        features,
                           ccs_configuration_t  *configuration);

extern ccs_result_t
ccs_features_tuner_get_optimums(ccs_features_tuner_t       features_tuner,
                                size_t                     num_evaluations,
                                ccs_features_evaluation_t *evaluations,
                                size_t                    *num_evaluations_ret);

extern ccs_result_t
ccs_features_tuner_get_history(ccs_features_tuner_t       features_tuner,
                               size_t                     num_evaluations,
                               ccs_features_evaluation_t *evaluations,
                               size_t                    *num_evaluations_ret);

extern ccs_result_t
ccs_create_random_features_tuner(const char                *name,
                                 ccs_configuration_space_t  configuration_space,
                                 ccs_features_space_t       features_space,
                                 ccs_objective_space_t      objective_space,
                                 void                      *user_data,
                                 ccs_features_tuner_t      *features_tuner_ret);

struct ccs_user_defined_features_tuner_vector_s {
	ccs_result_t (*del)(
		ccs_features_tuner_t features_tuner);

	ccs_result_t (*ask)(
		ccs_features_tuner_t  features_tuner,
		ccs_features_t        features,
		size_t                num_configurations,
		ccs_configuration_t  *configurations,
		size_t               *num_configurations_ret);

	ccs_result_t (*tell)(
		ccs_features_tuner_t       features_tuner,
		size_t                     num_evaluations,
		ccs_features_evaluation_t *evaluations);

	ccs_result_t (*get_optimums)(
		ccs_features_tuner_t       features_tuner,
		size_t                     num_evaluations,
		ccs_features_evaluation_t *evaluations,
		size_t                    *num_evaluations_ret);

	ccs_result_t (*get_history)(
		ccs_features_tuner_t       features_tuner,
		size_t                     num_evaluations,
		ccs_features_evaluation_t *evaluations,
		size_t                    *num_evaluations_ret);

	ccs_result_t (*suggest)(
		ccs_features_tuner_t  features_tuner,
		ccs_features_t        features,
		ccs_configuration_t  *configuration);
};
typedef struct ccs_user_defined_features_tuner_vector_s ccs_user_defined_features_tuner_vector_t;

extern ccs_result_t
ccs_create_user_defined_features_tuner(
		const char                               *name,
		ccs_configuration_space_t                 configuration_space,
		ccs_features_space_t                      features_space,
		ccs_objective_space_t                     objective_space,
		void                                     *user_data,
		ccs_user_defined_features_tuner_vector_t *vector,
		void                                     *features_tuner_data,
		ccs_features_tuner_t                     *features_tuner_ret);

extern ccs_result_t
ccs_user_defined_features_tuner_get_features_tuner_data(
		ccs_features_tuner_t   features_tuner,
		void                 **features_tuner_data_ret);

#ifdef __cplusplus
}
#endif

#endif //_CCS_FEATURES_TUNER_H
