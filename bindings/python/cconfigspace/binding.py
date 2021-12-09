import ctypes as ct
from .base import Object, Error, ccs_error, _ccs_get_function, ccs_context, ccs_binding, ccs_hyperparameter, ccs_datum, ccs_datum_fix, ccs_hash, ccs_int
from .hyperparameter import Hyperparameter

ccs_binding_get_context = _ccs_get_function("ccs_binding_get_context", [ccs_binding, ct.POINTER(ccs_context)])
ccs_binding_get_user_data = _ccs_get_function("ccs_binding_get_user_data", [ccs_binding, ct.POINTER(ct.c_void_p)])
ccs_binding_get_value = _ccs_get_function("ccs_binding_get_value", [ccs_binding, ct.c_size_t, ct.POINTER(ccs_datum)])
ccs_binding_set_value = _ccs_get_function("ccs_binding_set_value", [ccs_binding, ct.c_size_t, ccs_datum_fix])
ccs_binding_get_values = _ccs_get_function("ccs_binding_get_values", [ccs_binding, ct.c_size_t, ct.POINTER(ccs_datum), ct.POINTER(ct.c_size_t)])
ccs_binding_get_value_by_name = _ccs_get_function("ccs_binding_get_value_by_name", [ccs_binding, ct.c_char_p, ct.POINTER(ccs_datum)])
ccs_binding_hash = _ccs_get_function("ccs_binding_hash", [ccs_binding, ct.POINTER(ccs_hash)])
ccs_binding_cmp = _ccs_get_function("ccs_binding_cmp", [ccs_binding, ct.POINTER(ccs_int)])

class Binding(Object):

  @property
  def user_data(self):
    if hasattr(self, "_user_data"):
      return self._user_data
    v = ct.c_void_p()
    res = ccs_binding_get_user_data(self.handle, ct.byref(v))
    Error.check(res)
    self._user_data = v
    return v

  @property
  def context(self):
    if hasattr(self, "_context"):
      return self._context
    v = ccs_context()
    res = ccs_binding_get_context(self.handle, ct.byref(v))
    Error.check(res)
    self._context = Object.from_handle(v)
    return self._context

  @property
  def num_values(self):
    if hasattr(self, "_num_values"):
      return self._num_values
    v = ct.c_size_t()
    res = ccs_binding_get_values(self.handle, 0, None, ct.byref(v))
    Error.check(res)
    self._num_values = v.value
    return self._num_values

  def set_value(self, hyperparameter, value):
    if isinstance(hyperparameter, Hyperparameter):
      hyperparameter = self.context.hyperparameter_index(hyperparameter)
    elif isinstance(hyperparameter, str):
      hyperparameter = self.context.hyperparameter_index_by_name(hyperparameter)
    pv = ccs_datum(value)
    v = ccs_datum_fix()
    v.value = pv._value.i
    v.type = pv.type
    v.flags = pv.flags
    res = ccs_binding_set_value(self.handle, hyperparameter, v)
    Error.check(res)

  def value(self, hyperparameter):
    v = ccs_datum()
    if isinstance(hyperparameter, Hyperparameter):
      res = ccs_binding_get_value(self.handle, self.context.hyperparameter_index(hyperparameter), ct.byref(v))
    elif isinstance(hyperparameter, str):
      res = ccs_binding_get_value_by_name(self.handle, str.encode(hyperparameter), ct.byref(v))
    else:
      res = ccs_binding_get_value(self.handle, hyperparameter, ct.byref(v))
    Error.check(res)
    return v.value

  def __getitem__(self, idx):
    return self.values(idx)

  @property
  def values(self):
    """Returns a list copy of hyperparameter values of the current configuration."""
    sz = self.num_values
    if sz == 0:
      return []
    v = (ccs_datum * sz)()
    res = ccs_binding_get_values(self.handle, sz, v, None)
    Error.check(res)
    return [x.value for x in v]

  def cmp(self, other):
    v = ccs_int()
    res = ccs_binding_cmp(self.handle, other.handle, ct.byref(v))
    Error.check(res)
    return v.value

  def __lt__(self, other):
    v = ccs_int()
    res = ccs_binding_cmp(self.handle, other.handle, ct.byref(v))
    Error.check(res)
    return v.value < 0

  def __le__(self, other):
    v = ccs_int()
    res = ccs_binding_cmp(self.handle, other.handle, ct.byref(v))
    Error.check(res)
    return v.value <= 0

  def __gt__(self, other):
    v = ccs_int()
    res = ccs_binding_cmp(self.handle, other.handle, ct.byref(v))
    Error.check(res)
    return v.value > 0

  def __ge__(self, other):
    v = ccs_int()
    res = ccs_binding_cmp(self.handle, other.handle, ct.byref(v))
    Error.check(res)
    return v.value >= 0

  def __eq__(self, other):
    v = ccs_int()
    res = ccs_binding_cmp(self.handle, other.handle, ct.byref(v))
    Error.check(res)
    return v.value == 0

  def __ne__(self, other):
    v = ccs_int()
    res = ccs_binding_cmp(self.handle, other.handle, ct.byref(v))
    Error.check(res)
    return v.value != 0

  @property
  def hash(self):
    v = ccs_hash()
    res = ccs_binding_hash(self.handle, ct.byref(v))
    Error.check(res)
    return self.value

  def __hash__(self):
    return self.hash

  def asdict(self):
    """Returns a dict copy of the current configuration."""
    return {hp.name:hp_value for hp, hp_value in zip(self.context.hyperparameters, self.values)}
 