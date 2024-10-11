import ctypes as ct
from .base import Object, Error, CEnumeration, Result, _ccs_get_function, ccs_parameter, ccs_feature_space, ccs_features, ccs_bool
from .context import Context
from .parameter import Parameter

ccs_create_feature_space = _ccs_get_function("ccs_create_feature_space", [ct.c_char_p, ct.c_size_t, ct.POINTER(ccs_parameter), ct.POINTER(ccs_feature_space)])
ccs_feature_space_get_default_features = _ccs_get_function("ccs_feature_space_get_default_features", [ccs_feature_space, ct.POINTER(ccs_features)])

class FeatureSpace(Context):
  def __init__(self, handle = None, retain = False, auto_release = True,
               name = "", parameters = None):
    if handle is None:
      count = len(parameters)
      parameters = (ccs_parameter * count)(*[x.handle.value for x in parameters])
      handle = ccs_feature_space()
      res = ccs_create_feature_space(str.encode(name), count, parameters, ct.byref(handle))
      Error.check(res)
      super().__init__(handle = handle, retain = False)
    else:
      super().__init__(handle = handle, retain = retain, auto_release = auto_release)

  @classmethod
  def from_handle(cls, handle, retain = True, auto_release = True):
    return cls(handle = handle, retain = retain, auto_release = auto_release)

  @property
  def default_features(self):
    if hasattr(self, "_default_features"):
      return self._default_features
    v = ccs_features()
    res = ccs_feature_space_get_default_features(self.handle, ct.byref(v))
    Error.check(res)
    self._default_features = Features.from_handle(v)
    return self._default_features

from .features import Features
