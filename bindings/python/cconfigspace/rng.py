import ctypes as ct
from . import libcconfigspace
from .base import Object, Error, ccs_float, ccs_result, ccs_rng

ccs_rng_create = libcconfigspace.ccs_rng_create
ccs_rng_create.restype = ccs_result
ccs_rng_create.argtypes = [ct.POINTER(ccs_rng)]

ccs_rng_set_seed = libcconfigspace.ccs_rng_set_seed
ccs_rng_set_seed.restype = ccs_result
ccs_rng_set_seed.argtypes = [ccs_rng, ct.c_ulong]

ccs_rng_get = libcconfigspace.ccs_rng_get
ccs_rng_get.restype = ccs_result
ccs_rng_get.argtypes = [ccs_rng, ct.POINTER(ct.c_ulong)]

ccs_rng_uniform = libcconfigspace.ccs_rng_uniform
ccs_rng_uniform.restype = ccs_result
ccs_rng_uniform.argtypes = [ccs_rng, ct.POINTER(ccs_float)]

ccs_rng_min = libcconfigspace.ccs_rng_min
ccs_rng_min.restype = ccs_result
ccs_rng_min.argtypes = [ccs_rng, ct.POINTER(ct.c_ulong)]

ccs_rng_max = libcconfigspace.ccs_rng_max
ccs_rng_max.restype = ccs_result
ccs_rng_max.argtypes = [ccs_rng, ct.POINTER(ct.c_ulong)]

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

