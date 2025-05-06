#include <impl/Kokkos_Profiling_Interface.hpp>
#include <cconfigspace.h>
#include <string>
#include <iostream>
#include <cassert>
#include <map>
#include <set>
#include <stack>
#include <vector>
#include <time.h>
#include <stdio.h>
#include <cstdint>

#ifndef CCS_DEBUG
#define CCS_DEBUG 0
#endif

#if CCS_DEBUG
void
print_ccs_error_stack(void)
{
	ccs_error_stack_t       err;
	ccs_result_t            code;
	const char             *msg;
	size_t                  stack_depth;
	ccs_error_stack_elem_t *stack_elems;

	err = ccs_get_thread_error();
	if (!err)
		return;
	ccs_error_stack_get_code(err, &code);
	ccs_get_result_name(code, &msg);
	fprintf(stderr, "CCS Error: %s (%d): ", msg, code);
	ccs_error_stack_get_message(err, &msg);
	fprintf(stderr, "%s\n", msg);
	ccs_error_stack_get_elems(err, 0, NULL, &stack_depth);
	stack_elems = (ccs_error_stack_elem_t *)malloc(
		stack_depth * sizeof(ccs_error_stack_elem_t));
	ccs_error_stack_get_elems(err, stack_depth, stack_elems, NULL);
	for (size_t i = 0; i < stack_depth; i++) {
		fprintf(stderr, "\t%s:%d:%s\n", stack_elems[i].file,
			stack_elems[i].line, stack_elems[i].func);
	}
	free(stack_elems);
	ccs_release_object(err);
}
#define CCS_CHECK(expr)                                                        \
	do {                                                                   \
		ccs_result_t code = (expr);                                    \
		if (CCS_RESULT_SUCCESS != code) {                              \
			print_ccs_error_stack();                               \
		}                                                              \
		assert(CCS_RESULT_SUCCESS == code);                            \
	} while (0)
#else
#define CCS_CHECK(expr)                                                        \
	do {                                                                   \
		assert(CCS_RESULT_SUCCESS == (expr));                          \
	} while (0)
#endif

#define CCS_DEBUG_MSG(fmt)                                                     \
	do {                                                                   \
		if (CCS_DEBUG)                                                 \
			fprintf(stderr, fmt);                                  \
	} while (0)

#define CCS_DEBUG_MSG_ARGS(fmt, ...)                                           \
	do {                                                                   \
		if (CCS_DEBUG)                                                 \
			fprintf(stderr, fmt, __VA_ARGS__);                     \
	} while (0)

#ifndef CCS_PROFILE
#define CCS_PROFILE 0
#endif

#if CCS_PROFILE
#define CCS_PROFILE_START()                                                    \
	struct timespec prof_start, prof_stop;                                 \
	clock_gettime(CLOCK_MONOTONIC, &prof_start)

#define CCS_PROFILE_STOP()                                                     \
	clock_gettime(CLOCK_MONOTONIC, &prof_stop);                            \
	ccs_time +=                                                            \
		((int64_t)(prof_stop.tv_sec) - (int64_t)(prof_start.tv_sec)) * \
			1000000000 +                                           \
		(int64_t)(prof_stop.tv_nsec) - (int64_t)(prof_start.tv_nsec)

#else
#define CCS_PROFILE_START()                                                    \
	do {                                                                   \
	} while (0)
#define CCS_PROFILE_STOP()                                                     \
	do {                                                                   \
	} while (0)
#endif

#if CCS_PROFILE
static int64_t ccs_time = 0;
#endif

static constexpr const double epsilon = 1E-24;

using namespace Kokkos::Tools::Experimental;

// initialize the stack to true, so it should never be empty.
static std::stack<bool, std::vector<bool> > convergence_stack;
static constexpr const size_t               convergence_cutoff = 500;

static Kokkos::Tools::Experimental::ToolProgrammingInterface helper_functions;
static void
invoke_fence(uint32_t devID)
{
	if (!convergence_stack.top()) // if we are in a non converged state, we
				      // fence
		helper_functions.fence(devID);
}

extern "C" void
kokkosp_provide_tool_programming_interface(
	uint32_t                                              num_actions,
	Kokkos::Tools::Experimental::ToolProgrammingInterface action)
{
	(void)num_actions;
	helper_functions = action;
}

extern "C" void
kokkosp_request_tool_settings(
	uint32_t                                   num_responses,
	Kokkos::Tools::Experimental::ToolSettings *response)
{
	(void)num_responses;
	response->requires_global_fencing = false;
}

extern "C" void
kokkosp_begin_parallel_for(const char *name, const uint32_t devID, uint64_t *kID)
{
	(void)name;
	invoke_fence(devID);
	*kID = devID;
}

extern "C" void
kokkosp_end_parallel_for(const uint64_t kID)
{
	uint32_t devID = kID;
	invoke_fence(devID);
}

extern "C" void
kokkosp_begin_parallel_scan(
	const char    *name,
	const uint32_t devID,
	uint64_t      *kID)
{
	(void)name;
	invoke_fence(devID);
	*kID = devID;
}

extern "C" void
kokkosp_end_parallel_scan(const uint64_t kID)
{
	uint32_t devID = kID;
	invoke_fence(devID);
}

extern "C" void
kokkosp_begin_parallel_reduce(
	const char    *name,
	const uint32_t devID,
	uint64_t      *kID)
{
	(void)name;
	invoke_fence(devID);
	*kID = devID;
}

extern "C" void
kokkosp_end_parallel_reduce(const uint64_t kID)
{
	uint32_t devID = kID;
	invoke_fence(devID);
}

static std::map<size_t, ccs_parameter_t> features;
static std::map<size_t, ccs_parameter_t> parameters;
static std::map<
	std::set<size_t>,
	std::tuple<
		ccs_tuner_t,
		bool,
		std::map<size_t, size_t> *,
		std::map<size_t, size_t> *,
		ccs_datum_t *,
		ccs_datum_t *> >
	tuners;

extern "C" void
kokkosp_parse_args(int argc, char **argv)
{
	(void)argc;
	(void)argv;
}

extern "C" void
kokkosp_print_help(char *exe)
{
	(void)exe;
	std::string OPTIONS_BLOCK =
	    R"(
CConfigSpace connector for Kokkos
)";

	std::cout << OPTIONS_BLOCK;
}

extern "C" void
kokkosp_init_library(
	const int      loadSeq,
	const uint64_t interfaceVer,
	const uint32_t devInfoCount,
	void          *deviceInfo)
{
	(void)loadSeq;
	(void)devInfoCount;
	(void)deviceInfo;
	std::cout << "Initializing CConfigSpace adapter" << std::endl;
	assert(interfaceVer >= KOKKOSP_INTERFACE_VERSION);
	CCS_PROFILE_START();
	CCS_CHECK(ccs_init());
	CCS_PROFILE_STOP();
	convergence_stack.push(true);
}

extern "C" void
kokkosp_finalize_library()
{
	std::cout << "Finalizing CConfigSpace adapter" << std::endl;

	CCS_PROFILE_START();

	for (auto const &x : features)
		CCS_CHECK(ccs_release_object(x.second));
	features.clear();
	for (auto const &x : parameters)
		CCS_CHECK(ccs_release_object(x.second));
	parameters.clear();
	for (auto const &x : tuners) {
		CCS_CHECK(ccs_release_object(std::get<0>(x.second)));
		delete std::get<2>(x.second);
		delete std::get<3>(x.second);
		delete[] std::get<4>(x.second);
		delete[] std::get<5>(x.second);
	}
	tuners.clear();
	CCS_CHECK(ccs_fini());
	CCS_PROFILE_STOP();
#if CCS_PROFILE
	std::cout << "CCS profiling: " << (double)ccs_time / 1000000.0 << " ms"
		  << std::endl;
#endif
}

static ccs_parameter_t
set_to_parameters(const char *name, VariableInfo *info)
{
	ccs_parameter_t ret;
	CCS_DEBUG_MSG_ARGS("\tvalue set (%zu):", info->candidates.set.size);
	{
		ccs_datum_t *values =
			new ccs_datum_t[info->candidates.set.size];
		for (size_t i = 0; i < info->candidates.set.size; i++) {
			switch (info->type) {
			case ValueType::kokkos_value_double:
				CCS_DEBUG_MSG_ARGS(
					" %f", info->candidates.set.values
						       .double_value[i]);
				values[i] =
					ccs_float(info->candidates.set.values
							  .double_value[i]);
				break;
			case ValueType::kokkos_value_int64:
				CCS_DEBUG_MSG_ARGS(
					" %" PRId64, info->candidates.set.values
							     .int_value[i]);
				values[i] = ccs_int(info->candidates.set.values
							    .int_value[i]);
				break;
			case ValueType::kokkos_value_string:
				CCS_DEBUG_MSG_ARGS(
					" %s", info->candidates.set.values
						       .string_value[i]);
				values[i] =
					ccs_string(info->candidates.set.values
							   .string_value[i]);
				break;
			default:
				assert(false && "Unknown ValueType");
			}
		}
		switch (info->category) {
		case StatisticalCategory::kokkos_value_categorical:
			CCS_CHECK(ccs_create_categorical_parameter(
				name, info->candidates.set.size, values, 0,
				&ret));
			break;
		case StatisticalCategory::kokkos_value_ordinal:
			CCS_CHECK(ccs_create_ordinal_parameter(
				name, info->candidates.set.size, values, 0,
				&ret));
			break;
		case StatisticalCategory::kokkos_value_interval:
		case StatisticalCategory::kokkos_value_ratio:
			CCS_CHECK(ccs_create_discrete_parameter(
				name, info->candidates.set.size, values, 0,
				&ret));
			break;
		default:
			assert(false && "Unknown StatisticalCategory");
		}
		delete[] values;
	}
	CCS_DEBUG_MSG("\n");
	return ret;
}

static ccs_parameter_t
double_range_to_parameter(const char *name, VariableInfo *info)
{
	ccs_parameter_t ret;
	CCS_DEBUG_MSG_ARGS(
		"double: %s%f..%f%s step: %f\n",
		info->candidates.range.openLower ? "(" : "[",
		info->candidates.range.lower.double_value,
		info->candidates.range.upper.double_value,
		info->candidates.range.openUpper ? ")" : "]",
		info->candidates.range.step.double_value);
	ccs_float_t lower = info->candidates.range.lower.double_value;
	ccs_float_t upper = info->candidates.range.upper.double_value;
	ccs_float_t step  = info->candidates.range.step.double_value;
	assert(0.0 <= step);
	if (info->candidates.range.openLower) {
		if (step == 0.0)
			lower = lower + epsilon;
		else
			lower = lower + step;
	}
	if (!info->candidates.range.openUpper) {
		if (step == 0.0)
			upper = upper + epsilon;
		else // this is dubious/would require
		     // verification
			upper = upper + step;
	}
	CCS_CHECK(ccs_create_numerical_parameter(
		name, CCS_NUMERIC_TYPE_FLOAT, lower, upper, step, lower, &ret));
	return ret;
}

static ccs_parameter_t
int64_range_to_parameter(const char *name, VariableInfo *info)
{
	ccs_parameter_t ret;
	CCS_DEBUG_MSG_ARGS(
		"int: %s%" PRId64 "..%" PRId64 "%s step: %" PRId64 "\n",
		info->candidates.range.openLower ? "(" : "[",
		info->candidates.range.lower.int_value,
		info->candidates.range.upper.int_value,
		info->candidates.range.openUpper ? ")" : "]",
		info->candidates.range.step.int_value);
	ccs_int_t lower = info->candidates.range.lower.int_value;
	ccs_int_t upper = info->candidates.range.upper.int_value;
	ccs_int_t step  = info->candidates.range.step.int_value;
	assert(0 <= step);
	if (info->candidates.range.openLower) {
		if (step == 0)
			lower = lower + 1;
		else
			lower = lower + step;
	}
	if (!info->candidates.range.openUpper) {
		if (step == 0)
			upper = upper + epsilon;
		else // this is dubious/would require
		     // verification
			upper = upper + step;
	}
	CCS_CHECK(ccs_create_numerical_parameter(
		name, CCS_NUMERIC_TYPE_INT, lower, upper, step, lower, &ret));
	return ret;
}

static ccs_parameter_t
range_to_parameter(const char *name, VariableInfo *info)
{
	ccs_parameter_t ret;
	CCS_DEBUG_MSG("\tvalue range: ");
	switch (info->type) {
	case ValueType::kokkos_value_double:
		ret = double_range_to_parameter(name, info);
		break;
	case ValueType::kokkos_value_int64:
		ret = int64_range_to_parameter(name, info);
		break;
	default:
		assert(false && "Invalid ValueType");
	}
	return ret;
}

static ccs_parameter_t
unbounded_to_parameter(const char *name, VariableInfo *info)
{
	ccs_parameter_t ret;
	CCS_DEBUG_MSG("\tvalue unbounded\n");
	switch (info->type) {
	case ValueType::kokkos_value_double: {
		ccs_float_t lower = -CCS_INFINITY;
		ccs_float_t upper = CCS_INFINITY;
		ccs_float_t step  = 0.0;
		CCS_CHECK(ccs_create_numerical_parameter(
			name, CCS_NUMERIC_TYPE_FLOAT, lower, upper, step, lower,
			&ret));
	} break;
	case ValueType::kokkos_value_int64: {
		ccs_int_t lower = CCS_INT_MIN;
		ccs_int_t upper = CCS_INT_MAX;
		ccs_int_t step  = 0;
		CCS_CHECK(ccs_create_numerical_parameter(
			name, CCS_NUMERIC_TYPE_INT, lower, upper, step, lower,
			&ret));
	} break;
	case ValueType::kokkos_value_string:
		CCS_CHECK(ccs_create_string_parameter(name, &ret));
		break;
	default:
		assert(false && "Unknown ValueType");
	}
	return ret;
}

static ccs_parameter_t
variable_info_to_parameter(const char *name, VariableInfo *info)
{
	ccs_parameter_t ret;
	switch (info->valueQuantity) {
	case CandidateValueType::kokkos_value_set:
		ret = set_to_parameters(name, info);
		break;
	case CandidateValueType::kokkos_value_range:
		ret = range_to_parameter(name, info);
		break;
	case CandidateValueType::kokkos_value_unbounded:
		ret = unbounded_to_parameter(name, info);
		break;
	default:
		assert(false && "Unknown CandidateValueType");
	}
	return ret;
}

extern "C" void
kokkosp_declare_input_type(
	const char                                *name,
	const size_t                               id,
	Kokkos::Tools::Experimental::VariableInfo *info)
{
	CCS_PROFILE_START();

	CCS_DEBUG_MSG_ARGS("Got context variable: %s, id: %zu\n", name, id);
	features[id] = variable_info_to_parameter(name, info);
	CCS_DEBUG_MSG_ARGS("\t...mapped to %p\n", (void *)features[id]);

	CCS_PROFILE_STOP();
}

extern "C" void
kokkosp_declare_output_type(
	const char                                *name,
	const size_t                               id,
	Kokkos::Tools::Experimental::VariableInfo *info)
{
	CCS_PROFILE_START();

	CCS_DEBUG_MSG_ARGS("Got tuning variable: %s, id: %zu\n", name, id);
	parameters[id] = variable_info_to_parameter(name, info);
	CCS_DEBUG_MSG_ARGS("\t...mapped to %p\n", (void *)parameters[id]);

	CCS_PROFILE_STOP();
}

extern "C" void
kokkosp_declare_optimization_goal(
	size_t                                        contextId,
	Kokkos::Tools::Experimental::OptimizationGoal goal)
{
	(void)goal;
	CCS_DEBUG_MSG_ARGS(
		"Optimization goal, context: %zu, id: %zu\n", contextId,
		goal.type_id);
}

static inline void
set_value(
	Kokkos::Tools::Experimental::VariableValue *tuningValue,
	ccs_datum_t                                *d)
{
	switch (tuningValue->metadata->type) {
	case ValueType::kokkos_value_double:
		tuningValue->value.double_value = d->value.f;
		CCS_DEBUG_MSG_ARGS(
			"\tsent: %f\n", tuningValue->value.double_value);
		break;
	case ValueType::kokkos_value_int64:
		tuningValue->value.int_value = d->value.i;
		CCS_DEBUG_MSG_ARGS(
			"\tsent: %" PRId64 "\n", tuningValue->value.int_value);
		break;
	case ValueType::kokkos_value_string:
		strncpy(tuningValue->value.string_value, d->value.s,
			KOKKOS_TOOLS_TUNING_STRING_LENGTH);
		CCS_DEBUG_MSG_ARGS(
			"\tsent: %s\n", tuningValue->value.string_value);
		break;
	default:
		assert(false && "Unknown ValueType");
	}
}

static inline void
extract_value(
	Kokkos::Tools::Experimental::VariableValue *tuningValue,
	ccs_datum_t                                *d)
{
	if (!tuningValue->metadata) {
		*d = ccs_none;
		CCS_DEBUG_MSG("\treceived no value\n");
		return;
	}
	switch (tuningValue->metadata->type) {
	case ValueType::kokkos_value_double:
		CCS_DEBUG_MSG_ARGS(
			"\treceived: %f\n", tuningValue->value.double_value);
		*d = ccs_float(tuningValue->value.double_value);
		break;
	case ValueType::kokkos_value_int64:
		CCS_DEBUG_MSG_ARGS(
			"\treceived: %" PRId64 "\n",
			tuningValue->value.int_value);
		*d = ccs_int(tuningValue->value.int_value);
		break;
	case ValueType::kokkos_value_string:
		CCS_DEBUG_MSG_ARGS(
			"\treceived: %s\n", tuningValue->value.string_value);
		*d       = ccs_string(tuningValue->value.string_value);
		d->flags = CCS_DATUM_FLAG_TRANSIENT;
		break;
	default:
		assert(false && "Unknown ValueType");
	}
}

static std::map<
	size_t,
	std::tuple<struct timespec, ccs_tuner_t, ccs_configuration_t, bool> >
	   contexts;
static int regionCounter = 0;

static auto
create_tuner(
	std::set<size_t>                            regionId,
	size_t                                      numContextVariables,
	Kokkos::Tools::Experimental::VariableValue *contextValues,
	size_t                                      numTuningVariables,
	Kokkos::Tools::Experimental::VariableValue *tuningValues)
{
	ccs_parameter_t          *cs_parameters;
	ccs_configuration_space_t cs;
	ccs_feature_space_t       fs;
	ccs_objective_space_t     os;
	ccs_tuner_t               tuner;
	ccs_parameter_t           htime;
	ccs_expression_t          expression;
	ccs_objective_type_t      otype;
	std::map<size_t, size_t> *context_id_to_index;
	std::map<size_t, size_t> *tuning_id_to_index;
	ccs_datum_t              *context_values_buff;
	ccs_datum_t              *tuning_values_buff;

	CCS_DEBUG_MSG("\tCreating feature space\n");
	context_id_to_index = new std::map<size_t, size_t>;
	cs_parameters       = new ccs_parameter_t[numContextVariables];
	for (size_t i = 0; i < numContextVariables; i++) {
		CCS_DEBUG_MSG_ARGS(
			"\t\tLoooking up context variable: %zu\n",
			contextValues[i].type_id);
		cs_parameters[i] = features[contextValues[i].type_id];
		CCS_DEBUG_MSG_ARGS(
			"\t\tFound up context variable: %p\n",
			(void *)cs_parameters[i]);
		CCS_CHECK(ccs_parameter_copy(
			cs_parameters[i], &cs_parameters[i]));
		(*context_id_to_index)[contextValues[i].type_id] = i;
	}

	CCS_CHECK(ccs_create_feature_space(
		("fs (region: " + std::to_string(regionCounter) + ")").c_str(),
		numContextVariables, cs_parameters, &fs));
	delete[] cs_parameters;

	CCS_DEBUG_MSG("\tCreating configuration space\n");
	tuning_id_to_index = new std::map<size_t, size_t>;
	cs_parameters      = new ccs_parameter_t[numTuningVariables];
	for (size_t i = 0; i < numTuningVariables; i++) {
		CCS_DEBUG_MSG_ARGS(
			"\t\tLoooking up tuning variable: %zu\n",
			tuningValues[i].type_id);
		cs_parameters[i] = parameters[tuningValues[i].type_id];
		CCS_DEBUG_MSG_ARGS(
			"\t\tFound up tuning variable: %p\n",
			(void *)cs_parameters[i]);
		CCS_CHECK(ccs_parameter_copy(
			cs_parameters[i], &cs_parameters[i]));
		(*tuning_id_to_index)[tuningValues[i].type_id] = i;
	}

	CCS_CHECK(ccs_create_configuration_space(
		("cs (region: " + std::to_string(regionCounter) + ")").c_str(),
		numTuningVariables, cs_parameters, NULL, 0, NULL, fs, NULL,
		&cs));
	delete[] cs_parameters;

	// Objectives don't seem to be used, minimize time for now
	CCS_DEBUG_MSG("\tCreating objective space\n");
	ccs_int_t lower = 0;
	ccs_int_t upper = CCS_INT_MAX;
	ccs_int_t step  = 0;
	CCS_CHECK(ccs_create_numerical_parameter(
		"time", CCS_NUMERIC_TYPE_INT, lower, upper, step, lower,
		&htime));
	CCS_CHECK(ccs_create_variable(htime, &expression));
	otype = CCS_OBJECTIVE_TYPE_MINIMIZE;
	CCS_CHECK(ccs_create_objective_space(
		("os (region: " + std::to_string(regionCounter) + ")").c_str(),
		(ccs_search_space_t)cs, 1, &htime, 1, &expression, &otype,
		&os));
	CCS_CHECK(ccs_release_object(expression));
	CCS_CHECK(ccs_release_object(htime));

	CCS_DEBUG_MSG("\tCreating tuner\n");
	CCS_CHECK(ccs_create_random_tuner(
		("random tuner (region: " + std::to_string(regionCounter) + ")")
			.c_str(),
		os, &tuner));
	CCS_CHECK(ccs_release_object(cs));
	CCS_CHECK(ccs_release_object(fs));
	CCS_CHECK(ccs_release_object(os));

	context_values_buff = new ccs_datum_t[numContextVariables];
	tuning_values_buff  = new ccs_datum_t[numTuningVariables];
	return tuners
		.insert(std::make_pair(
			regionId,
			std::make_tuple(
				tuner, false, context_id_to_index,
				tuning_id_to_index, context_values_buff,
				tuning_values_buff)))
		.first;
}

extern "C" void
kokkosp_request_values(
	size_t                                      contextId,
	size_t                                      numContextVariables,
	Kokkos::Tools::Experimental::VariableValue *contextValues,
	size_t                                      numTuningVariables,
	Kokkos::Tools::Experimental::VariableValue *tuningValues)
{

	std::set<size_t>          regionId;
	ccs_tuner_t               tuner;
	ccs_features_t            feat;
	ccs_feature_space_t       feature_space;
	ccs_configuration_t       configuration;
	ccs_configuration_space_t configuration_space;
	struct timespec           start;
	bool                      converged;
	std::map<size_t, size_t> *context_id_to_index;
	std::map<size_t, size_t> *tuning_id_to_index;
	ccs_datum_t              *context_values_buff;
	ccs_datum_t              *tuning_values_buff;

	CCS_DEBUG_MSG_ARGS(
		"Querying variables, context: %zu, numContextVariables: %zu, numTuningVariables: %zu\n",
		contextId, numContextVariables, numTuningVariables);

	CCS_PROFILE_START();

	for (size_t i = 0; i < numContextVariables; i++)
		regionId.insert(contextValues[i].type_id);
	for (size_t i = 0; i < numTuningVariables; i++)
		regionId.insert(tuningValues[i].type_id);

	auto tun = tuners.find(regionId);
	if (tun == tuners.end()) {
		tun = create_tuner(
			regionId, numContextVariables, contextValues,
			numTuningVariables, tuningValues);
		regionCounter++;
	}
	tuner               = std::get<0>(tun->second);
	converged           = std::get<1>(tun->second);
	context_id_to_index = std::get<2>(tun->second);
	tuning_id_to_index  = std::get<3>(tun->second);
	context_values_buff = std::get<4>(tun->second);
	tuning_values_buff  = std::get<5>(tun->second);

	// Test convergence using history size, could be done better
	if (!converged) {
		size_t history_size;
		CCS_CHECK(ccs_tuner_get_history(
			tuner, NULL, 0, NULL, &history_size));
		converged = (history_size >= convergence_cutoff);
		if (converged)
			tuners[regionId] = std::make_tuple(
				tuner, converged, context_id_to_index,
				tuning_id_to_index, context_values_buff,
				tuning_values_buff);
	}
	if (convergence_stack.top()) // if we are in a converged region,
		convergence_stack.push(converged);
	else // else propagate unconverged status
		convergence_stack.push(false);

	CCS_CHECK(ccs_tuner_get_feature_space(tuner, &feature_space));
	{
		for (size_t i = 0; i < numContextVariables; i++) {
			extract_value(
				contextValues + i,
				context_values_buff +
					(*context_id_to_index)
						[contextValues[i].type_id]);
		}
		CCS_CHECK(ccs_create_features(
			feature_space, numContextVariables, context_values_buff,
			&feat));
	}

	if (!converged)
		CCS_CHECK(ccs_tuner_ask(
			tuner, feat, 1,
			(ccs_search_configuration_t *)&configuration, NULL));
	else
		CCS_CHECK(ccs_tuner_suggest(
			tuner, feat,
			(ccs_search_configuration_t *)&configuration));
	CCS_CHECK(ccs_release_object(feat));
	CCS_CHECK(ccs_tuner_get_search_space(
		tuner, (ccs_search_space_t *)&configuration_space));
	{
		CCS_CHECK(ccs_binding_get_values(
			(ccs_binding_t)configuration, numTuningVariables,
			tuning_values_buff, NULL));
		for (size_t i = 0; i < numTuningVariables; i++) {
			set_value(
				tuningValues + i,
				tuning_values_buff +
					(*tuning_id_to_index)[tuningValues[i]
								      .type_id]);
		}
	}

	CCS_PROFILE_STOP();

	clock_gettime(CLOCK_MONOTONIC, &start);
	contexts[contextId] =
		std::make_tuple(start, tuner, configuration, converged);
}

extern "C" void
kokkosp_begin_context(size_t contextId)
{
	CCS_DEBUG_MSG_ARGS("Entering region: %zu\n", contextId);
}

extern "C" void
kokkosp_end_context(
	size_t                                     contextId,
	Kokkos::Tools::Experimental::VariableValue goalValue)
{
	struct timespec       start, stop;
	ccs_tuner_t           tuner;
	ccs_configuration_t   configuration;
	ccs_objective_space_t objective_space;
	bool                  converged;

	clock_gettime(CLOCK_MONOTONIC, &stop);
	CCS_DEBUG_MSG_ARGS("Leaving region: %zu\n", contextId);
#if CCS_DEBUG
	ccs_datum_t datum;
	extract_value(&goalValue, &datum);
#else
	(void)goalValue;
#endif

	auto ctx = contexts.find(contextId);
	if (ctx == contexts.end())
		return;

	CCS_PROFILE_START();

	CCS_DEBUG_MSG("\tFound tuning context\n");

	convergence_stack.pop();
	auto context  = ctx->second;
	start         = std::get<0>(context);
	tuner         = std::get<1>(context);
	configuration = std::get<2>(context);
	converged     = std::get<3>(context);
	contexts.erase(contextId);

	if (!converged) { // do not report if not fencing and already converged.
		ccs_evaluation_t evaluation;
		// elapsed time in nanosecond
		ccs_datum_t      elapsed = ccs_int(
                        ((ccs_int_t)(stop.tv_sec) - (ccs_int_t)(start.tv_sec)) *
                                1000000000 +
                        (ccs_int_t)(stop.tv_nsec) - (ccs_int_t)(start.tv_nsec));
		CCS_DEBUG_MSG_ARGS(
			"\telapsed time: %f ms\n", elapsed.value.i / 1000000.0);

		CCS_CHECK(
			ccs_tuner_get_objective_space(tuner, &objective_space));
		CCS_CHECK(ccs_create_evaluation(
			objective_space,
			(ccs_search_configuration_t)configuration,
			CCS_RESULT_SUCCESS, 1, &elapsed, &evaluation));

		CCS_CHECK(ccs_tuner_tell(tuner, 1, &evaluation));
		CCS_CHECK(ccs_release_object(evaluation));
	}

	CCS_CHECK(ccs_release_object(configuration));

	CCS_PROFILE_STOP();
}
