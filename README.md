# CCS: C Configuration Space and Tuning Library

CCS provides a C interface for describing autotuning problems and autotuners,
enabling interoperability between autotuning frameworks and applications with
auto-tuning needs. It was greatly inspired by
[ConfigSpace](https://github.com/automl/ConfigSpace).

## Features

- **Parameter types**: numerical (integer/float), categorical, ordinal,
  discrete, and string parameters
- **Configuration spaces**: define search spaces with parameters, conditions,
  and forbidden clauses
- **Objective spaces**: specify optimization objectives (minimize/maximize)
- **Tuners**: built-in random tuner and user-defined tuner interface using an
  ask/tell pattern
- **Feature spaces**: support for contextual tuning with feature-dependent
  optimization
- **Tree spaces**: static and dynamic tree-structured search spaces
- **Expressions**: expression trees for conditions and forbidden clauses
- **Serialization**: binary serialization/deserialization of all objects
- **Thread safety**: optional thread-safe mode (`--enable-thread-safe`,
  enabled by default)
- **Bindings**: Ruby and Python bindings
- **Connectors**: Kokkos profiling connector
- **Reference counting**: all objects are reference-counted for safe memory
  management

## Dependencies

**Required:**
- C compiler (GCC or Clang) with C99 support
- C++ compiler with C++14 support (for the Kokkos connector)
- [GNU Autotools](https://www.gnu.org/software/automake/) (autoconf, automake,
  libtool)
- [GSL](https://www.gnu.org/software/gsl/) (GNU Scientific Library)

**Optional:**
- Ruby (>= 2.3) with FFI gem — for Ruby bindings and tests
- Python 3 (>= 3.6) with `ctypes` — for Python bindings and samples
- [Valgrind](https://valgrind.org/) — for memory checking

## Building

```sh
# Generate the configure script
./autogen.sh

# Configure (out-of-source build recommended)
mkdir build && cd build
../configure

# Build
make -j$(nproc)

# Run tests
make check
```

### Configure options

| Option | Default | Description |
|--------|---------|-------------|
| `--enable-strict` | no | Enable `-Werror` (treat warnings as errors) |
| `--enable-thread-safe` | yes | Build with thread safety (reader-writer locks) |
| `--enable-kokkos-connector` | yes | Build the Kokkos profiling connector |
| `--enable-samples` | yes | Build interoperability samples (requires Python 3) |

### Installation

```sh
make install
```

The library installs a pkg-config file (`cconfigspace.pc`) for easy
integration.

## Quick example

The following C example demonstrates the core workflow: define parameters,
create a configuration space and an objective space, then use a random tuner
with the ask/tell pattern to minimize a simple function.

```c
#include <cconfigspace.h>
#include <assert.h>
#include <math.h>

int main() {
    ccs_parameter_t           parameters[2];
    ccs_configuration_space_t cspace;
    ccs_objective_space_t     ospace;
    ccs_parameter_t           obj_param;
    ccs_expression_t          obj_expr;
    ccs_objective_type_t      obj_type;
    ccs_tuner_t               tuner;
    ccs_result_t              err;

    /* Create two numerical parameters: x in [-5, 5], y in [-5, 5] */
    err = ccs_create_numerical_parameter(
        "x", CCS_NUMERIC_TYPE_FLOAT,
        CCSF(-5.0), CCSF(5.0), CCSF(0.0), CCSF(0), &parameters[0]);
    assert(err == CCS_RESULT_SUCCESS);

    err = ccs_create_numerical_parameter(
        "y", CCS_NUMERIC_TYPE_FLOAT,
        CCSF(-5.0), CCSF(5.0), CCSF(0.0), CCSF(0), &parameters[1]);
    assert(err == CCS_RESULT_SUCCESS);

    /* Create a configuration space from the parameters */
    err = ccs_create_configuration_space(
        "2dplane", 2, parameters, NULL, 0, NULL,
        NULL, NULL, &cspace);
    assert(err == CCS_RESULT_SUCCESS);

    /* Create an objective space: minimize z */
    obj_param = NULL;
    err = ccs_create_numerical_parameter(
        "z", CCS_NUMERIC_TYPE_FLOAT,
        CCSF(-CCS_INFINITY), CCSF(CCS_INFINITY),
        CCSF(0.0), CCSF(0), &obj_param);
    assert(err == CCS_RESULT_SUCCESS);

    err = ccs_create_variable(obj_param, &obj_expr);
    assert(err == CCS_RESULT_SUCCESS);

    obj_type = CCS_OBJECTIVE_TYPE_MINIMIZE;
    err = ccs_create_objective_space(
        "height", (ccs_search_space_t)cspace, 1, &obj_param,
        1, &obj_expr, &obj_type, &ospace);
    assert(err == CCS_RESULT_SUCCESS);

    /* Create a random tuner */
    err = ccs_create_random_tuner("problem", ospace, &tuner);
    assert(err == CCS_RESULT_SUCCESS);

    /* Ask/tell loop: sample 100 configurations and report results */
    for (int i = 0; i < 100; i++) {
        ccs_datum_t                values[2], result;
        ccs_search_configuration_t config;
        ccs_evaluation_t           eval;

        err = ccs_tuner_ask(tuner, NULL, 1, &config, NULL);
        assert(err == CCS_RESULT_SUCCESS);

        err = ccs_binding_get_values(
            (ccs_binding_t)config, 2, values, NULL);
        assert(err == CCS_RESULT_SUCCESS);

        /* Objective: (x-1)^2 + (y-2)^2 (minimum at x=1, y=2) */
        double x = values[0].value.f, y = values[1].value.f;
        result = ccs_float((x - 1) * (x - 1) + (y - 2) * (y - 2));

        err = ccs_create_evaluation(
            ospace, config, CCS_RESULT_SUCCESS, 1, &result, &eval);
        assert(err == CCS_RESULT_SUCCESS);

        err = ccs_tuner_tell(tuner, 1, &eval);
        assert(err == CCS_RESULT_SUCCESS);

        ccs_release_object(config);
        ccs_release_object(eval);
    }

    /* Retrieve the best configuration found */
    ccs_evaluation_t best;
    ccs_datum_t      best_value;
    err = ccs_tuner_get_optima(tuner, NULL, 1, &best, NULL);
    assert(err == CCS_RESULT_SUCCESS);
    err = ccs_evaluation_get_objective_value(best, 0, &best_value);
    assert(err == CCS_RESULT_SUCCESS);

    /* Clean up */
    ccs_release_object(tuner);
    ccs_release_object(ospace);
    ccs_release_object(obj_expr);
    ccs_release_object(obj_param);
    ccs_release_object(cspace);
    ccs_release_object(parameters[0]);
    ccs_release_object(parameters[1]);

    return 0;
}
```

Compile with:
```sh
gcc -o example example.c $(pkg-config --cflags --libs cconfigspace) -lm
```

## Documentation

API documentation is generated with Doxygen and published at:
https://argonne-lcf.github.io/CCS/index.html

To build locally:
```sh
doxygen Doxyfile
```

## License

BSD 3-Clause. See [LICENSE](LICENSE) for details.

Copyright (c) 2020, UChicago Argonne, LLC.
