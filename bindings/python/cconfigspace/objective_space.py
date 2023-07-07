import ctypes as ct
from .base import Object, Error, CEnumeration, Result, _ccs_get_function, ccs_context, ccs_parameter, ccs_configuration_space, ccs_configuration, ccs_evaluation, ccs_rng, ccs_distribution, ccs_expression, Datum, ccs_objective_space, ccs_bool
from .context import Context
from .parameter import Parameter
from .expression import Expression
from .expression_parser import parser
from .configuration_space import ConfigurationSpace
from .configuration import Configuration
from parglare.parser import Context as PContext

class ObjectiveType(CEnumeration):
  _members_ = [
    ('MINIMIZE', 0),
    'MAXIMIZE' ]

ccs_create_objective_space = _ccs_get_function("ccs_create_objective_space", [ct.c_char_p, ct.c_size_t, ct.POINTER(ccs_parameter), ct.POINTER(ccs_objective_space)])
ccs_objective_space_add_objective = _ccs_get_function("ccs_objective_space_add_objective", [ccs_objective_space, ccs_expression, ObjectiveType])
ccs_objective_space_add_objectives = _ccs_get_function("ccs_objective_space_add_objectives", [ccs_objective_space, ct.c_size_t, ct.POINTER(ccs_expression), ct.POINTER(ObjectiveType)])
ccs_objective_space_get_objective = _ccs_get_function("ccs_objective_space_get_objective", [ccs_objective_space, ct.c_size_t, ct.POINTER(ccs_expression), ct.POINTER(ObjectiveType)])
ccs_objective_space_get_objectives = _ccs_get_function("ccs_objective_space_get_objectives", [ccs_objective_space, ct.c_size_t, ct.POINTER(ccs_expression), ct.POINTER(ObjectiveType), ct.POINTER(ct.c_size_t)])
ccs_objective_space_check_evaluation_values = _ccs_get_function("ccs_objective_space_check_evaluation_values", [ccs_objective_space, ct.c_size_t, ct.POINTER(Datum), ct.POINTER(ccs_bool)])

class ObjectiveSpace(Context):
  def __init__(self, handle = None, retain = False, auto_release = True,
               name = "", parameters = None):
    if handle is None:
      count = len(parameters)
      if count == 0:
        raise Error(Result(Result.ERROR_INVALID_VALUE))
      parameters = (ccs_parameter * count)(*[x.handle.value for x in parameters])
      handle = ccs_objective_space()
      res = ccs_create_objective_space(str.encode(name), count, parameters, ct.byref(handle))
      Error.check(res)
      super().__init__(handle = handle, retain = False)
    else:
      super().__init__(handle = handle, retain = retain, auto_release = auto_release)

  @classmethod
  def from_handle(cls, handle, retain = True, auto_release = True):
    return cls(handle = handle, retain = retain, auto_release = auto_release)

  def add_objective(self, expression, t = ObjectiveType.MINIMIZE):
    if isinstance(expression, str):
      expression = parser.parse(expression, context = PContext(extra=self))
    res = ccs_objective_space_add_objective(self.handle, expression.handle, t)
    Error.check(res)

  def add_objectives(self, expressions, types = None):
    if isinstance(expressions, dict):
      types = expressions.values()
      expressions = expressions.keys()
    expressions = [ parser.parse(expression, context = PContext(extra=self)) if isinstance(expression, str) else expression for expression in expressions ]
    sz = len(expressions)
    if sz == 0:
      return None
    if types:
      if len(types) != sz:
        raise Error(Result(Result.ERROR_INVALID_VALUE))
      types = (ObjectiveType * sz)(*types)
    else:
      types = (ObjectiveType * sz)(*([ObjectiveType.MINIMIZE] * sz))
    v = (ccs_expression * sz)(*[x.handle.value for x in expressions])
    res = ccs_objective_space_add_objectives(self.handle, sz, v, types)
    Error.check(res)

  def get_objective(self, index):
    v = ccs_expression()
    t = ObjectiveType()
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
    t = (ObjectiveType * sz)()
    res = ccs_objective_space_get_objectives(self.handle, sz, v, t, None)
    Error.check(res)
    return [(Expression.from_handle(ccs_expression(v[x])), t[x].value) for x in range(sz)]

  def check_values(self, values):
    count = len(values)
    if count != self.num_parameters:
      raise Error(Result(Result.ERROR_INVALID_VALUE))
    v = (Datum * count)()
    ss = []
    for i in range(count):
      v[i].set_value(values[i], string_store = ss)
    valid = ccs_bool()
    res = ccs_objective_space_check_evaluation_values(self.handle, count, v, ct.byref(valid))
    Error.check(res)
    return False if valid.value == 0 else True

