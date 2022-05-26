#ifndef _CCS_BASE_H
#define _CCS_BASE_H
#include <limits.h>
#include <math.h>
#include <string.h>

/**
 * @file base.h
 * Base definition of CCS objects and types.
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * A CCS floating point type
 */
typedef double   ccs_float_t;
/**
 * A CCS integer type
 */
typedef int64_t  ccs_int_t;
/**
 * A CCS boolean type
 */
typedef int32_t  ccs_bool_t;
/**
 * A CCS hashing value type
 */
typedef uint32_t ccs_hash_t;

/**
 * A structure representing a version of the CCS API.
 */
typedef struct {
	/** Revision version number */
	uint16_t revision;
	/** Patch version number */
	uint16_t patch;
	/** Minor version number */
	uint16_t minor;
	/** Major version number */
	uint16_t major;
} ccs_version_t;

/**
 * A variable containing the current version of the CCS library.
 */
extern const ccs_version_t ccs_version;

/**
 * A macro providing the true value of a ccs_bool_t.
 */
#define CCS_TRUE ((ccs_bool_t)(1))
/**
 * A macro providing the false value of a ccs_bool_t.
 */
#define CCS_FALSE ((ccs_bool_t)(0))

/**
 * A macro defining the maximum value of a ccs_int_t.
 */
#define CCS_INT_MAX INT64_MAX
/**
 * A macro defining the minimum value of a ccs_int_t.
 */
#define CCS_INT_MIN INT64_MIN
/**
 * A macro defining the (positive) infinity value of a ccs_float_t.
 */
#define CCS_INFINITY INFINITY

/**
 * An opaque type defining a CCS random generator.
 */
typedef struct _ccs_rng_s                 *ccs_rng_t;
/**
 * An opaque type defining a CCS distribution.
 */
typedef struct _ccs_distribution_s        *ccs_distribution_t;
/**
 * An opaque type defining a CCS hyperparameter.
 */
typedef struct _ccs_hyperparameter_s      *ccs_hyperparameter_t;
/**
 * An opaque type defining a CCS expression.
 */
typedef struct _ccs_expression_s          *ccs_expression_t;
/**
 * An opaque type defining a CCS context.
 */
typedef struct _ccs_context_s             *ccs_context_t;
/**
 * An opaque type defining a CCS configuration space.
 */
typedef struct _ccs_configuration_space_s *ccs_configuration_space_t;
/**
 * An opaque type defining a CCS binding.
 */
typedef struct _ccs_binding_s             *ccs_binding_t;
/**
 * An opaque type defining a CCS configuration.
 */
typedef struct _ccs_configuration_s       *ccs_configuration_t;
/**
 * An opaque type defining a CCS features space.
 */
typedef struct _ccs_features_space_s      *ccs_features_space_t;
/**
 * An opaque type defining a CCS features.
 */
typedef struct _ccs_features_s            *ccs_features_t;
/**
 * An opaque type defining a CCS objective space.
 */
typedef struct _ccs_objective_space_s     *ccs_objective_space_t;
/**
 * An opaque type defining a CCS evaluation.
 */
typedef struct _ccs_evaluation_s          *ccs_evaluation_t;
/**
 * An opaque type defining a CCS features evaluation.
 */
typedef struct _ccs_features_evaluation_s *ccs_features_evaluation_t;
/**
 * An opaque type defining a CCS tuner.
 */
typedef struct _ccs_tuner_s               *ccs_tuner_t;
/**
 * An opaque type defining a CCS features tuner.
 */
typedef struct _ccs_features_tuner_s      *ccs_features_tuner_t;
/**
 * An opaque type defining a CCS key-value store.
 */
typedef struct _ccs_map_s                 *ccs_map_t;

/**
 * The different possible return codes of a CCS function.
 * Error codes are returned negated.
 */
enum ccs_error_e {
	/** Success */
	CCS_SUCCESS,
	/** Not a CCS object or not initialized */
	CCS_INVALID_OBJECT,
	/** Parameter has an invalid value */
	CCS_INVALID_VALUE,
	/** The data type is invalid */
	CCS_INVALID_TYPE,
	/** The provided scale is invalid */
	CCS_INVALID_SCALE,
	/** The provided distribution is invalid */
	CCS_INVALID_DISTRIBUTION,
	/** The provided expression is invalid */
	CCS_INVALID_EXPRESSION,
	/** The provided hyperparameter is invalid */
	CCS_INVALID_HYPERPARAMETER,
	/** The provided configuration is invalid */
	CCS_INVALID_CONFIGURATION,
	/** The hyperparameter name is invalid */
	CCS_INVALID_NAME,
	/** The condition is invalid (unused) */
	CCS_INVALID_CONDITION,
	/** The provided tuner is invalid */
	CCS_INVALID_TUNER,
	/** The constraint graph would be invalid */
	CCS_INVALID_GRAPH,
	/** The type is not comparable (unused) */
	CCS_TYPE_NOT_COMPARABLE,
	/** The bounds are invalid (unused) */
	CCS_INVALID_BOUNDS,
	/** The index is out of bounds */
	CCS_OUT_OF_BOUNDS,
	/** Could not gather enough samples */
	CCS_SAMPLING_UNSUCCESSFUL,
	/** The expression evaluates to an inactive hyperparameter */
	CCS_INACTIVE_HYPERPARAMETER,
	/** An allocation failed due to lack of available memory */
	CCS_OUT_OF_MEMORY,
	/** The object does not support this operation */
	CCS_UNSUPPORTED_OPERATION,
	/** The provided evaluation is invalid */
	CCS_INVALID_EVALUATION,
	/** The provided features is invalid */
	CCS_INVALID_FEATURES,
	/** The provided features tuner is invalid */
	CCS_INVALID_FEATURES_TUNER,
	/** The provided file path is invalid */
	CCS_INVALID_FILE_PATH,
	/** The provided buffer or file is too short */
	CCS_NOT_ENOUGH_DATA,
	/** The handle was a duplicate */
	CCS_HANDLE_DUPLICATE,
	/** The handle was not found */
	CCS_INVALID_HANDLE,
	/** A system error occured */
	CCS_SYSTEM_ERROR,
	/** Try again */
	CCS_AGAIN,
	/** Guard */
	CCS_ERROR_MAX,
	/** Try forcing 32 bits value for bindings */
	CCS_ERROR_FORCE_32BIT = INT32_MAX
};

/**
 * A commodity type to represent CCS errors.
 */
typedef enum ccs_error_e ccs_error_t;

/**
 * The type returned by CCS functions.
 */
typedef int32_t ccs_result_t;

/**
 * CCS object types.
 */
enum ccs_object_type_e {
	CCS_RNG,                 /*!< A random number generator */
	CCS_DISTRIBUTION,        /*!< A numerical distribution */
	CCS_HYPERPARAMETER,      /*!< A hyperparameter */
	CCS_EXPRESSION,          /*!< An arithmetic expression */
	CCS_CONFIGURATION_SPACE, /*!< A configuration space */
	CCS_CONFIGURATION,       /*!< A configuration */
	CCS_OBJECTIVE_SPACE,     /*!< An objective space */
	CCS_EVALUATION,          /*!< An evaluation of a configuration */
	CCS_TUNER,               /*!< A tuner */
	CCS_FEATURES_SPACE,      /*!< A features space */
	CCS_FEATURES,            /*!< A features */
	CCS_FEATURES_EVALUATION, /*!< An evaluation of a configuration given specific features */
	CCS_FEATURES_TUNER,      /*!< A features aware tuner */
	CCS_MAP,                 /*!< A key value store */
	CCS_OBJECT_TYPE_MAX,     /*!< Guard */
	/** Try forcing 32 bits value for bindings */
	CCS_OBJECT_TYPE_FORCE_32BIT = INT32_MAX
};

/**
 * A commodity type to represent CCS object types.
 */
typedef enum ccs_object_type_e ccs_object_type_t;

/**
 * CCS supported data types.
 */
enum ccs_data_type_e {
	CCS_NONE,          /*!< An empty value */
	CCS_INTEGER,       /*!< A ccs_int_t */
	CCS_FLOAT,         /*!< A ccs_float_t */
	CCS_BOOLEAN,       /*!< A ccs_bool_t */
	CCS_STRING,        /*!< A pointer to a NULL terminated string */
	CCS_INACTIVE,      /*!< An inactive value */
	CCS_OBJECT,        /*!< A CCS object */
	CCS_DATA_TYPE_MAX, /*!< Guard */
	/** Try forcing 32 bits value for bindings */
	CCS_DATA_TYPE_FORCE_32BIT = INT32_MAX
};

/**
 * A commodity type to represent CCS data types.
 */
typedef enum ccs_data_type_e ccs_data_type_t;

/**
 * Flags that can be attached to a CCS datum.
 */
enum ccs_datum_flag_e {
	/** Empty default flags */
	CCS_FLAG_DEFAULT = 0,
	/**
	 * The value given to CCS is a pointer and is not guaranteed to stay
	 * allocated
	 */
	CCS_FLAG_TRANSIENT = (1 << 0),
	/**
	 * The value returned by CCS is a pointer and is not associated to a
	 * CCS object and needs to be freed by the user (unused).
	 */
	CCS_FLAG_UNPOOLED = (1 << 1),
	/**
	 * The object handle is just an identifier.
	 */
	CCS_FLAG_ID = (1 << 2),
	/** Try forcing 32 bits value for bindings */
	CCS_DATUM_FLAG_FORCE_32BIT = INT32_MAX
};

/**
 * A commodity type to represent CCS datum flags.
 */
typedef enum ccs_datum_flag_e ccs_datum_flag_t;

/**
 * A type representing the combination of flags that can be attached to a CCS
 * datum.
 */
typedef uint32_t ccs_datum_flags_t;

/**
 * The subset of CCS data types that represent numerical data.
 */
enum ccs_numeric_type_e {
	CCS_NUM_INTEGER = CCS_INTEGER, /*!< A ccs_int_t */
	CCS_NUM_FLOAT = CCS_FLOAT,     /*!< A ccs_float_t */
	CCS_NUM_TYPE_MAX,              /*!< Guard */
	/** Try forcing 32 bits value for bindings */
	CCS_NUM_TYPE_FORCE_32BIT = INT32_MAX
};

/**
 * A commodity type to represent CCS numeric types.
 */
typedef enum ccs_numeric_type_e ccs_numeric_type_t;

/**
 * A type representing a generic CCS object.
 */
typedef void * ccs_object_t;

/**
 * A union that can contain either a ccs_int_t or a ccs_float_t.
 */
union ccs_numeric_u {
	ccs_float_t   f; /*!< The floating point value of the union */
	ccs_int_t     i; /*!< The integer value of the union */
#ifdef __cplusplus
	ccs_numeric_u() : i(0L) {}
	ccs_numeric_u(float v) : f((ccs_float_t)v) {}
	ccs_numeric_u(int v) : i((ccs_int_t)v) {}
	ccs_numeric_u(ccs_int_t v) : i(v) {}
	ccs_numeric_u(ccs_float_t v) : f(v) {}
#endif
};

/**
 * A commodity type to represent CCS numeric values.
 */
typedef union ccs_numeric_u ccs_numeric_t;

#ifdef __cplusplus
#define CCSF(v) v
#define CCSI(v) v
#else
/**
 * A macro casting a value to a CCS floating point numeric.
 */
#define CCSF(v) ((ccs_numeric_t){ .f = v })
/**
 * A macro casting a value to a CCS integer numeric.
 */
#define CCSI(v) ((ccs_numeric_t){ .i = v })
#endif

/**
 * A union that represent a CCS datum value.
 */
union ccs_value_u {
	ccs_float_t   f; /*!< The floating point value of the union */
	ccs_int_t     i; /*!< The integer value of the union */
	const char   *s; /*!< The string value of the union */
	ccs_object_t  o; /*!< The CCS object value of the union */
#ifdef __cplusplus
	ccs_value_u() : i(0L) {}
	ccs_value_u(float v) : f((ccs_float_t)v) {}
	ccs_value_u(int v) : i((ccs_int_t)v) {}
	ccs_value_u(ccs_float_t v) : f(v) {}
	ccs_value_u(ccs_int_t v) : i(v) {}
	ccs_value_u(char *v) : s(v) {}
	ccs_value_u(ccs_object_t v) : o(v) {}
#endif
};

/**
 * A commodity type to represent a CCS datum value.
 */
typedef union ccs_value_u ccs_value_t;


/**
 * A Structure containing a CCS datum.
 */
struct ccs_datum_s {
	ccs_value_t value;       /*!< The value of the datum */
	ccs_data_type_t type;    /*!< The type of the datum */
	ccs_datum_flags_t flags; /*!< The flags attached to the datum */
};

/**
 * A commodity type to represent a CCS datum.
 */
typedef struct ccs_datum_s ccs_datum_t;

/**
 * A helper function to construct a datum containing a boolean value.
 * @param[in] v #CCS_TRUE or #CCS_FALSE
 * @return a CCS datum value
 */
static inline ccs_datum_t
ccs_bool(ccs_bool_t v) {
	ccs_datum_t d;
	d.type = CCS_BOOLEAN;
	d.value.i = v;
	d.flags = CCS_FLAG_DEFAULT;
	return d;
}

/**
 * A helper function to construct a datum containing a floating point value.
 * @param[in] v a floating point value.
 * @return a CCS datum value
 */
static inline ccs_datum_t
ccs_float(ccs_float_t v) {
	ccs_datum_t d;
	d.type = CCS_FLOAT;
	d.value.f = v;
	d.flags = CCS_FLAG_DEFAULT;
	return d;
}

/**
 * A helper function to construct a datum containing an integer value.
 * @param[in] v a signed integer.
 * @return a CCS datum value
 */
static inline ccs_datum_t
ccs_int(ccs_int_t v) {
	ccs_datum_t d;
	d.type = CCS_INTEGER;
	d.value.i = v;
	d.flags = CCS_FLAG_DEFAULT;
	return d;
}

/**
 * A helper function to construct a datum containing a CCS object value.
 * @param[in] v a CCS object.
 * @return a CCS datum value
 */
static inline ccs_datum_t
ccs_object(ccs_object_t v) {
	ccs_datum_t d;
	d.type = CCS_OBJECT;
	d.value.o = v;
	d.flags = CCS_FLAG_DEFAULT;
	return d;
}

/**
 * A helper function to construct a datum containing a string value.
 * @param[in] v a pointer to a NULL terminated string.
 * @return a CCS datum value
 */
static inline ccs_datum_t
ccs_string(const char *v) {
	ccs_datum_t d;
	d.type = CCS_STRING;
	d.value.s = v;
	d.flags = CCS_FLAG_DEFAULT;
	return d;
}

/**
 * A helper function providing a strict ordering of datum.
 * Flags are not taken into account.
 * @param[in] a the first datum
 * @param[in] b the second datum
 * @return -1, 0, or 1 if the first datum is found to be respectively lesser
 *         than, equal, or greater than the second datum
 */
static inline int ccs_datum_cmp(ccs_datum_t a, ccs_datum_t b) {
	if (a.type < b.type) {
		return -1;
	} else if (a.type > b.type) {
		return 1;
	} else {
		switch(a.type) {
		case CCS_STRING:
			if (a.value.s == b.value.s)
				return 0;
			else if (!a.value.s)
				return -1;
			else if (!b.value.s)
				return 1;
			else
				return strcmp(a.value.s, b.value.s);
			break;
		case CCS_INTEGER:
			return a.value.i < b.value.i ? -1 :
				a.value.i > b.value.i ? 1 : 0;
			break;
		case CCS_FLOAT:
			return a.value.f < b.value.f ? -1 :
				a.value.f > b.value.f ? 1 : 0;
			break;
		case CCS_NONE:
		case CCS_INACTIVE:
			return 0;
			break;
		default:
			return memcmp(&(a.value), &(b.value), sizeof(ccs_value_t));
		}
	}
}

/**
 * A variable containing an empty datum.
 */
extern const ccs_datum_t ccs_none;
/**
 * A variable containing an inactive datum.
 */
extern const ccs_datum_t ccs_inactive;
/**
 * A variable containing a boolean true datum.
 */
extern const ccs_datum_t ccs_true;
/**
 * A variable containing a boolean false datum.
 */
extern const ccs_datum_t ccs_false;

/**
 * A macro defining the none datum value.
 */
#define CCS_NONE_VAL {{0}, CCS_NONE, CCS_FLAG_DEFAULT}
/**
 * A macro defining the inactive datum value.
 */
#define CCS_INACTIVE_VAL {{0}, CCS_INACTIVE, CCS_FLAG_DEFAULT}
#ifdef __cplusplus
#define CCS_TRUE_VAL {{(ccs_int_t)CCS_TRUE}, CCS_BOOLEAN, CCS_FLAG_DEFAULT}
#define CCS_FALSE_VAL {{(ccs_int_t)CCS_FALSE}, CCS_BOOLEAN, CCS_FLAG_DEFAULT}
#else
/**
 * A macro defining the true boolean datum value.
 */
#define CCS_TRUE_VAL {{.i = CCS_TRUE}, CCS_BOOLEAN, CCS_FLAG_DEFAULT}
/**
 * A macro defining the false boolean datum value.
 */
#define CCS_FALSE_VAL {{.i = CCS_FALSE}, CCS_BOOLEAN, CCS_FLAG_DEFAULT}
#endif

/**
 * The library initialization function. Should be called before any operation
 * using the library are performed.
 * @return #CCS_SUCCESS
 */
extern ccs_result_t
ccs_init();

/**
 * The library deinitialization function. Should be called after all opertaions
 * using the library are performed.
 * @return #CCS_SUCCESS
 */
extern ccs_result_t
ccs_fini();

extern ccs_result_t
ccs_get_error_name(ccs_error_t error, const char **name);

/**
 * Query the library API version.
 * @return the library API version
 */
extern ccs_version_t
ccs_get_version();

/**
 * Retain a CCS object, incrementing the internal reference counting.
 * @param[in,out] object a CCS object
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if the object is found to be invalid
 */
extern ccs_result_t
ccs_retain_object(ccs_object_t object);

/**
 * Release a CCS object, decrementing the internal reference counting.
 * When the internsal reference count reaches zero, the destruction callbacks
 * are called and the object is freed
 * @param[in,out] object a CCS object
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if the object is found to be invalid
 * @return an error code given by the object destructor
 */
extern ccs_result_t
ccs_release_object(ccs_object_t object);

/**
 * Get a CCS object type.
 * @param[in] object a CCS object
 * @param[out] type_ret a pointer to a ccs_object_type_t variable that will
 *                      contain the type
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if the object is found to be invalid
 * @return -#CCS_INVALID_VALUE if type_ret is NULL
 */
extern ccs_result_t
ccs_object_get_type(ccs_object_t       object,
                    ccs_object_type_t *type_ret);

/**
 * Get an object internal reference counting.
 * @param[in] object a CCS object
 * @param[out] refcount_ret a pointer to a int32_t variable that will contain
 *                          the refcount
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p object is found to be invalid
 * @return -#CCS_INVALID_VALUE if \p refcount_ret is NULL
 */
extern ccs_result_t
ccs_object_get_refcount(ccs_object_t  object,
                        int32_t      *refcount_ret);

/**
 * The type of CCS object destruction callbacks.
 */
typedef void (*ccs_object_release_callback_t)(ccs_object_t object, void *user_data);

/**
 * Attach a destruction callback to a CCS object.
 * @param[in,out] object a CCS object
 * @param[in] callback the detruction callback to attach
 * @param[in] user_data an optional pointer that will be passed to the callback
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p object is found to be invalid
 * @return -#CCS_INVALID_VALUE if \p callback is NULL
 */
extern ccs_result_t
ccs_object_set_destroy_callback(ccs_object_t                   object,
                                ccs_object_release_callback_t  callback,
                                void                          *user_data);

/**
 * Set the associated `user_data` pointer of a CCS object.
 * @param[in] object a CCS object
 * @param[in] user_data a pointer to the user data to attach to this object
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p object is found to be invalid
 */
extern ccs_result_t
ccs_object_set_user_data(ccs_object_t  object,
                         void         *user_data);

/**
 * Get the associated `user_data` pointer of a CCS object.
 * @param[in] object a CCS object
 * @param[out] user_data_ret a pointer to a `void *` variable that will contain
 *                           the value of the `user_data`
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p object is found to be invalid
 * @return -#CCS_INVALID_VALUE if \p user_data_ret is NULL
 */
extern ccs_result_t
ccs_object_get_user_data(ccs_object_t   object,
                         void         **user_data_ret);
/**
 * The different serialization formats supported by CCS.
 */
enum ccs_serialize_format_e {
	/** A binary format that should be compact and performant. */
	CCS_SERIALIZE_FORMAT_BINARY,
	CCS_SERIALIZE_FORMAT_MAX,
	CCS_SERIALIZE_FORMAT_FORCE_32BIT = INT32_MAX
};
typedef enum ccs_serialize_format_e ccs_serialize_format_t;

/**
 * The different serialization operations supported by CCS.
 */
enum ccs_serialize_operation_e {
	/** Query the memory footprint of the serialized object */
	CCS_SERIALIZE_OPERATION_SIZE,
	CCS_SERIALIZE_OPERATION_MEMORY,
	CCS_SERIALIZE_OPERATION_FILE,
	CCS_SERIALIZE_OPERATION_FILE_DESCRIPTOR,
	CCS_SERIALIZE_OPERATION_MAX,
	CCS_SERIALIZE_OPERATION_FORCE_32BIT = INT32_MAX
};
typedef enum ccs_serialize_operation_e ccs_serialize_operation_t;

enum ccs_serialize_option_e {
	CCS_SERIALIZE_OPTION_END = 0,
	/** The next parameter is a pointer to a void * variable (initialized to NULL) that will hold the state */
	CCS_SERIALIZE_OPTION_NON_BLOCKING,
	CCS_SERIALIZE_OPTION_MAX,
	CCS_SERIALIZE_OPTION_FORCE_32BIT = INT32_MAX
};
typedef enum ccs_serialize_option_e ccs_serialize_option_t;

enum ccs_deserialize_option_e {
	CCS_DESERIALIZE_OPTION_END = 0,
	/** The next parameter is a ccs_handle_map_t object */
	CCS_DESERIALIZE_OPTION_HANDLE_MAP,
	/** The next parameter is a pointer to a ccs object vector struct */
	CCS_DESERIALIZE_OPTION_VECTOR,
	/** The next parameter is a pointer to a ccs object internal data */
	CCS_DESERIALIZE_OPTION_DATA,
	/** The next parameter is a pointer to a void * variable (initialized to NULL) that will hold the state */
	CCS_DESERIALIZE_OPTION_NON_BLOCKING,
	CCS_DESERIALIZE_OPTION_MAX,
	CCS_DESERIALIZE_OPTION_FORCE_32BIT = INT32_MAX
};
typedef enum ccs_deserialize_option_e ccs_deserialize_option_t;

extern ccs_result_t
ccs_object_serialize(ccs_object_t              object,
                     ccs_serialize_format_t    format,
                     ccs_serialize_operation_t type,
                     ...);

extern ccs_result_t
ccs_object_deserialize(ccs_object_t              *object_ret,
                       ccs_serialize_format_t     format,
                       ccs_serialize_operation_t  type,
                       ...);

#ifdef __cplusplus
}
#endif

#endif //_CCS_BASE_H
