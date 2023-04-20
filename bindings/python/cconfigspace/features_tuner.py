import ctypes as ct
from .base import Object, Error, CEnumeration, ccs_error, _ccs_get_function, ccs_context, ccs_parameter, ccs_configuration_space, ccs_configuration, ccs_features_space, ccs_features, ccs_datum, ccs_objective_space, ccs_features_evaluation, ccs_features_tuner, ccs_retain_object, _register_vector, _unregister_vector
from .context import Context
from .parameter import Parameter
from .configuration_space import ConfigurationSpace
from .configuration import Configuration
from .features_space import FeaturesSpace
from .features import Features
from .objective_space import ObjectiveSpace
from .features_evaluation import FeaturesEvaluation

class ccs_features_tuner_type(CEnumeration):
  _members_ = [
    ('RANDOM',0),
    'USER_DEFINED' ]

ccs_features_tuner_get_type = _ccs_get_function("ccs_features_tuner_get_type", [ccs_features_tuner, ct.POINTER(ccs_features_tuner_type)])
ccs_features_tuner_get_name = _ccs_get_function("ccs_features_tuner_get_name", [ccs_features_tuner, ct.POINTER(ct.c_char_p)])
ccs_features_tuner_get_configuration_space = _ccs_get_function("ccs_features_tuner_get_configuration_space", [ccs_features_tuner, ct.POINTER(ccs_configuration_space)])
ccs_features_tuner_get_objective_space = _ccs_get_function("ccs_features_tuner_get_objective_space", [ccs_features_tuner, ct.POINTER(ccs_objective_space)])
ccs_features_tuner_get_features_space = _ccs_get_function("ccs_features_tuner_get_features_space", [ccs_features_tuner, ct.POINTER(ccs_features_space)])
ccs_features_tuner_ask = _ccs_get_function("ccs_features_tuner_ask", [ccs_features_tuner, ccs_features, ct.c_size_t, ct.POINTER(ccs_configuration), ct.POINTER(ct.c_size_t)])
ccs_features_tuner_tell = _ccs_get_function("ccs_features_tuner_tell", [ccs_features_tuner, ct.c_size_t, ct.POINTER(ccs_features_evaluation)])
ccs_features_tuner_get_optimums = _ccs_get_function("ccs_features_tuner_get_optimums", [ccs_features_tuner, ccs_features, ct.c_size_t, ct.POINTER(ccs_features_evaluation), ct.POINTER(ct.c_size_t)])
ccs_features_tuner_get_history = _ccs_get_function("ccs_features_tuner_get_history", [ccs_features_tuner, ccs_features, ct.c_size_t, ct.POINTER(ccs_features_evaluation), ct.POINTER(ct.c_size_t)])
ccs_features_tuner_suggest = _ccs_get_function("ccs_features_tuner_suggest", [ccs_features_tuner, ccs_features, ct.POINTER(ccs_configuration)])

class FeaturesTuner(Object):
  @classmethod
  def from_handle(cls, handle, retain = True, auto_release = True):
    v = ccs_features_tuner_type(0)
    res = ccs_features_tuner_get_type(handle, ct.byref(v))
    Error.check(res)
    v = v.value
    if v == ccs_features_tuner_type.RANDOM:
      return RandomFeaturesTuner(handle = handle, retain = retain, auto_release = auto_release)
    elif v == ccs_features_tuner_type.USER_DEFINED:
      return UserDefinedFeaturesTuner(handle = handle, retain = retain, auto_release = auto_release)
    else:
      raise Error(ccs_error(ccs_error.INVALID_FEATURES_TUNER))

  @property
  def type(self):
    if hasattr(self, "_type"):
      return self._type
    v = ccs_features_tuner_type(0)
    res = ccs_features_tuner_get_type(self.handle, ct.byref(v))
    Error.check(res)
    self._type = v.value
    return self._type

  @property
  def name(self):
    if hasattr(self, "_name"):
      return self._name
    v = ct.c_char_p()
    res = ccs_features_tuner_get_name(self.handle, ct.byref(v))
    Error.check(res)
    self._name = v.value.decode()
    return self._name

  @property
  def objective_space(self):
    if hasattr(self, "_objective_space"):
      return self._objective_space
    v = ccs_objective_space()
    res = ccs_features_tuner_get_objective_space(self.handle, ct.byref(v))
    Error.check(res)
    self._objective_space = ObjectiveSpace.from_handle(v)
    return self._objective_space

  @property
  def features_space(self):
    if hasattr(self, "_features_space"):
      return self._features_space
    v = ccs_features_space()
    res = ccs_features_tuner_get_features_space(self.handle, ct.byref(v))
    Error.check(res)
    self._features_space = FeaturesSpace.from_handle(v)
    return self._features_space

  @property
  def configuration_space(self):
    if hasattr(self, "_configuration_space"):
      return self._configuration_space
    v = ccs_configuration_space()
    res = ccs_features_tuner_get_configuration_space(self.handle, ct.byref(v))
    Error.check(res)
    self._configuration_space = ConfigurationSpace.from_handle(v)
    return self._configuration_space

  def ask(self, features, count = 1):
    v = (ccs_configuration * count)()
    c = ct.c_size_t()
    res = ccs_features_tuner_ask(self.handle, features.handle, count, v, ct.byref(c))
    Error.check(res)
    count = c.value
    return [Configuration(handle = ccs_configuration(v[x]), retain = False) for x in range(count)]

  def tell(self, evaluations):
    count = len(evaluations)
    v = (ccs_features_evaluation * count)(*[x.handle.value for x in evaluations])
    res = ccs_features_tuner_tell(self.handle, count, v)
    Error.check(res)

  def history_size(self, features = None):
    v = ct.c_size_t()
    if features is not None:
      features = features.handle
    res = ccs_features_tuner_get_history(self.handle, features, 0, None, ct.byref(v))
    Error.check(res)
    return v.value

  def history(self, features = None):
    count = self.history_size(features)
    v = (ccs_features_evaluation * count)()
    if features is not None:
      features = features.handle
    res = ccs_features_tuner_get_history(self.handle, features, count, v, None)
    Error.check(res)
    return [FeaturesEvaluation.from_handle(ccs_features_evaluation(x)) for x in v]

  def num_optimums(self, features = None):
    v = ct.c_size_t()
    if features is not None:
      features = features.handle
    res = ccs_features_tuner_get_optimums(self.handle, features, 0, None, ct.byref(v))
    Error.check(res)
    return v.value

  def optimums(self, features = None):
    count = self.num_optimums(features)
    v = (ccs_features_evaluation * count)()
    if features is not None:
      features = features.handle
    res = ccs_features_tuner_get_optimums(self.handle, features, count, v, None)
    Error.check(res)
    return [FeaturesEvaluation.from_handle(ccs_features_evaluation(x)) for x in v]

  def suggest(self, features):
    config = ccs_configuration()
    res = ccs_features_tuner_suggest(self.handle, features.handle, ct.byref(config))
    Error.check(res)
    return Configuration(handle = config, retain = False)

ccs_create_random_features_tuner = _ccs_get_function("ccs_create_random_features_tuner", [ct.c_char_p, ccs_configuration_space, ccs_features_space, ccs_objective_space, ct.POINTER(ccs_features_tuner)])

class RandomFeaturesTuner(FeaturesTuner):
  def __init__(self, handle = None, retain = False, auto_release = True,
               name = None, configuration_space = None, features_space = None, objective_space = None):
    if handle is None:
      handle = ccs_features_tuner()
      res = ccs_create_random_features_tuner(str.encode(name), configuration_space.handle, features_space.handle, objective_space.handle, ct.byref(handle))
      Error.check(res)
      super().__init__(handle = handle, retain = False)
    else:
      super().__init__(handle = handle, retain = retain, auto_release = auto_release)


ccs_user_defined_features_tuner_del_type = ct.CFUNCTYPE(ccs_error, ccs_features_tuner)
ccs_user_defined_features_tuner_ask_type = ct.CFUNCTYPE(ccs_error, ccs_features_tuner, ccs_features, ct.c_size_t, ct.POINTER(ccs_configuration), ct.POINTER(ct.c_size_t))
ccs_user_defined_features_tuner_tell_type = ct.CFUNCTYPE(ccs_error, ccs_features_tuner, ct.c_size_t, ct.POINTER(ccs_features_evaluation))
ccs_user_defined_features_tuner_get_optimums_type = ct.CFUNCTYPE(ccs_error, ccs_features_tuner, ccs_features, ct.c_size_t, ct.POINTER(ccs_features_evaluation), ct.POINTER(ct.c_size_t))
ccs_user_defined_features_tuner_get_history_type = ct.CFUNCTYPE(ccs_error, ccs_features_tuner, ccs_features, ct.c_size_t, ct.POINTER(ccs_features_evaluation), ct.POINTER(ct.c_size_t))
ccs_user_defined_features_tuner_suggest_type = ct.CFUNCTYPE(ccs_error, ccs_features_tuner, ccs_features, ct.POINTER(ccs_configuration))
ccs_user_defined_features_tuner_serialize_type = ct.CFUNCTYPE(ccs_error, ccs_features_tuner, ct.c_size_t, ct.c_void_p, ct.POINTER(ct.c_size_t))
ccs_user_defined_features_tuner_deserialize_type = ct.CFUNCTYPE(ccs_error, ccs_features_tuner, ct.c_size_t, ct.POINTER(ccs_features_evaluation), ct.c_size_t, ct.POINTER(ccs_features_evaluation), ct.c_size_t, ct.c_void_p)

class ccs_user_defined_features_tuner_vector(ct.Structure):
  _fields_ = [
    ('delete', ccs_user_defined_features_tuner_del_type),
    ('ask', ccs_user_defined_features_tuner_ask_type),
    ('tell', ccs_user_defined_features_tuner_tell_type),
    ('get_optimums', ccs_user_defined_features_tuner_get_optimums_type),
    ('get_history', ccs_user_defined_features_tuner_get_history_type),
    ('suggest', ccs_user_defined_features_tuner_suggest_type),
    ('serialize', ccs_user_defined_features_tuner_serialize_type),
    ('deserialize', ccs_user_defined_features_tuner_deserialize_type) ]

ccs_create_user_defined_features_tuner = _ccs_get_function("ccs_create_user_defined_features_tuner", [ct.c_char_p, ccs_configuration_space, ccs_features_space, ccs_objective_space, ct.POINTER(ccs_user_defined_features_tuner_vector), ct.py_object, ct.POINTER(ccs_features_tuner)])
ccs_user_defined_features_tuner_get_tuner_data = _ccs_get_function("ccs_user_defined_features_tuner_get_tuner_data", [ccs_features_tuner, ct.POINTER(ct.c_void_p)])

def _wrap_user_defined_features_tuner_callbacks(delete, ask, tell, get_optimums, get_history, suggest, serialize, deserialize):
  def delete_wrapper(tun):
    try:
      tun = ct.cast(tun, ccs_features_tuner)
      delete(Object.from_handle(tun))
      _unregister_vector(tun)
      return ccs_error.SUCCESS
    except Exception as e:
      return Error.set_error(e)

  def ask_wrapper(tun, features, count, p_configurations, p_count):
    try:
      tun = ct.cast(tun, ccs_features_tuner)
      p_confs = ct.cast(p_configurations, ct.c_void_p)
      p_c = ct.cast(p_count, ct.c_void_p)
      (configurations, count_ret) = ask(FeaturesTuner.from_handle(tun), Features.from_handle(features), count if p_confs.value else None)
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
    except Exception as e:
      return Error.set_error(e)

  def tell_wrapper(tun, count, p_evaluations):
    try:
      tun = ct.cast(tun, ccs_features_tuner)
      if count == 0:
        return ccs_error.SUCCESS
      p_evals = ct.cast(p_evaluations, ct.c_void_p)
      if p_evals.value is None:
        raise Error(ccs_error(ccs_error.INVALID_VALUE))
      evals = [FeaturesEvaluation.from_handle(ccs_features_evaluation(p_evaluations[i])) for i in range(count)]
      tell(FeaturesTuner.from_handle(tun), evals)
      return ccs_error.SUCCESS
    except Exception as e:
      return Error.set_error(e)

  def get_optimums_wrapper(tun, features, count, p_evaluations, p_count):
    try:
      tun = ct.cast(tun, ccs_features_tuner)
      p_evals = ct.cast(p_evaluations, ct.c_void_p)
      p_c = ct.cast(p_count, ct.c_void_p)
      optimums = get_optimums(FeaturesTuner.from_handle(tun), Features.from_handle(features) if features else None)
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
    except Exception as e:
      return Error.set_error(e)

  def get_history_wrapper(tun, features, count, p_evaluations, p_count):
    try:
      tun = ct.cast(tun, ccs_features_tuner)
      p_evals = ct.cast(p_evaluations, ct.c_void_p)
      p_c = ct.cast(p_count, ct.c_void_p)
      history = get_history(FeaturesTuner.from_handle(tun), Features.from_handle(features) if features else None)
      count_ret = (len(history) if history else 0)
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
    except Exception as e:
      return Error.set_error(e)

  if suggest is not None:
    def suggest_wrapper(tun, features, p_configuration):
      try:
        tun = ct.cast(tun, ccs_features_tuner)
        configuration = suggest(FeaturesTuner.from_handle(tun), Features.from_handle(features))
        res = ccs_retain_object(configuration.handle)
        Error.check(res)
        p_configuration[0] = configuration.handle.value
        return ccs_error.SUCCESS
      except Exception as e:
        return Error.set_error(e)
  else:
    suggest_wrapper = 0

  if serialize is not None:
    def serialize_wrapper(tun, state_size, p_state, p_state_size):
      try:
        tun = ct.cast(tun, ccs_features_tuner)
        p_s = ct.cast(p_state, ct.c_void_p)
        p_sz = ct.cast(p_state_size, ct.c_void_p)
        state = serialize(FeaturesTuner.from_handle(tun), True if state_size == 0 else False)
        if p_s.value is not None and state_size < ct.sizeof(state):
          raise Error(ccs_error(ccs_error.INVALID_VALUE))
        if p_s.value is not None:
          ct.memmove(p_s, ct.byref(state), ct.sizeof(state))
        if p_sz.value is not None:
          p_state_size[0] = ct.sizeof(state)
        return ccs_error.SUCCESS
      except Exception as e:
        return Error.set_error(e)
  else:
    serialize_wrapper = 0

  if deserialize is not None:
    def deserialize_wrapper(tun, size_history, p_history, num_optimums, p_optimums, state_size, p_state):
      try:
        tun = ct.cast(tun, ccs_features_tuner)
        p_h = ct.cast(p_history, ct.c_void_p)
        p_o = ct.cast(p_optimums, ct.c_void_p)
        p_s = ct.cast(p_state, ct.c_void_p)
        if p_h.value is None:
          history = []
        else:
          history = [FeaturesEvaluation.from_handle(ccs_features_evaluation(p_h[i])) for i in range(size_history)]
        if p_o.value is None:
          optimums = []
        else:
          optimums = [FeaturesEvaluation.from_handle(ccs_features_evaluation(p_o[i])) for i in range(num_optimums)]
        if p_s.value is None:
          state = None
        else:
          state = ct.cast(p_s, POINTER(c_byte * state_size))
        deserialize(FeaturesTuner.from_handle(tun), history, optimums, state)
        return ccs_error.SUCCESS
      except Exception as e:
        return Error.set_error(e)
  else:
    deserialize_wrapper = 0

  return (delete_wrapper,
          ask_wrapper,
          tell_wrapper,
          get_optimums_wrapper,
          get_history_wrapper,
          suggest_wrapper,
          serialize_wrapper,
          deserialize_wrapper,
          ccs_user_defined_features_tuner_del_type(delete_wrapper),
          ccs_user_defined_features_tuner_ask_type(ask_wrapper),
          ccs_user_defined_features_tuner_tell_type(tell_wrapper),
          ccs_user_defined_features_tuner_get_optimums_type(get_optimums_wrapper),
          ccs_user_defined_features_tuner_get_history_type(get_history_wrapper),
          ccs_user_defined_features_tuner_suggest_type(suggest_wrapper),
          ccs_user_defined_features_tuner_serialize_type(serialize_wrapper),
          ccs_user_defined_features_tuner_deserialize_type(deserialize_wrapper))


class UserDefinedFeaturesTuner(FeaturesTuner):
  def __init__(self, handle = None, retain = False, auto_release = True,
               name = None, configuration_space = None, features_space = None, objective_space = None, delete = None, ask = None, tell = None, get_optimums = None, get_history = None, suggest = None, serialize = None, deserialize = None, tuner_data = None ):
    if handle is None:
      if delete is None or ask is None or tell is None or get_optimums is None or get_history is None:
        raise Error(ccs_error(ccs_error.INVALID_VALUE))

      (delete_wrapper,
       ask_wrapper,
       tell_wrapper,
       get_optimums_wrapper,
       get_history_wrapper,
       suggest_wrapper,
       serialize_wrapper,
       deserialize_wrapper,
       delete_wrapper_func,
       ask_wrapper_func,
       tell_wrapper_func,
       get_optimums_wrapper_func,
       get_history_wrapper_func,
       suggest_wrapper_func,
       serialize_wrapper_func,
       deserialize_wrapper_func) = _wrap_user_defined_features_tuner_callbacks(delete, ask, tell, get_optimums, get_history, suggest, serialize, deserialize)
      handle = ccs_features_tuner()
      vec = ccs_user_defined_features_tuner_vector()
      vec.delete = delete_wrapper_func
      vec.ask = ask_wrapper_func
      vec.tell = tell_wrapper_func
      vec.get_optimums = get_optimums_wrapper_func
      vec.get_history = get_history_wrapper_func
      vec.suggest = suggest_wrapper_func
      vec.serialize = serialize_wrapper_func
      vec.deserialize = deserialize_wrapper_func
      if tuner_data is not None:
        c_tuner_data = ct.py_object(tuner_data)
      else:
        c_tuner_data = None
      res = ccs_create_user_defined_features_tuner(str.encode(name), configuration_space.handle, features_space.handle, objective_space.handle, ct.byref(vec), c_tuner_data, ct.byref(handle))
      Error.check(res)
      super().__init__(handle = handle, retain = False)
      _register_vector(handle, [delete_wrapper, ask_wrapper, tell_wrapper, get_optimums_wrapper, get_history_wrapper, suggest_wrapper, serialize_wrapper, deserialize_wrapper, delete_wrapper_func, ask_wrapper_func, tell_wrapper_func, get_optimums_wrapper_func, get_history_wrapper_func, suggest_wrapper_func, serialize_wrapper_func, deserialize_wrapper_func, tuner_data])
    else:
      super().__init__(handle = handle, retain = retain, auto_release = auto_release)

  @classmethod
  def deserialize(cls, delete, ask, tell, get_optimums, get_history, suggest = None, serialize = None, deserialize = None, tuner_data = None, format = 'binary', handle_map = None, path = None, buffer = None, file_descriptor = None, callback = None, callback_data = None):
    if delete is None or ask is None or tell is None or get_optimums is None or get_history is None:
      raise Error(ccs_error(ccs_error.INVALID_VALUE))
    (delete_wrapper,
     ask_wrapper,
     tell_wrapper,
     get_optimums_wrapper,
     get_history_wrapper,
     suggest_wrapper,
     serialize_wrapper,
     deserialize_wrapper,
     delete_wrapper_func,
     ask_wrapper_func,
     tell_wrapper_func,
     get_optimums_wrapper_func,
     get_history_wrapper_func,
     suggest_wrapper_func,
     serialize_wrapper_func,
     deserialize_wrapper_func) = _wrap_user_defined_features_tuner_callbacks(delete, ask, tell, get_optimums, get_history, suggest, serialize, deserialize)
    vector = ccs_user_defined_features_tuner_vector()
    vector.delete = delete_wrapper_func
    vector.ask = ask_wrapper_func
    vector.tell = tell_wrapper_func
    vector.get_optimums = get_optimums_wrapper_func
    vector.get_history = get_history_wrapper_func
    vector.suggest = suggest_wrapper_func
    vector.serialize = serialize_wrapper_func
    vector.deserialize = deserialize_wrapper_func
    res = super().deserialize(format = format, handle_map = handle_map, vector = vector, data = tuner_data, path = path, buffer = buffer, file_descriptor = file_descriptor, callback = callback, callback_data = callback_data)
    _register_vector(res.handle, [delete_wrapper, ask_wrapper, tell_wrapper, get_optimums_wrapper, get_history_wrapper, suggest_wrapper, serialize_wrapper, deserialize_wrapper, delete_wrapper_func, ask_wrapper_func, tell_wrapper_func, get_optimums_wrapper_func, get_history_wrapper_func, suggest_wrapper_func, serialize_wrapper_func, deserialize_wrapper_func, tuner_data])
    return res

  @property
  def tuner_data(self):
    if hasattr(self, "_tuner_data"):
      return self._tuner_data
    v = ct.c_void_p()
    res = ccs_user_defined_features_tuner_get_tuner_data(self.handle, ct.byref(v))
    Error.check(res)
    if v:
      self._tuner_data = ct.cast(v, ct.py_object).value
    else:
      self._tuner_data = None
    return self._tuner_data


