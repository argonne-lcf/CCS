#ifndef _CCS_FEATURES_TUNER_H
#define _CCS_FEATURES_TUNER_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file features_tuner.h
 * A CCS features tuner defines an ask and tell interface to optimize an
 * objective space (see objective_space.h) given a configuration spaces (see
 * configuration_space.h) and a feature space (see feature_space.h). The tuner
 * will propose configurations (see configuration.h) and the user will return
 * features evaluations (see features_evaluation.h).
 */

/**
 * CCS supported features tuner types.
 */
enum ccs_features_tuner_type_e {
	/** A random features tuner */
	CCS_FEATURES_TUNER_TYPE_RANDOM,
	/** A user defined features tuner */
	CCS_FEATURES_TUNER_TYPE_USER_DEFINED,
	/** Guard */
	CCS_FEATURES_TUNER_TYPE_MAX,
	/** Try forcing 32 bits value for bindings */
	CCS_FEATURES_TUNER_TYPE_32BIT = INT_MAX
};

/**
 * A commodity type to represent CCS features tuner types.
 */
typedef enum ccs_features_tuner_type_e ccs_features_tuner_type_t;

/**
 * Get the type of a features tuner.
 * @param [in] features_tuner
 * @param [out] type_ret a pointer to the variable that will contain the
 *                       returned features tuner type
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p features_tuner is not a valid
 * CCS features tuner
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p type_ret is NULL
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_features_tuner_get_type(
	ccs_features_tuner_t       features_tuner,
	ccs_features_tuner_type_t *type_ret);

/**
 * Get the name of a features tuner.
 * @param[in] features_tuner
 * @param[out] name_ret a pointer to the variable that will contain a pointer to
 *                      the name of the features tuner
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p name_ret is NULL
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p features_tuner is not a valid
 * CCS features tuner
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_features_tuner_get_name(
	ccs_features_tuner_t features_tuner,
	const char         **name_ret);

/**
 * Get the associated configuration space.
 * @param[in] features_tuner
 * @param[out] configuration_space_ret a pointer to the variable that will
 *                                     contain the configuration space
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p features_tuner is not a valid
 * CCS features tuner
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p configuration_space_ret is NULL
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_features_tuner_get_configuration_space(
	ccs_features_tuner_t       features_tuner,
	ccs_configuration_space_t *configuration_space_ret);

/**
 * Get the associated objective space.
 * @param[in] features_tuner
 * @param[out] objective_space_ret a pointer to the variable that will
 *                                     contain the objective space
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p features_tuner is not a valid
 * CCS features tuner
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p objective_space_ret is NULL
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_features_tuner_get_objective_space(
	ccs_features_tuner_t   features_tuner,
	ccs_objective_space_t *objective_space_ret);

/**
 * Get the associated feature space.
 * @param[in] features_tuner
 * @param[out] feature_space_ret a pointer to the variable that will
 *                                     contain the feature space
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p features_tuner is not a valid
 * CCS features tuner
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p feature_space_ret is NULL
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_features_tuner_get_feature_space(
	ccs_features_tuner_t features_tuner,
	ccs_feature_space_t *feature_space_ret);

/**
 * Ask a features tuner for a set of configurations to evaluate given some
 * features. Configuration's ownership is transferred to the user who doesn't
 * need to retain them, but will need to release them once the user is done
 * using them.
 * @param[in,out] features_tuner
 * @param[in] features the specific features to ask the configurations for
 * @param[in] num_configurations the number of configurations requested by the
 *                               user
 * @param[out] configurations an array of \p num_configurations configurations
 *                            that will contain the returned configurations. Can
 *                            be NULL
 * @param[out] num_configurations_ret a pointer to the variable that will
 *                                    contain the number of configuration that
 *                                    are returned, or, if \p configurations is
 *                                    NULL, a suggestion for the number of
 *                                    configuration to ask for
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p features_tuner is not a valid
 * CCS features tuner; or if \p features is not a valid CCS features
 * @return #CCS_RESULT_ERROR_INVALID_FEATURES if \p features is not a valid CCS
 * features for the tuner feature space
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p configurations is NULL and \p
 * num_configurations is greater than 0; or if \p configurations and \p
 * num_configurations_ret are both NULL
 * @return #CCS_RESULT_ERROR_SAMPLING_UNSUCCESSFUL if no or not enough valid
 * configurations could be sampled. Configurations that could be sampled will be
 * returned contiguously, and the rest will be NULL
 * @return #CCS_RESULT_ERROR_OUT_OF_MEMORY if there was not enough memory to
 * allocate new configurations. Configurations that could be allocated will be
 * returned, and the rest will be NULL
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_features_tuner_ask(
	ccs_features_tuner_t features_tuner,
	ccs_features_t       features,
	size_t               num_configurations,
	ccs_configuration_t *configurations,
	size_t              *num_configurations_ret);

/**
 * Give a list of results to a features tuner through evaluations.
 * @param[in,out] features_tuner
 * @param[in] num_evaluations the size of the \p evaluations array
 * @param[in] evaluations an array of \p num_evaluations to provide to the
 *                        features tuner
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p features_tuner is not a valid
 * CCS features tuner; or if one of the evaluations is not a valid CCS features
 * evaluation
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p evaluations is NULL and \p
 * num_evaluations is greater than 0
 * @return #CCS_RESULT_ERROR_INVALID_EVALUATION if an evaluation is not a valid
 * features evaluation for the problem the features tuner is optimizing
 * @return #CCS_RESULT_ERROR_OUT_OF_MEMORY if there was not enough memory to
 * allocate internal data structures.
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_features_tuner_tell(
	ccs_features_tuner_t       features_tuner,
	size_t                     num_evaluations,
	ccs_features_evaluation_t *evaluations);

/**
 * Ask a features tuner to suggest a good configuration given some features.
 * @param[in,out] features_tuner
 * @param[in] features the specific features to suggest a configuration for
 * @param[out] configuration a pointer to the variable that will contain the
 *                           suggested configuration
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p features_tuner is not a valid
 * CCS features tuner; or if \p features is not a valid CCS features
 * @return #CCS_RESULT_ERROR_INVALID_FEATURES if \p features is not a valid CCS
 * features for the tuner feature space
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p configuration is NULL
 * @return #CCS_RESULT_ERROR_UNSUPPORTED_OPERATION if the features tuner does
 * not support the suggest interface
 * @return #CCS_RESULT_ERROR_OUT_OF_MEMORY if there was not enough memory to
 * allocate new configurations
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_features_tuner_suggest(
	ccs_features_tuner_t features_tuner,
	ccs_features_t       features,
	ccs_configuration_t *configuration);

/**
 * Ask a features tuner for the discovered Pareto front. For single objective
 * objective spaces this would be the best point found.
 * @param[in] features_tuner
 * @param[in] features the specific features to get the optimal values for.
 *                     Optional, can be NULL
 * @param[in] num_evaluations the size of the \p evaluations array
 * @param[out] evaluations an array of \p num_evaluations that will contain the
 *                         optimal evaluations. If \p features is given, only
 *                         return optimal evaluations for the given features
 * @param[out] num_evaluations_ret a pointer to the variable that will contain
 *                                 the number of evaluations that are or would
 *                                 be returned
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p features_tuner is not a valid
 * CCS features tuner; or if \p features is not NULL and \p features is not a
 * valid CCS features
 * @return #CCS_RESULT_ERROR_INVALID_FEATURES if \p features is not a valid CCS
 * features for the features tuner feature space
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p evaluations is NULL and
 * num_evaluations is greater than 0; or if \p evaluations is NULL and \p
 * num_evaluations_ret is NULL
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_features_tuner_get_optima(
	ccs_features_tuner_t       features_tuner,
	ccs_features_t             features,
	size_t                     num_evaluations,
	ccs_features_evaluation_t *evaluations,
	size_t                    *num_evaluations_ret);

/**
 * Ask a features tuner for the evaluation history.
 * @param[in] features_tuner
 * @param[in] features the specific features to get the history for. Optional,
 *                     can be NULL
 * @param[in] num_evaluations the size of the \p evaluations array
 * @param[out] evaluations an array of \p num_evaluations that will contain the
 *                         the history. If \p features is given, only
 *                         return history of evaluations for the given features
 * @param[out] num_evaluations_ret a pointer to the variable that will contain
 *                                 the number of evaluations that are or would
 *                                 be returned
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p features_tuner is not a valid
 * CCS features tuner or if \p features is not NULL and \p features is not a
 * valid CCS features
 * @return #CCS_RESULT_ERROR_INVALID_FEATURES if \p features is not a valid CCS
 * features for the features tuner feature space
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p evaluations is NULL and
 * num_evaluations is greater than 0; or if \p evaluations is NULL and \p
 * num_evaluations_ret is NULL
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_features_tuner_get_history(
	ccs_features_tuner_t       features_tuner,
	ccs_features_t             features,
	size_t                     num_evaluations,
	ccs_features_evaluation_t *evaluations,
	size_t                    *num_evaluations_ret);

/**
 * Create a new random features tuner. The random features tuner should be
 * viewed as a baseline for evaluating features tuners, and as a tool for
 * developing interfaces.
 * @param[in] name the name of the features tuner
 * @param[in] configuration_space the configuration space to explore
 * @param[in] feature_space the feature space
 * @param[in] objective_space the objective space to potimize
 * @param[out] features_tuner_ret a pointer to the variable that will contain
 *                                the newly created features tuner
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p configuration_space is not a
 * valid CCS configuration space; or if \p objective_space is not a valid CCS
 * objective space; or if \p feature_space is not a valid CCS feature space
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p name is NULL; or if \p features
 * tuner_ret is NULL
 * @return #CCS_RESULT_ERROR_OUT_OF_MEMORY if there was not enough memory to
 * allocate the new features tuner instance
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_create_random_features_tuner(
	const char               *name,
	ccs_configuration_space_t configuration_space,
	ccs_feature_space_t       feature_space,
	ccs_objective_space_t     objective_space,
	ccs_features_tuner_t     *features_tuner_ret);

/**
 * A structure that define the callbacks the user must provide to create a user
 * defined features tuner.
 */
struct ccs_user_defined_features_tuner_vector_s {
	/**
	 * The deletion callback that will be called once the reference count
	 * of the features tuner reaches 0.
	 */
	ccs_result_t (*del)(ccs_features_tuner_t features_tuner);

	/** The features tuner ask interface see ccs_features_tuner_ask */
	ccs_result_t (*ask)(
		ccs_features_tuner_t features_tuner,
		ccs_features_t       features,
		size_t               num_configurations,
		ccs_configuration_t *configurations,
		size_t              *num_configurations_ret);

	/** The features tuner tell interface see ccs_features_tuner_tell */
	ccs_result_t (*tell)(
		ccs_features_tuner_t       features_tuner,
		size_t                     num_evaluations,
		ccs_features_evaluation_t *evaluations);

	/**
	 * The features tuner get_optima interface see
	 * ccs_features_tuner_get_optima
	 */
	ccs_result_t (*get_optima)(
		ccs_features_tuner_t       features_tuner,
		ccs_features_t             features,
		size_t                     num_evaluations,
		ccs_features_evaluation_t *evaluations,
		size_t                    *num_evaluations_ret);

	/**
	 * The features tuner get_history interface see
	 * ccs_features_tuner_get_history
	 */
	ccs_result_t (*get_history)(
		ccs_features_tuner_t       features_tuner,
		ccs_features_t             features,
		size_t                     num_evaluations,
		ccs_features_evaluation_t *evaluations,
		size_t                    *num_evaluations_ret);

	/**
	 * The features tuner suggest interface see ccs_features_tuner_suggest,
	 * can be NULL */
	ccs_result_t (*suggest)(
		ccs_features_tuner_t features_tuner,
		ccs_features_t       features,
		ccs_configuration_t *configuration);

	/**
	 * The tuner serialization interface, can be NULL, in which case
	 * common tuner data, history and optima will be serialized
	 */
	ccs_result_t (*serialize_user_state)(
		ccs_features_tuner_t features_tuner,
		size_t               sate_size,
		void                *state,
		size_t              *state_size_ret);

	/**
	 * The features_tuner deserialization interface, can be NULL, in which
	 * case, the history will be set through the tell interface
	 */
	ccs_result_t (*deserialize_state)(
		ccs_features_tuner_t       features_tuner,
		size_t                     size_history,
		ccs_features_evaluation_t *history,
		size_t                     num_optima,
		ccs_features_evaluation_t *optima,
		size_t                     state_size,
		const void                *state);
};

/**
 * a commodity type to represent a user defined features tuner callback vector.
 */
typedef struct ccs_user_defined_features_tuner_vector_s
	ccs_user_defined_features_tuner_vector_t;

/**
 * Create a new user defined features tuner.
 * @param[in] name the name of the features tuner
 * @param[in] configuration_space the configuration space to explore
 * @param[in] feature_space the feature space
 * @param[in] objective_space the objective space to optimize
 * @param[in] vector the vector of callbacks implementing the features tuner
 * interface
 * @param[in] tuner_data a pointer to the features tuner internal data
 *                       structures. Can be NULL
 * @param[out] features_tuner_ret a pointer to the variable that will contain
 *                                the newly created features tuner
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p configuration_space is not a
 * valid CCS configuration space; or if \p objective_space is not a valid CCS
 * objective space; or if \p feature_space is not a valid CCS feature space
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p name is NULL; or if \p
 * features_tuner_ret is NULL; or if \p vector is NULL; or if any interface
 * pointer except suggest is NULL
 * @return #CCS_RESULT_ERROR_OUT_OF_MEMORY if there was not enough memory to
 * allocate the new features tuner instance
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_create_user_defined_features_tuner(
	const char                               *name,
	ccs_configuration_space_t                 configuration_space,
	ccs_feature_space_t                       feature_space,
	ccs_objective_space_t                     objective_space,
	ccs_user_defined_features_tuner_vector_t *vector,
	void                                     *tuner_data,
	ccs_features_tuner_t                     *features_tuner_ret);

/**
 * Get the user defined features tuner internal data pointer.
 * @param[in] features_tuner
 * @param[out] tuner_data_ret
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p features_tuner is not a valid
 * CCS features tuner
 * @return #CCS_RESULT_ERROR_INVALID_FEATURES_TUNER if \p tuner is not a user
 * defined features tuner
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p tuner_data_ret is NULL
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_user_defined_features_tuner_get_tuner_data(
	ccs_features_tuner_t features_tuner,
	void               **tuner_data_ret);

#ifdef __cplusplus
}
#endif

#endif //_CCS_FEATURES_TUNER_H
