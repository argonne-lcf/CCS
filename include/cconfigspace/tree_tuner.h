#ifndef _CCS_TREE_TUNER_H
#define _CCS_TREE_TUNER_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file tree_tuner.h A CCS tree tuner defines an ask and tell interface to
 * optimize an objective space (see objective_space.h) given a tree space (see
 * tree_space.h). The tuner will propose tree configurations (see
 * tree_configuration.h) and the user will return tree evaluations (see
 * tree_evaluation.h).
 */

/**
 * CCS supported tuner types.
 */
enum ccs_tree_tuner_type_e {
	/** A random tuner */
	CCS_TREE_TUNER_TYPE_RANDOM,
	/** A user defined tuner */
	CCS_TREE_TUNER_TYPE_USER_DEFINED,
	/** Guard */
	CCS_TREE_TUNER_TYPE_MAX,
	/** Try forcing 32 bits value for bindings */
	CCS_TREE_TUNER_TYPE_32BIT = INT_MAX
};

/**
 * A commodity type to represent CCS tree tuner types.
 */
typedef enum ccs_tree_tuner_type_e ccs_tree_tuner_type_t;

/**
 * Get the type of a tree tuner.
 * @param [in] tuner
 * @param [out] type_ret a pointer to the variable that will contain the
 *                       returned tree tuner type
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p tuner is not a valid CCS tree
 * tuner
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p type_ret is NULL
 */
extern ccs_result_t
ccs_tree_tuner_get_type(ccs_tree_tuner_t tuner, ccs_tree_tuner_type_t *type_ret);

/**
 * Get the name of a tree tuner.
 * @param[in] tuner
 * @param[out] name_ret a pointer to the variable that will contain a pointer to
 *                      the name of the tree tuner
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p tuner is not a valid CCS tuner
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p name_ret is NULL
 */
extern ccs_result_t
ccs_tree_tuner_get_name(ccs_tree_tuner_t tuner, const char **name_ret);

/**
 * Get the associated tree space.
 * @param[in] tuner
 * @param[out] tree_space_ret a pointer to the variable that will contain
 *                            the tree space
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p tuner is not a valid CCS tree
 * tuner
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p tree_space_ret is NULL
 */
extern ccs_result_t
ccs_tree_tuner_get_tree_space(
	ccs_tree_tuner_t  tuner,
	ccs_tree_space_t *tree_space_ret);

/**
 * Get the associated objective space.
 * @param[in] tuner
 * @param[out] objective_space_ret a pointer to the variable that will
 *                                 contain the objective space
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p tuner is not a valid CCS tree
 * tuner
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p objective_space_ret is NULL
 */
extern ccs_result_t
ccs_tree_tuner_get_objective_space(
	ccs_tree_tuner_t       tuner,
	ccs_objective_space_t *objective_space_ret);

/**
 * Ask a tree tuner for a set of configurations to evaluate. Configuration's
 * ownership is transferred to the user who doesn't need to retain them, but
 * will need to release them once the user is done using them.
 * @param[in,out] tuner
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
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p tuner is not a valid CCSi tree
 * tuner
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p configurations is NULL and \p
 * num_configurations is greater than 0; or if \p configurations and \p
 * num_configurations_ret are both NULL
 * @return #CCS_RESULT_ERROR_SAMPLING_UNSUCCESSFUL if no or not enough valid
 * configurations could be sampled. Configurations that could be sampled will be
 * returned contiguously, and the rest will be NULL
 * @return #CCS_RESULT_ERROR_OUT_OF_MEMORY if there was not enough memory to
 * allocate new configurations. Configurations that could be allocated will be
 * returned, and the rest will be NULL
 */
extern ccs_result_t
ccs_tree_tuner_ask(
	ccs_tree_tuner_t          tuner,
	size_t                    num_configurations,
	ccs_tree_configuration_t *configurations,
	size_t                   *num_configurations_ret);

/**
 * Give a list of results to a tree tuner through tree evaluations.
 * @param[in,out] tuner
 * @param[in] num_evaluations the size of the \p evaluations array
 * @param[in] evaluations an array of \p num_evaluations to provide to the
 *                        tuner
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p tuner is not a valid CCS tree
 * tuner; or if one of the evaluations is not a valid CCS tree evaluation
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p evaluations is NULL and \p
 * num_evaluations is greater than 0
 * @return #CCS_RESULT_ERROR_INVALID_EVALUATION if an evaluation is not a valid
 * tree evaluation for the problem the tuner is optimizing
 * @return #CCS_RESULT_ERROR_OUT_OF_MEMORY if there was not enough memory to
 * allocate internal data structures
 */
extern ccs_result_t
ccs_tree_tuner_tell(
	ccs_tree_tuner_t       tuner,
	size_t                 num_evaluations,
	ccs_tree_evaluation_t *evaluations);

/**
 * Ask a tree tuner to suggest a good configuration.
 * @param[in,out] tuner
 * @param[out] configuration a pointer to the variable that will contain the
 *                           suggested configuration
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p tuner is not a valid CCS tree
 * tuner
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p configuration is NULL
 * @return #CCS_RESULT_ERROR_UNSUPPORTED_OPERATION if the tuner does not support
 * the suggest interface
 * @return #CCS_RESULT_ERROR_OUT_OF_MEMORY if there was not enough memory to
 * allocate new configurations
 */
extern ccs_result_t
ccs_tree_tuner_suggest(
	ccs_tree_tuner_t          tuner,
	ccs_tree_configuration_t *configuration);

/**
 * Ask a tree tuner for the discovered Pareto front. For single objective
 * objective spaces this would be the best point found.
 * @param[in] tuner
 * @param[in] num_evaluations the size of the \p evaluations array
 * @param[out] evaluations an array of \p num_evaluations that will contain the
 *                         optimal evaluations
 * @param[out] num_evaluations_ret a pointer to the variable that will contain
 *                                 the number of evaluations that are or would
 *                                 be returned
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p tuner is not a valid CCS tree
 * tuner
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p evaluations is NULL and
 * num_evaluations is greater than 0; or if \p evaluations is NULL and \p
 * num_evaluations_ret is NULL
 */
extern ccs_result_t
ccs_tree_tuner_get_optima(
	ccs_tree_tuner_t       tuner,
	size_t                 num_evaluations,
	ccs_tree_evaluation_t *evaluations,
	size_t                *num_evaluations_ret);

/**
 * Ask a tree tuner for the evaluation history.
 * @param[in] tuner
 * @param[in] num_evaluations the size of the \p evaluations array
 * @param[out] evaluations an array of \p num_evaluations that will contain the
 *                         the history
 * @param[out] num_evaluations_ret a pointer to the variable that will contain
 *                                 the number of evaluations that are or would
 *                                 be returned
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p tuner is not a valid CCS tree
 * tuner
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p evaluations is NULL and
 * num_evaluations is greater than 0; or if \p evaluations is NULL and \p
 * num_evaluations_ret is NULL
 */
extern ccs_result_t
ccs_tree_tuner_get_history(
	ccs_tree_tuner_t       tuner,
	size_t                 num_evaluations,
	ccs_tree_evaluation_t *evaluations,
	size_t                *num_evaluations_ret);

/**
 * Create a new random tree tuner. The random tuner should be viewed as a
 * baseline for evaluating tuners, and as a tool for developing interfaces.
 * @param[in] name the name of the tree tuner
 * @param[in] tree_space the configuration space to explore
 * @param[in] objective_space the objective space to potimize
 * @param[out] tuner_ret a pointer to the variable that will contain the
 *                            newly created tree tuner
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p tree_space is not a valid CCS
 * tree space; or if \p objective_space is not a valid CCS objective space
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p name is NULL; or if \p
 * tuner_ret is NULL
 * @return #CCS_RESULT_ERROR_OUT_OF_MEMORY if there was not enough memory to
 * allocate the new tree tuner instance
 */
extern ccs_result_t
ccs_create_random_tree_tuner(
	const char           *name,
	ccs_tree_space_t      tree_space,
	ccs_objective_space_t objective_space,
	ccs_tree_tuner_t     *tuner_ret);

/**
 * A structure that define the callbacks the user must provide to create a user
 * defined tree tuner.
 */
struct ccs_user_defined_tree_tuner_vector_s {
	/**
	 * The deletion callback that will be called once the reference count
	 * of the tuner reaches 0.
	 */
	ccs_result_t (*del)(ccs_tree_tuner_t tuner);

	/** The tree tuner ask interface see ccs_tree_tuner_ask */
	ccs_result_t (*ask)(
		ccs_tree_tuner_t          tuner,
		size_t                    num_configurations,
		ccs_tree_configuration_t *configurations,
		size_t                   *num_configurations_ret);

	/** The tree tuner tell interface see ccs_tree_tuner_tell */
	ccs_result_t (*tell)(
		ccs_tree_tuner_t       tuner,
		size_t                 num_evaluations,
		ccs_tree_evaluation_t *evaluations);

	/**
	 * The tree tuner get_optima interface see
	 * ccs_tree_tuner_get_optima
	 */
	ccs_result_t (*get_optima)(
		ccs_tree_tuner_t       tuner,
		size_t                 num_evaluations,
		ccs_tree_evaluation_t *evaluations,
		size_t                *num_evaluations_ret);

	/**
	 * The tree tuner get_history interface see ccs_tree_tuner_get_history
	 */
	ccs_result_t (*get_history)(
		ccs_tree_tuner_t       tuner,
		size_t                 num_evaluations,
		ccs_tree_evaluation_t *evaluations,
		size_t                *num_evaluations_ret);

	/** The tree tuner suggest interface see ccs_tree_tuner_suggest, can be
	 * NULL
	 */
	ccs_result_t (*suggest)(
		ccs_tree_tuner_t          tuner,
		ccs_tree_configuration_t *configuration);

	/**
	 * The tree tuner serialization interface, can be NULL, in which case
	 * common tuner data, history and optima will be serialized
	 */
	ccs_result_t (*serialize_user_state)(
		ccs_tree_tuner_t tuner,
		size_t           sate_size,
		void            *state,
		size_t          *state_size_ret);

	/**
	 * The tree tuner deserialization interface, can be NULL, in which case,
	 * the history will be set through the tell interface
	 */
	ccs_result_t (*deserialize_state)(
		ccs_tree_tuner_t       tuner,
		size_t                 size_history,
		ccs_tree_evaluation_t *history,
		size_t                 num_optima,
		ccs_tree_evaluation_t *optima,
		size_t                 state_size,
		const void            *state);
};

/**
 * a commodity type to represent a user defined tuner callback vector.
 */
typedef struct ccs_user_defined_tree_tuner_vector_s
	ccs_user_defined_tree_tuner_vector_t;

/**
 * Create a new user defined tree tuner.
 * @param[in] name the name of the tuner
 * @param[in] tree_space the tree space to explore
 * @param[in] objective_space the objective space to optimize
 * @param[in] vector the vector of callbacks implementing the tuner interface
 * @param[in] tuner_data a pointer to the tuner internal data structures. Can be
 *                       NULL
 * @param[out] tuner_ret a pointer to the variable that will contain the newly
 *                       created tuner
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p tree_space is not a valid CCS
 *                             tree space; or if \p objective_space is
 *                             not a valid CCS objective space
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p name is NULL; or if \p
 * tuner_ret is NULL; or if \p vector is NULL; or if any interface pointer
 * except suggest is NULL
 * @return #CCS_RESULT_ERROR_OUT_OF_MEMORY if there was not enough memory to
 * allocate the newi tree tuner instance
 */
extern ccs_result_t
ccs_create_user_defined_tree_tuner(
	const char                           *name,
	ccs_tree_space_t                      tree_space,
	ccs_objective_space_t                 objective_space,
	ccs_user_defined_tree_tuner_vector_t *vector,
	void                                 *tuner_data,
	ccs_tree_tuner_t                     *tuner_ret);

/**
 * Get the user defined tree tuner internal data pointer.
 * @param[in] tuner
 * @param[out] tuner_data_ret
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p tuner is not a valid CCS tree
 * tuner
 * @return #CCS_RESULT_ERROR_INVALID_TUNER if \p tuner is not a user defined
 * tree tuner
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p tuner_data_ret is NULL
 */
extern ccs_result_t
ccs_user_defined_tree_tuner_get_tuner_data(
	ccs_tree_tuner_t tuner,
	void           **tuner_data_ret);

#ifdef __cplusplus
}
#endif

#endif //_CCS_TREE_TUNER_H
