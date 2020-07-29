import ctypes as ct
from . import libcconfigspace
from .base import Object, Error, ccs_float, ccs_result, ccs_rng, _ccs_get_function

ccs_rng_create = _ccs_get_function("ccs_rng_create", [ct.POINTER(ccs_rng)])
ccs_rng_set_seed = _ccs_get_function("ccs_rng_set_seed", [ccs_rng, ct.c_ulong])
ccs_rng_get = _ccs_get_function("ccs_rng_get", [ccs_rng, ct.POINTER(ct.c_ulong)])
ccs_rng_uniform = _ccs_get_function("ccs_rng_uniform", [ccs_rng, ct.POINTER(ccs_float)])
ccs_rng_min = _ccs_get_function("ccs_rng_min", [ccs_rng, ct.POINTER(ct.c_ulong)])
ccs_rng_max = _ccs_get_function("ccs_rng_max", [ccs_rng, ct.POINTER(ct.c_ulong)])

class Rng(Object):
  def __init__(self, handle = None, retain = False):
    if handle is None:
      handle = ccs_rng(0)
      res = ccs_rng_create(ct.byref(handle))
      Error.check(res)
      super().__init__(handle = handle, retain = False)
    else:
      super().__init__(handle = handle, retain = retain)

  @classmethod
  def from_handle(cls, handle):
    return cls(handle, retain = True)

  def set_seed(self, value):
    res = ccs_rng_set_seed(self.handle, value)
    Error.check(res)
    return self

  def get(self):
    v = ct.c_ulong(0)
    res = ccs_rng_get(self.handle, ct.byref(v))
    Error.check(res)
    return v.value

  def uniform(self):
    v = ccs_float(0.0)
    res = ccs_rng_uniform(self.handle, ct.byref(v))
    Error.check(res)
    return v.value

  def min(self):
    v = ct.c_ulong(0)
    res = ccs_rng_min(self.handle, ct.byref(v))
    Error.check(res)
    return v.value

  def max(self):
    v = ct.c_ulong(0)
    res = ccs_rng_max(self.handle, ct.byref(v))
    Error.check(res)
    return v.value


