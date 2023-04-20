import ctypes as ct
from .base import Object, Error, CEnumeration, ccs_result, _ccs_get_function, ccs_context, ccs_parameter, ccs_features_space, ccs_features, ccs_datum, ccs_bool
from .context import Context
from .parameter import Parameter

ccs_create_features_space = _ccs_get_function("ccs_create_features_space", [ct.c_char_p, ct.POINTER(ccs_features_space)])
ccs_features_space_add_parameter = _ccs_get_function("ccs_features_space_add_parameter", [ccs_features_space, ccs_parameter])
ccs_features_space_add_parameters = _ccs_get_function("ccs_features_space_add_parameters", [ccs_features_space, ct.c_size_t, ct.POINTER(ccs_parameter)])
ccs_features_space_check_features = _ccs_get_function("ccs_features_space_check_features", [ccs_features_space, ccs_features, ct.POINTER(ccs_bool)])
ccs_features_space_check_features_values = _ccs_get_function("ccs_features_space_check_features_values", [ccs_features_space, ct.c_size_t, ct.POINTER(ccs_datum), ct.POINTER(ccs_bool)])

class FeaturesSpace(Context):
  def __init__(self, handle = None, retain = False, auto_release = True,
               name = ""):
    if handle is None:
      handle = ccs_features_space()
      res = ccs_create_features_space(str.encode(name), ct.byref(handle))
      Error.check(res)
      super().__init__(handle = handle, retain = False)
    else:
      super().__init__(handle = handle, retain = retain, auto_release = auto_release)

  @classmethod
  def from_handle(cls, handle, retain = True, auto_release = True):
    return cls(handle = handle, retain = retain, auto_release = auto_release)

  def add_parameter(self, parameter):
    res = ccs_features_space_add_parameter(self.handle, parameter.handle)
    Error.check(res)

  def add_parameters(self, parameters, distributions = None):
    count = len(parameters)
    if count == 0:
      return None
    parameters = (ccs_parameter * count)(*[x.handle.value for x in parameters])
    res = ccs_features_space_add_parameters(self.handle, count, parameters)
    Error.check(res)

  def check(self, features):
    valid = ccs_bool()
    res = ccs_features_space_check_features(self.handle, features.handle, ct.byref(valid))
    Error.check(res)
    return False if valid.value == 0 else True

  def check_values(self, values):
    count = len(values)
    if count != self.num_parameters:
      raise Error(ccs_result(ccs_result.INVALID_VALUE))
    v = (ccs_datum * count)()
    ss = []
    for i in range(count):
      v[i].set_value(values[i], string_store = ss)
    valid = ccs_bool()
    res = ccs_features_space_check_features_values(self.handle, count, v, ct.byref(valid))
    Error.check(res)
    return False if valid.value == 0 else True
