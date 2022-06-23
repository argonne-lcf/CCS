import ctypes as ct
from .base import Object, Error, CEnumeration, ccs_error, ccs_result, _ccs_get_function, ccs_context, ccs_hyperparameter, ccs_configuration_space, ccs_configuration, ccs_features_space, ccs_features, ccs_datum, ccs_datum_fix, ccs_objective_space, ccs_features_evaluation, ccs_bool
from .context import Context
from .hyperparameter import Hyperparameter
from .configuration_space import ConfigurationSpace
from .configuration import Configuration
from .features_space import FeaturesSpace
from .features import Features
from .objective_space import ObjectiveSpace
from .evaluation import ccs_comparison
from .binding import Binding

ccs_create_features_evaluation = _ccs_get_function("ccs_create_features_evaluation", [ccs_objective_space, ccs_configuration, ccs_features, ccs_result, ct.c_size_t, ct.POINTER(ccs_datum), ct.POINTER(ccs_features_evaluation)])
ccs_features_evaluation_get_objective_space = _ccs_get_function("ccs_features_evaluation_get_objective_space", [ccs_features_evaluation, ct.POINTER(ccs_objective_space)])
ccs_features_evaluation_get_configuration = _ccs_get_function("ccs_features_evaluation_get_configuration", [ccs_features_evaluation, ct.POINTER(ccs_configuration)])
ccs_features_evaluation_get_features = _ccs_get_function("ccs_features_evaluation_get_features", [ccs_features_evaluation, ct.POINTER(ccs_features)])
ccs_features_evaluation_get_error = _ccs_get_function("ccs_features_evaluation_get_error", [ccs_features_evaluation, ct.POINTER(ccs_result)])
ccs_features_evaluation_set_error = _ccs_get_function("ccs_features_evaluation_set_error", [ccs_features_evaluation, ccs_result])
ccs_features_evaluation_get_objective_value = _ccs_get_function("ccs_features_evaluation_get_objective_value", [ccs_features_evaluation, ct.c_size_t, ct.POINTER(ccs_datum)])
ccs_features_evaluation_get_objective_values = _ccs_get_function("ccs_features_evaluation_get_objective_values", [ccs_features_evaluation, ct.c_size_t, ct.POINTER(ccs_datum), ct.POINTER(ct.c_size_t)])
ccs_features_evaluation_compare = _ccs_get_function("ccs_features_evaluation_compare", [ccs_features_evaluation, ccs_features_evaluation, ct.POINTER(ccs_comparison)])
ccs_features_evaluation_check = _ccs_get_function("ccs_features_evaluation_check", [ccs_features_evaluation, ct.POINTER(ccs_bool)])

class FeaturesEvaluation(Binding):
  def __init__(self, handle = None, retain = False, auto_release = True,
               objective_space = None, configuration = None, features = None, error = ccs_error.SUCCESS, values = None):
    if handle is None:
      count = 0
      if values:
        count = len(values)
        vals = (ccs_datum * count)()
        ss = []
        for i in range(count):
          vals[i].set_value(values[i], string_store = ss)
      else:
        vals = None
      handle = ccs_features_evaluation()
      res = ccs_create_features_evaluation(objective_space.handle, configuration.handle, features.handle, error, count, vals, ct.byref(handle))
      Error.check(res)
      super().__init__(handle = handle, retain = False)
    else:
      super().__init__(handle = handle, retain = retain, auto_release = auto_release)

  @classmethod
  def from_handle(cls, handle, retain = True, auto_release = True):
    return cls(handle = handle, retain = retain, auto_release = auto_release)

  @property
  def objective_space(self):
    if hasattr(self, "_objective_space"):
      return self._objective_space
    v = ccs_objective_space()
    res = ccs_features_evaluation_get_objective_space(self.handle, ct.byref(v))
    Error.check(res)
    self._objective_space = ObjectiveSpace.from_handle(v)
    return self._objective_space

  @property
  def configuration(self):
    if hasattr(self, "_configuration"):
      return self._configuration
    v = ccs_configuration()
    res = ccs_features_evaluation_get_configuration(self.handle, ct.byref(v))
    Error.check(res)
    self._configuration = Configuration.from_handle(v)
    return self._configuration

  @property
  def features(self):
    if hasattr(self, "_features"):
      return self._features
    v = ccs_features()
    res = ccs_features_evaluation_get_features(self.handle, ct.byref(v))
    Error.check(res)
    self._features = Features.from_handle(v)
    return self._features

  @property
  def error(self):
    v = ccs_result()
    res = ccs_features_evaluation_get_error(self.handle, ct.byref(v)) 
    Error.check(res)
    return v.value

  @error.setter
  def error(self, v):
    res = ccs_features_evaluation_set_error(self.handle, v)
    Error.check(res)

  @property
  def num_objective_values(self):
    if hasattr(self, "_num_objective_values"):
      return self._num_objective_values
    v = ct.c_size_t()
    res = ccs_features_evaluation_get_objective_values(self.handle, 0, None, ct.byref(v))
    Error.check(res)
    self._num_objective_values = v.value
    return self._num_objective_values

  @property
  def objective_values(self):
    sz = self.num_objective_values
    if sz == 0:
      return []
    v = (ccs_datum * sz)()
    res = ccs_features_evaluation_get_objective_values(self.handle, sz, v, None)
    Error.check(res)
    return [x.value for x in v]

  def compare(self, other):
    v = ccs_comparison(0)
    res = ccs_features_evaluation_compare(self.handle, other.handle, ct.byref(v))
    Error.check(res)
    return v.value

  def check(self):
    valid = ccs_bool()
    res = res = ccs_features_evaluation(self.handle, ct.byref(valid))
    Error.check(res)
    return False if valid.value == 0 else True
