import ctypes as ct
from . import libcconfigspace
from .base import Object, Error, ccs_float, ccs_result, ccs_rng, _ccs_get_function

ccs_create_rng = _ccs_get_function("ccs_create_rng", [ct.POINTER(ccs_rng)])
ccs_rng_set_seed = _ccs_get_function("ccs_rng_set_seed", [ccs_rng, ct.c_ulong])
ccs_rng_get = _ccs_get_function("ccs_rng_get", [ccs_rng, ct.POINTER(ct.c_ulong)])
ccs_rng_uniform = _ccs_get_function("ccs_rng_uniform", [ccs_rng, ct.POINTER(ccs_float)])
ccs_rng_min = _ccs_get_function("ccs_rng_min", [ccs_rng, ct.POINTER(ct.c_ulong)])
ccs_rng_max = _ccs_get_function("ccs_rng_max", [ccs_rng, ct.POINTER(ct.c_ulong)])

class Rng(Object):
  def __init__(self, handle = None, retain = False, auto_release = True):
    if handle is None:
      handle = ccs_rng(0)
      res = ccs_create_rng(ct.byref(handle))
      Error.check(res)
      super().__init__(handle = handle, retain = False)
    else:
      super().__init__(handle = handle, retain = retain, auto_release = auto_release)

  @classmethod
  def from_handle(cls, handle, retain = True, auto_release = True):
    return cls(handle, retain = retain, auto_release = auto_release)

  def __setattr__(self, name, value):
    if name == 'seed':
      res = ccs_rng_set_seed(self.handle, value)
      Error.check(res)
      return None
    super().__setattr__(name, value)

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

  @property
  def min(self):
    v = ct.c_ulong(0)
    res = ccs_rng_min(self.handle, ct.byref(v))
    Error.check(res)
    return v.value

  @property
  def max(self):
    v = ct.c_ulong(0)
    res = ccs_rng_max(self.handle, ct.byref(v))
    Error.check(res)
    return v.value

ccs_default_rng = Rng()
