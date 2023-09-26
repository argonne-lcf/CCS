import ctypes as ct
from .base import Object, Error, Result, _ccs_get_function, ccs_configuration_space, ccs_distribution, ccs_distribution_space
from .distribution import Distribution
from .parameter import Parameter
from .configuration_space import ConfigurationSpace

ccs_create_distribution_space = _ccs_get_function("ccs_create_distribution_space", [ccs_configuration_space, ct.POINTER(ccs_distribution_space)])
ccs_distribution_space_get_configuration_space = _ccs_get_function("ccs_distribution_space_get_configuration_space", [ccs_distribution_space, ct.POINTER(ccs_configuration_space)])
ccs_distribution_space_set_distribution = _ccs_get_function("ccs_distribution_space_set_distribution", [ccs_distribution_space, ccs_distribution, ct.POINTER(ct.c_size_t)])
ccs_distribution_space_get_parameter_distribution = _ccs_get_function("ccs_distribution_space_get_parameter_distribution", [ccs_distribution_space, ct.c_size_t, ct.POINTER(ccs_distribution), ct.POINTER(ct.c_size_t)])

class DistributionSpace(Object):

  @property
  def configuration_space(self):
    if hasattr(self, "_configuration_space"):
      return self._configuration_space
    v = ccs_configuration_space()
    res = ccs_distribution_space_get_configuration_space(self.handle, ct.byref(v))
    Error.check(res)
    self._configuration_space = ConfigurationSpace.from_handle(v)
    return self._configuration_space

  def __init__(self, handle = None, retain = False, auto_release = True,
               configuration_space = None):
    if handle is None:
      handle = ccs_distribution_space()
      res = ccs_create_distribution_space(configuration_space.handle, ct.byref(handle))
      Error.check(res)
      super().__init__(handle = handle, retain = False)
    else:
      super().__init__(handle = handle, retain = retain, auto_release = auto_release)

  def set_distribution(self, distribution, parameters):
    count = distribution.dimension
    if count != len(parameters):
        raise Error(Result(Result.ERROR_INVALID_VALUE))
    hyps = []
    for h in parameters:
      if isinstance(h, Parameter):
        hyps.append(self.configuration_space.parameter_index(h))
      elif isinstance(h, str):
        hyps.append(self.configuration_space.parameter_index_by_name(h))
      else:
        hyps.append(h)
    v = (ct.c_size_t * count)(*hyps)
    res = ccs_distribution_space_set_distribution(self.handle, distribution.handle, v)
    Error.check(res)

  def get_parameter_distribution(self, parameter):
    if isinstance(parameter, Parameter):
      parameter = self.configuration_space.parameter_index(parameter)
    elif isinstance(parameter, str):
      parameter = self.configuration_space.parameter_index_by_name(parameter)
    v1 = ccs_distribution()
    v2 = ct.c_size_t()
    res = ccs_distribution_space_get_parameter_distribution(self.handle, parameter, ct.byref(v1), ct.byref(v2))
    Error.check(res)
    return [Distribution.from_handle(v1), v2.value]


