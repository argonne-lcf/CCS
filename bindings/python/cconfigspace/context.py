import ctypes as ct
from .base import Object, Error, ccs_error, _ccs_get_function, ccs_context, ccs_hyperparameter, ccs_datum, ccs_datum_fix
from .hyperparameter import Hyperparameter

ccs_context_get_name = _ccs_get_function("ccs_context_get_name", [ccs_context, ct.POINTER(ct.c_char_p)])
ccs_context_get_num_hyperparameters = _ccs_get_function("ccs_context_get_num_hyperparameters", [ccs_context, ct.POINTER(ct.c_size_t)])
ccs_context_get_hyperparameter = _ccs_get_function("ccs_context_get_hyperparameter", [ccs_context, ct.c_size_t, ct.POINTER(ccs_hyperparameter)])
ccs_context_get_hyperparameter_by_name = _ccs_get_function("ccs_context_get_hyperparameter_by_name", [ccs_context, ct.c_char_p, ct.POINTER(ccs_hyperparameter)])
ccs_context_get_hyperparameter_index_by_name = _ccs_get_function("ccs_context_get_hyperparameter_index_by_name", [ccs_context, ct.c_char_p, ct.POINTER(ct.c_size_t)])
ccs_context_get_hyperparameter_index = _ccs_get_function("ccs_context_get_hyperparameter_index", [ccs_context, ccs_hyperparameter, ct.POINTER(ct.c_size_t)])
ccs_context_get_hyperparameter_indexes = _ccs_get_function("ccs_context_get_hyperparameter_indexes", [ccs_context, ct.c_size_t, ct.POINTER(ccs_hyperparameter), ct.POINTER(ct.c_size_t)])
ccs_context_get_hyperparameters = _ccs_get_function("ccs_context_get_hyperparameters", [ccs_context, ct.c_size_t, ct.POINTER(ccs_hyperparameter), ct.POINTER(ct.c_size_t)])
ccs_context_validate_value = _ccs_get_function("ccs_context_validate_value", [ccs_context, ct.c_size_t, ccs_datum_fix, ct.POINTER(ccs_datum)])

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

  def hyperparameter(self, index):
    v = ccs_hyperparameter()
    res = ccs_context_get_hyperparameter(self.handle, index, ct.byref(v))
    Error.check(res)
    return Hyperparameter.from_handle(v)

  def hyperparameter_by_name(self, name):
    v = ccs_hyperparameter()
    res = ccs_context_get_hyperparameter_by_name(self.handle, str.encode(name), ct.byref(v))
    Error.check(res)
    return Hyperparameter.from_handle(v)

  def hyperparameter_index(self, hyperparameter):
    v = ct.c_size_t()
    res = ccs_context_get_hyperparameter_index(self.handle, hyperparameter.handle, ct.byref(v))
    Error.check(res)
    return v.value

  def hyperparameter_index_by_name(self, name):
    v = ct.c_size_t()
    res = ccs_context_get_hyperparameter_index_by_name(self.handle, str.encode(name), ct.byref(v))
    Error.check(res)
    return v.value

  @property
  def num_hyperparameters(self):
    v = ct.c_size_t(0)
    res = ccs_context_get_num_hyperparameters(self.handle, ct.byref(v))
    Error.check(res)
    return v.value

  @property
  def hyperparameters(self):
    count = self.num_hyperparameters
    if count == 0:
      return []
    v = (ccs_hyperparameter * count)()
    res = ccs_context_get_hyperparameters(self.handle, count, v, None)
    Error.check(res)
    return [Hyperparameter.from_handle(ccs_hyperparameter(x)) for x in v]

  def validate_value(self, hyperparameter, value):
    if isinstance(hyperparameter, Hyperparameter):
      hyperparameter = hyperparameter_index(hyperparameter)
    elif isinstance(hyperparameter, str):
      hyperparameter = hyperparameter_index_by_name(hyperparameter)
    pv = ccs_datum(value)
    v = ccs_datum_fix(pv)
    vo = ccs_datum()
    res = ccs_context_validate_value(self.handle, hyperparameter, v, ct.byref(vo))
    Error.check(res)
    return vo.value
