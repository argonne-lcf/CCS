import ctypes as ct
from .base import Object, Error, CEnumeration, ccs_error, ccs_result, _ccs_get_function, ccs_context, ccs_hyperparameter, ccs_configuration_space, ccs_configuration, ccs_datum, ccs_objective_space, ccs_evaluation, ccs_tuner
from .context import Context
from .hyperparameter import Hyperparameter
from .configuration_space import ConfigurationSpace
from .configuration import Configuration
from .objective_space import ObjectiveSpace
from .evaluation import Evaluation

class ccs_tuner_type(CEnumeration):
  _members_ = [
    ('TUNER_RANDOM',0),
    'TUNER_USER_DEFINED' ]

ccs_tuner_get_type = _ccs_get_function("ccs_tuner_get_type", [ccs_tuner, ct.POINTER(ccs_tuner_type)])
ccs_tuner_get_name = _ccs_get_function("ccs_tuner_get_name", [ccs_tuner, ct.POINTER(ct.c_char_p)])
ccs_tuner_get_user_data = _ccs_get_function("ccs_tuner_get_user_data", [ccs_tuner, ct.POINTER(ct.c_void_p)])
ccs_tuner_get_configuration_space = _ccs_get_function("ccs_tuner_get_configuration_space", [ccs_tuner, ct.POINTER(ccs_configuration_space)])
ccs_tuner_get_objective_space = _ccs_get_function("ccs_tuner_get_objective_space", [ccs_tuner, ct.POINTER(ccs_objective_space)])
ccs_tuner_ask = _ccs_get_function("ccs_tuner_ask", [ccs_tuner, ct.c_size_t, ct.POINTER(ccs_configuration), ct.POINTER(ct.c_size_t)])
ccs_tuner_tell = _ccs_get_function("ccs_tuner_tell", [ccs_tuner, ct.c_size_t, ct.POINTER(ccs_evaluation)])
ccs_tuner_get_optimums = _ccs_get_function("ccs_tuner_get_optimums", [ccs_tuner, ct.c_size_t, ct.POINTER(ccs_evaluation), ct.POINTER(ct.c_size_t)])
ccs_tuner_get_history = _ccs_get_function("ccs_tuner_get_history", [ccs_tuner, ct.c_size_t, ct.POINTER(ccs_evaluation), ct.POINTER(ct.c_size_t)])

class Tuner(Object):
  @classmethod
  def from_handle(cls, handle):
    v = ccs_tuner_type(0)
    res = ccs_tuner_get_type(handle, ct.byref(v))
    Error.check(res)
    v = v.value
    if v == ccs_tuner_type.RANDOM:
      return RandomTuner(handle = handle, retain = True)
    elif v == ccs_hyperparameter_type.USER_DEFIND:
      return UserDefinedTuner(handle = handle, retain = True)
    else:
      raise Error(ccs_error(ccs_error.INVALID_TUNER))

  @property
  def type(self):
    if hasattr(self, "_type"):
      return self._type
    v = ccs_tuner_type(0)
    res = ccs_tuner_get_type(self.handle, ct.byref(v))
    Error.check(res)
    self._type = v
    return v

  @property
  def user_data(self):
    if hasattr(self, "_user_data"):
      return self._user_data
    v = ct.c_void_p()
    res = ccs_tuner_get_user_data(self.handle, ct.byref(v))
    Error.check(res)
    self._user_data = v
    return v

  @property
  def name(self):
    if hasattr(self, "_name"):
      return self._name
    v = ct.c_char_p()
    res = ccs_tuner_get_name(self.handle, ct.byref(v))
    Error.check(res)
    self._name = v.value.decode()
    return self._name

  @property
  def objective_space(self):
    if hasattr(self, "_objective_space"):
      return self._objective_space
    v = ccs_objective_space()
    res = ccs_tuner_get_objective_space(self.handle, ct.byref(v))
    Error.check(res)
    self._objective_space = ObjectiveSpace.from_handle(v)
    return self._objective_space

  @property
  def configuration_space(self):
    if hasattr(self, "_configuration_space"):
      return self._configuration_space
    v = ccs_configuration_space()
    res = ccs_tuner_get_configuration_space(self.handle, ct.byref(v))
    Error.check(res)
    self._configuration_space = ConfigurationSpace.from_handle(v)
    return self._configuration_space

  def ask(self, count = 1):
    v = (ccs_configuration * count)()
    c = ct.c_size_t()
    res = ccs_tuner_ask(self.handle, count, v, ct.byref(c))
    Error.check(res)
    count = c.value
    return [Configuration.from_handle(ccs_configuration(v[x])) for x in range(count)]

  def tell(self, evaluations):
    count = len(evaluations)
    v = (ccs_evaluation * count)(*[x.handle.value for x in evaluations])
    res = ccs_tuner_tell(self.handle, count, v)
    Error.check(res)

  @property
  def history_size(self):
    v = ct.c_size_t()
    res = ccs_tuner_get_history(self.handle, 0, None, ct.byref(v))
    Error.check(res)
    return v.value

  @property
  def history(self):
    count = self.history_size
    v = (ccs_evaluation * count)()
    res = ccs_tuner_get_history(self.handle, count, v, None)
    Error.check(res)
    return [Evaluation.from_handle(ccs_evaluation(x)) for x in v]

  @property
  def num_optimums(self):
    v = ct.c_size_t()
    res = ccs_tuner_get_optimums(self.handle, 0, None, ct.byref(v))
    Error.check(res)
    return v.value

  @property
  def optimums(self):
    count = self.num_optimums
    v = (ccs_evaluation * count)()
    res = ccs_tuner_get_optimums(self.handle, count, v, None)
    Error.check(res)
    return [Evaluation.from_handle(ccs_evaluation(x)) for x in v]

ccs_create_random_tuner = _ccs_get_function("ccs_create_random_tuner", [ct.c_char_p, ccs_configuration_space, ccs_objective_space, ct.c_void_p, ct.POINTER(ccs_tuner)])

class RandomTuner(Tuner):
  def __init__(self, handle = None, retain = False, name = None, configuration_space = None, objective_space = None, user_data = None):
    if handle is None:
      handle = ccs_tuner()
      res = ccs_create_random_tuner(str.encode(name), configuration_space.handle, objective_space.handle, user_data, ct.byref(handle))
      Error.check(res)
      super().__init__(handle = handle, retain = False)
    else:
      super().__init__(handle = handle, retain = retain)

