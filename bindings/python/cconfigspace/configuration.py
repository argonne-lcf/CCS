import ctypes as ct
from .base import Object, Error, ccs_error, _ccs_get_function, ccs_context, ccs_hyperparameter, ccs_configuration_space, ccs_configuration, ccs_rng, ccs_distribution, ccs_expression, ccs_datum, ccs_hash, ccs_int
from .context import Context
from .rng import Rng
from .hyperparameter import Hyperparameter
from .configuration_space import ConfigurationSpace

ccs_create_configuration = _ccs_get_function("ccs_create_configuration", [ccs_configuration_space, ct.c_size_t, ct.POINTER(ccs_datum), ct.c_void_p, ct.POINTER(ccs_configuration)])
ccs_configuration_get_configuration_space = _ccs_get_function("ccs_configuration_get_configuration_space", [ccs_configuration, ct.POINTER(ccs_configuration_space)])
ccs_configuration_get_user_data = _ccs_get_function("ccs_configuration_get_user_data", [ccs_configuration, ct.POINTER(ct.c_void_p)])
ccs_configuration_get_value = _ccs_get_function("ccs_configuration_get_value", [ccs_configuration, ct.c_size_t, ct.POINTER(ccs_datum)])
ccs_configuration_set_value = _ccs_get_function("ccs_configuration_set_value", [ccs_configuration, ct.c_size_t, ccs_datum])
ccs_configuration_get_values = _ccs_get_function("ccs_configuration_get_values", [ccs_configuration, ct.c_size_t, ct.POINTER(ccs_datum), ct.POINTER(ct.c_size_t)])
ccs_configuration_get_value_by_name = _ccs_get_function("ccs_configuration_get_value_by_name", [ccs_configuration, ct.c_char_p, ct.POINTER(ccs_datum)])
ccs_configuration_check = _ccs_get_function("ccs_configuration_check", [ccs_configuration])
ccs_configuration_hash = _ccs_get_function("ccs_configuration_hash", [ccs_configuration, ct.POINTER(ccs_hash)])
ccs_configuration_cmp = _ccs_get_function("ccs_configuration_cmp", [ccs_configuration, ccs_configuration, ct.POINTER(ccs_int)])

class Configuration(Object):
  def __init__(self, handle = None, retain = False, auto_release = True,
               configuration_space = None, values = None, user_data = None):
    if handle is None:
      count = 0
      if values:
        count = len(values)
        vals = (ccs_datum * count)()
        for i in range(count):
          vals[i].value = values[i]
      else:
        vals = None
      handle = ccs_configuration()
      res = ccs_create_configuration(configuration_space.handle, count, vals, ct.byref(handle))
      Error.check(res)
      super().__init__(handle = handle, retain = False)
    else:
      super().__init__(handle = handle, retain = retain, auto_release = auto_release)

  @classmethod
  def from_handle(cls, handle, retain = True, auto_release = True):
    return cls(handle = handle, retain = retain, auto_release = auto_release)

  @property
  def user_data(self):
    if hasattr(self, "_user_data"):
      return self._user_data
    v = ct.c_void_p()
    res = ccs_configuration_get_user_data(self.handle, ct.byref(v))
    Error.check(res)
    self._user_data = v
    return v

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
  def num_values(self):
    if hasattr(self, "_num_values"):
      return self._num_values
    v = ct.c_size_t()
    res = ccs_configuration_get_values(self.handle, 0, None, ct.byref(v))
    Error.check(res)
    self._num_values = v.value
    return self._num_values

  @property
  def hash(self):
    v = ccs_hash()
    res = ccs_configuration_hash(self.handle, ct.byref(v))
    Error.check(res)
    return self.value

  def set_value(self, hyperparameter, value):
    if isinstance(hyperparameter, Hyperparameter):
      hyperparameter = self.configuration_space.hyperparameter_index(hyperparameter)
    elif isinstance(hyperparameter, str):
      hyperparameter = self.configuration_space.hyperparameter_index_by_name(hyperparameter)
    pv = ccs_datum(value)
    v = ccs_datum_fix()
    v.value = pv._value.i
    v.type = pv.type
    res = ccs_configuration_set_value(self.handle, hyperparameter, v)
    Error.check(res)

  def value(self, hyperparameter):
    v = ccs_datum()
    if isinstance(hyperparameter, Hyperparameter):
      res = ccs_configuration_get_value(self.handle, self.configuration_space.hyperparameter_index(hyperparameter), ct.byref(v))
    elif isinstance(hyperparameter, str):
      res = ccs_configuration_get_value_by_name(self.handle, str.encode(hyperparameter), ct.byref(v))
    else:
      res = ccs_configuration_get_value(self.handle, hyperparameter, ct.byref(v))
    Error.check(res)
    return v.value

  @property
  def values(self):
    sz = self.num_values
    if sz == 0:
      return []
    v = (ccs_datum * sz)()
    res = ccs_configuration_get_values(self.handle, sz, v, None)
    Error.check(res)
    return [x.value for x in v]

  def check(self):
    res = ccs_configuration_check(self.handle)
    Error.check(res)

  def cmp(self, other):
    v = ccs_int()
    res = ccs_configuration_cmp(self.handle, other.handle, ct.byref(v))
    Error.check(res)
    return v.value

  def __lt__(self, other):
    v = ccs_int()
    res = ccs_configuration_cmp(self.handle, other.handle, ct.byref(v))
    Error.check(res)
    return v.value < 0

  def __le__(self, other):
    v = ccs_int()
    res = ccs_configuration_cmp(self.handle, other.handle, ct.byref(v))
    Error.check(res)
    return v.value <= 0

  def __gt__(self, other):
    v = ccs_int()
    res = ccs_configuration_cmp(self.handle, other.handle, ct.byref(v))
    Error.check(res)
    return v.value > 0

  def __ge__(self, other):
    v = ccs_int()
    res = ccs_configuration_cmp(self.handle, other.handle, ct.byref(v))
    Error.check(res)
    return v.value >= 0

  def __eq__(self, other):
    v = ccs_int()
    res = ccs_configuration_cmp(self.handle, other.handle, ct.byref(v))
    Error.check(res)
    return v.value == 0

  def __ne__(self, other):
    v = ccs_int()
    res = ccs_configuration_cmp(self.handle, other.handle, ct.byref(v))
    Error.check(res)
    return v.value != 0

  def __hash__(self):
    return self.hash

  def asdict(self):
    res = {}
    hyperparameters = self.configuration_space.hyperparameters
    values = self.values
    for i in range(len(hyperparameters)):
      res[hyperparameters[i].name] = values[i]
    return res
