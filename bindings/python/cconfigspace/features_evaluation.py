import ctypes as ct
from .base import Error, Datum, Result, _ccs_get_function, ccs_configuration, ccs_features, ccs_features_evaluation, ccs_objective_space, ccs_evaluation_result
from .evaluation_binding import EvaluationBinding
from .configuration import Configuration
from .features import Features

ccs_create_features_evaluation = _ccs_get_function("ccs_create_features_evaluation", [ccs_objective_space, ccs_configuration, ccs_features, ccs_evaluation_result, ct.c_size_t, ct.POINTER(Datum), ct.POINTER(ccs_features_evaluation)])
ccs_features_evaluation_get_configuration = _ccs_get_function("ccs_features_evaluation_get_configuration", [ccs_features_evaluation, ct.POINTER(ccs_configuration)])
ccs_features_evaluation_get_features = _ccs_get_function("ccs_features_evaluation_get_features", [ccs_features_evaluation, ct.POINTER(ccs_features)])

class FeaturesEvaluation(EvaluationBinding):
  def __init__(self, handle = None, retain = False, auto_release = True,
               objective_space = None, configuration = None, features = None, result = Result.SUCCESS, values = None):
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
      handle = ccs_features_evaluation()
      res = ccs_create_features_evaluation(objective_space.handle, configuration.handle, features.handle, result, count, vals, ct.byref(handle))
      Error.check(res)
      super().__init__(handle = handle, retain = False)
    else:
      super().__init__(handle = handle, retain = retain, auto_release = auto_release)

  @classmethod
  def from_handle(cls, handle, retain = True, auto_release = True):
    return cls(handle = handle, retain = retain, auto_release = auto_release)

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
