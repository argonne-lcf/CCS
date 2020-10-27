import ctypes as ct
from .base import Object, Error, CEnumeration, ccs_error, ccs_result, _ccs_get_function, ccs_context, ccs_hyperparameter, ccs_configuration_space, ccs_configuration, ccs_datum, ccs_objective_space, ccs_evaluation, ccs_tuner, ccs_retain_object, _callbacks
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
  def from_handle(cls, handle, retain = True, auto_release = True):
    v = ccs_tuner_type(0)
    res = ccs_tuner_get_type(handle, ct.byref(v))
    Error.check(res)
    v = v.value
    if v == ccs_tuner_type.TUNER_RANDOM:
      return RandomTuner(handle = handle, retain = retain, auto_release = auto_release)
    elif v == ccs_tuner_type.TUNER_USER_DEFINED:
      return UserDefinedTuner(handle = handle, retain = retain, auto_release = auto_release)
    else:
      raise Error(ccs_error(ccs_error.INVALID_TUNER))

  @property
  def type(self):
    if hasattr(self, "_type"):
      return self._type
    v = ccs_tuner_type(0)
    res = ccs_tuner_get_type(self.handle, ct.byref(v))
    Error.check(res)
    self._type = v.value
    return self._type

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
    return [Configuration(handle = ccs_configuration(v[x]), retain = False) for x in range(count)]

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
  def __init__(self, handle = None, retain = False, auto_release = True,
               name = None, configuration_space = None, objective_space = None, user_data = None):
    if handle is None:
      handle = ccs_tuner()
      res = ccs_create_random_tuner(str.encode(name), configuration_space.handle, objective_space.handle, user_data, ct.byref(handle))
      Error.check(res)
      super().__init__(handle = handle, retain = False)
    else:
      super().__init__(handle = handle, retain = retain, auto_release = auto_release)


class ccs_tuner_common_data(ct.Structure):
  _fields_ = [
    ('type', ccs_tuner_type),
    ('name', ct.c_char_p),
    ('user_data', ct.c_void_p),
    ('configuration_space', ccs_configuration_space),
    ('objective_space', ccs_objective_space) ]

ccs_user_defined_tuner_del_type = ct.CFUNCTYPE(ccs_result, ccs_tuner)
ccs_user_defined_tuner_ask_type = ct.CFUNCTYPE(ccs_result, ccs_tuner, ct.c_size_t, ct.POINTER(ccs_configuration), ct.POINTER(ct.c_size_t))
ccs_user_defined_tuner_tell_type = ct.CFUNCTYPE(ccs_result, ccs_tuner, ct.c_size_t, ct.POINTER(ccs_evaluation))
ccs_user_defined_tuner_get_optimums_type = ct.CFUNCTYPE(ccs_result, ccs_tuner, ct.c_size_t, ct.POINTER(ccs_evaluation), ct.POINTER(ct.c_size_t))
ccs_user_defined_tuner_get_history_type = ct.CFUNCTYPE(ccs_result, ccs_tuner, ct.c_size_t, ct.POINTER(ccs_evaluation), ct.POINTER(ct.c_size_t))

class ccs_user_defined_tuner_vector(ct.Structure):
  _fields_ = [
    ('delete', ccs_user_defined_tuner_del_type),
    ('ask', ccs_user_defined_tuner_ask_type),
    ('tell', ccs_user_defined_tuner_tell_type),
    ('get_optimums', ccs_user_defined_tuner_get_optimums_type),
    ('get_history', ccs_user_defined_tuner_get_history_type) ]

ccs_create_user_defined_tuner = _ccs_get_function("ccs_create_user_defined_tuner", [ct.c_char_p, ccs_configuration_space, ccs_objective_space, ct.c_void_p, ct.POINTER(ccs_user_defined_tuner_vector), ct.c_void_p, ct.POINTER(ccs_tuner)])
ccs_user_defined_tuner_get_tuner_data = _ccs_get_function("ccs_user_defined_tuner_get_tuner_data", [ccs_tuner, ct.POINTER(ct.c_void_p)])

def _wrap_user_defined_callbacks(delete, ask, tell, get_optimums, get_history):
  ptr = ct.c_int(33)
  def delete_wrapper(tun):
    try:
      delete(Object.from_handle(tun))
      del _callbacks[ct.addressof(ptr)]
      return ccs_error.SUCCESS
    except Error as e:
      return -e.message.value

  def ask_wrapper(tun, count, p_configurations, p_count):
    try:
      p_confs = ct.cast(p_configurations, ct.c_void_p)
      p_c = ct.cast(p_count, ct.c_void_p)
      (configurations, count_ret) = ask(Tuner.from_handle(tun), count if p_confs.value else None)
      if p_confs.value is not None and count < count_ret:
        raise Error(ccs_error(ccs_error.INVALID_VALUE))
      if p_confs.value is not None:
        for i in range(len(configurations)):
          res = ccs_retain_object(configurations[i].handle)
          Error.check(res)
          p_configurations[i] = configurations[i].handle.value
        for i in range(len(configurations), count):
          p_configurations[i] = None
      if p_c.value is not None:
        p_count[0] = count_ret
      return ccs_error.SUCCESS
    except Error as e:
      return -e.message.value

  def tell_wrapper(tun, count, p_evaluations):
    try:
      if count == 0:
        return ccs_error.SUCCESS
      p_evals = ct.cast(p_evaluations, ct.c_void_p)
      if p_evals.value is None:
        raise Error(ccs_error(ccs_error.INVALID_VALUE))
      evals = [Evaluation.from_handle(ccs_evaluation(p_evaluations[i])) for i in range(count)]
      tell(Tuner.from_handle(tun), evals)
      return ccs_error.SUCCESS
    except Error as e:
      return -e.message.value

  def get_optimums_wrapper(tun, count, p_evaluations, p_count):
    try:
      p_evals = ct.cast(p_evaluations, ct.c_void_p)
      p_c = ct.cast(p_count, ct.c_void_p)
      optimums = get_optimums(Tuner.from_handle(tun))
      count_ret = len(optimums)
      if p_evals.value is not None and count < count_ret:
        raise Error(ccs_error(ccs_error.INVALID_VALUE))
      if p_evals.value is not None:
        for i in range(count_ret):
          p_evaluations[i] = optimums[i].handle.value
        for i in range(count_ret, count):
          p_evaluations[i] = None
      if p_c.value is not None:
          p_count[0] = count_ret
      return ccs_error.SUCCESS
    except Error as e:
      return -e.message.value

  def get_history_wrapper(tun, count, p_evaluations, p_count):
    try:
      p_evals = ct.cast(p_evaluations, ct.c_void_p)
      p_c = ct.cast(p_count, ct.c_void_p)
      history = get_history(Tuner.from_handle(tun))
      count_ret = len(history)
      if p_evals.value is not None and count < count_ret:
        raise Error(ccs_error(ccs_error.INVALID_VALUE))
      if p_evals.value is not None:
        for i in range(count_ret):
          p_evaluations[i] = history[i].handle.value
        for i in range(count_ret, count):
          p_evaluations[i] = None
      if p_c.value is not None:
          p_count[0] = count_ret
      return ccs_error.SUCCESS
    except Error as e:
      return -e.message.value

  return (ptr,
          delete_wrapper,
          ask_wrapper,
          tell_wrapper,
          get_optimums_wrapper,
          get_history_wrapper,
          ccs_user_defined_tuner_del_type(delete_wrapper),
          ccs_user_defined_tuner_ask_type(ask_wrapper),
          ccs_user_defined_tuner_tell_type(tell_wrapper),
          ccs_user_defined_tuner_get_optimums_type(get_optimums_wrapper),
          ccs_user_defined_tuner_get_history_type(get_history_wrapper))


class UserDefinedTuner(Tuner):
  def __init__(self, handle = None, retain = False, auto_release = True,
               name = None, configuration_space = None, objective_space = None, user_data = None, delete = None, ask = None, tell = None, get_optimums = None, get_history = None, tuner_data = None ):
    if handle is None:
      if delete is None or ask is None or tell is None or get_optimums is None or get_history is None:
        raise Error(ccs_error(ccs_error.INVALID_VALUE))

      (ptr,
       delete_wrapper,
       ask_wrapper,
       tell_wrapper,
       get_optimums_wrapper,
       get_history_wrapper,
       delete_wrapper_func,
       ask_wrapper_func,
       tell_wrapper_func,
       get_optimums_wrapper_func,
       get_history_wrapper_func) = _wrap_user_defined_callbacks(delete, ask, tell, get_optimums, get_history)
      handle = ccs_tuner()
      vec = ccs_user_defined_tuner_vector()
      vec.delete = delete_wrapper_func
      vec.ask = ask_wrapper_func
      vec.tell = tell_wrapper_func
      vec.get_optimums = get_optimums_wrapper_func
      vec.get_history = get_history_wrapper_func
      res = ccs_create_user_defined_tuner(str.encode(name), configuration_space.handle, objective_space.handle, user_data, ct.byref(vec), tuner_data, ct.byref(handle))
      Error.check(res)
      super().__init__(handle = handle, retain = False)
      _callbacks[ct.addressof(ptr)] = [ptr, delete_wrapper, ask_wrapper, tell_wrapper, get_optimums_wrapper, get_history_wrapper, delete_wrapper_func, ask_wrapper_func, tell_wrapper_func, get_optimums_wrapper_func, get_history_wrapper_func, user_data, tuner_data]
    else:
      super().__init__(handle = handle, retain = retain, auto_release = auto_release)

  @property
  def tuner_data(self):
    if hasattr(self, "_tuner_data"):
      return self._tuner_data
    v = ct.c_void_p()
    res = ccs_user_defined_tuner_get_tuner_data(self.handle, ct.byref(v))
    Error.check(res)
    self._tuner_data = v
    return v


