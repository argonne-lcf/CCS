import ctypes as ct
from . import libcconfigspace
from .base import Object, CCSError

ccs_rng_create = libcconfigspace.ccs_rng_create
ccs_rng_create.restype = ct.c_int
ccs_rng_create.argtypes = [ct.c_void_p]

ccs_rng_set_seed = libcconfigspace.ccs_rng_set_seed
ccs_rng_set_seed.restype = ct.c_int
ccs_rng_set_seed.argtypes = [ct.c_void_p, ct.c_ulong]

ccs_rng_get = libcconfigspace.ccs_rng_get
ccs_rng_get.restype = ct.c_int
ccs_rng_get.argtypes = [ct.c_void_p, ct.c_void_p]
#attach_function :ccs_rng_get, [:ccs_rng_t, :pointer], :ccs_result_t
#attach_function :ccs_rng_min, [:ccs_rng_t, :pointer], :ccs_result_t
#attach_function :ccs_rng_max, [:ccs_rng_t, :pointer], :ccs_result_t

class Rng(Object):
  def __init__(self, handle = None, retain = False):
    if handle is None:
      handle = ct.c_void_p(0)
      res = ccs_rng_create(ct.byref(handle))
      CCSError.check(res)
      super().__init__(handle = handle, retain = False)
    else:
      super().__init__(handle = handle, retain = retain)

  @classmethod
  def from_handle(cls, handle):
    cls(handle, retain = True)

  def set_seed(self, value):
    res = ccs_rng_set_seed(self.handle, value)
    CCSError.check(res)
    return self


