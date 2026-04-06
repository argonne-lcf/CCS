# CCS Conceptual Guide

This guide explains the architecture, object model, and workflow of CCS (C
Configuration Space and Tuning Library). For build instructions and a quick
example see [README.md](README.md).

## Table of Contents

1. <a href="#overview">Overview</a>
2. <a href="#object-model">Object Model</a>
3. <a href="#parameters">Parameters</a>
4. <a href="#distributions">Distributions</a>
5. <a href="#configuration-spaces">Configuration Spaces</a>
6. <a href="#objective-spaces">Objective Spaces</a>
7. <a href="#tuners-and-the-asktell-pattern">Tuners and the Ask/Tell Pattern</a>
8. <a href="#feature-spaces-and-contextual-tuning">Feature Spaces and Contextual Tuning</a>
9. <a href="#tree-spaces">Tree Spaces</a>
10. <a href="#expressions">Expressions</a>
11. <a href="#serialization">Serialization</a>
12. <a href="#distribution-spaces">Distribution Spaces</a>

---

<a id="overview"></a>
## Overview

CCS is a C library for describing autotuning problems and autotuners. It was
inspired by Python's [ConfigSpace](https://github.com/automl/ConfigSpace) and
provides a language-neutral interface that enables interoperability between
autotuning frameworks and applications.

The core workflow follows the **ask/tell pattern**:

1. Describe the search space (configuration space or tree space).
2. Describe the optimization objectives (objective space).
3. Create a tuner.
4. **Ask** the tuner for candidate configurations.
5. Evaluate candidates externally (run your application/benchmark).
6. **Tell** the tuner the results.
7. Retrieve the best configurations found so far.

This separation of concerns lets application developers describe their tuning
problem while autotuning researchers provide the optimization strategy, all
through the same C interface.

---

<a id="object-model"></a>
## Object Model

### Reference Counting

Every CCS object is reference-counted. Objects start with a reference count of
one at creation. Use `ccs_retain_object()` to increment the count and
`ccs_release_object()` to decrement it. When the count reaches zero the object
is freed. You can query the current count with `ccs_object_get_refcount()`.

```c
ccs_retain_object(param);    /* refcount: 1 -> 2 */
ccs_release_object(param);   /* refcount: 2 -> 1 */
ccs_release_object(param);   /* refcount: 1 -> 0, object freed */
```

### Object Types

All CCS objects share a common base and carry a type tag
(`ccs_object_type_e`). The principal types are:

| Type | Description |
|------|-------------|
| `CCS_OBJECT_TYPE_RNG` | Random number generator (GSL wrapper) |
| `CCS_OBJECT_TYPE_DISTRIBUTION` | Probability distribution |
| `CCS_OBJECT_TYPE_PARAMETER` | Parameter definition |
| `CCS_OBJECT_TYPE_EXPRESSION` | Expression tree node |
| `CCS_OBJECT_TYPE_CONFIGURATION_SPACE` | Search space over parameters |
| `CCS_OBJECT_TYPE_CONFIGURATION` | Bound parameter values |
| `CCS_OBJECT_TYPE_OBJECTIVE_SPACE` | Optimization objective definition |
| `CCS_OBJECT_TYPE_EVALUATION` | Objective function result |
| `CCS_OBJECT_TYPE_TUNER` | Optimization driver |
| `CCS_OBJECT_TYPE_FEATURES` | Bound feature values |
| `CCS_OBJECT_TYPE_FEATURE_SPACE` | Contextual feature definitions |
| `CCS_OBJECT_TYPE_TREE` | Tree node |
| `CCS_OBJECT_TYPE_TREE_SPACE` | Tree-structured search space |
| `CCS_OBJECT_TYPE_TREE_CONFIGURATION` | Bound tree position |
| `CCS_OBJECT_TYPE_DISTRIBUTION_SPACE` | Distribution-to-parameter mapping |
| `CCS_OBJECT_TYPE_MAP` | Key-value store |
| `CCS_OBJECT_TYPE_ERROR_STACK` | Error stack |

### Data Types

CCS uses a tagged union `ccs_datum_t` to represent values generically:

```c
struct ccs_datum_s {
    ccs_value_t       value;   /* union: int, float, bool, string, object */
    ccs_data_type_t   type;    /* CCS_DATA_TYPE_{NONE,INT,FLOAT,BOOL,STRING,INACTIVE,OBJECT} */
    ccs_datum_flags_t flags;
};
```

Helper constructors make datum creation concise: `ccs_int(42)`,
`ccs_float(3.14)`, `ccs_bool(CCS_TRUE)`, `ccs_string("hello")`,
`ccs_object(handle)`.

The `CCS_DATA_TYPE_INACTIVE` type is used when a parameter is deactivated by a
condition (see <a href="#configuration-spaces">Configuration Spaces</a>).

### Error Handling

Every CCS function returns `ccs_result_t`. On success it returns
`CCS_RESULT_SUCCESS` (zero); on failure it returns a negative error code such
as `CCS_RESULT_ERROR_INVALID_VALUE` or `CCS_RESULT_ERROR_OUT_OF_MEMORY`.

For richer diagnostics CCS maintains a thread-local error stack. Retrieve it
with `ccs_get_thread_error()` and set it with `ccs_set_thread_error()`. Error
stacks carry a message, an error code, and source location context (file, line,
function).

---

<a id="parameters"></a>
## Parameters

A **parameter** defines one dimension of a search space. CCS provides five
parameter types (`ccs_parameter_type_e`):

### Numerical

Created with `ccs_create_numerical_parameter()`. Represents a bounded numeric
range, either integer (`CCS_NUMERIC_TYPE_INT`) or floating-point
(`CCS_NUMERIC_TYPE_FLOAT`). Supports optional quantization. Use this for
continuous or discrete numeric ranges like learning rates or batch sizes.

Convenience wrappers: `ccs_create_int_numerical_parameter()`,
`ccs_create_float_numerical_parameter()`.

### Categorical

Created with `ccs_create_categorical_parameter()`. An unordered set of values
(strings, numbers, or other datum types). Use this when there is no meaningful
ordering, e.g. choosing between algorithms ("sgd", "adam", "rmsprop").

### Ordinal

Created with `ccs_create_ordinal_parameter()`. An ordered set of values where
the ordering is defined by the order given at creation. Unlike numerical
parameters, values need not be numbers. Use this for options with a natural
ranking like ("low", "medium", "high").

### Discrete

Created with `ccs_create_discrete_parameter()`. A set of specific numerical
values (integer or float). Use this for a small fixed set of numeric choices,
e.g. thread counts {1, 2, 4, 8, 16}.

### String

Created with `ccs_create_string_parameter()`. A free-form string value that
**cannot be sampled** by distributions. Use this primarily in feature spaces to
represent contextual information such as hardware names.

### Common Operations

All parameter types support:

- `ccs_parameter_get_name()` / `ccs_parameter_get_type()` — introspection
- `ccs_parameter_get_default_value()` — query the default
- `ccs_parameter_check_value()` / `ccs_parameter_validate_value()` — validation
- `ccs_parameter_sample()` / `ccs_parameter_samples()` — sampling (except string)
- `ccs_parameter_get_default_distribution()` — query the default distribution

---

<a id="distributions"></a>
## Distributions

A **distribution** governs how parameter values are sampled. CCS provides five
distribution types (`ccs_distribution_type_e`):

### Uniform

Created with `ccs_create_uniform_distribution()`. Samples uniformly over an
interval. Supports both integer and float types, linear or logarithmic scale
(`CCS_SCALE_TYPE_LINEAR`, `CCS_SCALE_TYPE_LOGARITHMIC`), and optional
quantization.

### Normal

Created with `ccs_create_normal_distribution()`. Samples from a Gaussian with
a given mean (mu) and standard deviation (sigma). Also supports scale and
quantization options.

### Roulette

Created with `ccs_create_roulette_distribution()`. A discrete distribution
where each index is selected with probability proportional to its area (weight).
Use this for weighted categorical selection.

### Mixture

Created with `ccs_create_mixture_distribution()`. Combines multiple
distributions with normalized weights. On each sample, one component
distribution is selected according to its weight, then sampled. All components
must have the same dimensionality.

### Multivariate

Created with `ccs_create_multivariate_distribution()`. Composes multiple
independent distributions into a single higher-dimensional distribution. The
total dimensionality is the sum of its components. Use this to sample multiple
parameters jointly through a single distribution object.

### Sampling

Distributions provide several sampling layouts:

- `ccs_distribution_sample()` — single sample
- `ccs_distribution_samples()` — array-of-structures layout
- `ccs_distribution_strided_samples()` — custom memory stride
- `ccs_distribution_soa_samples()` — structure-of-arrays layout

Each parameter has a default distribution (typically uniform over its range).
You can override the default by associating a custom distribution through a
distribution space (see <a href="#distribution-spaces">Distribution Spaces</a>).

---

<a id="configuration-spaces"></a>
## Configuration Spaces

A **configuration space** (`ccs_configuration_space_t`) groups parameters into a
search space and adds optional constraints. Create one with
`ccs_create_configuration_space()`.

### Parameters

A configuration space owns an ordered list of parameters. Each parameter is
identified by its index and name within the space.

### Conditions

A condition is an expression that determines whether a parameter is **active**.
When a condition evaluates to false for a given configuration, the parameter's
value is set to `CCS_DATA_TYPE_INACTIVE`. This models hierarchical search
spaces—for instance, a "learning_rate" parameter that is only active when
"optimizer" is "sgd".

Conditions must form a directed acyclic graph (no circular dependencies).

### Forbidden Clauses

A forbidden clause is an expression that defines invalid parameter combinations.
Any configuration where a forbidden clause evaluates to true is rejected during
sampling and validation. For example, you might forbid the combination
(algorithm="A", precision="half") if algorithm A does not support half
precision.

### Feature Space Attachment

A configuration space can optionally reference a feature space, linking it to
contextual tuning (see <a href="#feature-spaces-and-contextual-tuning">Feature Spaces and Contextual Tuning</a>).

### Custom RNG

By default, a configuration space uses an internal random number generator for
sampling. You can supply your own RNG at creation time or retrieve it with
`ccs_configuration_space_get_rng()` and adjust the seed with
`ccs_rng_set_seed()`.

### Configurations

Sampling a configuration space produces `ccs_configuration_t` objects—bindings
that associate each parameter with a concrete value. Use
`ccs_binding_get_values()` to read parameter values and
`ccs_binding_get_value_by_name()` to look up values by parameter name.

---

<a id="objective-spaces"></a>
## Objective Spaces

An **objective space** (`ccs_objective_space_t`) defines what to optimize.
Create one with `ccs_create_objective_space()`.

It consists of:

- **Search space**: a reference to a configuration space or tree space that
  defines the input dimensions.
- **Parameters**: output parameters representing the quantities being measured
  (e.g., execution time, accuracy).
- **Objectives**: expressions over the output parameters, each tagged with a
  type:
  - `CCS_OBJECTIVE_TYPE_MINIMIZE` — lower is better
  - `CCS_OBJECTIVE_TYPE_MAXIMIZE` — higher is better

An objective expression is typically just a variable referencing one of the
output parameters, but it can be any valid expression (e.g., a ratio of two
parameters).

### Evaluations

An `ccs_evaluation_t` binds an objective space to concrete objective values
for a given configuration. Create one with `ccs_create_evaluation()`, passing
the configuration that was evaluated, a result code, and the measured values.

Evaluations support multi-objective comparison through
`ccs_evaluation_compare()`, which returns `CCS_COMPARISON_BETTER`,
`CCS_COMPARISON_EQUIVALENT`, `CCS_COMPARISON_WORSE`, or
`CCS_COMPARISON_NOT_COMPARABLE` (for Pareto-incomparable evaluations).

---

<a id="tuners-and-the-asktell-pattern"></a>
## Tuners and the Ask/Tell Pattern

A **tuner** (`ccs_tuner_t`) drives the optimization loop. The core workflow:

```
                    +-----------+
                    |   Tuner   |
                    +-----------+
                     /         \
               ask()/           \tell()
                   v             ^
          +--------------+  +--------------+
          |Configuration |->| Evaluation   |
          +--------------+  +--------------+
                   |             ^
                   v             |
              (external evaluation)
```

1. **Create** a tuner with an objective space.
2. `ccs_tuner_ask()` — request one or more candidate configurations.
3. **Evaluate** each configuration externally (run your experiment/benchmark).
4. `ccs_create_evaluation()` — wrap the results in an evaluation object.
5. `ccs_tuner_tell()` — report evaluations back to the tuner.
6. `ccs_tuner_get_optima()` — retrieve the best evaluation(s) found so far.

Repeat steps 2-6 as many times as needed.

### Tuner Types

| Type | Description |
|------|-------------|
| `CCS_TUNER_TYPE_RANDOM` | Samples configurations uniformly at random. Created with `ccs_create_random_tuner()`. |
| `CCS_TUNER_TYPE_USER_DEFINED` | Delegates to user-provided callbacks. Created with `ccs_create_user_defined_tuner()`. |

The **random tuner** is a simple baseline that requires no tuning strategy—it
randomly explores the search space and tracks the best evaluations seen.

The **user-defined tuner** lets you implement custom optimization strategies
(Bayesian optimization, evolutionary algorithms, etc.) by providing a callback
vector with `ask`, `tell`, `get_optima`, and other operations.

### Additional Tuner Operations

- `ccs_tuner_get_history()` — retrieve all past evaluations
- `ccs_tuner_get_search_space()` / `ccs_tuner_get_objective_space()` — introspection
- `ccs_tuner_get_state()` / `ccs_tuner_set_state()` — state serialization for checkpointing

---

<a id="feature-spaces-and-contextual-tuning"></a>
## Feature Spaces and Contextual Tuning

A **feature space** (`ccs_feature_space_t`) defines contextual parameters that
describe the environment in which optimization takes place. Create one with
`ccs_create_feature_space()`.

For example, when tuning a kernel for multiple GPUs, you might define feature
parameters like "gpu_name" (string), "memory_gb" (numerical), and
"compute_capability" (ordinal). Each combination of feature values represents a
different tuning context.

### Features

A `ccs_features_t` object binds concrete values to a feature space, representing
one specific context. Pass features to `ccs_tuner_ask()` so the tuner can
generate configurations appropriate for that context.

### Contextual Workflow

1. Create a feature space with contextual parameters.
2. Attach it to a configuration space (via `ccs_create_configuration_space()`).
3. Create the objective space referencing that configuration space.
4. Create a tuner.
5. For each context:
   - Create a `ccs_features_t` with the current context values.
   - Call `ccs_tuner_ask(tuner, features, ...)` to get context-aware
     configurations.
   - Evaluate and tell as usual.

This enables the tuner to learn separate (or transfer) models for different
hardware, datasets, or other environmental factors.

---

<a id="tree-spaces"></a>
## Tree Spaces

A **tree space** (`ccs_tree_space_t`) defines a search space over
tree-structured decisions, as opposed to the flat parameter vectors of a
configuration space.

### Tree Nodes

Each `ccs_tree_t` node has:

- An **arity** (number of child slots).
- An associated **datum value**.
- **Subtree weights** that influence sampling probability.

### Static vs. Dynamic

| Type | Description |
|------|-------------|
| `CCS_TREE_SPACE_TYPE_STATIC` | The entire tree is built in memory upfront. Created with `ccs_create_static_tree_space()`. |
| `CCS_TREE_SPACE_TYPE_DYNAMIC` | Nodes are generated on demand via a `get_child()` callback. Created with `ccs_create_dynamic_tree_space()`. |

Dynamic tree spaces are useful when the tree is too large to materialize fully,
or when the structure depends on runtime state.

### Tree Configurations

A `ccs_tree_configuration_t` represents a position in the tree (a path from
root to a node). Tree configurations play the same role as flat configurations
in the ask/tell workflow: the tuner asks for tree configurations, you evaluate
them, and tell the results back.

Tree spaces can also have an attached feature space for contextual tree tuning.

---

<a id="expressions"></a>
## Expressions

The **expression system** provides an AST (abstract syntax tree) for building
conditions, forbidden clauses, and objective formulas.

### Expression Types

Expressions (`ccs_expression_type_e`) include:

- **Boolean**: `OR` (`||`), `AND` (`&&`), `NOT` (`!`)
- **Comparison**: `EQUAL` (`==`), `NOT_EQUAL` (`!=`), `LESS` (`<`),
  `GREATER` (`>`), `LESS_OR_EQUAL` (`<=`), `GREATER_OR_EQUAL` (`>=`)
- **Arithmetic**: `ADD` (`+`), `SUBSTRACT` (`-`), `MULTIPLY` (`*`),
  `DIVIDE` (`/`), `MODULO` (`%`), `POSITIVE`, `NEGATIVE`
- **Collection**: `IN` (`#`) — membership test
- **Structural**: `LIST` — variable-arity list of values
- **Terminals**: `LITERAL` (constant), `VARIABLE` (parameter reference)

### Building Expressions

```c
/* Create: x > 0 */
ccs_expression_t var, lit, expr;
ccs_create_variable(x_param, &var);
ccs_create_literal(ccs_int(0), &lit);
ccs_create_binary_expression(CCS_EXPRESSION_TYPE_GREATER, var, lit, &expr);
```

Convenience functions: `ccs_create_binary_expression()`,
`ccs_create_unary_expression()`, `ccs_create_literal()`,
`ccs_create_variable()`.

Expressions are evaluated against bindings (configurations or evaluations)
using `ccs_expression_eval()`.

---

<a id="serialization"></a>
## Serialization

CCS supports serialization of all object types in two formats:

- **Binary** (`CCS_SERIALIZE_FORMAT_BINARY`) — compact and fast, suitable for
  checkpointing and IPC
- **JSON** (`CCS_SERIALIZE_FORMAT_JSON`) — human-readable, suitable for
  inspection, interoperability, and version control of configuration spaces

Both formats support the same set of operations:

- **Memory buffer** (`CCS_SERIALIZE_OPERATION_MEMORY`) — serialize into a
  caller-provided buffer
- **Library-allocated buffer** (`CCS_SERIALIZE_OPERATION_BUFFER`) — the library
  allocates the buffer; the caller must free it with `ccs_release_buffer()`
- **File path** (`CCS_SERIALIZE_OPERATION_FILE`)
- **File descriptor** (`CCS_SERIALIZE_OPERATION_FILE_DESCRIPTOR`)
- **Size query** (`CCS_SERIALIZE_OPERATION_SIZE`) — compute the buffer size
  needed for `MEMORY`

The JSON format handles non-finite IEEE 754 values (`Infinity`, `-Infinity`,
`NaN`) by encoding them as strings while preserving the numeric type, so they
survive round-trip serialization.

Use `ccs_object_serialize()` to serialize and `ccs_object_deserialize()` to
restore. For user-defined objects (tuners, expressions), custom serialization
callbacks can be registered with `ccs_object_set_serialize_callback()`.

---

<a id="distribution-spaces"></a>
## Distribution Spaces

A **distribution space** (`ccs_distribution_space_t`) customizes how
parameters are sampled within a configuration space. Create one with
`ccs_create_distribution_space()`.

By default, each parameter in a configuration space is sampled from its own
default distribution (typically uniform). A distribution space lets you override
this by mapping custom distributions to specific parameters via
`ccs_distribution_space_set_distribution()`. This is useful for:

- Biasing sampling toward promising regions of the search space.
- Using multivariate distributions to capture correlations between parameters.
- Applying non-uniform priors based on domain knowledge.

Sample configurations from a distribution space directly with
`ccs_distribution_space_sample()` or `ccs_distribution_space_samples()`.
