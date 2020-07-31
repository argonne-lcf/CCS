import ctypes as ct
from .base import Object, Error, CEnumeration, ccs_error, _ccs_get_function, ccs_context, ccs_hyperparameter, ccs_configuration_space, ccs_configuration, ccs_rng, ccs_distribution, ccs_expression, ccs_datum, ccs_objective_space
from .context import Context
from .hyperparameter import Hyperparameter
from .expression import Expression
from .configuration_space import ConfigurationSpace
from .configuration import Configuration

class ccs_objective_type(CEnumeration):
  _members_ = [
    ('MINIMIZE', 0),
    'MAXINIZE' ]

ccs_create_objective_space = _ccs_get_function("ccs_create_objective_space", [ct.c_char_p, ct.c_void_p, ct.POINTER(ccs_objective_space)])
ccs_objective_space_get_name = _ccs_get_function("ccs_objective_space_get_name", [ccs_objective_space, ct.POINTER(ct.c_char_p)])
ccs_objective_space_get_user_data = _ccs_get_function("ccs_objective_space_get_user_data", [ccs_objective_space, ct.POINTER(ct.c_void_p)])
ccs_objective_space_add_hyperparameter = _ccs_get_function("ccs_objective_space_add_hyperparameter", [ccs_objective_space, ccs_hyperparameter])
ccs_objective_space_add_hyperparameters = _ccs_get_function("ccs_objective_space_add_hyperparameters", [ccs_objective_space, ct.c_size_t, ct.POINTER(ccs_hyperparameter)])
ccs_objective_space_get_num_hyperparameters = _ccs_get_function("ccs_objective_space_get_num_hyperparameters", [ccs_objective_space, ct.POINTER(ct.c_size_t,)])
ccs_objective_space_get_hyperparameter = _ccs_get_function("ccs_objective_space_get_hyperparameter", [ccs_objective_space, ct.c_size_t, ct.POINTER(ccs_hyperparameter)])
ccs_objective_space_get_hyperparameter_by_name = _ccs_get_function("ccs_objective_space_get_hyperparameter_by_name", [ccs_objective_space, ct.c_char_p, ct.POINTER(ccs_hyperparameter)])
ccs_objective_space_get_hyperparameter_index_by_name = _ccs_get_function("ccs_objective_space_get_hyperparameter_index_by_name", [ccs_objective_space, ct.c_char_p, ct.POINTER(ct.c_size_t)])
ccs_objective_space_get_hyperparameter_index = _ccs_get_function("ccs_objective_space_get_hyperparameter_index", [ccs_objective_space, ccs_hyperparameter, ct.POINTER(ct.c_size_t)])
ccs_objective_space_get_hyperparameters = _ccs_get_function("ccs_objective_space_get_hyperparameters", [ccs_objective_space, ct.c_size_t, ct.POINTER(ccs_hyperparameter), ct.POINTER(ct.c_size_t)])
ccs_objective_space_add_objective = _ccs_get_function("ccs_objective_space_add_objective", [ccs_objective_space, ccs_expression, ccs_objective_type])
ccs_objective_space_add_objectives = _ccs_get_function("ccs_objective_space_add_objectives", [ccs_objective_space, ct.c_size_t, ct.POINTER(ccs_expression), ct.POINTER(ccs_objective_type)])
ccs_objective_space_get_objective = _ccs_get_function("ccs_objective_space_get_objective", [ccs_objective_space, ct.c_size_t, ct.POINTER(ccs_expression), ct.POINTER(ccs_objective_type)])
ccs_objective_space_get_objectives = _ccs_get_function("ccs_objective_space_get_objectives", [ccs_objective_space, ct.c_size_t, ct.POINTER(ccs_expression), ct.POINTER(ccs_objective_type), ct.POINTER(ct.c_size_t)])

class ObjectiveSpace(Context):
  def __init__(self, handle = None, retain = False, name = "", user_data = None):
    if handle is None:
      handle = ccs_objective_space()
      res = ccs_create_objective_space(str.encode(name), user_data, ct.byref(handle))
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
    res = ccs_objective_space_get_user_data(self.handle, ct.byref(v))
    Error.check(res)
    self._user_data = v
    return v

  @property
  def name(self):
    if hasattr(self, "_name"):
      return self._name
    v = ct.c_char_p()
    res = ccs_objective_space_get_name(self.handle, ct.byref(v))
    Error.check(res)
    self._name = v.value.decode()
    return self._name

  def add_hyperparameter(self, hyperparameter):
    res = ccs_objective_space_add_hyperparameter(self.handle, hyperparameter.handle)
    Error.check(res)

  def add_hyperparameters(self, hyperparameters):
    count = len(hyperparameters)
    if count == 0:
      return None
    hypers = (ccs_hyperparameter * count)(*[x.handle.value for x in hyperparameters])
    res = ccs_objective_space_add_hyperparameters(self.handle, count, hypers)
    Error.check(res)

  def hyperparameter(self, index):
    v = ccs_hyperparameter()
    res = ccs_objective_space_get_hyperparameter(self.handle, index, ct.byref(v))
    Error.check(res)
    return Hyperparameter.from_handle(v)

  def hyperparameter_by_name(self, name):
    v = ccs_hyperparameter()
    res = ccs_objective_space_get_hyperparameter_by_name(self.handle, str.encode(name), ct.byref(v))
    Error.check(res)
    return Hyperparameter.from_handle(v)

  def hyperparameter_index(self, hyperparameter):
    v = ct.c_size_t()
    res = ccs_objective_space_get_hyperparameter_index(self.handle, hyperparameter.handle, ct.byref(v))
    Error.check(res)
    return v.value

  def hyperparameter_index_by_name(self, name):
    v = ct.c_size_t()
    res = ccs_objective_space_get_hyperparameter_index_by_name(self.handle, str.encode(name), ct.byref(v))
    Error.check(res)
    return v.value

  @property
  def num_hyperparameters(self):
    v = ct.c_size_t(0)
    res = ccs_objective_space_get_num_hyperparameters(self.handle, ct.byref(v))
    Error.check(res)
    return v.value

  @property
  def hyperparameters(self):
    count = self.num_hyperparameters
    if count == 0:
      return []
    v = (ccs_hyperparameter * count)()
    res = ccs_objective_space_get_hyperparameters(self.handle, count, v, None)
    Error.check(res)
    return [Hyperparameter.from_handle(ccs_hyperparameter(x)) for x in v]

  def add_objective(self, expression, t = ccs_objective_type.MINIMIZE):
    res = ccs_objective_space_add_objective(self.handle, expression.handle, t)
    Error.check(res)

  def add_objectives(self, expressions, types = None):
    sz = len(expressions)
    if sz == 0:
      return None
    if types:
      if len(types) != sz:
        raise Error(ccs_error(ccs_error.INVALID_VALUE))
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
    return [(Expression.from_handle(ccs_expression(v[x])), t[x]) for x in range(sz)]
