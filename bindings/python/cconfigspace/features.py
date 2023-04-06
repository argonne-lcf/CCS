import ctypes as ct
from .base import Object, Error, ccs_error, _ccs_get_function, ccs_context, ccs_parameter, ccs_features_space, ccs_features, ccs_datum, ccs_hash, ccs_int, ccs_bool
from .context import Context
from .parameter import Parameter
from .features_space import FeaturesSpace
from .binding import Binding

ccs_create_features = _ccs_get_function("ccs_create_features", [ccs_features_space, ct.c_size_t, ct.POINTER(ccs_datum), ct.POINTER(ccs_features)])
ccs_features_get_features_space = _ccs_get_function("ccs_features_get_features_space", [ccs_features, ct.POINTER(ccs_features_space)])
ccs_features_check = _ccs_get_function("ccs_features_check", [ccs_features, ct.POINTER(ccs_bool)])

class Features(Binding):
  def __init__(self, handle = None, retain = False, auto_release = True,
               features_space = None, values = None):
    if handle is None:
      count = 0
      if values:
        count = len(values)
        vals = (ccs_datum * count)()
        ss = []
        for i in range(count):
          vals[i].set_value(values[i], string_store = ss)
      else:
        vals = None
      handle = ccs_features()
      res = ccs_create_features(features_space.handle, count, vals, ct.byref(handle))
      Error.check(res)
      super().__init__(handle = handle, retain = False)
    else:
      super().__init__(handle = handle, retain = retain, auto_release = auto_release)

  @classmethod
  def from_handle(cls, handle, retain = True, auto_release = True):
    return cls(handle = handle, retain = retain, auto_release = auto_release)

  @property
  def features_space(self):
    if hasattr(self, "_features_space"):
      return self._features_space
    v = ccs_features_space()
    res = ccs_features_get_features_space(self.handle, ct.byref(v))
    Error.check(res)
    self._features_space = ConfigurationSpace.from_handle(v)
    return self._features_space

  def check(self):
    valid = ccs_bool()
    res = ccs_features_check(self.handle, ct.byref(valid))
    Error.check(res)
    return False if valid.value == 0 else True
