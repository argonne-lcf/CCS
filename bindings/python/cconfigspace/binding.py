import ctypes as ct
from .base import Object, Error, Result, _ccs_get_function, ccs_context, ccs_binding, ccs_parameter, Datum, DatumFix, ccs_hash, ccs_int
from .parameter import Parameter

ccs_binding_get_context = _ccs_get_function("ccs_binding_get_context", [ccs_binding, ct.POINTER(ccs_context)])
ccs_binding_get_value = _ccs_get_function("ccs_binding_get_value", [ccs_binding, ct.c_size_t, ct.POINTER(Datum)])
ccs_binding_get_values = _ccs_get_function("ccs_binding_get_values", [ccs_binding, ct.c_size_t, ct.POINTER(Datum), ct.POINTER(ct.c_size_t)])
ccs_binding_get_value_by_name = _ccs_get_function("ccs_binding_get_value_by_name", [ccs_binding, ct.c_char_p, ct.POINTER(Datum)])
ccs_binding_get_value_by_parameter = _ccs_get_function("ccs_binding_get_value_by_parameter", [ccs_binding, ccs_parameter, ct.POINTER(Datum)])
ccs_binding_hash = _ccs_get_function("ccs_binding_hash", [ccs_binding, ct.POINTER(ccs_hash)])
ccs_binding_cmp = _ccs_get_function("ccs_binding_cmp", [ccs_binding, ccs_binding, ct.POINTER(ct.c_int)])

class Binding(Object):

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

  def value(self, parameter):
    v = Datum()
    if isinstance(parameter, Parameter):
      res = ccs_binding_get_value_by_parameter(self.handle, parameter.handle, ct.byref(v))
    elif isinstance(parameter, str):
      res = ccs_binding_get_value_by_name(self.handle, str.encode(parameter), ct.byref(v))
    else:
      res = ccs_binding_get_value(self.handle, parameter, ct.byref(v))
    Error.check(res)
    return v.value

  @property
  def values(self):
    if hasattr(self, "_values"):
      return self._values
    sz = self.num_values
    v = (Datum * sz)()
    res = ccs_binding_get_values(self.handle, sz, v, None)
    Error.check(res)
    self._values = tuple(x.value for x in v)
    return self._values

  def cmp(self, other):
    v = ct.c_int()
    res = ccs_binding_cmp(self.handle, other.handle, ct.byref(v))
    Error.check(res)
    return v.value

  def __lt__(self, other):
    v = ct.c_int()
    res = ccs_binding_cmp(self.handle, other.handle, ct.byref(v))
    Error.check(res)
    return v.value < 0

  def __le__(self, other):
    v = ct.c_int()
    res = ccs_binding_cmp(self.handle, other.handle, ct.byref(v))
    Error.check(res)
    return v.value <= 0

  def __gt__(self, other):
    v = ct.c_int()
    res = ccs_binding_cmp(self.handle, other.handle, ct.byref(v))
    Error.check(res)
    return v.value > 0

  def __ge__(self, other):
    v = ct.c_int()
    res = ccs_binding_cmp(self.handle, other.handle, ct.byref(v))
    Error.check(res)
    return v.value >= 0

  def __eq__(self, other):
    v = ct.c_int()
    res = ccs_binding_cmp(self.handle, other.handle, ct.byref(v))
    Error.check(res)
    return v.value == 0

  def __ne__(self, other):
    v = ct.c_int()
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
    res = {}
    parameters = self.context.parameters
    values = self.values
    for i in range(len(parameters)):
      res[parameters[i].name] = values[i]
    return res
