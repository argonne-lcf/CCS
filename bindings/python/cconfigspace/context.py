import ctypes as ct
from .base import Object, Error, ccs_error, _ccs_get_function, ccs_context, ccs_hyperparameter

ccs_context_get_hyperparameter_index = _ccs_get_function("ccs_context_get_hyperparameter_index", [ccs_context, ccs_hyperparameter, ct.POINTER(ct.c_size_t)])

class Context(Object):
  def hyperparameter_index(self, hyperparameter):
    v = ct.c_sizeof_t()
    res = ccs_context_get_hyperparameter_index(self.handle, hyperparameter.handle, ct.byref(v))
    Error.check(res)
    return v.value

