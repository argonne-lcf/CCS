#ifndef _CCS_TUNER_H
#define _CCS_TUNER_H

#ifdef __cplusplus
extern "C" {
#endif

enum ccs_tuner_type_e {
	CCS_TUNER_RANDOM,
	CCS_TUNER_USER_DEFINED,
	CCS_TUNER_TYPE_MAX,
	CCS_TUNER_TYPE_32BIT = INT_MAX
};
typedef enum ccs_tuner_type_e ccs_tuner_type_t;

extern ccs_result_t
ccs_tuner_get_type(ccs_tuner_t       tuner,
                   ccs_tuner_type_t *type_ret);

extern ccs_result_t
ccs_tuner_get_name(ccs_tuner_t   tuner,
                   const char  **name_ret);

extern ccs_result_t
ccs_tuner_get_user_data(ccs_tuner_t   tuner,
                        void        **user_data_ret);

extern ccs_result_t
ccs_tuner_get_configuration_space(ccs_tuner_t                tuner,
                                  ccs_configuration_space_t *configuration_space_ret);

extern ccs_result_t
ccs_tuner_get_objective_space(ccs_tuner_t            tuner,
                              ccs_objective_space_t *objective_space_ret);

extern ccs_result_t
ccs_tuner_ask(ccs_tuner_t          tuner,
              size_t               num_configurations,
              ccs_configuration_t *configurations,
              size_t              *num_configurations_ret);

extern ccs_result_t
ccs_tuner_tell(ccs_tuner_t       tuner,
               size_t            num_evaluations,
               ccs_evaluation_t *evaluations);

extern ccs_result_t
ccs_tuner_get_optimums(ccs_tuner_t       tuner,
                       size_t            num_evaluations,
                       ccs_evaluation_t *evaluations,
                       size_t           *num_evaluations_ret);

extern ccs_result_t
ccs_tuner_get_history(ccs_tuner_t       tuner,
                      size_t            num_evaluations,
                      ccs_evaluation_t *evaluations,
                      size_t           *num_evaluations_ret);

extern ccs_result_t
ccs_create_random_tuner(const char                *name,
                        ccs_configuration_space_t  configuration_space,
                        ccs_objective_space_t      objective_space,
                        void                      *user_data,
                        ccs_tuner_t               *tuner_ret);

struct ccs_tuner_common_data_s {
	ccs_tuner_type_t           type;
	const char                *name;
	void                      *user_data;
	ccs_configuration_space_t  configuration_space;
	ccs_objective_space_t      objective_space;
};
typedef struct ccs_tuner_common_data_s ccs_tuner_common_data_t;



struct ccs_user_defined_tuner_data_s;
typedef struct ccs_user_defined_tuner_data_s ccs_user_defined_tuner_data_t;

struct ccs_tuner_vector_s {
	ccs_result_t (*del)(
		ccs_user_defined_tuner_data_t *data);

	ccs_result_t (*ask)(
		ccs_user_defined_tuner_data_t *data,
		size_t                         num_configurations,
		ccs_configuration_t           *configurations,
		size_t                        *num_configurations_ret);

	ccs_result_t (*tell)(
		ccs_user_defined_tuner_data_t *data,
		size_t                         num_evaluations,
		ccs_evaluation_t              *evaluations);

	ccs_result_t (*get_optimums)(
		ccs_user_defined_tuner_data_t *data,
		size_t                         num_evaluations,
		ccs_evaluation_t              *evaluations,
		size_t                        *num_evaluations_ret);

	ccs_result_t (*get_history)(
		ccs_user_defined_tuner_data_t *data,
		size_t                         num_evaluations,
		ccs_evaluation_t              *evaluations,
		size_t                        *num_evaluations_ret);
};
typedef struct ccs_tuner_vector_s ccs_tuner_vector_t;

struct ccs_user_defined_tuner_data_s {
	ccs_tuner_common_data_t  common_data;
	ccs_tuner_vector_t       vector;
	void                    *tuner_data;
};

extern ccs_result_t
ccs_create_user_defined_tuner(const char                *name,
                              ccs_configuration_space_t  configuration_space,
                              ccs_objective_space_t      objective_space,
                              void                      *user_data,
                              ccs_tuner_vector_t        *vector,
                              void                      *tuner_data,
                              ccs_tuner_t               *tuner_ret);


#ifdef __cplusplus
}
#endif

#endif //_CCS_TUNER_H
