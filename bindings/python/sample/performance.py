"""Provides the following type of outputs:

test_linear_space(k=10) -> 0.084 sec ± 0.001
test_linear_space(k=20) -> 0.136 sec ± 0.002
test_hierarchical() -> 0.047 sec ± 0.001
"""
import time
import numpy as np

import cconfigspace as ccs


def repeat(f, *args, repetitions=10, **kwargs):
    T = []
    for _ in range(repetitions):
        t1 = time.time()

        f(*args, **kwargs)

        duration = time.time() - t1
        T.append(duration)

    mean_duration = np.mean(T)
    std_duration = np.std(T)
    return mean_duration, std_duration


def config_to_str(config):
    args = ""
    args_list = config.get("args", [])
    for i, arg in enumerate(args_list):
        args += str(arg)
        if i < len(args_list) - 1:
            args += ", "

    kwargs = ""
    kwargs_dict = config.get("kwargs", {})
    for i, (kw_k, kw_v) in enumerate(kwargs_dict.items()):
        kwargs += f"{kw_k}={kw_v}"
        if i < len(kwargs_dict) - 1:
            kwargs += ", "

    if args == "" and kwargs == "":
        return f"{config['f'].__name__}()"
    elif args != "" and kwargs == "":
        return f"{config['f'].__name__}({args})"
    elif args == "" and kwargs != "":
        return f"{config['f'].__name__}({kwargs})"
    else:
        return f"{config['f'].__name__}({args}, {kwargs})"


def test_linear_space(k):
    """Test the sampling performance of a configuration space without conditions or forbiddens."""
    space_ccs = ccs.ConfigurationSpace()
    hps = [
        ccs.NumericalHyperparameter(lower=0, upper=1, name=f"x{i}") for i in range(k)
    ]
    space_ccs.add_hyperparameters(hps)

    configs = space_ccs.samples(10000)
    configs = [c.values for c in configs]


def test_hierarchical():
    """Test the sampling performance of a configuration space conditions."""
    space_ccs = ccs.ConfigurationSpace()
    x = ccs.OrdinalHyperparameter(values=[0, 1], name="x", default_index=1)
    y = ccs.NumericalHyperparameter(lower=0, upper=1, name="y")
    z = ccs.CategoricalHyperparameter(values=["0", "1"], name="z")
    space_ccs.add_hyperparameters([x, y, z])

    space_ccs.set_condition(y, "x == 1")
    space_ccs.set_condition(z, "x == 1")

    configs = space_ccs.samples(10000)
    configs = [c.values for c in configs]


if __name__ == "__main__":

    funcs = [
        {"f": test_linear_space, "kwargs": {"k": 10}},
        {"f": test_linear_space, "kwargs": {"k": 20}},
        {"f": test_hierarchical},
    ]
    repetitions = 10

    for f_config in funcs:
        mean_dur, std_dur = repeat(
            f_config["f"], *f_config.get("args", []), **f_config.get("kwargs", {})
        )
        print(f"{config_to_str(f_config)} -> {mean_dur:.3f} sec ± {std_dur:.3f}")
