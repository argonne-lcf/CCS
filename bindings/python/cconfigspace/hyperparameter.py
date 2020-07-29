import ctypes as ct
from . import libcconfigspace
from .base import Object, Error, ccs_error, CEnumeration, _ccs_get_function, ccs_hyperparameter, ccs_datum, ccs_distribution, ccs_rng, ccs_int, ccs_data_type, ccs_bool, ccs_numeric_type, ccs_numeric, _ccs_get_id
from .rng import ccs_default_rng
from .distribution import Distribution

class ccs_hyperparameter_type(CEnumeration):
  _members_ = [
    ('NUMERICAL', 0),
    'CATEGORICAL',
    'ORDINAL',
    'DISCRETE'
  ]

class ccs_datum_fix(ct.Structure):
  _fields_ = [('value', ccs_int),
              ('type', ccs_data_type)]

ccs_hyperparameter_get_type = _ccs_get_function("ccs_hyperparameter_get_type", [ccs_hyperparameter, ct.POINTER(ccs_hyperparameter_type)])
ccs_hyperparameter_get_default_value = _ccs_get_function("ccs_hyperparameter_get_default_value", [ccs_hyperparameter, ct.POINTER(ccs_datum)])
ccs_hyperparameter_get_name = _ccs_get_function("ccs_hyperparameter_get_name", [ccs_hyperparameter, ct.POINTER(ct.c_char_p)])
ccs_hyperparameter_get_user_data = _ccs_get_function("ccs_hyperparameter_get_user_data", [ccs_hyperparameter, ct.POINTER(ct.c_void_p)])
ccs_hyperparameter_get_default_distribution = _ccs_get_function("ccs_hyperparameter_get_default_distribution", [ccs_hyperparameter, ct.POINTER(ccs_distribution)])
ccs_hyperparameter_check_value = _ccs_get_function("ccs_hyperparameter_check_value", [ccs_hyperparameter, ccs_datum_fix, ct.POINTER(ccs_bool)])
ccs_hyperparameter_check_values = _ccs_get_function("ccs_hyperparameter_check_values", [ccs_hyperparameter, ct.c_size_t, ct.POINTER(ccs_datum), ct.POINTER(ccs_bool)])
ccs_hyperparameter_sample = _ccs_get_function("ccs_hyperparameter_sample", [ccs_hyperparameter, ccs_distribution, ccs_rng, ct.POINTER(ccs_datum)])
ccs_hyperparameter_samples = _ccs_get_function("ccs_hyperparameter_samples", [ccs_hyperparameter, ccs_distribution, ccs_rng, ct.c_size_t, ct.POINTER(ccs_datum)])

class Hyperparameter(Object):

  @classmethod
  def from_handle(cls, handle):
    v = ccs_hyperparameter_type(0)
    res = ccs_hyperparameter_get_type(handle, ct.byref(v))
    Error.check(res)
    v = v.value
    if v == ccs_hyperparameter_type.NUMERICAL:
      return NumericalHyperparameter(handle = handle, retain = True)
    elif v == ccs_hyperparameter_type.CATEGORICAL:
      return CategoricalHyperparameter(handle = handle, retain = True)
    elif v == ccs_hyperparameter_type.ORDINAL:
      return OrdinalHyperparameter(handle = handle, retain = True)
    elif v == ccs_hyperparameter_type.DISCRETE:
      return DiscreteHyperparameter(handle = handle, retain = True)
    else:
      raise Error(ccs_error.INVALID_HYPERPARAMETER)

  @classmethod
  def default_name(cls):
    return "param%03d" % _ccs_get_id()

  @property
  def type(self):
    if hasattr(self, "_type"):
      return self._type
    v = ccs_hyperparameter_type(0)
    res = ccs_hyperparameter_get_type(self.handle, ct.byref(v))
    Error.check(res)
    self._type = v
    return v

  @property
  def user_data(self):
    if hasattr(self, "_user_data"):
      return self._user_data
    v = ct.c_void_p()
    res = ccs_hyperparameter_get_user_data(self.handle, ct.byref(v))
    Error.check(res)
    self._user_data = v
    return v

  @property
  def name(self):
    if hasattr(self, "_name"):
      return self._name
    v = ct.c_char_p()
    res = ccs_hyperparameter_get_name(self.handle, ct.byref(v))
    Error.check(res)
    self._name = v.value.decode()
    return self._name

  @property
  def default_value(self):
    if hasattr(self, "_default_value"):
      return self._default_value
    v = ccs_datum()
    res = ccs_hyperparameter_get_default_value(self.handle, ct.byref(v))
    Error.check(res)
    self._default_value = v.value
    return self._default_value

  @property
  def default_distribution(self):
    if hasattr(self, "_default_distribution"):
      return self._default_distribution
    v = ccs_distribution()
    res = ccs_hyperparameter_get_default_distribution(self.handle, ct.byref(v))
    Error.check(res)
    self._default_distribution = Distribution.from_handle(v)
    return self._default_distribution

  def check_value(self, value):
    proxy = ccs_datum()
    proxy.value = value
    v = ccs_datum_fix()
    v.value = proxy._value.i
    v.type = proxy.type
    b = ccs_bool()
    res = ccs_hyperparameter_check_value(self.handle, v, ct.byref(b))
    Error.check(res)
    return False if b.value == 0 else True

  def check_values(self, values):
    sz = len(values)
    v = (ccs_datum * sz)(*values)
    b = (ccs_bool * sz)()
    res = ccs_hyperparameter_check_values(self.handle, sz, v, b)
    Error.check(res)
    return [lambda x: False if x == 0 else true for x in b]

  def sample(self, distribution = None, rng = None):
    if distribution is None:
      distribution = self.default_distribution
    if rng is None:
      rng = ccs_default_rng
    v = ccs_datum()
    res = ccs_hyperparameter_sample(self.handle, distribution.handle, rng.handle, ct.byref(v))
    Error.check(res)
    return v.value

  def samples(self, count, distribution = None, rng = None):
    if distribution is None:
      distribution = self.default_distribution
    if rng is None:
      rng = ccs_default_rng
    v = (ccs_datum * count)()
    res = ccs_hyperparameter_samples(self.handle, distribution.handle, rng.handle, count, v)
    Error.check(res)
    return [x.value for x in v]

  def __eql__(self, other):
    return self.__class__ == other.__class__ and self.handle == other.handle


ccs_create_numerical_hyperparameter = _ccs_get_function("ccs_create_numerical_hyperparameter", [ct.c_char_p, ccs_numeric_type, ccs_int, ccs_int, ccs_int, ccs_int, ct.c_void_p, ct.POINTER(ccs_hyperparameter)])
ccs_numerical_hyperparameter_get_parameters = _ccs_get_function("ccs_numerical_hyperparameter_get_parameters", [ccs_hyperparameter, ct.POINTER(ccs_numeric_type), ct.POINTER(ccs_numeric), ct.POINTER(ccs_numeric), ct.POINTER(ccs_numeric)])

class NumericalHyperparameter(Hyperparameter):
  def __init__(self, handle = None, retain = False, name = None, data_type = ccs_numeric_type.NUM_FLOAT, lower = 0.0, upper = 1.0, quantization = 0.0, default = None, user_data = None):
    if handle is None:
      if name is None:
        name = NumericalHyperparameter.default_name()
      if default is None:
        default = lower
      l = ccs_numeric()
      u = ccs_numeric()
      q = ccs_numeric()
      d = ccs_numeric()
      if data_type == ccs_numeric_type.NUM_FLOAT:
        l.f = lower
        u.f = upper
        q.f = quantization
        d.f = default
      elif data_type == ccs_numeric_type.NUM_INTEGER:
        l.i = lower
        u.i = upper
        q.i = quantization
        d.i = default
      else:
        raise Error(ccs_error.INVALID_VALUE)
      handle = ccs_hyperparameter()
      res = ccs_create_numerical_hyperparameter(str.encode(name), data_type, l.i, u.i, q.i, d.i, user_data, ct.byref(handle))
      Error.check(res)
      super().__init__(handle = handle, retain = False)
    else:
      super().__init__(handle = handle, retain = retain)

  @classmethod
  def int(cls, lower, upper, name = None, quantization = 0, default = None, user_data = None):
    return cls(handle = None, name = name, data_type =  ccs_numeric_type.NUM_INTEGER, lower = lower, upper = upper, quantization = quantization, default = default, user_data = user_data)
  
  @classmethod
  def float(cls, lower, upper, name = None, quantization = 0.0, default = None, user_data = None):
    return cls(handle = None, name = name, data_type =  ccs_numeric_type.NUM_FLOAT, lower = lower, upper = upper, quantization = quantization, default = default, user_data = user_data)

  @property
  def data_type(self):
    if hasattr(self, "_data_type"):
      return self._data_type
    v = ccs_numeric_type(0)
    res = ccs_numerical_hyperparameter_get_parameters(self.handle, ct.byref(v), None, None, None)
    Error.check(res)
    self._data_type = v
    return v

  @property
  def lower(self):
    if hasattr(self, "_lower"):
      return self._lower
    v = ccs_numeric()
    res = ccs_numerical_hyperparameter_get_parameters(self.handle, None, ct.byref(v), None, None)
    Error.check(res)
    t = self.data_type.value
    if t == ccs_numeric_type.NUM_INTEGER:
      self._lower = v.i
    elif t == ccs_numeric_type.NUM_FLOAT:
      self._lower = v.f
    else:
      raise Error(ccs_error.INVALID_VALUE)
    return self._lower

  @property
  def upper(self):
    if hasattr(self, "_upper"):
      return self._upper
    v = ccs_numeric()
    res = ccs_numerical_hyperparameter_get_parameters(self.handle, None, None, ct.byref(v), None)
    Error.check(res)
    t = self.data_type.value
    if t == ccs_numeric_type.NUM_INTEGER:
      self._upper = v.i
    elif t == ccs_numeric_type.NUM_FLOAT:
      self._upper = v.f
    else:
      raise Error(ccs_error.INVALID_VALUE)
    return self._upper

  @property
  def quantization(self):
    if hasattr(self, "_quantization"):
      return self._quantization
    v = ccs_numeric(0)
    res = ccs_numerical_hyperparameter_get_parameters(self.handle, None, None, None, ct.byref(v))
    Error.check(res)
    t = self.data_type.value
    if t == ccs_numeric_type.NUM_INTEGER:
      self._quantization = v.i
    elif t == ccs_numeric_type.NUM_FLOAT:
      self._quantization = v.f
    else:
      raise Error(ccs_error.INVALID_VALUE)
    return self._quantization


