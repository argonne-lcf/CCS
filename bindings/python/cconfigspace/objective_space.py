import ctypes as ct
from .base import Object, Error, CEnumeration, Result, _ccs_get_function, ccs_context, ccs_parameter, ccs_configuration_space, ccs_configuration, ccs_evaluation, ccs_rng, ccs_distribution, ccs_expression, Datum, ccs_objective_space, ccs_bool
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

ccs_create_objective_space = _ccs_get_function("ccs_create_objective_space", [ct.c_char_p, ct.c_size_t, ct.POINTER(ccs_parameter), ct.c_size_t, ct.POINTER(ccs_expression), ct.POINTER(ObjectiveType), ct.c_size_t, ct.POINTER(ccs_context), ct.POINTER(ccs_objective_space)])
ccs_objective_space_get_objective = _ccs_get_function("ccs_objective_space_get_objective", [ccs_objective_space, ct.c_size_t, ct.POINTER(ccs_expression), ct.POINTER(ObjectiveType)])
ccs_objective_space_get_objectives = _ccs_get_function("ccs_objective_space_get_objectives", [ccs_objective_space, ct.c_size_t, ct.POINTER(ccs_expression), ct.POINTER(ObjectiveType), ct.POINTER(ct.c_size_t)])
ccs_objective_space_check_evaluation_values = _ccs_get_function("ccs_objective_space_check_evaluation_values", [ccs_objective_space, ct.c_size_t, ct.POINTER(Datum), ct.POINTER(ccs_bool)])
ccs_objective_space_get_contexts = _ccs_get_function("ccs_objective_space_get_contexts", [ccs_objective_space, ct.c_size_t, ct.POINTER(ccs_context), ct.POINTER(ct.c_size_t)])

class ObjectiveSpace(Context):
  def __init__(self, handle = None, retain = False, auto_release = True,
               name = "", parameters = [], objectives = [], types = None, contexts = None):
    if handle is None:
      count = len(parameters)
      ctx_params = parameters
      if contexts is not None:
        for x in contexts:
          ctx_params = ctx_params + x.parameters

      ctx = dict(zip([x.name for x in ctx_params], ctx_params))
      if isinstance(objectives, dict):
        types = objectives.values()
        objectives = objectives.keys()
      objectives = [ parser.parse(objective, extra = ctx) if isinstance(objective, str) else objective for objective in objectives ]
      sz = len(objectives)
      if types is not None:
        if len(types) != sz:
          raise Error(Result(Result.ERROR_INVALID_VALUE))
        types = (ObjectiveType * sz)(*types)
      else:
        types = (ObjectiveType * sz)(*([ObjectiveType.MINIMIZE] * sz))
      sz_contexts = 0
      if contexts is not None:
        sz_contexts = len(contexts)
        contexts = (ccs_context * sz_contexts)(*[x.handle.value for x in contexts])
      parameters = (ccs_parameter * count)(*[x.handle.value for x in parameters])
      objectives = (ccs_expression * sz)(*[x.handle.value for x in objectives])
      handle = ccs_objective_space()
      res = ccs_create_objective_space(str.encode(name), count, parameters, sz, objectives, types, sz_contexts, contexts, ct.byref(handle))
      Error.check(res)
      super().__init__(handle = handle, retain = False)
    else:
      super().__init__(handle = handle, retain = retain, auto_release = auto_release)

  @classmethod
  def from_handle(cls, handle, retain = True, auto_release = True):
    return cls(handle = handle, retain = retain, auto_release = auto_release)

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

  @property
  def num_contexts(self):
    if hasattr(self, "_num_contexts"):
      return self._num_contexts
    v = ct.c_size_t()
    res = ccs_objective_space_get_contexts(self.handle, 0, None, ct.byref(v))
    Error.check(res)
    self._num_contexts = v.value
    return self._num_contexts

  @property
  def contexts(self):
    if hasattr(self, "_contexts"):
      return self._contexts
    sz = self.num_contexts
    v = (ccs_context * sz)()
    res = ccs_objective_space_get_contexts(self.handle, sz, v, None)
    Error.check(res)
    self._contexts = tuple(Context.from_handle(ccs_context(x)) for x in v)
    return self._contexts
