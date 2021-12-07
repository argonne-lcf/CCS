import ctypes as ct
from .base import Object, Error, ccs_error, _ccs_get_function, ccs_context, ccs_hyperparameter, ccs_configuration_space, ccs_configuration, ccs_rng, ccs_distribution, ccs_expression, ccs_datum, ccs_hash, ccs_int
from .context import Context
from .rng import Rng
from .hyperparameter import Hyperparameter
from .configuration_space import ConfigurationSpace
from .binding import Binding

ccs_create_configuration = _ccs_get_function("ccs_create_configuration", [ccs_configuration_space, ct.c_size_t, ct.POINTER(ccs_datum), ct.c_void_p, ct.POINTER(ccs_configuration)])
ccs_configuration_get_configuration_space = _ccs_get_function("ccs_configuration_get_configuration_space", [ccs_configuration, ct.POINTER(ccs_configuration_space)])
ccs_configuration_check = _ccs_get_function("ccs_configuration_check", [ccs_configuration])

class Configuration(Binding):
  """Class which represents a configuration of hyperparameters.

  Args:
    handle ([type], optional): [description]. Defaults to None.
    retain (bool, optional): [description]. Defaults to False.
    auto_release (bool, optional): [description]. Defaults to True.
    configuration_space ([type], optional): [description]. Defaults to None.
    values ([type], optional): [description]. Defaults to None.
    user_data ([type], optional): [description]. Defaults to None.
  """

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
  def configuration_space(self):
    """Returns the configuration space which sampled the current configuration.
    """
    if hasattr(self, "_configuration_space"):
      return self._configuration_space
    v = ccs_configuration_space()
    res = ccs_configuration_get_configuration_space(self.handle, ct.byref(v))
    Error.check(res)
    self._configuration_space = ConfigurationSpace.from_handle(v)
    return self._configuration_space

  def check(self):
    """Check the validity of the current configuration based on the generative configuration space.
    """
    res = ccs_configuration_check(self.handle)
    Error.check(res)

  @property
  def hyperparameters(self):
    """Returns the list of hyperparameters of the current configuration."""
    return self.configuration_space.hyperparameters

  def __getitem__(self, idx):
    if type(idx) is int:
      return self.values[idx]
    elif type(idx) is str:
      return self.value(idx)
    else:
      raise ValueError("index should be str or int")

  def to_list(self):
    """Returns a list copy of hyperparameter values of the current configuration."""
    return self.values[:]

  def to_dict(self):
    """Returns a dict copy of the current configuration."""
    return {hp.name:hp_value for hp, hp_value in zip(self.hyperparameters, self.values)}
