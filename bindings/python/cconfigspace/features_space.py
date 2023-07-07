import ctypes as ct
from .base import Object, Error, CEnumeration, Result, _ccs_get_function, ccs_context, ccs_parameter, ccs_features_space, ccs_features, Datum, ccs_bool
from .context import Context
from .parameter import Parameter

ccs_create_features_space = _ccs_get_function("ccs_create_features_space", [ct.c_char_p, ct.c_size_t, ct.POINTER(ccs_parameter), ct.POINTER(ccs_features_space)])
ccs_features_space_check_features = _ccs_get_function("ccs_features_space_check_features", [ccs_features_space, ccs_features, ct.POINTER(ccs_bool)])
ccs_features_space_check_features_values = _ccs_get_function("ccs_features_space_check_features_values", [ccs_features_space, ct.c_size_t, ct.POINTER(Datum), ct.POINTER(ccs_bool)])

class FeaturesSpace(Context):
  def __init__(self, handle = None, retain = False, auto_release = True,
               name = "", parameters = None):
    if handle is None:
      count = len(parameters)
      parameters = (ccs_parameter * count)(*[x.handle.value for x in parameters])
      handle = ccs_features_space()
      res = ccs_create_features_space(str.encode(name), count, parameters, ct.byref(handle))
      Error.check(res)
      super().__init__(handle = handle, retain = False)
    else:
      super().__init__(handle = handle, retain = retain, auto_release = auto_release)

  @classmethod
  def from_handle(cls, handle, retain = True, auto_release = True):
    return cls(handle = handle, retain = retain, auto_release = auto_release)

  def check(self, features):
    valid = ccs_bool()
    res = ccs_features_space_check_features(self.handle, features.handle, ct.byref(valid))
    Error.check(res)
    return False if valid.value == 0 else True

  def check_values(self, values):
    count = len(values)
    if count != self.num_parameters:
      raise Error(Result(Result.ERROR_INVALID_VALUE))
    v = (Datum * count)()
    ss = []
    for i in range(count):
      v[i].set_value(values[i], string_store = ss)
    valid = ccs_bool()
    res = ccs_features_space_check_features_values(self.handle, count, v, ct.byref(valid))
    Error.check(res)
    return False if valid.value == 0 else True
