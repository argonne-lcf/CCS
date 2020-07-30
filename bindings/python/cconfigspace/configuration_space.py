import ctypes as ct
from .base import Object, Error, ccs_error, _ccs_get_function, ccs_context, ccs_hyperparameter, ccs_configuration_space, ccs_configuration, ccs_rng, ccs_distribution, ccs_expression, ccs_datum
from .context import Context
from .hyperparameter import Hyperparameter
from .expression import Expression
from .rng import Rng

ccs_create_configuration_space = _ccs_get_function("ccs_create_configuration_space", [ct.c_char_p, ct.c_void_p, ct.POINTER(ccs_configuration_space)])
ccs_configuration_space_get_name = _ccs_get_function("ccs_configuration_space_get_name", [ccs_configuration_space, ct.POINTER(ct.c_char_p)])
ccs_configuration_space_get_user_data = _ccs_get_function("ccs_configuration_space_get_user_data", [ccs_configuration_space, ct.POINTER(ct.c_void_p)])
ccs_configuration_space_set_rng = _ccs_get_function("ccs_configuration_space_set_rng", [ccs_configuration_space, ccs_rng])
ccs_configuration_space_get_rng = _ccs_get_function("ccs_configuration_space_get_rng", [ccs_configuration_space, ct.POINTER(ccs_rng)])
ccs_configuration_space_add_hyperparameter = _ccs_get_function("ccs_configuration_space_add_hyperparameter", [ccs_configuration_space, ccs_hyperparameter, ccs_distribution])
ccs_configuration_space_add_hyperparameters = _ccs_get_function("ccs_configuration_space_add_hyperparameters", [ccs_configuration_space, ct.c_size_t, ct.POINTER(ccs_hyperparameter), ct.POINTER(ccs_distribution)])
ccs_configuration_space_get_num_hyperparameters = _ccs_get_function("ccs_configuration_space_get_num_hyperparameters", [ccs_configuration_space, ct.POINTER(ct.c_size_t)])
ccs_configuration_space_get_hyperparameter = _ccs_get_function("ccs_configuration_space_get_hyperparameter", [ccs_configuration_space, ct.c_size_t, ct.POINTER(ccs_hyperparameter)])
ccs_configuration_space_get_hyperparameter_by_name = _ccs_get_function("ccs_configuration_space_get_hyperparameter_by_name", [ccs_configuration_space, ct.c_char_p, ct.POINTER(ccs_hyperparameter)])
ccs_configuration_space_get_hyperparameter_index_by_name = _ccs_get_function("ccs_configuration_space_get_hyperparameter_index_by_name", [ccs_configuration_space, ct.c_char_p, ct.POINTER(ct.c_size_t)])
ccs_configuration_space_get_hyperparameter_index = _ccs_get_function("ccs_configuration_space_get_hyperparameter_index", [ccs_configuration_space, ccs_hyperparameter, ct.POINTER(ct.c_size_t)])
ccs_configuration_space_get_hyperparameter_indexes = _ccs_get_function("ccs_configuration_space_get_hyperparameter_indexes", [ccs_configuration_space, ct.c_size_t, ct.POINTER(ccs_hyperparameter), ct.POINTER(ct.c_size_t)])
ccs_configuration_space_get_hyperparameters = _ccs_get_function("ccs_configuration_space_get_hyperparameters", [ccs_configuration_space, ct.c_size_t, ct.POINTER(ccs_hyperparameter), ct.POINTER(ct.c_size_t)])
ccs_configuration_space_set_condition = _ccs_get_function("ccs_configuration_space_set_condition", [ccs_configuration_space, ct.c_size_t, ccs_expression])
ccs_configuration_space_get_condition = _ccs_get_function("ccs_configuration_space_get_condition", [ccs_configuration_space, ct.c_size_t, ct.POINTER(ccs_expression)])
ccs_configuration_space_get_conditions = _ccs_get_function("ccs_configuration_space_get_conditions", [ccs_configuration_space, ct.c_size_t, ct.POINTER(ccs_expression), ct.POINTER(ct.c_size_t)])
ccs_configuration_space_add_forbidden_clause = _ccs_get_function("ccs_configuration_space_add_forbidden_clause", [ccs_configuration_space, ccs_expression])
ccs_configuration_space_add_forbidden_clauses = _ccs_get_function("ccs_configuration_space_add_forbidden_clauses", [ccs_configuration_space, ct.c_size_t, ccs_expression])
ccs_configuration_space_get_forbidden_clause = _ccs_get_function("ccs_configuration_space_get_forbidden_clause", [ccs_configuration_space, ct.c_size_t, ct.POINTER(ccs_expression)])
ccs_configuration_space_get_forbidden_clauses = _ccs_get_function("ccs_configuration_space_get_forbidden_clauses", [ccs_configuration_space, ct.c_size_t, ct.POINTER(ccs_expression), ct.POINTER(ct.c_size_t)])
ccs_configuration_space_check_configuration = _ccs_get_function("ccs_configuration_space_check_configuration", [ccs_configuration_space, ccs_configuration])
ccs_configuration_space_check_configuration_values = _ccs_get_function("ccs_configuration_space_check_configuration_values", [ccs_configuration_space, ct.c_size_t, ct.POINTER(ccs_datum)])
ccs_configuration_space_get_default_configuration = _ccs_get_function("ccs_configuration_space_get_default_configuration", [ccs_configuration_space, ct.POINTER(ccs_configuration)])
ccs_configuration_space_sample = _ccs_get_function("ccs_configuration_space_sample", [ccs_configuration_space, ct.POINTER(ccs_configuration)])
ccs_configuration_space_samples = _ccs_get_function("ccs_configuration_space_samples", [ccs_configuration_space, ct.c_size_t, ct.POINTER(ccs_configuration)])

class ConfigurationSpace(Context):
  def __init__(self, handle = None, retain = False, name = "", user_data = None):
    if handle is None:
      handle = ccs_configuration_space()
      res = ccs_create_configuration_space(str.encode(name), user_data, ct.byref(handle))
      Error.check(res)
      super().__init__(handle = handle, retain = False)
    else:
      super().__init__(handle = handle, retain = retain)

  @classmethod
  def from_handle(cls, handle):
    return cls(handle = handle, retain = True)

  @property
  def user_data(self):
    if hasattr(self, "_user_data"):
      return self._user_data
    v = ct.c_void_p()
    res = ccs_configuration_space_get_user_data(self.handle, ct.byref(v))
    Error.check(res)
    self._user_data = v
    return v

  @property
  def name(self):
    if hasattr(self, "_name"):
      return self._name
    v = ct.c_char_p()
    res = ccs_configuration_space_get_name(self.handle, ct.byref(v))
    Error.check(res)
    self._name = v.value.decode()
    return self._name

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

  def add_hyperparameter(self, hyperparameter, distribution = None):
    if distribution:
      distribution = distribution.handle
    res = ccs_configuration_space_add_hyperparameter(self.handle, hyperparameter.handle, distribution)
    Error.check(res)

  def add_hyperparameters(self, hyperparameters, distributions = None):
    count = len(hyperparameters)
    if count == 0:
      return None
    if distributions:
      if count != len(distributions):
        raise Error(ccs_error.INVALID_VALUE)
      distribs = (ccs_distribution * count)(*[x.handle.value if x else x for x in distributions])
    else:
      distribs = None
    hypers = (ccs_hyperparameter * count)(*[x.handle.value for x in hyperparameters])
    res = ccs_configuration_space_add_hyperparameters(self.handle, count, hypers, distribs)
    Error.check(res)

  def hyperparameter(self, index):
    v = ccs_hyperparameter()
    res = ccs_configuration_space_get_hyperparameter(self.handle, index, ct.byref(v))
    Error.check(res)
    return Hyperparameter.from_handle(v)

  def hyperparameter_by_name(self, name):
    v = ccs_hyperparameter()
    res = ccs_configuration_space_get_hyperparameter_by_name(self.handle, str.encode(name), ct.byref(v))
    Error.check(res)
    return Hyperparameter.from_handle(v)

  def hyperparameter_index(self, hyperparameter):
    v = ct.c_size_t()
    res = ccs_configuration_space_get_hyperparameter_index(self.handle, hyperparameter.handle, ct.byref(v))
    Error.check(res)
    return v.value

  def hyperparameter_index_by_name(self, name):
    v = ct.c_size_t()
    res = ccs_configuration_space_get_hyperparameter_index_by_name(self.handle, str.encode(name), ct.byref(v))
    Error.check(res)
    return v.value

  @property
  def num_hyperparameters(self):
    v = ct.c_size_t(0)
    res = ccs_configuration_space_get_num_hyperparameters(self.handle, ct.byref(v))
    Error.check(res)
    return v.value

  @property
  def hyperparameters(self):
    count = self.num_hyperparameters
    if count == 0:
      return []
    v = (ccs_hyperparameter * count)()
    res = ccs_configuration_space_get_hyperparameters(self.handle, count, v, None)
    Error.check(res)
    return [Hyperparameter.from_handle(ccs_hyperparameter(x)) for x in v]

  def set_condition(self, hyperparameter, expression):
    if isinstance(hyperparameter, Hyperparameter):
      hyperparameter = self.hyperparameter_index(hyperparameter)
    elif isinstance(hyperparameter, str):
      hyperparameter = self.hyperparameter_index_by_name(hyperparameter)
    res = ccs_configuration_space_set_condition(self.handle, hyperparameter, expression.handle)
    Error.check(res)

  def condition(self, hyperparameter):
    if isinstance(hyperparameter, Hyperparameter):
      hyperparameter = self.hyperparameter_index(hyperparameter)
    elif isinstance(hyperparameter, str):
      hyperparameter = self.hyperparameter_index_by_name(hyperparameter)
    v = ccs_expression()
    res = ccs_configuration_space_get_condition(self.handle, hyperparameter, ct.byref(v)) 
    Error.check(res)
    if v.value is None:
      return None
    else:
      return Expression.from_handle(v)

  @property
  def num_conditions(self):
    return self.num_hyperparameters

  @property
  def conditions(self):
    sz = self.num_hyperparameters
    if sz == 0:
      return []
    v = (ccs_expression * sz)()
    res = ccs_configuration_space_get_conditions(self.handle, sz, v, None)
    Error.check(res)
    return [Expression.from_handle(ccs_expression(x)) if x else None for x in v]    

  def add_forbidden_clause(self, expression):
    res = ccs_configuration_space_add_forbidden_clause(self.handle, expression.handle)
    Error.check(res)

  def add_forbidden_clauses(self, expressions):
    sz = len(expressions)
    if sz == 0:
      return None
    v = (ccs_expression * sz)(*[x.handle.value if x else x for x in expressions])
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
    res = ccs_configuration_space_check_configuration(self.handle, configuration.handle)
    Error.check(res)

  def check_values(self, values):
    count = len(values)
    if count != self.num_hyperparameters:
      raise Error(ccs_error.INVALID_VALUE)
    v = (ccs_datum * count)()
    for i in range(count):
      v[i].value = values[i]
    res = ccs_configuration_space_check_configuration_values(self.handle, count, v)
    Error.check(res)

  @property
  def default_configuration(self):
    v = ccs_configuration()
    res = ccs_configuration_space_get_default_configuration(self.handle, ct.byref(v))
    Error.check(res)
    return Configuration.from_handle(v)

  def sample(self):
    v = ccs_configuration()
    res = ccs_configuration_space_sample(self.handle, ct.byref(v))
    Error.check(res)
    return Configuration.from_handle(v)

  def samples(self, count):
    if count == 0:
      return []
    v = (ccs_configuration * count)()
    res = ccs_configuration_space_samples(self.handle, count, v)
    Error.check(res)
    return [Configuration.from_handle(x) for x in v]

from .configuration import Configuration
