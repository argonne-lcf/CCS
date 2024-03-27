#include <stdlib.h>
#include <assert.h>
#include <cconfigspace.h>
#include <string.h>
#include <Python.h>

ccs_parameter_t
create_numerical(const char *name, double lower, double upper)
{
	ccs_parameter_t parameter;
	ccs_result_t    err;
	err = ccs_create_numerical_parameter(
		name, CCS_NUMERIC_TYPE_FLOAT, CCSF(lower), CCSF(upper),
		CCSF(0.0), CCSF(0), &parameter);
	assert(err == CCS_RESULT_SUCCESS);
	return parameter;
}

void
create_problem(ccs_configuration_space_t *cs, ccs_objective_space_t *os)
{
	ccs_parameter_t           parameter1, parameter2;
	ccs_parameter_t           parameters[2];
	ccs_parameter_t           parameter3;
	ccs_configuration_space_t cspace;
	ccs_objective_space_t     ospace;
	ccs_expression_t          expression;
	ccs_objective_type_t      otype;
	ccs_result_t              err;

	parameters[0] = parameter1 = create_numerical("x", -5.0, 5.0);
	parameters[1] = parameter2 = create_numerical("y", -5.0, 5.0);

	err                        = ccs_create_configuration_space(
                "2dplane", 2, parameters, NULL, 0, NULL, NULL, &cspace);
	assert(err == CCS_RESULT_SUCCESS);

	parameter3 = create_numerical("z", -CCS_INFINITY, CCS_INFINITY);
	err        = ccs_create_variable(parameter3, &expression);
	assert(err == CCS_RESULT_SUCCESS);
	otype = CCS_OBJECTIVE_TYPE_MINIMIZE;

	err   = ccs_create_objective_space(
                "height", 1, &parameter3, 1, &expression, &otype, &ospace);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_release_object(parameter1);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(parameter2);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(parameter3);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(expression);
	assert(err == CCS_RESULT_SUCCESS);

	*cs = cspace;
	*os = ospace;
}

void
test_tuner(ccs_tuner_t tuner, ccs_objective_space_t ospace)
{
	ccs_result_t err;

	for (size_t i = 0; i < 100; i++) {
		ccs_datum_t         values[2], res;
		ccs_configuration_t configuration;
		ccs_evaluation_t    evaluation;
		err = ccs_tuner_ask(tuner, 1, &configuration, NULL);
		assert(err == CCS_RESULT_SUCCESS);
		err = ccs_binding_get_values(
			(ccs_binding_t)configuration, 2, values, NULL);
		assert(err == CCS_RESULT_SUCCESS);
		res = ccs_float(
			(values[0].value.f - 1) * (values[0].value.f - 1) +
			(values[1].value.f - 2) * (values[1].value.f - 2));
		ccs_create_evaluation(
			ospace, configuration, CCS_RESULT_SUCCESS, 1, &res,
			&evaluation);
		err = ccs_tuner_tell(tuner, 1, &evaluation);
		assert(err == CCS_RESULT_SUCCESS);
		err = ccs_release_object(configuration);
		assert(err == CCS_RESULT_SUCCESS);
		err = ccs_release_object(evaluation);
		assert(err == CCS_RESULT_SUCCESS);
	}

	size_t           count;
	ccs_evaluation_t history[100];
	ccs_datum_t      min = ccs_float(INFINITY);
	err = ccs_tuner_get_history(tuner, 100, history, &count);
	assert(err == CCS_RESULT_SUCCESS);
	assert(count == 100);

	for (size_t i = 0; i < 100; i++) {
		ccs_datum_t res;
		err = ccs_evaluation_binding_get_objective_value(
			(ccs_evaluation_binding_t)history[i], 0, &res);
		assert(err == CCS_RESULT_SUCCESS);
		if (res.value.f < min.value.f)
			min.value.f = res.value.f;
	}

	ccs_evaluation_t evaluation;
	ccs_datum_t      res;
	err = ccs_tuner_get_optima(tuner, 1, &evaluation, NULL);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_evaluation_binding_get_objective_value(
		(ccs_evaluation_binding_t)evaluation, 0, &res);
	assert(res.value.f == min.value.f);
}

void
test(void)
{
	ccs_tuner_t               t;
	ccs_configuration_space_t cs;
	ccs_objective_space_t     os;
	ccs_result_t              err;

	create_problem(&cs, &os);

	PyObject *pName, *pModule, *pFunc;
	PyObject *pArgs, *pValue, *pHandle, *pAddr;
#if PY_VERSION_HEX < 0x030b0000
	Py_SetProgramName(L"test_python");
	Py_Initialize();
#else
	PyStatus status;
	PyConfig config;
	PyConfig_InitPythonConfig(&config);
	config.configure_c_stdio       = 0;
	config.install_signal_handlers = 0;
	config.parse_argv              = 0;
	config.pathconfig_warnings     = 0;
	status                         = PyConfig_SetString(
                &config, &config.program_name, L"test_python");
	if (PyStatus_Exception(status)) {
		goto exception;
	}
	status = Py_InitializeFromConfig(&config);
	if (PyStatus_Exception(status)) {
		goto exception;
	}
	PyConfig_Clear(&config);
#endif
	PyRun_SimpleString("import sys\n"
			   "sys.path.insert(1, './')\n"
			   "sys.path.insert(1, '" BINDINGS_PATH "')\n");
	pName   = PyUnicode_DecodeFSDefault("test_python");
	pModule = PyImport_Import(pName);
	Py_DECREF(pName);

	if (pModule != NULL) {
		pFunc  = PyObject_GetAttrString(pModule, "create_test_tuner");
		pArgs  = PyTuple_New(2);
		pValue = PyLong_FromVoidPtr(cs);
		PyTuple_SetItem(pArgs, 0, pValue);
		pValue = PyLong_FromVoidPtr(os);
		PyTuple_SetItem(pArgs, 1, pValue);
		pValue = PyObject_CallObject(pFunc, pArgs);
		Py_DECREF(pArgs);
		Py_DECREF(pFunc);
		pHandle = PyObject_GetAttrString(pValue, "handle");
		pAddr   = PyObject_GetAttrString(pHandle, "value");
		t       = (ccs_tuner_t)PyLong_AsVoidPtr(pAddr);
		err     = ccs_retain_object(t);
		Py_DECREF(pHandle);
		Py_DECREF(pAddr);
		Py_DECREF(pValue);
		assert(err == CCS_RESULT_SUCCESS);
		test_tuner(t, os);
		err = ccs_release_object(t);
		assert(err == CCS_RESULT_SUCCESS);
	} else {
		if (PyErr_Occurred())
			PyErr_Print();
		assert(0);
	}
	err = ccs_release_object(os);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(cs);
	assert(err == CCS_RESULT_SUCCESS);
	Py_Finalize();
	return;
#if PY_VERSION_HEX >= 0x030b0000
exception:
	PyConfig_Clear(&config);
	Py_ExitStatusException(status);
#endif
}

int
main(void)
{
	ccs_init();
	test();
	return 0;
}
