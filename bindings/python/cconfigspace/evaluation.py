import ctypes as ct
from .base import Object, Error, CEnumeration, ccs_result, ccs_evaluation_result, _ccs_get_function, ccs_context, ccs_parameter, ccs_configuration, ccs_datum, ccs_datum_fix, ccs_objective_space, ccs_evaluation, ccs_bool
from .binding import Binding
from .configuration import Configuration
from .objective_space import ObjectiveSpace

class ccs_comparison(CEnumeration):
  _members_ = [
    ('BETTER', -1),
    ('EQUIVALENT', 0),
    ('WORSE', 1),
    ('NOT_COMPARABLE', 2) ]

ccs_create_evaluation = _ccs_get_function("ccs_create_evaluation", [ccs_objective_space, ccs_configuration, ccs_evaluation_result, ct.c_size_t, ct.POINTER(ccs_datum), ct.POINTER(ccs_evaluation)])
ccs_evaluation_get_objective_space = _ccs_get_function("ccs_evaluation_get_objective_space", [ccs_evaluation, ct.POINTER(ccs_objective_space)])
ccs_evaluation_get_configuration = _ccs_get_function("ccs_evaluation_get_configuration", [ccs_evaluation, ct.POINTER(ccs_configuration)])
ccs_evaluation_get_result = _ccs_get_function("ccs_evaluation_get_result", [ccs_evaluation, ct.POINTER(ccs_evaluation_result)])
ccs_evaluation_set_result = _ccs_get_function("ccs_evaluation_set_result", [ccs_evaluation, ccs_evaluation_result])
ccs_evaluation_get_objective_value = _ccs_get_function("ccs_evaluation_get_objective_value", [ccs_evaluation, ct.c_size_t, ct.POINTER(ccs_datum)])
ccs_evaluation_get_objective_values = _ccs_get_function("ccs_evaluation_get_objective_values", [ccs_evaluation, ct.c_size_t, ct.POINTER(ccs_datum), ct.POINTER(ct.c_size_t)])
ccs_evaluation_compare = _ccs_get_function("ccs_evaluation_compare", [ccs_evaluation, ccs_evaluation, ct.POINTER(ccs_comparison)])
ccs_evaluation_check = _ccs_get_function("ccs_evaluation_check", [ccs_evaluation, ct.POINTER(ccs_bool)])

class Evaluation(Binding):
  def __init__(self, handle = None, retain = False, auto_release = True,
               objective_space = None, configuration = None, result = ccs_result.SUCCESS, values = None):
    if handle is None:
      count = 0
      if values:
        count = len(values)
        vals = (ccs_datum * count)()
        ss = []
        for i in range(count):
          vals[i].set_value(values[i], string_store = ss)
      else:
        vals = None
      handle = ccs_evaluation()
      res = ccs_create_evaluation(objective_space.handle, configuration.handle, result, count, vals, ct.byref(handle))
      Error.check(res)
      super().__init__(handle = handle, retain = False)
    else:
      super().__init__(handle = handle, retain = retain, auto_release = auto_release)

  @classmethod
  def from_handle(cls, handle, retain = True, auto_release = True):
    return cls(handle = handle, retain = retain, auto_release = auto_release)

  @property
  def objective_space(self):
    if hasattr(self, "_objective_space"):
      return self._objective_space
    v = ccs_objective_space()
    res = ccs_evaluation_get_objective_space(self.handle, ct.byref(v))
    Error.check(res)
    self._objective_space = ObjectiveSpace.from_handle(v)
    return self._objective_space

  @property
  def configuration(self):
    if hasattr(self, "_configuration"):
      return self._configuration
    v = ccs_configuration()
    res = ccs_evaluation_get_configuration(self.handle, ct.byref(v))
    Error.check(res)
    self._configuration = Configuration.from_handle(v)
    return self._configuration

  @property
  def result(self):
    v = ccs_evaluation_result()
    res = ccs_evaluation_get_result(self.handle, ct.byref(v))
    Error.check(res)
    return v.value

  @result.setter
  def result(self, v):
    res = ccs_evaluation_set_result(self.handle, v)
    Error.check(res)

  @property
  def num_objective_values(self):
    if hasattr(self, "_num_objective_values"):
      return self._num_objective_values
    v = ct.c_size_t()
    res = ccs_evaluation_get_objective_values(self.handle, 0, None, ct.byref(v))
    Error.check(res)
    self._num_objective_values = v.value
    return self._num_objective_values

  @property
  def objective_values(self):
    sz = self.num_objective_values
    if sz == 0:
      return []
    v = (ccs_datum * sz)()
    res = ccs_evaluation_get_objective_values(self.handle, sz, v, None)
    Error.check(res)
    return [x.value for x in v]

  def compare(self, other):
    v = ccs_comparison(0)
    res = ccs_evaluation_compare(self.handle, other.handle, ct.byref(v))
    Error.check(res)
    return v.value

  def check(self):
    valid = ccs_bool()
    res = ccs_evaluation_check(self.handle, ct.byref(valid))
    Error.check(res)
    return False if valid.value == 0 else True
