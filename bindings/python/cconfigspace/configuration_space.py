import ctypes as ct
from .base import Object, Error, Result, _ccs_get_function, ccs_context, ccs_parameter, ccs_configuration_space, ccs_configuration, ccs_rng, ccs_expression, Datum, ccs_bool, ccs_distribution_space
from .context import Context
from .distribution import Distribution
from .parameter import Parameter
from .expression import Expression
from .expression_parser import parser
from .rng import Rng

ccs_create_configuration_space = _ccs_get_function("ccs_create_configuration_space", [ct.c_char_p, ct.c_size_t, ct.POINTER(ccs_parameter), ct.POINTER(ccs_expression), ct.c_size_t, ct.POINTER(ccs_expression), ct.POINTER(ccs_configuration_space)])
ccs_configuration_space_set_rng = _ccs_get_function("ccs_configuration_space_set_rng", [ccs_configuration_space, ccs_rng])
ccs_configuration_space_get_rng = _ccs_get_function("ccs_configuration_space_get_rng", [ccs_configuration_space, ct.POINTER(ccs_rng)])
ccs_configuration_space_get_condition = _ccs_get_function("ccs_configuration_space_get_condition", [ccs_configuration_space, ct.c_size_t, ct.POINTER(ccs_expression)])
ccs_configuration_space_get_conditions = _ccs_get_function("ccs_configuration_space_get_conditions", [ccs_configuration_space, ct.c_size_t, ct.POINTER(ccs_expression), ct.POINTER(ct.c_size_t)])
ccs_configuration_space_get_forbidden_clause = _ccs_get_function("ccs_configuration_space_get_forbidden_clause", [ccs_configuration_space, ct.c_size_t, ct.POINTER(ccs_expression)])
ccs_configuration_space_get_forbidden_clauses = _ccs_get_function("ccs_configuration_space_get_forbidden_clauses", [ccs_configuration_space, ct.c_size_t, ct.POINTER(ccs_expression), ct.POINTER(ct.c_size_t)])
ccs_configuration_space_check_configuration = _ccs_get_function("ccs_configuration_space_check_configuration", [ccs_configuration_space, ccs_configuration, ct.POINTER(ccs_bool)])
ccs_configuration_space_check_configuration_values = _ccs_get_function("ccs_configuration_space_check_configuration_values", [ccs_configuration_space, ct.c_size_t, ct.POINTER(Datum), ct.POINTER(ccs_bool)])
ccs_configuration_space_get_default_configuration = _ccs_get_function("ccs_configuration_space_get_default_configuration", [ccs_configuration_space, ct.POINTER(ccs_configuration)])
ccs_configuration_space_sample = _ccs_get_function("ccs_configuration_space_sample", [ccs_configuration_space, ccs_distribution_space, ccs_rng, ct.POINTER(ccs_configuration)])
ccs_configuration_space_samples = _ccs_get_function("ccs_configuration_space_samples", [ccs_configuration_space, ccs_distribution_space, ccs_rng, ct.c_size_t, ct.POINTER(ccs_configuration)])

class ConfigurationSpace(Context):
  def __init__(self, handle = None, retain = False, auto_release = True,
               name = "", parameters = None, conditions = None, forbidden_clauses = None):
    if handle is None:
      count = len(parameters)

      if forbidden_clauses is not None:
        numfc = len(forbidden_clauses)
        if numfc > 0:
          ctx = dict(zip([x.name for x in parameters], parameters))
          forbidden_clauses = [ parser.parse(fc, extra = ctx) if isinstance(fc, str) else fc for fc in forbidden_clauses ]
          fcv = (ccs_expression * numfc)(*[x.handle.value for x in forbidden_clauses])
        else:
          fcv = None
      else:
        numfc = 0
        fcv = None

      if conditions is not None:
        namedict = dict(zip([x.name for x in parameters], parameters))
        indexdict = dict(reversed(ele) for ele in enumerate(parameters))
        cv = (ccs_expression * count)()
        conditions = dict( (k, parser.parse(v, extra = namedict) if isinstance(v, str) else v) for (k, v) in conditions.items() )
        for (k, v) in conditions.items():
          if isinstance(k, Parameter):
            cv[indexdict[k]] = v.handle.value
          elif isinstance(k, str):
            cv[indexdict[namedict[k]]] = v.handle.value
          else:
            cv[k] = v.handle.value
      else:
        cv = None

      parameters = (ccs_parameter * count)(*[x.handle.value for x in parameters])
      handle = ccs_configuration_space()
      res = ccs_create_configuration_space(str.encode(name), count, parameters, cv, numfc, fcv, ct.byref(handle))
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

  def forbidden_clause(self, index):
    v = ccs_expression()
    res = ccs_configuration_space_get_forbidden_clause(self.handle, index, ct.byref(v))
    Error.check(res)
    return Expression.from_handle(v)

  @property
  def num_forbidden_clauses(self):
    if hasattr(self, "_num_forbidden_clauses"):
      return self._num_forbidden_clauses
    v = ct.c_size_t()
    res = ccs_configuration_space_get_forbidden_clauses(self.handle, 0, None, ct.byref(v))
    Error.check(res)
    self._num_forbidden_clauses = v.value
    return self._num_forbidden_clauses

  @property
  def forbidden_clauses(self):
    if hasattr(self, "_forbidden_clauses"):
      return self._forbidden_clauses
    sz = self.num_forbidden_clauses
    v = (ccs_expression * sz)()
    res = ccs_configuration_space_get_forbidden_clauses(self.handle, sz, v, None)
    Error.check(res)
    self._forbidden_clauses = tuple(Expression.from_handle(ccs_expression(x)) for x in v)
    return self._forbidden_clauses

  def check(self, configuration):
    valid = ccs_bool()
    res = ccs_configuration_space_check_configuration(self.handle, configuration.handle, ct.byref(valid))
    Error.check(res)
    return False if valid.value == 0 else True

  def check_values(self, values):
    count = len(values)
    if count != self.num_parameters:
      raise Error(Result(Result.ERROR_INVALID_VALUE))
    v = (Datum * count)()
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

  def sample(self, distribution_space = None, rng = None):
    v = ccs_configuration()
    res = ccs_configuration_space_sample(self.handle, distribution_space.handle if distribution_space is not None else None, rng.handle if rng is not None else None, ct.byref(v))
    Error.check(res)
    return Configuration(handle = v, retain = False)

  def samples(self, count, distribution_space = None, rng = None):
    v = (ccs_configuration * count)()
    res = ccs_configuration_space_samples(self.handle, distribution_space.handle if distribution_space is not None else None, rng.handle if rng is not None else None, count, v)
    Error.check(res)
    return [Configuration(handle = ccs_configuration(x), retain = False) for x in v]

from .configuration import Configuration
