import ctypes as ct
from . import libcconfigspace
from .base import Object, Error, Result, CEnumeration, _ccs_get_function, ccs_parameter, Datum, DatumFix, ccs_distribution, ccs_rng, ccs_int, DataType, ccs_bool, NumericType, Numeric, _ccs_get_id
from .rng import ccs_default_rng
from .distribution import Distribution

class ParameterType(CEnumeration):
  _scope_ = True
  _members_ = [
    ('NUMERICAL', 0),
    'CATEGORICAL',
    'ORDINAL',
    'DISCRETE',
    'STRING'
  ]

ccs_parameter_get_type = _ccs_get_function("ccs_parameter_get_type", [ccs_parameter, ct.POINTER(ParameterType)])
ccs_parameter_get_default_value = _ccs_get_function("ccs_parameter_get_default_value", [ccs_parameter, ct.POINTER(Datum)])
ccs_parameter_get_name = _ccs_get_function("ccs_parameter_get_name", [ccs_parameter, ct.POINTER(ct.c_char_p)])
ccs_parameter_get_default_distribution = _ccs_get_function("ccs_parameter_get_default_distribution", [ccs_parameter, ct.POINTER(ccs_distribution)])
ccs_parameter_check_value = _ccs_get_function("ccs_parameter_check_value", [ccs_parameter, DatumFix, ct.POINTER(ccs_bool)])
ccs_parameter_check_values = _ccs_get_function("ccs_parameter_check_values", [ccs_parameter, ct.c_size_t, ct.POINTER(Datum), ct.POINTER(ccs_bool)])
ccs_parameter_sample = _ccs_get_function("ccs_parameter_sample", [ccs_parameter, ccs_distribution, ccs_rng, ct.POINTER(Datum)])
ccs_parameter_samples = _ccs_get_function("ccs_parameter_samples", [ccs_parameter, ccs_distribution, ccs_rng, ct.c_size_t, ct.POINTER(Datum)])

class Parameter(Object):

  @classmethod
  def from_handle(cls, handle, retain = True, auto_release = True):
    v = ParameterType(0)
    res = ccs_parameter_get_type(handle, ct.byref(v))
    Error.check(res)
    v = v.value
    if v == ParameterType.NUMERICAL:
      return NumericalParameter(handle = handle, retain = retain, auto_release = auto_release)
    elif v == ParameterType.CATEGORICAL:
      return CategoricalParameter(handle = handle, retain = retain, auto_release = auto_release)
    elif v == ParameterType.ORDINAL:
      return OrdinalParameter(handle = handle, retain = retain, auto_release = auto_release)
    elif v == ParameterType.DISCRETE:
      return DiscreteParameter(handle = handle, retain = retain, auto_release = auto_release)
    elif v == ParameterType.STRING:
      return StringParameter(handle = handle, retain = retain, auto_release = auto_release)
    else:
      raise Error(Result(Result.ERROR_INVALID_PARAMETER))

  @classmethod
  def default_name(cls):
    return "param%03d" % _ccs_get_id()

  @property
  def type(self):
    if hasattr(self, "_type"):
      return self._type
    v = ParameterType(0)
    res = ccs_parameter_get_type(self.handle, ct.byref(v))
    Error.check(res)
    self._type = v.value
    return self._type

  @property
  def name(self):
    if hasattr(self, "_name"):
      return self._name
    v = ct.c_char_p()
    res = ccs_parameter_get_name(self.handle, ct.byref(v))
    Error.check(res)
    self._name = v.value.decode()
    return self._name

  @property
  def default_value(self):
    if hasattr(self, "_default_value"):
      return self._default_value
    v = Datum()
    res = ccs_parameter_get_default_value(self.handle, ct.byref(v))
    Error.check(res)
    self._default_value = v.value
    return self._default_value

  @property
  def default_distribution(self):
    if hasattr(self, "_default_distribution"):
      return self._default_distribution
    v = ccs_distribution()
    res = ccs_parameter_get_default_distribution(self.handle, ct.byref(v))
    Error.check(res)
    self._default_distribution = Distribution.from_handle(v)
    return self._default_distribution

  def check_value(self, value):
    pv = Datum(value)
    v = DatumFix(pv)
    b = ccs_bool()
    res = ccs_parameter_check_value(self.handle, v, ct.byref(b))
    Error.check(res)
    return False if b.value == 0 else True

  def check_values(self, values):
    sz = len(values)
    v = (Datum * sz)()
    ss = []
    for i in range(sz):
      v[i].set_value(values[i], string_store = ss)
    b = (ccs_bool * sz)()
    res = ccs_parameter_check_values(self.handle, sz, v, b)
    Error.check(res)
    return [False if x == 0 else True for x in b]

  def sample(self, distribution = None, rng = None):
    if distribution is None:
      distribution = self.default_distribution
    if rng is None:
      rng = ccs_default_rng
    v = Datum()
    res = ccs_parameter_sample(self.handle, distribution.handle, rng.handle, ct.byref(v))
    Error.check(res)
    return v.value

  def samples(self, count, distribution = None, rng = None):
    if distribution is None:
      distribution = self.default_distribution
    if rng is None:
      rng = ccs_default_rng
    v = (Datum * count)()
    res = ccs_parameter_samples(self.handle, distribution.handle, rng.handle, count, v)
    Error.check(res)
    return [x.value for x in v]

  def __eq__(self, other):
    return self.__class__ == other.__class__ and self.handle.value == other.handle.value


ccs_create_numerical_parameter = _ccs_get_function("ccs_create_numerical_parameter", [ct.c_char_p, NumericType, ccs_int, ccs_int, ccs_int, ccs_int, ct.POINTER(ccs_parameter)])
ccs_numerical_parameter_get_properties = _ccs_get_function("ccs_numerical_parameter_get_properties", [ccs_parameter, ct.POINTER(NumericType), ct.POINTER(Numeric), ct.POINTER(Numeric), ct.POINTER(Numeric)])

class NumericalParameter(Parameter):
  def __init__(self, handle = None, retain = False, auto_release = True,
               name = None, data_type = NumericType.FLOAT, lower = 0.0, upper = 1.0, quantization = 0.0, default = None):
    if handle is None:
      if name is None:
        name = NumericalParameter.default_name()
      if default is None:
        default = lower
      l = Numeric()
      u = Numeric()
      q = Numeric()
      d = Numeric()
      if data_type == NumericType.FLOAT:
        l.f = lower
        u.f = upper
        q.f = quantization
        d.f = default
      elif data_type == NumericType.INT:
        l.i = lower
        u.i = upper
        q.i = quantization
        d.i = default
      else:
        raise Error(Result(Result.ERROR_INVALID_VALUE))
      handle = ccs_parameter()
      res = ccs_create_numerical_parameter(str.encode(name), data_type, l.i, u.i, q.i, d.i, ct.byref(handle))
      Error.check(res)
      super().__init__(handle = handle, retain = False)
    else:
      super().__init__(handle = handle, retain = retain, auto_release = auto_release)

  @classmethod
  def int(cls, lower, upper, name = None, quantization = 0, default = None):
    return cls(handle = None, name = name, data_type =  NumericType.INT, lower = lower, upper = upper, quantization = quantization, default = default)
  
  @classmethod
  def float(cls, lower, upper, name = None, quantization = 0.0, default = None):
    return cls(handle = None, name = name, data_type =  NumericType.FLOAT, lower = lower, upper = upper, quantization = quantization, default = default)

  @property
  def data_type(self):
    if hasattr(self, "_data_type"):
      return self._data_type
    v = NumericType(0)
    res = ccs_numerical_parameter_get_properties(self.handle, ct.byref(v), None, None, None)
    Error.check(res)
    self._data_type = v.value
    return self._data_type

  @property
  def lower(self):
    if hasattr(self, "_lower"):
      return self._lower
    v = Numeric()
    res = ccs_numerical_parameter_get_properties(self.handle, None, ct.byref(v), None, None)
    Error.check(res)
    t = self.data_type
    if t == NumericType.INT:
      self._lower = v.i
    elif t == NumericType.FLOAT:
      self._lower = v.f
    else:
      raise Error(Result(Result.ERROR_INVALID_VALUE))
    return self._lower

  @property
  def upper(self):
    if hasattr(self, "_upper"):
      return self._upper
    v = Numeric()
    res = ccs_numerical_parameter_get_properties(self.handle, None, None, ct.byref(v), None)
    Error.check(res)
    t = self.data_type
    if t == NumericType.INT:
      self._upper = v.i
    elif t == NumericType.FLOAT:
      self._upper = v.f
    else:
      raise Error(Result(Result.ERROR_INVALID_VALUE))
    return self._upper

  @property
  def quantization(self):
    if hasattr(self, "_quantization"):
      return self._quantization
    v = Numeric(0)
    res = ccs_numerical_parameter_get_properties(self.handle, None, None, None, ct.byref(v))
    Error.check(res)
    t = self.data_type
    if t == NumericType.INT:
      self._quantization = v.i
    elif t == NumericType.FLOAT:
      self._quantization = v.f
    else:
      raise Error(Result(Result.ERROR_INVALID_VALUE))
    return self._quantization

ccs_create_categorical_parameter = _ccs_get_function("ccs_create_categorical_parameter", [ct.c_char_p, ct.c_size_t, ct.POINTER(Datum), ct.c_size_t, ct.POINTER(ccs_parameter)])
ccs_categorical_parameter_get_values = _ccs_get_function("ccs_categorical_parameter_get_values", [ccs_parameter, ct.c_size_t, ct.POINTER(Datum), ct.POINTER(ct.c_size_t)])

class CategoricalParameter(Parameter):
  def __init__(self, handle = None, retain = False, auto_release = True,
               name = None, values = [], default_index = 0):
    if handle is None:
      if name is None:
        name = NumericalParameter.default_name()
      sz = len(values)
      handle = ccs_parameter()
      v = (Datum*sz)()
      ss = []
      for i in range(sz):
        v[i].set_value(values[i], string_store = ss)
      res = ccs_create_categorical_parameter(str.encode(name), sz, v, default_index, ct.byref(handle))
      Error.check(res)
      super().__init__(handle = handle, retain = False)
    else:
      super().__init__(handle = handle, retain = retain, auto_release = auto_release)

  @property
  def values(self):
    sz = ct.c_size_t()
    res = ccs_categorical_parameter_get_values(self.handle, 0, None, ct.byref(sz))
    Error.check(res)
    v = (Datum*sz.value)()
    res = ccs_categorical_parameter_get_values(self.handle, sz, v, None)
    Error.check(res)
    return [x.value for x in v]

ccs_create_ordinal_parameter = _ccs_get_function("ccs_create_ordinal_parameter", [ct.c_char_p, ct.c_size_t, ct.POINTER(Datum), ct.c_size_t, ct.POINTER(ccs_parameter)])
ccs_ordinal_parameter_compare_values = _ccs_get_function("ccs_ordinal_parameter_compare_values", [ccs_parameter, DatumFix, DatumFix, ct.POINTER(ccs_int)])
ccs_ordinal_parameter_get_values = _ccs_get_function("ccs_ordinal_parameter_get_values", [ccs_parameter, ct.c_size_t, ct.POINTER(Datum), ct.POINTER(ct.c_size_t)])

class OrdinalParameter(Parameter):
  def __init__(self, handle = None, retain = False, auto_release = True,
               name = None, values = [], default_index = 0):
    if handle is None:
      if name is None:
        name = NumericalParameter.default_name()
      sz = len(values)
      handle = ccs_parameter()
      v = (Datum*sz)()
      ss = []
      for i in range(sz):
        v[i].set_value(values[i], string_store = ss)
      res = ccs_create_ordinal_parameter(str.encode(name), sz, v, default_index, ct.byref(handle))
      Error.check(res)
      super().__init__(handle = handle, retain = False)
    else:
      super().__init__(handle = handle, retain = retain, auto_release = auto_release)

  @property
  def values(self):
    sz = ct.c_size_t()
    res = ccs_ordinal_parameter_get_values(self.handle, 0, None, ct.byref(sz))
    Error.check(res)
    v = (Datum*sz.value)()
    res = ccs_ordinal_parameter_get_values(self.handle, sz, v, None)
    Error.check(res)
    return [x.value for x in v]

  def compare(self, value1, value2):
    pv1 = Datum(value1)
    pv2 = Datum(value2)
    v1 = DatumFix(pv1)
    v2 = DatumFix(pv2)
    c = ccs_int()
    res = ccs_ordinal_parameter_compare_values(self.handle, v1, v2, ct.byref(c))
    Error.check(res)
    return c.value

ccs_create_discrete_parameter = _ccs_get_function("ccs_create_discrete_parameter", [ct.c_char_p, ct.c_size_t, ct.POINTER(Datum), ct.c_size_t, ct.POINTER(ccs_parameter)])
ccs_discrete_parameter_get_values = _ccs_get_function("ccs_discrete_parameter_get_values", [ccs_parameter, ct.c_size_t, ct.POINTER(Datum), ct.POINTER(ct.c_size_t)])

class DiscreteParameter(Parameter):
  def __init__(self, handle = None, retain = False, auto_release = True,
               name = None, values = [], default_index = 0):
    if handle is None:
      if name is None:
        name = NumericalParameter.default_name()
      sz = len(values)
      handle = ccs_parameter()
      v = (Datum*sz)()
      ss = []
      for i in range(sz):
        v[i].set_value(values[i], string_store = ss)
      res = ccs_create_discrete_parameter(str.encode(name), sz, v, default_index, ct.byref(handle))
      Error.check(res)
      super().__init__(handle = handle, retain = False)
    else:
      super().__init__(handle = handle, retain = retain, auto_release = auto_release)

  @property
  def values(self):
    sz = ct.c_size_t()
    res = ccs_discrete_parameter_get_values(self.handle, 0, None, ct.byref(sz))
    Error.check(res)
    v = (Datum*sz.value)()
    res = ccs_discrete_parameter_get_values(self.handle, sz, v, None)
    Error.check(res)
    return [x.value for x in v]

ccs_create_string_parameter = _ccs_get_function("ccs_create_string_parameter", [ct.c_char_p, ct.POINTER(ccs_parameter)])

class StringParameter(Parameter):
  def __init__(self, handle = None, retain = False, auto_release = True,
               name = None):
    if handle is None:
      if name is None:
        name = NumericalParameter.default_name()
      handle = ccs_parameter()
      res = ccs_create_string_parameter(str.encode(name), ct.byref(handle))
      Error.check(res)
      super().__init__(handle = handle, retain = False)
    else:
      super().__init__(handle = handle, retain = retain, auto_release = auto_release)
