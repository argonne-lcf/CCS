#ifndef _CCS_PARAMETER_H
#define _CCS_PARAMETER_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file parameter.h
 * Parameters are parameters the when grouped together define a tuning
 * context (see context.h). Most parameters can be sampled according to a
 * distribution's dimension (see distribution.h).
 */

/**
 * CCS parameter types.
 */
enum ccs_parameter_type_e {
	/**
	 * A numerical parameter, over intergers or floating point values.
	 */
	CCS_PARAMETER_TYPE_NUMERICAL,
	/**
	 * A categorical parameter. A set of values  with no intrinsic
	 * ordering between its elements.
	 */
	CCS_PARAMETER_TYPE_CATEGORICAL,
	/**
	 * An ordinal parameter. A set of values  with an intrinsic
	 * ordering between its elements.
	 */
	CCS_PARAMETER_TYPE_ORDINAL,
	/**
	 * A discrete parameter. A set of numerical values.
	 */
	CCS_PARAMETER_TYPE_DISCRETE,
	/**
	 * A string parameter. Cannot be sampled.
	 */
	CCS_PARAMETER_TYPE_STRING,
	/** Guard */
	CCS_PARAMETER_TYPE_MAX,
	/** Try forcing 32 bits value for bindings */
	CCS_PARAMETER_TYPE_FORCE_32BIT = INT_MAX
};

/**
 * A commodity type to represent a parameter type.
 */
typedef enum ccs_parameter_type_e ccs_parameter_type_t;

/**
 * Create a new numerical parameter of the specified data_type.
 * @param[in] name name of the parameter
 * @param[in] data_type type of numerical data
 * @param[in] lower lower bound (included)
 * @param[in] upper upper bound (excluded)
 * @param[in] quantization quantization of the values. 0 means no quantization
 * @param[in] default_value default value of the parameter
 * @param[out] parameter_ret a pointer to the variable that will hold the
 *                                newly created numerical parameter
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_VALUE if \p name is NULL; or if \p parameter_ret
 *                             is NULL; or if quantization is less than 0; or if
 *                             default value is not a valid value for the
 *                             parameter
 * @return #CCS_INVALID_TYPE if data_type is neither #CCS_NUM_FLOAT or
 *                            #CCS_NUM_INTEGER
 * @return #CCS_OUT_OF_MEMORY if there was a lack of memory to allocate the new
 *                             numerical parameter
 */
extern ccs_error_t
ccs_create_numerical_parameter(
	const char        *name,
	ccs_numeric_type_t data_type,
	ccs_numeric_t      lower,
	ccs_numeric_t      upper,
	ccs_numeric_t      quantization,
	ccs_numeric_t      default_value,
	ccs_parameter_t   *parameter_ret);

/**
 * Get the properties used to create a numerical parameter.
 * @param[in] parameter
 * @param[out] data_type_ret a pointer to the variable that will contain the
 *                           data type of the numerical parameter
 * @param[out] lower_ret a pointer to the variable that will contain the lower
 *                       bound of the numerical parameter
 * @param[out] upper_ret a pointer to the variable that will contain the upper
 *                       bound of the numerical parameter
 * @param[out] quantization_ret a pointer that will contain the quantization of
 *                              the numerical parameter
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_OBJECT if \p parameter is not a valid CCS
 *                              parameter
 * @return #CCS_INVALID_PARAMETER if \p parameter is not a
 *                                      numerical parameter
 * @return #CCS_INVALID_VALUE if \p data_type_ret is NULL and \p lower_ret is
 *                             NULL and \p upper_ret is NULL and \p
 *                             quantization_ret is NULL
 */
extern ccs_error_t
ccs_numerical_parameter_get_properties(
	ccs_parameter_t     parameter,
	ccs_numeric_type_t *data_type_ret,
	ccs_numeric_t      *lower_ret,
	ccs_numeric_t      *upper_ret,
	ccs_numeric_t      *quantization_ret);

/**
 * Create a new categorical parameter. Categorical parameters can be
 * sampled using distributions over integers.
 * @param[in] name name of the parameter
 * @param[in] num_possible_values the size of the \p possible_values array
 * @param[in] possible_values an array of \p num_possible_values values that
 *            represent the possible values that the parameter can take
 * @param[in] default_value_index the index of the default value in the
 *                                \p possible_values array
 * @param[out] parameter_ret a pointer to the variable that will hold the
 *                                newly created categorical parameter
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_VALUE if \p name is NULL; or if \p parameter_ret
 *                             is NULL
 * @return #CCS_OUT_OF_MEMORY if there was a lack of memory to allocate the new
 *                             parameter
 */
extern ccs_error_t
ccs_create_categorical_parameter(
	const char      *name,
	size_t           num_possible_values,
	ccs_datum_t     *possible_values,
	size_t           default_value_index,
	ccs_parameter_t *parameter_ret);

/**
 * Get the possible values of a categorical parameter.
 * @param[in] parameter
 * @param[in] num_possible_values \p the size of the \p possible_values array
 * @param[out] possible_values an array of \p num_possible_values values that
 *                             will contain the returned possible values. Can be
 *                             NULL
 * @param[out] num_possible_values_ret a pointer to a variable that will contain
 *                                     the number of values that are or would be
 *                                     returned. Can be NULL
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_OBJECT if \p parameter is not a valid CCS
 *                              parameter
 * @return #CCS_INVALID_PARAMETER if \p parameter is not a
 *                                      categorical parameter
 * @return #CCS_INVALID_VALUE if \p possible_values is NULL and \p
 *                             num_possible_values is greater than 0; or if \p
 *                             possible_values is NULL and \p
 *                             num_possible_values_ret is NULL; or if
 *                             num_possible_values is less than the number of
 *                             values that would be returned
 */
extern ccs_error_t
ccs_categorical_parameter_get_values(
	ccs_parameter_t parameter,
	size_t          num_possible_values,
	ccs_datum_t    *possible_values,
	size_t         *num_possible_values_ret);

/**
 * Create a new ordinal parameter. Ordinal parameters can be
 * sampled using distributions over integers. Their elements have an intrinsic
 * ordering which is the same as the orders of the provided values.
 * @param[in] name name of the parameter
 * @param[in] num_possible_values the size of the \p possible_values array
 * @param[in] possible_values an array of \p num_possible_values values that
 *            represent the possible values that the parameter can take
 * @param[in] default_value_index the index of the default value in the
 *                                \p possible_values array
 * @param[out] parameter_ret a pointer to the variable that will hold the
 *                                newly created ordinal parameter
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_VALUE if \p name is NULL; or if \p parameter_ret
 *                             is NULL
 * @return #CCS_OUT_OF_MEMORY if there was a lack of memory to allocate the new
 *                             parameter
 */
extern ccs_error_t
ccs_create_ordinal_parameter(
	const char      *name,
	size_t           num_possible_values,
	ccs_datum_t     *possible_values,
	size_t           default_value_index,
	ccs_parameter_t *parameter_ret);

/**
 * Get the possible values of an ordinal parameter.
 * @param[in] parameter
 * @param[in] num_possible_values \p the size of the \p possible_values array
 * @param[out] possible_values an array of \p num_possible_values values that
 *                             will contain the returned possible values. Can be
 *                             NULL
 * @param[out] num_possible_values_ret a pointer to a variable that will contain
 *                                     the number of values that are or would be
 *                                     returned. Can be NULL
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_OBJECT if \p parameter is not a valid CCS
 *                              parameter
 * @return #CCS_INVALID_PARAMETER if \p parameter is not an ordinal
 *                                      parameter
 * @return #CCS_INVALID_VALUE if \p possible_values is NULL and \p
 *                             num_possible_values is greater than 0; or if \p
 *                             possible_values is NULL and \p
 *                             num_possible_values_ret is NULL; or if
 *                             num_possible_values is less than the number of
 *                             values that would be returned
 */
extern ccs_error_t
ccs_ordinal_parameter_get_values(
	ccs_parameter_t parameter,
	size_t          num_possible_values,
	ccs_datum_t    *possible_values,
	size_t         *num_possible_values_ret);

/**
 * Compare two values in the context of an ordinal parameter.
 * @param[in] parameter
 * @param[in] value1 the first value to compare
 * @param[in] value2 the second value to compare
 * @param[out] comp_ret the result of the comparison, -1, 1, or 0 if \p value1
 *                      is found to be respectively lesser than, grater then or
 *                      equal to value2
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_OBJECT if \p parameter is not a valid CCS
 * @return #CCS_INVALID_PARAMETER if \p parameter is not an ordinal
 *                                      parameter
 * @return #CCS_INVALID_VALUE if \p comp_ret is NULL; or if value1 or value2
 *                             are not one of the parameter possible values
 */
extern ccs_error_t
ccs_ordinal_parameter_compare_values(
	ccs_parameter_t parameter,
	ccs_datum_t     value1,
	ccs_datum_t     value2,
	ccs_int_t      *comp_ret);

/**
 * Create a new discrete parameter. Discrete parameters can be
 * sampled using distributions over integers. Their elements must be of type
 * #CCS_INTEGER or #CCS_FLOAT.
 * @param[in] name name of the parameter
 * @param[in] num_possible_values the size of the \p possible_values array
 * @param[in] possible_values an array of \p num_possible_values values that
 *            represent the possible values that the parameter can take.
 * @param[in] default_value_index the index of the default value in the
 *                                \p possible_values array
 * @param[out] parameter_ret a pointer to the variable that will hold the
 *                                newly created discrete parameter
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_VALUE if \p name is NULL; or if \p parameter_ret
 *                             is NULL
 * @return #CCS_OUT_OF_MEMORY if there was a lack of memory to allocate the new
 *                             parameter
 */
extern ccs_error_t
ccs_create_discrete_parameter(
	const char      *name,
	size_t           num_possible_values,
	ccs_datum_t     *possible_values,
	size_t           default_value_index,
	ccs_parameter_t *parameter_ret);

/**
 * Get the possible values of an discrete parameter.
 * @param[in] parameter
 * @param[in] num_possible_values \p the size of the \p possible_values array
 * @param[out] possible_values an array of \p num_possible_values values that
 *                             will contain the returned possible values. Can be
 *                             NULL
 * @param[out] num_possible_values_ret a pointer to a variable that will contain
 *                                     the number of values that are or would be
 *                                     returned. Can be NULL
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_OBJECT if \p parameter is not a valid CCS
 *                              parameter
 * @return #CCS_INVALID_PARAMETER if \p parameter is not a discrete
 *                                      parameter
 * @return #CCS_INVALID_VALUE if \p possible_values is NULL and \p
 *                             num_possible_values is greater than 0; or if \p
 *                             possible_values is NULL and \p
 *                             num_possible_values_ret is NULL; or if
 *                             num_possible_values is less than the number of
 *                             values that would be returned
 */
extern ccs_error_t
ccs_discrete_parameter_get_values(
	ccs_parameter_t parameter,
	size_t          num_possible_values,
	ccs_datum_t    *possible_values,
	size_t         *num_possible_values_ret);

/**
 * Create an new parameter representing an undetermined string, to be used
 * within feature space. Cannot be sampled and thus doesn't have a default
 * value. Checks will always return valid unless the value is not a string.
 * @param[in] name name of the parameter
 * @param[out] parameter_ret a pointer to the variable that will hold the
 *                                newly created string parameter
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_VALUE if \p name is NULL; or if \p parameter_ret
 *                             is NULL
 * @return #CCS_OUT_OF_MEMORY if there was a lack of memory to allocate the new
 *                             parameter
 */
extern ccs_error_t
ccs_create_string_parameter(const char *name, ccs_parameter_t *parameter_ret);

/**
 * Get the type of a parameter.
 * @param[in] parameter
 * @param[out] type_ret a pointer to the variable that will contain the type of
 *                      the parameter
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_VALUE if \p type_ret is NULL
 * @return #CCS_INVALID_OBJECT if \p distribution is not a valid CCS
 *                              parameter
 */
extern ccs_error_t
ccs_parameter_get_type(
	ccs_parameter_t       parameter,
	ccs_parameter_type_t *type_ret);

/**
 * Get the default value of an parameter.
 * @param[in] parameter
 * @param[out] value_ret pointer to the variable that will contain the default
 *                       value of the parameter
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_VALUE if \p value_ret is NULL
 * @return #CCS_INVALID_OBJECT if \p distribution is not a valid CCS
 *                              parameter
 */
extern ccs_error_t
ccs_parameter_get_default_value(
	ccs_parameter_t parameter,
	ccs_datum_t    *value_ret);

/**
 * Get the name of a parameter.
 * @param[in] parameter
 * @param[out] name_ret a pointer to the variable that will contain a pointer to
 *                      the name of the parameter
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_VALUE if \p name_ret is NULL
 * @return #CCS_INVALID_OBJECT if \p distribution is not a valid CCS
 *                              parameter
 */
extern ccs_error_t
ccs_parameter_get_name(ccs_parameter_t parameter, const char **name_ret);

/**
 * Get the default distribution of a parameter.
 * @param[in] parameter
 * @param[out] distribution_ret a pointer to the variable that will contained
 *                              the returned distribution
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_OBJECT if \p parameter is not a valid CCS
 *                              parameter
 * @return #CCS_INVALID_VALUE if \p distribution_ret is NULL
 */
extern ccs_error_t
ccs_parameter_get_default_distribution(
	ccs_parameter_t     parameter,
	ccs_distribution_t *distribution_ret);

/**
 * Check if a value is acceptable for a parameter.
 * @param[in] parameter
 * @param[in] value the value to check the validity of
 * @param[out] result_ret a pointer to the variable that will contain the result
 *                        of the check
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_OBJECT if \p parameter is not a valid CCS
 *                              parameter
 * @return #CCS_INVALID_VALUE if \p result_ret is NULL
 */
extern ccs_error_t
ccs_parameter_check_value(
	ccs_parameter_t parameter,
	ccs_datum_t     value,
	ccs_bool_t     *result_ret);

/**
 * Check if an array of values are acceptable for a parameter.
 * @param[in] parameter
 * @param[in] num_values the size of the \p values array
 * @param[in] values an array of \p num_values values to check
 * @param[out] results an array of \p num_values cc_bool_t values that will
 *                     contain the results the individual checks
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_OBJECT if \p parameter is not a valid CCS
 *                              parameter
 * @return #CCS_INVALID_VALUE if \p values or \p results are NULL and \p
 *                             num_values is greater than 0
 */
extern ccs_error_t
ccs_parameter_check_values(
	ccs_parameter_t    parameter,
	size_t             num_values,
	const ccs_datum_t *values,
	ccs_bool_t        *results);

/**
 * Check if a value is acceptable for a parameter and if it is return a
 * marshalled value.
 * @param[in] parameter
 * @param[in] value the value to check the validity of
 * @param[out] value_ret a pointer to the variable that will contain the
 *                       marshalled value or #ccs_inactive if the value is not
 *                       valid
 * @param[out] result_ret a pointer to the variable that will contain the result
 *                        of the check
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_OBJECT if \p parameter is not a valid CCS
 *                              parameter
 * @return #CCS_INVALID_VALUE if \p result_ret or \p value_ret is NULL
 */
extern ccs_error_t
ccs_parameter_validate_value(
	ccs_parameter_t parameter,
	ccs_datum_t     value,
	ccs_datum_t    *value_ret,
	ccs_bool_t     *result_ret);

/**
 * Check if an array of values are acceptable for a parameter and return
 * marshalled values if they are valid.
 * @param[in] parameter
 * @param[in] num_values the size of the \p values array
 * @param[in] values an array of \p num_values values to check
 * @param[out] values_ret an array of \p num_values values that will contain the
 *                        marshalled values, or #ccs_inactive if values are
 *                        invalid
 * @param[out] results an array of \p num_values cc_bool_t values that will
 *                     contain the results the individual checks
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_OBJECT if \p parameter is not a valid CCS
 *                              parameter
 * @return #CCS_INVALID_VALUE if \p values or \p results or \p values_ret are
 *                             NULL and \p num_values is greater than 0
 */
extern ccs_error_t
ccs_parameter_validate_values(
	ccs_parameter_t    parameter,
	size_t             num_values,
	const ccs_datum_t *values,
	ccs_datum_t       *values_ret,
	ccs_bool_t        *results);

/**
 * Convert numerical samples into samples from the parameter.
 * @param[in] parameter
 * @param[in] oversampling if #CCS_TRUE values can be outside of the
 *                         parameter sampling interval (see
 *                         #ccs_parameter_sampling_interval), else they
 *                         must be within the sampling interval
 * @param[in] num_values the size of the \p values array
 * @param[in] values an array of \p num_values values to convert
 * @param[out] results an array of \p num_values values that will contain the
 *                     converted values. If oversampling is #CCS_TRUE values
 *                     outside of the sampling interval will be converted to
 *                     #ccs_inactive
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_OBJECT if \p parameter is not a valid CCS
 *                              parameter
 * @return #CCS_INVALID_VALUE if \p values or \p results are NULL and \p
 *                             num_values is greater than 0
 */
extern ccs_error_t
ccs_parameter_convert_samples(
	ccs_parameter_t      parameter,
	ccs_bool_t           oversampling,
	size_t               num_values,
	const ccs_numeric_t *values,
	ccs_datum_t         *results);

/**
 * Get a sample from the parameter sampled by a given distribution.
 * @param[in] parameter
 * @param[in] distribution the distribution to sample the parameter with
 * @param[in] rng the random number generator to use
 * @param[out] value_ret a pointer to the variable that will contain the
 *                       sampled value
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_OBJECT if \p parameter is not a valid CCS
 *                              parameter; or if \p distribution is not a
 *                              valid CCS distribution; or if \p rng is not a
 *                              valid CCS random number generator
 * @return #CCS_INVALID_VALUE if \p value_ret is NULL
 * @return #CCS_SAMPLING_UNSUCCESSFUL if the sample could not be generated,
 *                                     because the probability of obtaining a
 *                                     valid sample is too low
 */
extern ccs_error_t
ccs_parameter_sample(
	ccs_parameter_t    parameter,
	ccs_distribution_t distribution,
	ccs_rng_t          rng,
	ccs_datum_t       *value_ret);

/**
 * Get samples from the parameter sampled by a given distribution.
 * @param[in] parameter
 * @param[in] distribution the distribution to sample the parameter with
 * @param[in] rng the random number generator to use
 * @param[in] num_values the number of queried values
 * @param[out] values an array of \p num_values values will contain the
 *                    sampled values
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_OBJECT if \p parameter is not a valid CCS
 *                              parameter; or if \p distribution is not a
 *                              valid CCS distribution; or if \p rng is not a
 *                              valid CCS random number generator
 * @return #CCS_INVALID_VALUE if \p values is NULL and \p num_values is greater
 *                             than 0
 * @return #CCS_SAMPLING_UNSUCCESSFUL if not enough samples could not be
 *                                     generated, because the probability of
 *                                     obtaining a valid sample is too low
 */
extern ccs_error_t
ccs_parameter_samples(
	ccs_parameter_t    parameter,
	ccs_distribution_t distribution,
	ccs_rng_t          rng,
	size_t             num_values,
	ccs_datum_t       *values);

/**
 * Get the valid sampling interval of the parameter.
 * @param[in] parameter
 * @param[out] interval_ret a pointer to the variable that will contain the
 *                          returned interval
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_OBJECT if \p parameter is not a valid CCS
 *                              parameter
 * @return #CCS_INVALID_VALUE if \p interval_ret is NULL
 */
extern ccs_error_t
ccs_parameter_sampling_interval(
	ccs_parameter_t parameter,
	ccs_interval_t *interval_ret);
#ifdef __cplusplus
}
#endif

#endif //_CCS_PARAMETER_H
