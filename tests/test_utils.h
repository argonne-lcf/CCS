#include <cconfigspace.h>

extern void
print_ccs_error_stack(void);

extern ccs_parameter_t
create_numerical(const char *name, double lower, double upper);

extern ccs_configuration_space_t
create_2d_plane(ccs_feature_space_t fspace);

extern ccs_objective_space_t
create_height_objective(ccs_configuration_space_t cspace);

extern ccs_feature_space_t
create_knobs(ccs_features_t *features_on, ccs_features_t *features_off);
