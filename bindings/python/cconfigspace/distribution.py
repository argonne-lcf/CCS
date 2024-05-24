import ctypes as ct
from . import libcconfigspace
from .base import Object, Error, Result, ccs_int, ccs_float, ccs_bool, ccs_rng, ccs_distribution, NumericType, Numeric, CEnumeration, _ccs_get_function, ccs_false, ccs_true
from .interval import Interval

class DistributionType(CEnumeration):
  _members_ = [
    ('UNIFORM', 0),
    'NORMAL',
    'ROULETTE',
    'MIXTURE',
    'MULTIVARIATE' ]

class ScaleType(CEnumeration):
  _members_ = [
    ('LINEAR', 0),
    'LOGARITHMIC' ]

ccs_distribution_get_type = _ccs_get_function("ccs_distribution_get_type", [ccs_distribution, ct.POINTER(DistributionType)])
ccs_distribution_get_data_types = _ccs_get_function("ccs_distribution_get_data_types", [ccs_distribution, ct.POINTER(NumericType)])
ccs_distribution_get_dimension = _ccs_get_function("ccs_distribution_get_dimension", [ccs_distribution, ct.POINTER(ct.c_size_t)])
ccs_distribution_get_bounds = _ccs_get_function("ccs_distribution_get_bounds", [ccs_distribution, ct.POINTER(Interval)])
ccs_distribution_check_oversampling = _ccs_get_function("ccs_distribution_check_oversampling", [ccs_distribution, ct.POINTER(Interval), ct.POINTER(ccs_bool)])
ccs_distribution_sample = _ccs_get_function("ccs_distribution_sample", [ccs_distribution, ccs_rng, ct.POINTER(Numeric)])
ccs_distribution_samples = _ccs_get_function("ccs_distribution_samples", [ccs_distribution, ccs_rng, ct.c_size_t, ct.POINTER(Numeric)])

class Distribution(Object):

  @classmethod
  def from_handle(cls, handle, retain = True, auto_release = True):
    v = DistributionType(0)
    res = ccs_distribution_get_type(handle, ct.byref(v))
    Error.check(res)
    v = v.value
    if v == DistributionType.UNIFORM:
      return UniformDistribution.from_handle(handle, retain = retain, auto_release = auto_release)
    elif v == DistributionType.NORMAL:
      return NormalDistribution.from_handle(handle, retain = retain, auto_release = auto_release)
    elif v == DistributionType.ROULETTE:
      return RouletteDistribution(handle = handle, retain = retain, auto_release = auto_release)
    elif v == DistributionType.MIXTURE:
      return MixtureDistribution(handle = handle, retain = retain, auto_release = auto_release)
    elif v == DistributionType.MULTIVARIATE:
      return MultivariateDistribution(handle = handle, retain = retain, auto_release = auto_release)
    else:
      raise Error(Result(Result.ERROR_INVALID_DISTRIBUTION))

  @property
  def type(self):
    if hasattr(self, "_type"):
      return self._type
    v = DistributionType(0)
    res = ccs_distribution_get_type(self.handle, ct.byref(v))
    Error.check(res)
    self._type = v.value
    return self._type

  @property
  def data_types(self):
    if hasattr(self, "_data_types"):
      return self._data_types
    v = (NumericType*self.dimension)()
    res = ccs_distribution_get_data_types(self.handle, v)
    Error.check(res)
    self._data_types = [t.value for t in v]
    return self._data_types

  @property
  def dimension(self):
    if hasattr(self, "_dimension"):
      return self._dimension
    v = ct.c_size_t()
    res = ccs_distribution_get_dimension(self.handle, ct.byref(v))
    Error.check(res)
    self._dimension = v.value
    return v.value

  @property
  def bounds(self):
    if hasattr(self, "_bounds"):
      return self._bounds
    v = Interval()
    res = ccs_distribution_get_bounds(self.handle, ct.byref(v))
    Error.check(res)
    self._bounds = v
    return v

  def is_oversampling(self, interval):
    v = ccs_bool()
    res = ccs_distribution_check_oversampling(self.handle, ct.byref(interval), ct.byref(v))
    Error.check(res)
    return False if v.value == ccs_false else True

  def sample(self, rng):
    dim = self.dimension
    v = (Numeric*dim)()
    res = ccs_distribution_sample(self.handle, rng.handle, v)
    Error.check(res)
    if dim == 1:
      t = self.data_types[0]
      if t == NumericType.INT:
        return v[0].i
      elif t == NumericType.FLOAT:
        return v[0].f
      else:
        raise Error(Result(Result.ERROR_INVALID_VALUE))
    else:
      return [ v[i].i if self.data_types[i] == NumericType.INT else v[i].f for i in range(dim) ]

  def samples(self, rng, count):
    if count == 0:
      return []
    dim = self.dimension
    if dim == 1:
      t = self.data_types[0]
      if t == NumericType.INT:
        v = (ccs_int * (count * dim))()
      elif t == NumericType.FLOAT:
        v = (ccs_float * (count * dim))()
      else:
        raise Error(Result(Result.ERROR_INVALID_VALUE))
      res = ccs_distribution_samples(self.handle, rng.handle, count, ct.cast(v, ct.POINTER(Numeric)))
      Error.check(res)
      return list(v)
    else:
      v = (Numeric*dim*count)()
      res = ccs_distribution_samples(self.handle, rng.handle, count, v)
      Error.check(res)
      return [ [v[j][i].i if self.data_types[i] == NumericType.INT else v[j][i].f for i in range(dim) ] for j in range(count) ]

ccs_create_uniform_int_distribution = _ccs_get_function("ccs_create_uniform_int_distribution", [ccs_int, ccs_int, ScaleType, ccs_int, ct.POINTER(ccs_distribution)])
ccs_create_uniform_float_distribution = _ccs_get_function("ccs_create_uniform_float_distribution", [ccs_float, ccs_float, ScaleType, ccs_float, ct.POINTER(ccs_distribution)])
ccs_uniform_distribution_get_properties = _ccs_get_function("ccs_uniform_distribution_get_properties", [ccs_distribution, ct.POINTER(Numeric), ct.POINTER(Numeric), ct.POINTER(ScaleType), ct.POINTER(Numeric)])

class UniformDistribution(Distribution):

  @classmethod
  def from_handle(cls, handle, retain = True, auto_release = True):
    v = NumericType(0)
    res = ccs_distribution_get_data_types(handle, ct.byref(v))
    Error.check(res)
    v = v.value
    if v == NumericType.FLOAT:
      return UniformFloatDistribution(handle = handle, retain = retain, auto_release = auto_release)
    elif v == NumericType.INT:
      return UniformIntDistribution(handle = handle, retain = retain, auto_release = auto_release)
    else:
      raise Error(Result(Result.ERROR_INVALID_DISTRIBUTION))

  @property
  def data_type(self):
    if hasattr(self, "_data_type"):
      return self._data_type
    self._data_type = self.data_types[0]
    return self._data_type

  @property
  def lower(self):
    if hasattr(self, "_lower"):
      return self._lower
    v = Numeric()
    res = ccs_uniform_distribution_get_properties(self.handle, ct.byref(v), None, None, None)
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
    res = ccs_uniform_distribution_get_properties(self.handle, None, ct.byref(v), None, None)
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
  def scale_type(self):
    if hasattr(self, "_scale_type"):
      return self._scale_type
    v = ScaleType(0)
    res = ccs_uniform_distribution_get_properties(self.handle, None, None, ct.byref(v), None)
    Error.check(res)
    self._scale_type = v.value
    return self._scale_type

  scale = scale_type

  @property
  def quantization(self):
    if hasattr(self, "_quantization"):
      return self._quantization
    v = Numeric(0)
    res = ccs_uniform_distribution_get_properties(self.handle, None, None, None, ct.byref(v))
    Error.check(res)
    t = self.data_type
    if t == NumericType.INT:
      self._quantization = v.i
    elif t == NumericType.FLOAT:
      self._quantization = v.f
    else:
      raise Error(Result(Result.ERROR_INVALID_VALUE))
    return self._quantization

Distribution.Uniform = UniformDistribution

class UniformFloatDistribution(UniformDistribution):
  def __init__(self, handle = None, retain = False, auto_release = True,
               lower = 0.0, upper = 1.0, scale = ScaleType.LINEAR, quantization = 0.0):
    if handle is None:
      handle = ccs_distribution(0)
      res = ccs_create_uniform_float_distribution(lower, upper, scale, quantization, ct.byref(handle))
      Error.check(res)
      super().__init__(handle = handle, retain = False)
    else:
      super().__init__(handle = handle, retain = retain, auto_release = auto_release)

UniformDistribution.Float = UniformFloatDistribution

class UniformIntDistribution(UniformDistribution):
  def __init__(self, handle = None, retain = False, auto_release = True,
               lower = 0, upper = 100, scale = ScaleType.LINEAR, quantization = 0):
    if handle is None:
      handle = ccs_distribution(0)
      res = ccs_create_uniform_int_distribution(lower, upper, scale, quantization, ct.byref(handle))
      Error.check(res)
      super().__init__(handle = handle, retain = False)
    else:
      super().__init__(handle = handle, retain = retain, auto_release = auto_release)

UniformDistribution.Int = UniformIntDistribution

ccs_create_normal_int_distribution = _ccs_get_function("ccs_create_normal_int_distribution", [ccs_float, ccs_float, ScaleType, ccs_int, ct.POINTER(ccs_distribution)])
ccs_create_normal_float_distribution = _ccs_get_function("ccs_create_normal_float_distribution", [ccs_float, ccs_float, ScaleType, ccs_float, ct.POINTER(ccs_distribution)])
ccs_normal_distribution_get_properties = _ccs_get_function("ccs_normal_distribution_get_properties", [ccs_distribution, ct.POINTER(ccs_float), ct.POINTER(ccs_float), ct.POINTER(ScaleType), ct.POINTER(Numeric)])

class NormalDistribution(Distribution):

  @classmethod
  def from_handle(cls, handle, retain = True, auto_release = True):
    v = NumericType(0)
    res = ccs_distribution_get_data_types(handle, ct.byref(v))
    Error.check(res)
    v = v.value
    if v == NumericType.FLOAT:
      return NormalFloatDistribution(handle = handle, retain = retain, auto_release = auto_release)
    elif v == NumericType.INT:
      return NormalIntDistribution(handle = handle, retain = retain, auto_release = auto_release)
    else:
      raise Error(Result(Result.ERROR_INVALID_DISTRIBUTION))

  @property
  def data_type(self):
    if hasattr(self, "_data_type"):
      return self._data_type
    self._data_type = self.data_types[0]
    return self._data_type

  @property
  def mu(self):
    if hasattr(self, "_mu"):
      return self._mu
    v = ccs_float()
    res = ccs_normal_distribution_get_properties(self.handle, ct.byref(v), None, None, None)
    Error.check(res)
    self._mu = v.value
    return self._mu

  @property
  def sigma(self):
    if hasattr(self, "_sigma"):
      return self._sigma
    v = ccs_float()
    res = ccs_normal_distribution_get_properties(self.handle, None, ct.byref(v), None, None)
    Error.check(res)
    self._sigma = v.value
    return self._sigma

  @property
  def scale_type(self):
    if hasattr(self, "_scale_type"):
      return self._scale_type
    v = ScaleType(0)
    res = ccs_normal_distribution_get_properties(self.handle, None, None, ct.byref(v), None)
    Error.check(res)
    self._scale_type = v.value
    return self._scale_type

  scale = scale_type

  @property
  def quantization(self):
    if hasattr(self, "_quantization"):
      return self._quantization
    v = Numeric(0)
    res = ccs_normal_distribution_get_properties(self.handle, None, None, None, ct.byref(v))
    Error.check(res)
    t = self.data_type
    if t == NumericType.INT:
      self._quantization = v.i
    elif t == NumericType.FLOAT:
      self._quantization = v.f
    else:
      raise Error(Result(Result.ERROR_INVALID_VALUE))
    return self._quantization

Distribution.Normal = NormalDistribution

class NormalFloatDistribution(NormalDistribution):

  def __init__(self, handle = None, retain = False, auto_release = True,
               mu = 0.0, sigma = 1.0, scale = ScaleType.LINEAR, quantization = 0.0):
    if handle is None:
      handle = ccs_distribution(0)
      res = ccs_create_normal_float_distribution(mu, sigma, scale, quantization, ct.byref(handle))
      Error.check(res)
      super().__init__(handle = handle, retain = False)
    else:
      super().__init__(handle = handle, retain = retain, auto_release = auto_release)

NormalDistribution.Float = NormalFloatDistribution

class NormalIntDistribution(NormalDistribution):

  def __init__(self, handle = None, retain = False, auto_release = True,
               mu = 0.0, sigma = 1.0, scale = ScaleType.LINEAR, quantization = 0):
    if handle is None:
      handle = ccs_distribution(0)
      res = ccs_create_normal_int_distribution(mu, sigma, scale, quantization, ct.byref(handle))
      Error.check(res)
      super().__init__(handle = handle, retain = False)
    else:
      super().__init__(handle = handle, retain = retain, auto_release = auto_release)

NormalDistribution.Int = NormalIntDistribution

ccs_create_roulette_distribution = _ccs_get_function("ccs_create_roulette_distribution", [ct.c_size_t, ct.POINTER(ccs_float), ct.POINTER(ccs_distribution)])
ccs_roulette_distribution_get_areas = _ccs_get_function("ccs_roulette_distribution_get_areas", [ccs_distribution, ct.c_size_t, ct.POINTER(ccs_float), ct.POINTER(ct.c_size_t)])

class RouletteDistribution(Distribution):
  def __init__(self, handle = None, retain = False, auto_release = True,
               areas = []):
    if handle is None:
      handle = ccs_distribution(0)
      v = (ccs_float * len(areas))(*areas)
      res = ccs_create_roulette_distribution(len(areas), v, ct.byref(handle))
      Error.check(res)
      super().__init__(handle = handle, retain = False)
    else:
      super().__init__(handle = handle, retain = retain, auto_release = auto_release)

  @property
  def data_type(self):
    if hasattr(self, "_data_type"):
      return self._data_type
    self._data_type = self.data_types[0]
    return self._data_type

  @property
  def num_areas(self):
    if hasattr(self, "_num_areas"):
      return self._num_areas
    v = ct.c_size_t()
    res = ccs_roulette_distribution_get_areas(self.handle, 0, None, ct.byref(v))
    Error.check(res)
    self._num_areas = v.value
    return self._num_areas

  @property
  def areas(self):
    if hasattr(self, "_areas"):
      return self._areas
    v = (ccs_float * self.num_areas)()
    res = ccs_roulette_distribution_get_areas(self.handle, self.num_areas, v, None)
    Error.check(res)
    self._areas = list(v)
    return self._areas

Distribution.Roulette = RouletteDistribution

ccs_create_mixture_distribution = _ccs_get_function("ccs_create_mixture_distribution", [ct.c_size_t, ct.POINTER(ccs_distribution), ct.POINTER(ccs_float), ct.POINTER(ccs_distribution)])
ccs_mixture_distribution_get_distributions = _ccs_get_function("ccs_mixture_distribution_get_distributions", [ccs_distribution, ct.c_size_t, ct.POINTER(ccs_distribution), ct.POINTER(ct.c_size_t)])
ccs_mixture_distribution_get_weights = _ccs_get_function("ccs_mixture_distribution_get_weights", [ccs_distribution, ct.c_size_t, ct.POINTER(ccs_float), ct.POINTER(ct.c_size_t)])

class MixtureDistribution(Distribution):
  def __init__(self, handle = None, retain = False, auto_release = True,
               distributions = [], weights = None):
    if handle is None:
      handle = ccs_distribution(0)
      if weights is None:
        weights = [1.0] * len(distributions)
      ws = (ccs_float * len(distributions))(*weights)
      ds = (ccs_distribution * len(distributions))(*[x.handle.value for x in distributions])
      res = ccs_create_mixture_distribution(len(distributions), ds, ws, ct.byref(handle))
      Error.check(res)
      super().__init__(handle = handle, retain = False)
    else:
      super().__init__(handle = handle, retain = retain, auto_release = auto_release)

  @property
  def num_distributions(self):
    if hasattr(self, "_num_distributions"):
      return self._num_distributions
    v = ct.c_size_t()
    res = ccs_mixture_distribution_get_distributions(self.handle, 0, None, ct.byref(v))
    Error.check(res)
    self._num_distributions = v.value
    return self._num_distributions

  @property
  def weights(self):
    if hasattr(self, "_weights"):
      return self._weights
    v = (ccs_float * self.num_distributions)()
    res = ccs_mixture_distribution_get_weights(self.handle, self.num_distributions, v, None)
    Error.check(res)
    self._weights = tuple(v)
    return self._weights

  @property
  def distributions(self):
    if hasattr(self, "_distributions"):
      return self._distributions
    v = (ccs_distribution * self.num_distributions)()
    res = ccs_mixture_distribution_get_distributions(self.handle, self.num_distributions, v, None)
    Error.check(res)
    self._distributions = tuple(Distribution.from_handle(ccs_distribution(x)) for x in v)
    return self._distributions

Distribution.Mixture = MixtureDistribution

ccs_create_multivariate_distribution = _ccs_get_function("ccs_create_multivariate_distribution", [ct.c_size_t, ct.POINTER(ccs_distribution), ct.POINTER(ccs_distribution)])
ccs_multivariate_distribution_get_distributions = _ccs_get_function("ccs_multivariate_distribution_get_distributions", [ccs_distribution, ct.c_size_t, ct.POINTER(ccs_distribution), ct.POINTER(ct.c_size_t)])

class MultivariateDistribution(Distribution):
  def __init__(self, handle = None, retain = False, auto_release = True,
               distributions = [], weights = None):
    if handle is None:
      handle = ccs_distribution(0)
      ds = (ccs_distribution * len(distributions))(*[x.handle.value for x in distributions])
      res = ccs_create_multivariate_distribution(len(distributions), ds, ct.byref(handle))
      Error.check(res)
      super().__init__(handle = handle, retain = False)
    else:
      super().__init__(handle = handle, retain = retain, auto_release = auto_release)

  @property
  def num_distributions(self):
    if hasattr(self, "_num_distributions"):
      return self._num_distributions
    v = ct.c_size_t()
    res = ccs_multivariate_distribution_get_distributions(self.handle, 0, None, ct.byref(v))
    Error.check(res)
    self._num_distributions = v.value
    return self._num_distributions

  @property
  def distributions(self):
    if hasattr(self, "_distributions"):
      return self._distributions
    v = (ccs_distribution * self.num_distributions)()
    res = ccs_multivariate_distribution_get_distributions(self.handle, self.num_distributions, v, None)
    Error.check(res)
    self._distributions = tuple(Distribution.from_handle(ccs_distribution(x)) for x in v)
    return self._distributions

Distribution.Multivariate = MultivariateDistribution
