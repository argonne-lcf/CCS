import ctypes as ct
from .base import Object, Error, Result, _ccs_get_function, ccs_configuration_space, ccs_configuration, ccs_features, Datum, ccs_hash, ccs_int, ccs_bool
from .configuration_space import ConfigurationSpace
from .binding import Binding
from .features import Features

ccs_create_configuration = _ccs_get_function("ccs_create_configuration", [ccs_configuration_space, ccs_features, ct.c_size_t, ct.POINTER(Datum), ct.POINTER(ccs_configuration)])
ccs_configuration_get_configuration_space = _ccs_get_function("ccs_configuration_get_configuration_space", [ccs_configuration, ct.POINTER(ccs_configuration_space)])
ccs_configuration_get_features = _ccs_get_function("ccs_configuration_get_features", [ccs_configuration, ct.POINTER(ccs_features)])
ccs_configuration_check = _ccs_get_function("ccs_configuration_check", [ccs_configuration, ct.POINTER(ccs_bool)])

class Configuration(Binding):
  def __init__(self, handle = None, retain = False, auto_release = True,
               configuration_space = None, features = None, values = None):
    if handle is None:
      count = 0
      if values:
        count = len(values)
        vals = (Datum * count)()
        ss = []
        for i in range(count):
          vals[i].set_value(values[i], string_store = ss)
      else:
        vals = None
      if features is not None:
        features = features.handle
      handle = ccs_configuration()
      res = ccs_create_configuration(configuration_space.handle, features, count, vals, ct.byref(handle))
      Error.check(res)
      super().__init__(handle = handle, retain = False)
    else:
      super().__init__(handle = handle, retain = retain, auto_release = auto_release)

  @classmethod
  def from_handle(cls, handle, retain = True, auto_release = True):
    return cls(handle = handle, retain = retain, auto_release = auto_release)

  @property
  def configuration_space(self):
    if hasattr(self, "_configuration_space"):
      return self._configuration_space
    v = ccs_configuration_space()
    res = ccs_configuration_get_configuration_space(self.handle, ct.byref(v))
    Error.check(res)
    self._configuration_space = ConfigurationSpace.from_handle(v)
    return self._configuration_space

  @property
  def features(self):
    if hasattr(self, "_features"):
      return self._features
    v = ccs_features()
    res = ccs_configuration_get_features(self.handle, ct.byref(v))
    Error.check(res)
    if bool(v):
      self._features = Features.from_handle(v)
    else:
      self._features = None
    return self._features

  def check(self):
    valid = ccs_bool()
    res = ccs_configuration_check(self.handle, ct.byref(valid))
    Error.check(res)
    return False if valid.value == 0 else True
