import ctypes as ct
from .base import Object, Error, ccs_error, _ccs_get_function, ccs_context, ccs_parameter, ccs_configuration_space, ccs_configuration, ccs_rng, ccs_distribution, ccs_expression, ccs_datum, ccs_bool
from .context import Context
from .distribution import Distribution
from .parameter import Parameter
from .expression import Expression
from .expression_parser import ccs_parser
from .rng import Rng
from parglare.parser import Context as PContext

ccs_create_configuration_space = _ccs_get_function("ccs_create_configuration_space", [ct.c_char_p, ct.POINTER(ccs_configuration_space)])
ccs_configuration_space_set_rng = _ccs_get_function("ccs_configuration_space_set_rng", [ccs_configuration_space, ccs_rng])
ccs_configuration_space_get_rng = _ccs_get_function("ccs_configuration_space_get_rng", [ccs_configuration_space, ct.POINTER(ccs_rng)])
ccs_configuration_space_add_parameter = _ccs_get_function("ccs_configuration_space_add_parameter", [ccs_configuration_space, ccs_parameter, ccs_distribution])
ccs_configuration_space_add_parameters = _ccs_get_function("ccs_configuration_space_add_parameters", [ccs_configuration_space, ct.c_size_t, ct.POINTER(ccs_parameter), ct.POINTER(ccs_distribution)])
ccs_configuration_space_set_distribution = _ccs_get_function("ccs_configuration_space_set_distribution", [ccs_configuration_space, ccs_distribution, ct.POINTER(ct.c_size_t)])
ccs_configuration_space_get_parameter_distribution = _ccs_get_function("ccs_configuration_space_get_parameter_distribution", [ccs_configuration_space, ct.c_size_t, ct.POINTER(ccs_distribution), ct.POINTER(ct.c_size_t)])
ccs_configuration_space_set_condition = _ccs_get_function("ccs_configuration_space_set_condition", [ccs_configuration_space, ct.c_size_t, ccs_expression])
ccs_configuration_space_get_condition = _ccs_get_function("ccs_configuration_space_get_condition", [ccs_configuration_space, ct.c_size_t, ct.POINTER(ccs_expression)])
ccs_configuration_space_get_conditions = _ccs_get_function("ccs_configuration_space_get_conditions", [ccs_configuration_space, ct.c_size_t, ct.POINTER(ccs_expression), ct.POINTER(ct.c_size_t)])
ccs_configuration_space_add_forbidden_clause = _ccs_get_function("ccs_configuration_space_add_forbidden_clause", [ccs_configuration_space, ccs_expression])
ccs_configuration_space_add_forbidden_clauses = _ccs_get_function("ccs_configuration_space_add_forbidden_clauses", [ccs_configuration_space, ct.c_size_t, ccs_expression])
ccs_configuration_space_get_forbidden_clause = _ccs_get_function("ccs_configuration_space_get_forbidden_clause", [ccs_configuration_space, ct.c_size_t, ct.POINTER(ccs_expression)])
ccs_configuration_space_get_forbidden_clauses = _ccs_get_function("ccs_configuration_space_get_forbidden_clauses", [ccs_configuration_space, ct.c_size_t, ct.POINTER(ccs_expression), ct.POINTER(ct.c_size_t)])
ccs_configuration_space_check_configuration = _ccs_get_function("ccs_configuration_space_check_configuration", [ccs_configuration_space, ccs_configuration, ct.POINTER(ccs_bool)])
ccs_configuration_space_check_configuration_values = _ccs_get_function("ccs_configuration_space_check_configuration_values", [ccs_configuration_space, ct.c_size_t, ct.POINTER(ccs_datum), ct.POINTER(ccs_bool)])
ccs_configuration_space_get_default_configuration = _ccs_get_function("ccs_configuration_space_get_default_configuration", [ccs_configuration_space, ct.POINTER(ccs_configuration)])
ccs_configuration_space_sample = _ccs_get_function("ccs_configuration_space_sample", [ccs_configuration_space, ct.POINTER(ccs_configuration)])
ccs_configuration_space_samples = _ccs_get_function("ccs_configuration_space_samples", [ccs_configuration_space, ct.c_size_t, ct.POINTER(ccs_configuration)])

class ConfigurationSpace(Context):
  def __init__(self, handle = None, retain = False, auto_release = True,
               name = ""):
    if handle is None:
      handle = ccs_configuration_space()
      res = ccs_create_configuration_space(str.encode(name), ct.byref(handle))
      Error.check(res)
      super().__init__(handle = handle, retain = False)
    else:
      super().__init__(handle = handle, retain = retain, auto_release = auto_release)

  @classmethod
  def from_handle(cls, handle, retain = True, auto_release = True):
    return cls(handle = handle, retain = retain, auto_release = auto_release)

  @property
  def rng(self):
    v = ccs_rng()
    res = ccs_configuration_space_get_rng(self.handle, ct.byref(v))
    Error.check(res)
    return Rng.from_handle(v)

  @rng.setter
  def rng(self, r):
    res = ccs_configuration_space_set_rng(self.handle, r.handle)
    Error.check(res)

  def add_parameter(self, parameter, distribution = None):
    if distribution:
      distribution = distribution.handle
    res = ccs_configuration_space_add_parameter(self.handle, parameter.handle, distribution)
    Error.check(res)

  def add_parameters(self, parameters, distributions = None):
    count = len(parameters)
    if count == 0:
      return None
    if distributions:
      if count != len(distributions):
        raise Error(ccs_error(ccs_error.INVALID_VALUE))
      distribs = (ccs_distribution * count)(*[x.handle.value if x else x for x in distributions])
    else:
      distribs = None
    parameters = (ccs_parameter * count)(*[x.handle.value for x in parameters])
    res = ccs_configuration_space_add_parameters(self.handle, count, parameters, distribs)
    Error.check(res)

  def set_distribution(self, distribution, parameters):
    count = distribution.dimension
    if count != len(parameters):
        raise Error(ccs_error(ccs_error.INVALID_VALUE))
    hyps = []
    for h in parameters:
      if isinstance(h, Parameter):
        hyps.append(self.parameter_index(h))
      elif isinstance(h, str):
        hyps.append(self.parameter_index_by_name(h))
      else:
        hyps.append(h)
    v = (ct.c_size_t * count)(*hyps)
    res = ccs_configuration_space_set_distribution(self.handle, distribution.handle, v)
    Error.check(res)

  def get_parameter_distribution(self, parameter):
    if isinstance(parameter, Parameter):
      parameter = self.parameter_index(parameter)
    elif isinstance(parameter, str):
      parameter = self.parameter_index_by_name(parameter)
    v1 = ccs_distribution()
    v2 = ct.c_size_t()
    res = ccs_configuration_space_get_parameter_distribution(self.handle, parameter, ct.byref(v1), ct.byref(v2))
    Error.check(res)
    return [Distribution.from_handle(v1), v2.value]

  def set_condition(self, parameter, expression):
    if isinstance(expression, str):
      expression = ccs_parser.parse(expression, context = PContext(extra=self))
    if isinstance(parameter, Parameter):
      parameter = self.parameter_index(parameter)
    elif isinstance(parameter, str):
      parameter = self.parameter_index_by_name(parameter)
    res = ccs_configuration_space_set_condition(self.handle, parameter, expression.handle)
    Error.check(res)

  def condition(self, parameter):
    if isinstance(parameter, Parameter):
      parameter = self.parameter_index(parameter)
    elif isinstance(parameter, str):
      parameter = self.parameter_index_by_name(parameter)
    v = ccs_expression()
    res = ccs_configuration_space_get_condition(self.handle, parameter, ct.byref(v)) 
    Error.check(res)
    if v.value is None:
      return None
    else:
      return Expression.from_handle(v)

  @property
  def num_conditions(self):
    return self.num_parameters

  @property
  def conditions(self):
    sz = self.num_parameters
    if sz == 0:
      return []
    v = (ccs_expression * sz)()
    res = ccs_configuration_space_get_conditions(self.handle, sz, v, None)
    Error.check(res)
    return [Expression.from_handle(ccs_expression(x)) if x else None for x in v]    

  @property
  def conditional_parameters(self):
    hps = self.parameters
    conds = self.conditions
    return [x for x, y in zip(hps, conds) if y is not None]

  @property
  def unconditional_parameters(self):
    hps = self.parameters
    conds = self.conditions
    return [x for x, y in zip(hps, conds) if y is None]

  def add_forbidden_clause(self, expression):
    if isinstance(expression, str):
      expression = ccs_parser.parse(expression, context = PContext(extra=self))
    res = ccs_configuration_space_add_forbidden_clause(self.handle, expression.handle)
    Error.check(res)

  def add_forbidden_clauses(self, expressions):
    sz = len(expressions)
    if sz == 0:
      return None
    expressions = [ ccs_parser.parse(expression, context = PContext(extra=self)) if isinstance(expression, str) else expression for expression in expressions ]
    v = (ccs_expression * sz)(*[x.handle.value for x in expressions])
    res = ccs_configuration_space_add_forbidden_clauses(self.handle, sz, v)
    Error.check(res)

  def forbidden_clause(self, index):
    v = ccs_expression()
    res = ccs_configuration_space_get_forbidden_clause(self.handle, index, ct.byref(v))
    Error.check(res)
    return Expression.from_handle(v)

  @property
  def num_forbidden_clauses(self):
    v = ct.c_size_t()
    res = ccs_configuration_space_get_forbidden_clauses(self.handle, 0, None, ct.byref(v))
    Error.check(res)
    return v.value

  @property
  def forbidden_clauses(self):
    sz = self.num_forbidden_clauses
    if sz == 0:
      return []
    v = (ccs_expression * sz)()
    res = ccs_configuration_space_get_forbidden_clauses(self.handle, sz, v, None)
    Error.check(res)
    return [Expression.from_handle(ccs_expression(x)) for x in v]

  def check(self, configuration):
    valid = ccs_bool()
    res = ccs_configuration_space_check_configuration(self.handle, configuration.handle, ct.byref(valid))
    Error.check(res)
    return False if valid.value == 0 else True

  def check_values(self, values):
    count = len(values)
    if count != self.num_parameters:
      raise Error(ccs_error(ccs_error.INVALID_VALUE))
    v = (ccs_datum * count)()
    ss = []
    for i in range(count):
      v[i].set_value(values[i], string_store = ss)
    valid = ccs_bool()
    res = ccs_configuration_space_check_configuration_values(self.handle, count, v, ct.byref(valid))
    Error.check(res)
    return False if valid.value == 0 else True

  @property
  def default_configuration(self):
    v = ccs_configuration()
    res = ccs_configuration_space_get_default_configuration(self.handle, ct.byref(v))
    Error.check(res)
    return Configuration(handle = v, retain = False)

  def sample(self):
    v = ccs_configuration()
    res = ccs_configuration_space_sample(self.handle, ct.byref(v))
    Error.check(res)
    return Configuration(handle = v, retain = False)

  def samples(self, count):
    if count == 0:
      return []
    v = (ccs_configuration * count)()
    res = ccs_configuration_space_samples(self.handle, count, v)
    Error.check(res)
    return [Configuration(handle = ccs_configuration(x), retain = False) for x in v]

from .configuration import Configuration
