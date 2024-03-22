import ctypes as ct
from .base import Error, CEnumeration, Result, ccs_evaluation_result, _ccs_get_function, Datum, DatumFix, ccs_evaluation_binding, ccs_objective_space, ccs_bool
from .binding import Binding
from .objective_space import ObjectiveSpace

class Comparison(CEnumeration):
  _members_ = [
    ('BETTER', -1),
    ('EQUIVALENT', 0),
    ('WORSE', 1),
    ('NOT_COMPARABLE', 2) ]

ccs_evaluation_binding_get_objective_space = _ccs_get_function("ccs_evaluation_binding_get_objective_space", [ccs_evaluation_binding, ct.POINTER(ccs_objective_space)])
ccs_evaluation_binding_get_result = _ccs_get_function("ccs_evaluation_binding_get_result", [ccs_evaluation_binding, ct.POINTER(ccs_evaluation_result)])
ccs_evaluation_binding_get_objective_values = _ccs_get_function("ccs_evaluation_binding_get_objective_values", [ccs_evaluation_binding, ct.c_size_t, ct.POINTER(Datum), ct.POINTER(ct.c_size_t)])
ccs_evaluation_binding_compare = _ccs_get_function("ccs_evaluation_binding_compare", [ccs_evaluation_binding, ccs_evaluation_binding, ct.POINTER(Comparison)])
ccs_evaluation_binding_check = _ccs_get_function("ccs_evaluation_binding_check", [ccs_evaluation_binding, ct.POINTER(ccs_bool)])

class EvaluationBinding(Binding):

  @property
  def objective_space(self):
    if hasattr(self, "_objective_space"):
      return self._objective_space
    v = ccs_objective_space()
    res = ccs_evaluation_binding_get_objective_space(self.handle, ct.byref(v))
    Error.check(res)
    self._objective_space = ObjectiveSpace.from_handle(v)
    return self._objective_space

  @property
  def result(self):
    if hasattr(self, "_result"):
      return self._result
    v = ccs_evaluation_result()
    res = ccs_evaluation_binding_get_result(self.handle, ct.byref(v))
    Error.check(res)
    self._result = v.value
    return self._result

  @property
  def num_objective_values(self):
    if hasattr(self, "_num_objective_values"):
      return self._num_objective_values
    v = ct.c_size_t()
    res = ccs_evaluation_binding_get_objective_values(self.handle, 0, None, ct.byref(v))
    Error.check(res)
    self._num_objective_values = v.value
    return self._num_objective_values

  @property
  def objective_values(self):
    if hasattr(self, "_objective_values"):
      return self._objective_values
    sz = self.num_objective_values
    v = (Datum * sz)()
    res = ccs_evaluation_binding_get_objective_values(self.handle, sz, v, None)
    Error.check(res)
    self._objective_values = tuple(x.value for x in v)
    return self._objective_values

  def compare(self, other):
    v = Comparison(0)
    res = ccs_evaluation_binding_compare(self.handle, other.handle, ct.byref(v))
    Error.check(res)
    return v.value

  def check(self):
    valid = ccs_bool()
    res = ccs_evaluation_binding_check(self.handle, ct.byref(valid))
    Error.check(res)
    return False if valid.value == 0 else True
