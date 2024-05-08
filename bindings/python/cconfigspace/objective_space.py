import ctypes as ct
from .base import Object, Error, CEnumeration, Result, _ccs_get_function, ccs_context, ccs_parameter, ccs_configuration_space, ccs_configuration, ccs_evaluation, ccs_rng, ccs_distribution, ccs_expression, Datum, ccs_objective_space, ccs_bool, ccs_search_space
from .context import Context
from .parameter import Parameter
from .expression import Expression
from .expression_parser import parser
from .configuration_space import ConfigurationSpace
from .configuration import Configuration

class ObjectiveType(CEnumeration):
  _members_ = [
    ('MINIMIZE', 0),
    'MAXIMIZE' ]

ccs_create_objective_space = _ccs_get_function("ccs_create_objective_space", [ct.c_char_p, ccs_search_space, ct.c_size_t, ct.POINTER(ccs_parameter), ct.c_size_t, ct.POINTER(ccs_expression), ct.POINTER(ObjectiveType), ct.POINTER(ccs_objective_space)])
ccs_objective_space_get_search_space = _ccs_get_function("ccs_objective_space_get_search_space", [ccs_objective_space, ct.POINTER(ccs_search_space)])
ccs_objective_space_get_objective = _ccs_get_function("ccs_objective_space_get_objective", [ccs_objective_space, ct.c_size_t, ct.POINTER(ccs_expression), ct.POINTER(ObjectiveType)])
ccs_objective_space_get_objectives = _ccs_get_function("ccs_objective_space_get_objectives", [ccs_objective_space, ct.c_size_t, ct.POINTER(ccs_expression), ct.POINTER(ObjectiveType), ct.POINTER(ct.c_size_t)])
ccs_objective_space_check_evaluation = _ccs_get_function("ccs_objective_space_check_evaluation", [ccs_objective_space, ccs_evaluation, ct.POINTER(ccs_bool)])

class ObjectiveSpace(Context):
  def __init__(self, handle = None, retain = False, auto_release = True,
               name = "", search_space = None, parameters = [], objectives = [], types = None):
    if handle is None:
      count = len(parameters)
      ctx = dict(zip([x.name for x in parameters], parameters))
      if isinstance(objectives, dict):
        types = objectives.values()
        objectives = objectives.keys()
      objectives = [ parser.parse(objective, extra = ctx) if isinstance(objective, str) else objective for objective in objectives ]
      sz = len(objectives)
      if types:
        if len(types) != sz:
          raise Error(Result(Result.ERROR_INVALID_VALUE))
        types = (ObjectiveType * sz)(*types)
      else:
        types = (ObjectiveType * sz)(*([ObjectiveType.MINIMIZE] * sz))
      parameters = (ccs_parameter * count)(*[x.handle.value for x in parameters])
      objectives = (ccs_expression * sz)(*[x.handle.value for x in objectives])
      handle = ccs_objective_space()
      res = ccs_create_objective_space(str.encode(name), search_space.handle, count, parameters, sz, objectives, types, ct.byref(handle))
      Error.check(res)
      super().__init__(handle = handle, retain = False)
    else:
      super().__init__(handle = handle, retain = retain, auto_release = auto_release)

  @classmethod
  def from_handle(cls, handle, retain = True, auto_release = True):
    return cls(handle = handle, retain = retain, auto_release = auto_release)

  @property
  def search_space(self):
    if hasattr(self, "_search_space"):
      return self._search_space
    s = ccs_search_space()
    res = ccs_objective_space_get_search_space(self.handle, ct.byref(s))
    Error.check(res)
    self._search_space = Object.from_handle(s)
    return self._search_space

  def get_objective(self, index):
    v = ccs_expression()
    t = ObjectiveType()
    res = ccs_objective_space_get_objective(self.handle, index, ct.byref(v), ct.byref(t))
    Error.check(res)
    return (Expression.from_handle(v), t.value)

  @property
  def num_objectives(self):
    if hasattr(self, "_num_objectives"):
      return self._num_objectives
    v = ct.c_size_t()
    res = ccs_objective_space_get_objectives(self.handle, 0, None, None, ct.byref(v))
    Error.check(res)
    self._num_objectives = v.value
    return self._num_objectives

  @property
  def objectives(self):
    if hasattr(self, "_objectives"):
      return self._objectives
    sz = self.num_objectives
    v = (ccs_expression * sz)()
    t = (ObjectiveType * sz)()
    res = ccs_objective_space_get_objectives(self.handle, sz, v, t, None)
    Error.check(res)
    self._objectives = tuple((Expression.from_handle(ccs_expression(v[x])), t[x].value) for x in range(sz))
    return self._objectives

  def check(self, evaluation):
    valid = ccs_bool()
    res = ccs_objective_space_check_evaluation(self.handle, evaluation.handle, ct.byref(valid))
    Error.check(res)
    return False if valid.value == 0 else True

