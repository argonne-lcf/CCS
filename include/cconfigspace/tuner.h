#ifndef _CCS_TUNER_H
#define _CCS_TUNER_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file tuner.h
 * A CCS tuner defines an ask and tell interface to optimize an objective space
 * (see objective_space.h) given a configuration spaces (see
 * configuration_space.h). The tuner will propose configurations (see
 * configuration.h) and the user will return evaluations (see evaluation.h).
 */

/**
 * CCS supported tuner types.
 */
enum ccs_tuner_type_e {
	/** A random tuner */
	CCS_TUNER_RANDOM,
	/** A user defined tuner */
	CCS_TUNER_USER_DEFINED,
	/** Guard */
	CCS_TUNER_TYPE_MAX,
	/** Try forcing 32 bits value for bindings */
	CCS_TUNER_TYPE_32BIT = INT_MAX
};

/**
 * A commodity type to represent CCS tuner types.
 */
typedef enum ccs_tuner_type_e ccs_tuner_type_t;

/**
 * Get the type of a tuner.
 * @param [in] tuner
 * @param [out] type_ret a pointer to the variable that will contain the
 *                       returned tuner type
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p tuner is not a valid CCS tuner
 * @return -#CCS_INVALID_VALUE if \p type_ret is NULL
 */
extern ccs_result_t
ccs_tuner_get_type(ccs_tuner_t       tuner,
                   ccs_tuner_type_t *type_ret);

/**
 * Get the name of a tuner.
 * @param[in] tuner
 * @param[out] name_ret a pointer to the variable that will contain a pointer to
 *                      the name of the tuner
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_VALUE if \p name_ret is NULL
 * @return -#CCS_INVALID_OBJECT if \p tuner is not a valid CCS tuner
 */
extern ccs_result_t
ccs_tuner_get_name(ccs_tuner_t   tuner,
                   const char  **name_ret);

/**
 * Get the associated configuration space.
 * @param[in] tuner
 * @param[out] configuration_space_ret a pointer to the variable that will
 *                                     contain the configuration space
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p tuner is not a valid CCS tuner
 * @return -#CCS_INVALID_VALUE if \p configuration_space_ret is NULL
 */
extern ccs_result_t
ccs_tuner_get_configuration_space(
	ccs_tuner_t                tuner,
	ccs_configuration_space_t *configuration_space_ret);

/**
 * Get the associated objective space.
 * @param[in] tuner
 * @param[out] objective_space_ret a pointer to the variable that will
 *                                     contain the objective space
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p tuner is not a valid CCS tuner
 * @return -#CCS_INVALID_VALUE if \p objective_space_ret is NULL
 */
extern ccs_result_t
ccs_tuner_get_objective_space(ccs_tuner_t            tuner,
                              ccs_objective_space_t *objective_space_ret);

/**
 * Ask a tuner for a set of configurations to evaluate. Configuration's
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
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p tuner is not a valid CCS tuner
 * @return -#CCS_INVALID_VALUE if \p configurations is NULL and \p
 *                             num_configurations is greater than 0; or if \p
 *                             configurations and \p num_configurations_ret are
 *                             both NULL
 * @return -#CCS_SAMPLING_UNSUCCESSFUL if no or not enough valid configurations
 *                                     could be sampled. Configurations that
 *                                     could be sampled will be returned
 *                                     contiguously, and the rest will be NULL
 * @return -#CCS_OUT_OF_MEMORY if there was not enough memory to allocate new
 *                             configurations. Configurations that could be
 *                             allocated will be returned, and the rest will be
 *                             NULL
 */
extern ccs_result_t
ccs_tuner_ask(ccs_tuner_t          tuner,
              size_t               num_configurations,
              ccs_configuration_t *configurations,
              size_t              *num_configurations_ret);

/**
 * Give a list of results to a tuner through evaluations.
 * @param[in,out] tuner
 * @param[in] num_evaluations the size of the \p evaluations array
 * @param[in] evaluations an array of \p num_evaluations to provide to the
 *                        tuner
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p tuner is not a valid CCS tuner; or if one
 *                              of the evaluations is not a valid CCS
 *                              evaluation
 * @return -#CCS_INVALID_VALUE if \p evaluations is NULL and \p num_evaluations
 *                             is greater than 0
 * @return -#CCS_INVALID_EVALUATION if an evaluation is not a valid evaluation
 *                                  for the problem the tuner is optimizing
 * @return -#CCS_OUT_OF_MEMORY if there was not enough memory to allocate
 *                             internal data structures
 */
extern ccs_result_t
ccs_tuner_tell(ccs_tuner_t       tuner,
               size_t            num_evaluations,
               ccs_evaluation_t *evaluations);

/**
 * Ask a tuner to suggest a good configuration.
 * @param[in,out] tuner
 * @param[out] configuration a pointer to the variable that will contain the
 *                           suggested configuration
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p tuner is not a valid CCS tuner
 * @return -#CCS_INVALID_VALUE if \p configuration is NULL
 * @return -#CCS_UNSUPPORTED_OPERATION if the tuner does not support the suggest
 *                                     interface
 * @return -#CCS_OUT_OF_MEMORY if there was not enough memory to allocate new
 *                             configurations
 */
extern ccs_result_t
ccs_tuner_suggest(ccs_tuner_t          tuner,
                  ccs_configuration_t *configuration);

/**
 * Ask a tuner for the discovered Pareto front. For single objective objective
 * spaces this would be the best point found.
 * @param[in] tuner
 * @param[in] num_evaluations the size of the \p evaluations array
 * @param[out] evaluations an array of \p num_evaluations that will contain the
 *                         optimal evaluations
 * @param[out] num_evaluations_ret a pointer to the variable that will contain
 *                                 the number of evaluations that are or would
 *                                 be returned
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p tuner is not a valid CCS tuner
 * @return -#CCS_INVALID_VALUE if \p evaluations is NULL and num_evaluations is
 *                             greater than 0; or if \p evaluations is NULL and
 *                             \p num_evaluations_ret is NULL
 */
extern ccs_result_t
ccs_tuner_get_optimums(ccs_tuner_t       tuner,
                       size_t            num_evaluations,
                       ccs_evaluation_t *evaluations,
                       size_t           *num_evaluations_ret);

/**
 * Ask a tuner for the evaluation history.
 * @param[in] tuner
 * @param[in] num_evaluations the size of the \p evaluations array
 * @param[out] evaluations an array of \p num_evaluations that will contain the
 *                         the history
 * @param[out] num_evaluations_ret a pointer to the variable that will contain
 *                                 the number of evaluations that are or would
 *                                 be returned
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p tuner is not a valid CCS tuner
 * @return -#CCS_INVALID_VALUE if \p evaluations is NULL and num_evaluations is
 *                             greater than 0; or if \p evaluations is NULL and
 *                             \p num_evaluations_ret is NULL
 */
extern ccs_result_t
ccs_tuner_get_history(ccs_tuner_t       tuner,
                      size_t            num_evaluations,
                      ccs_evaluation_t *evaluations,
                      size_t           *num_evaluations_ret);

/**
 * Create a new random tuner. The random tuner should be viewed as a baseline
 * for evaluating tuners, and as a tool for developing interfaces.
 * @param[in] name the name of the tuner
 * @param[in] configuration_space the configuration space to explore
 * @param[in] objective_space the objective space to potimize
 * @param[in] user_data a pointer to the user data to attach to this
 *                      tuner instance
 * @param[out] tuner_ret a pointer to the variable that will contain the newly
 *                       created tuner
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p configuration_space is not a valid CCS
 *                              configuration space; or if \p objective_space is
 *                              not a valid CCS objective space
 * @return -#CCS_INVALID_VALUE if \p name is NULL; or if \p tuner_ret is NULL
 * @return -#CCS_OUT_OF_MEMORY if there was not enough memory to allocate the
 *                             new tuner instance
 */
extern ccs_result_t
ccs_create_random_tuner(const char                *name,
                        ccs_configuration_space_t  configuration_space,
                        ccs_objective_space_t      objective_space,
                        void                      *user_data,
                        ccs_tuner_t               *tuner_ret);

/**
 * A structure that define the callbacks the user must provide to create a user
 * defined tuner.
 */
struct ccs_user_defined_tuner_vector_s {
	/**
	 * The deletion callback that will be called once the reference count
	 * of the tuner reaches 0.
	 */
	ccs_result_t (*del)(
		ccs_tuner_t tuner);

	/** The tuner ask interface see ccs_tuner_ask */
	ccs_result_t (*ask)(
		ccs_tuner_t          tuner,
		size_t               num_configurations,
		ccs_configuration_t *configurations,
		size_t              *num_configurations_ret);

	/** The tuner tell interface see ccs_tuner_tell */
	ccs_result_t (*tell)(
		ccs_tuner_t       tuner,
		size_t            num_evaluations,
		ccs_evaluation_t *evaluations);

	/** The tuner get_optimums interface see ccs_tuner_get_optimums */
	ccs_result_t (*get_optimums)(
		ccs_tuner_t       tuner,
		size_t            num_evaluations,
		ccs_evaluation_t *evaluations,
		size_t           *num_evaluations_ret);

	/** The tuner get_history interface see ccs_tuner_get_history */
	ccs_result_t (*get_history)(
		ccs_tuner_t       tuner,
		size_t            num_evaluations,
		ccs_evaluation_t *evaluations,
		size_t           *num_evaluations_ret);

	/** The tuner suggest interface see ccs_tuner_suggest, can be NULL */
	ccs_result_t (*suggest)(
		ccs_tuner_t          tuner,
		ccs_configuration_t *configuration);

	/** The tuner serialization interface, can be NULL, in which case
            common tuner data, history and optimums will be serialized */
	ccs_result_t (*serialize_user_state)(
		ccs_tuner_t   tuner,
		size_t        sate_size,
		void         *state,
		size_t       *state_size_ret);

	/** The tuner deserialization interface, can be NULL, in which case,
            the history will be set through the tell interface */
	ccs_result_t (*deserialize_state)(
		ccs_tuner_t       tuner,
		size_t            size_history,
		ccs_evaluation_t *history,
		size_t            num_optimums,
		ccs_evaluation_t *optimums,
		size_t            state_size,
		const void       *state);
};

/**
 * a commodity type to represent a user defined tuner callback vector.
 */
typedef struct ccs_user_defined_tuner_vector_s ccs_user_defined_tuner_vector_t;

/**
 * Create a new user defined tuner.
 * @param[in] name the name of the tuner
 * @param[in] configuration_space the configuration space to explore
 * @param[in] objective_space the objective space to potimize
 * @param[in] user_data a pointer to the user data to attach to this
 *                      tuner instance
 * @param[in] vector the vector of callbacks implementing the tuner interface
 * @param[in] tuner_data a pointer to the tuner internal data structures. Can be
 *                       NULL
 * @param[out] tuner_ret a pointer to the variable that will contain the newly
 *                       created tuner
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p configuration_space is not a valid CCS
 *                              configuration space; or if \p objective_space is
 *                              not a valid CCS objective space
 * @return -#CCS_INVALID_VALUE if \p name is NULL; or if \p tuner_ret is NULL;
 *                             or if \p vector is NULL; or if any interface
 *                             pointer except suggest is NULL
 * @return -#CCS_OUT_OF_MEMORY if there was not enough memory to allocate the
 *                             new tuner instance
 */
extern ccs_result_t
ccs_create_user_defined_tuner(const char                      *name,
                              ccs_configuration_space_t        configuration_space,
                              ccs_objective_space_t            objective_space,
                              void                            *user_data,
                              ccs_user_defined_tuner_vector_t *vector,
                              void                            *tuner_data,
                              ccs_tuner_t                     *tuner_ret);

/**
 * Get the user defined tuner internal data pointer.
 * @param[in] tuner
 * @param[out] tuner_data_ret
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p tuner is not a valid CCS tuner
 * @return -#CCS_INVALID_TUNER if \p tuner is not a user defined tuner
 * @return -#CCS_INVALID_VALUE if \p tuner_data_ret is NULL
 */
extern ccs_result_t
ccs_user_defined_tuner_get_tuner_data(ccs_tuner_t   tuner,
                                      void        **tuner_data_ret);

#ifdef __cplusplus
}
#endif

#endif //_CCS_TUNER_H
