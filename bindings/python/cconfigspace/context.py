import ctypes as ct
from .base import Object, Error, Result, _ccs_get_function, ccs_context, ccs_parameter, Datum, DatumFix, ccs_bool
from .parameter import Parameter

ccs_context_get_name = _ccs_get_function("ccs_context_get_name", [ccs_context, ct.POINTER(ct.c_char_p)])
ccs_context_get_parameter = _ccs_get_function("ccs_context_get_parameter", [ccs_context, ct.c_size_t, ct.POINTER(ccs_parameter)])
ccs_context_get_parameter_by_name = _ccs_get_function("ccs_context_get_parameter_by_name", [ccs_context, ct.c_char_p, ct.POINTER(ccs_parameter)])
ccs_context_get_parameter_index_by_name = _ccs_get_function("ccs_context_get_parameter_index_by_name", [ccs_context, ct.c_char_p, ct.POINTER(ccs_bool), ct.POINTER(ct.c_size_t)])
ccs_context_get_parameter_index = _ccs_get_function("ccs_context_get_parameter_index", [ccs_context, ccs_parameter, ct.POINTER(ccs_bool), ct.POINTER(ct.c_size_t)])
ccs_context_get_parameter_indexes = _ccs_get_function("ccs_context_get_parameter_indexes", [ccs_context, ct.c_size_t, ct.POINTER(ccs_parameter), ct.POINTER(ccs_bool), ct.POINTER(ct.c_size_t)])
ccs_context_get_parameters = _ccs_get_function("ccs_context_get_parameters", [ccs_context, ct.c_size_t, ct.POINTER(ccs_parameter), ct.POINTER(ct.c_size_t)])
ccs_context_validate_value = _ccs_get_function("ccs_context_validate_value", [ccs_context, ct.c_size_t, DatumFix, ct.POINTER(Datum)])

class Context(Object):

  @property
  def name(self):
    if hasattr(self, "_name"):
      return self._name
    v = ct.c_char_p()
    res = ccs_context_get_name(self.handle, ct.byref(v))
    Error.check(res)
    self._name = v.value.decode()
    return self._name

  def parameter(self, index):
    v = ccs_parameter()
    res = ccs_context_get_parameter(self.handle, index, ct.byref(v))
    Error.check(res)
    return Parameter.from_handle(v)

  def parameter_by_name(self, name):
    v = ccs_parameter()
    res = ccs_context_get_parameter_by_name(self.handle, str.encode(name), ct.byref(v))
    Error.check(res)
    if v.value is None:
      return None
    else:
      return Parameter.from_handle(v)

  def parameter_index(self, parameter):
    v = ct.c_size_t()
    res = ccs_context_get_parameter_index(self.handle, parameter.handle, None, ct.byref(v))
    Error.check(res)
    return v.value

  def parameter_index_by_name(self, name):
    v = ct.c_size_t()
    res = ccs_context_get_parameter_index_by_name(self.handle, str.encode(name), None, ct.byref(v))
    Error.check(res)
    return v.value

  @property
  def num_parameters(self):
    if hasattr(self, "_num_parameters"):
      return self._num_parameters
    v = ct.c_size_t(0)
    res = ccs_context_get_parameters(self.handle, 0, None, ct.byref(v))
    Error.check(res)
    self._num_parameters = v.value
    return self._num_parameters

  @property
  def parameters(self):
    if hasattr(self, "_parameters"):
      return self._parameters
    count = self.num_parameters
    v = (ccs_parameter * count)()
    res = ccs_context_get_parameters(self.handle, count, v, None)
    Error.check(res)
    self._parameters = tuple(Parameter.from_handle(ccs_parameter(x)) for x in v)
    return self._parameters

  def validate_value(self, parameter, value):
    if isinstance(parameter, Parameter):
      parameter = parameter_index(parameter)
    elif isinstance(parameter, str):
      parameter = parameter_index_by_name(parameter)
    pv = Datum(value)
    v = DatumFix(pv)
    vo = Datum()
    res = ccs_context_validate_value(self.handle, parameter, v, ct.byref(vo))
    Error.check(res)
    return vo.value
