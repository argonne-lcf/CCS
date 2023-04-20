import ctypes as ct
from .base import Object, Error, CEnumeration, ccs_result, _ccs_get_function, ccs_context, ccs_parameter, ccs_configuration_space, ccs_configuration, ccs_evaluation, ccs_rng, ccs_distribution, ccs_expression, ccs_datum, ccs_objective_space, ccs_bool
from .context import Context
from .parameter import Parameter
from .expression import Expression
from .expression_parser import ccs_parser
from .configuration_space import ConfigurationSpace
from .configuration import Configuration
from parglare.parser import Context as PContext

class ccs_objective_type(CEnumeration):
  _members_ = [
    ('MINIMIZE', 0),
    'MAXIMIZE' ]

ccs_create_objective_space = _ccs_get_function("ccs_create_objective_space", [ct.c_char_p, ct.POINTER(ccs_objective_space)])
ccs_objective_space_add_parameter = _ccs_get_function("ccs_objective_space_add_parameter", [ccs_objective_space, ccs_parameter])
ccs_objective_space_add_parameters = _ccs_get_function("ccs_objective_space_add_parameters", [ccs_objective_space, ct.c_size_t, ct.POINTER(ccs_parameter)])
ccs_objective_space_add_objective = _ccs_get_function("ccs_objective_space_add_objective", [ccs_objective_space, ccs_expression, ccs_objective_type])
ccs_objective_space_add_objectives = _ccs_get_function("ccs_objective_space_add_objectives", [ccs_objective_space, ct.c_size_t, ct.POINTER(ccs_expression), ct.POINTER(ccs_objective_type)])
ccs_objective_space_get_objective = _ccs_get_function("ccs_objective_space_get_objective", [ccs_objective_space, ct.c_size_t, ct.POINTER(ccs_expression), ct.POINTER(ccs_objective_type)])
ccs_objective_space_get_objectives = _ccs_get_function("ccs_objective_space_get_objectives", [ccs_objective_space, ct.c_size_t, ct.POINTER(ccs_expression), ct.POINTER(ccs_objective_type), ct.POINTER(ct.c_size_t)])
ccs_objective_space_check_evaluation_values = _ccs_get_function("ccs_objective_space_check_evaluation_values", [ccs_objective_space, ct.c_size_t, ct.POINTER(ccs_datum), ct.POINTER(ccs_bool)])

class ObjectiveSpace(Context):
  def __init__(self, handle = None, retain = False, auto_release = True,
               name = ""):
    if handle is None:
      handle = ccs_objective_space()
      res = ccs_create_objective_space(str.encode(name), ct.byref(handle))
      Error.check(res)
      super().__init__(handle = handle, retain = False)
    else:
      super().__init__(handle = handle, retain = retain, auto_release = auto_release)

  @classmethod
  def from_handle(cls, handle, retain = True, auto_release = True):
    return cls(handle = handle, retain = retain, auto_release = auto_release)

  def add_parameter(self, parameter):
    res = ccs_objective_space_add_parameter(self.handle, parameter.handle)
    Error.check(res)

  def add_parameters(self, parameters):
    count = len(parameters)
    if count == 0:
      return None
    parameters = (ccs_parameter * count)(*[x.handle.value for x in parameters])
    res = ccs_objective_space_add_parameters(self.handle, count, parameters)
    Error.check(res)

  def add_objective(self, expression, t = ccs_objective_type.MINIMIZE):
    if isinstance(expression, str):
      expression = ccs_parser.parse(expression, context = PContext(extra=self))
    res = ccs_objective_space_add_objective(self.handle, expression.handle, t)
    Error.check(res)

  def add_objectives(self, expressions, types = None):
    if isinstance(expressions, dict):
      types = expressions.values()
      expressions = expressions.keys()
    expressions = [ ccs_parser.parse(expression, context = PContext(extra=self)) if isinstance(expression, str) else expression for expression in expressions ]
    sz = len(expressions)
    if sz == 0:
      return None
    if types:
      if len(types) != sz:
        raise Error(ccs_result(ccs_result.INVALID_VALUE))
      types = (ccs_objective_type * sz)(*types)
    else:
      types = (ccs_objective_type * sz)(*([ccs_objective_type.MINIMIZE] * sz))
    v = (ccs_expression * sz)(*[x.handle.value for x in expressions])
    res = ccs_objective_space_add_objectives(self.handle, sz, v, types)
    Error.check(res)

  def get_objective(self, index):
    v = ccs_expression()
    t = ccs_objective_type()
    res = ccs_objective_space_get_objective(self.handle, index, ct.byref(v), ct.byref(t))
    Error.check(res)
    return (Expression.from_handle(v), t.value)

  @property
  def num_objective(self):
    v = ct.c_size_t()
    res = ccs_objective_space_get_objectives(self.handle, 0, None, None, ct.byref(v))
    Error.check(res)
    return v.value

  @property
  def objectives(self):
    sz = self.num_objective
    v = (ccs_expression * sz)()
    t = (ccs_objective_type * sz)()
    res = ccs_objective_space_get_objectives(self.handle, sz, v, t, None)
    Error.check(res)
    return [(Expression.from_handle(ccs_expression(v[x])), t[x].value) for x in range(sz)]

  def check_values(self, values):
    count = len(values)
    if count != self.num_parameters:
      raise Error(ccs_result(ccs_result.INVALID_VALUE))
    v = (ccs_datum * count)()
    ss = []
    for i in range(count):
      v[i].set_value(values[i], string_store = ss)
    valid = ccs_bool()
    res = ccs_objective_space_check_evaluation_values(self.handle, count, v, ct.byref(valid))
    Error.check(res)
    return False if valid.value == 0 else True

