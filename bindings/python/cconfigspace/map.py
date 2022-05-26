import ctypes as ct
from . import libcconfigspace
from .base import Object, Error, ccs_datum, ccs_datum_fix, ccs_result, ccs_map, _ccs_get_function, ccs_bool

ccs_create_map = _ccs_get_function("ccs_create_map", [ct.POINTER(ccs_map)])
ccs_map_set = _ccs_get_function("ccs_map_set", [ccs_map, ccs_datum_fix, ccs_datum_fix])
ccs_map_exist = _ccs_get_function("ccs_map_exist", [ccs_map, ccs_datum_fix, ct.POINTER(ccs_bool)])
ccs_map_get = _ccs_get_function("ccs_map_get", [ccs_map, ccs_datum_fix, ct.POINTER(ccs_datum)])
ccs_map_del = _ccs_get_function("ccs_map_del", [ccs_map, ccs_datum_fix])
ccs_map_get_keys = _ccs_get_function("ccs_map_get_keys", [ccs_map, ct.c_size_t, ct.POINTER(ccs_datum), ct.POINTER(ct.c_size_t)])
ccs_map_get_values = _ccs_get_function("ccs_map_get_values", [ccs_map, ct.c_size_t, ct.POINTER(ccs_datum), ct.POINTER(ct.c_size_t)])
ccs_map_get_pairs = _ccs_get_function("ccs_map_get_pairs", [ccs_map, ct.c_size_t, ct.POINTER(ccs_datum), ct.POINTER(ccs_datum), ct.POINTER(ct.c_size_t)])

class Map(Object):
  def __init__(self, handle = None, retain = False, auto_release = True):
    if handle is None:
      handle = ccs_map(0)
      res = ccs_create_map(ct.byref(handle))
      Error.check(res)
      super().__init__(handle = handle, retain = False)
    else:
      super().__init__(handle = handle, retain = retain, auto_release = auto_release)

  def __len__(self):
    v = ct.c_size_t()
    res = ccs_map_get_keys(self.handle, 0, None, ct.byref(v))
    return v.value

  def __getitem__(self, key):
    v = ccs_datum()
    pk = ccs_datum(key)
    k = ccs_datum_fix(pk)
    res = ccs_map_get(self.handle, k, ct.byref(v))
    Error.check(res)
    return v.value

  def __setitem__(self, key, val):
    pv = ccs_datum(val)
    v = ccs_datum_fix(pv)
    pk = ccs_datum(key)
    k = ccs_datum_fix(pk)
    res = ccs_map_set(self.handle, k, v)
    Error.check(res)

  def __delitem__(self, key):
    pk = ccs_datum(key)
    k = ccs_datum_fix(pk)
    res = ccs_map_del(self.handle, k)
    Error.check(res)

  def has_key(self, key):
    pk = ccs_datum(key)
    k = ccs_datum_fix(pk)
    b = ccs_bool()
    res = ccs_map_exist(self.handle, k, ct.byref(b))
    Error.check(res)
    return False if b._value.i == ccs_false else True

  def keys(self):
    sz = self.__len__()
    if sz == 0:
      return []
    v = (ccs_datum * sz)()
    res = ccs_map_get_keys(self.handle, sz, v, None)
    Error.check(res)
    return [x.value for x in v]

  def values(self):
    sz = self.__len__()
    if sz == 0:
      return []
    v = (ccs_datum * sz)()
    res = ccs_map_get_values(self.handle, sz, v, None)
    Error.check(res)
    return [x.value for x in v]
