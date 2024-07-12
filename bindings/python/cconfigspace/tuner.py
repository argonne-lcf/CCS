import ctypes as ct
import sys
from .base import Object, Error, CEnumeration, Result, _ccs_get_function, ccs_context, ccs_parameter, ccs_search_space, ccs_search_configuration, ccs_feature_space, ccs_features, Datum, ccs_objective_space, ccs_evaluation, ccs_tuner, ccs_retain_object
from .context import Context
from .parameter import Parameter
from .features import Features
from .feature_space import FeatureSpace
from .objective_space import ObjectiveSpace
from .evaluation import Evaluation

class TunerType(CEnumeration):
  _members_ = [
    ('RANDOM',0),
    'USER_DEFINED' ]

ccs_tuner_get_type = _ccs_get_function("ccs_tuner_get_type", [ccs_tuner, ct.POINTER(TunerType)])
ccs_tuner_get_name = _ccs_get_function("ccs_tuner_get_name", [ccs_tuner, ct.POINTER(ct.c_char_p)])
ccs_tuner_get_search_space = _ccs_get_function("ccs_tuner_get_search_space", [ccs_tuner, ct.POINTER(ccs_search_space)])
ccs_tuner_get_objective_space = _ccs_get_function("ccs_tuner_get_objective_space", [ccs_tuner, ct.POINTER(ccs_objective_space)])
ccs_tuner_get_feature_space = _ccs_get_function("ccs_tuner_get_feature_space", [ccs_tuner, ct.POINTER(ccs_feature_space)])
ccs_tuner_ask = _ccs_get_function("ccs_tuner_ask", [ccs_tuner, ccs_features, ct.c_size_t, ct.POINTER(ccs_search_configuration), ct.POINTER(ct.c_size_t)])
ccs_tuner_tell = _ccs_get_function("ccs_tuner_tell", [ccs_tuner, ct.c_size_t, ct.POINTER(ccs_evaluation)])
ccs_tuner_get_optima = _ccs_get_function("ccs_tuner_get_optima", [ccs_tuner, ccs_features, ct.c_size_t, ct.POINTER(ccs_evaluation), ct.POINTER(ct.c_size_t)])
ccs_tuner_get_history = _ccs_get_function("ccs_tuner_get_history", [ccs_tuner, ccs_features, ct.c_size_t, ct.POINTER(ccs_evaluation), ct.POINTER(ct.c_size_t)])
ccs_tuner_suggest = _ccs_get_function("ccs_tuner_suggest", [ccs_tuner, ccs_features, ct.POINTER(ccs_search_configuration)])

class Tuner(Object):
  @classmethod
  def from_handle(cls, handle, retain = True, auto_release = True):
    v = TunerType(0)
    res = ccs_tuner_get_type(handle, ct.byref(v))
    Error.check(res)
    v = v.value
    if v == TunerType.RANDOM:
      return RandomTuner(handle = handle, retain = retain, auto_release = auto_release)
    elif v == TunerType.USER_DEFINED:
      return UserDefinedTuner(handle = handle, retain = retain, auto_release = auto_release)
    else:
      raise Error(Result(Result.ERROR_INVALID_TUNER))

  @property
  def type(self):
    if hasattr(self, "_type"):
      return self._type
    v = TunerType(0)
    res = ccs_tuner_get_type(self.handle, ct.byref(v))
    Error.check(res)
    self._type = v.value
    return self._type

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
  def feature_space(self):
    if hasattr(self, "_feature_space"):
      return self._feature_space
    v = ccs_feature_space()
    res = ccs_tuner_get_feature_space(self.handle, ct.byref(v))
    Error.check(res)
    self._feature_space = FeatureSpace.from_handle(v)
    return self._feature_space

  @property
  def search_space(self):
    if hasattr(self, "_search_space"):
      return self._search_space
    v = ccs_search_space()
    res = ccs_tuner_get_search_space(self.handle, ct.byref(v))
    Error.check(res)
    self._search_space = Object.from_handle(v)
    return self._search_space

  def ask(self, count = 1, features = None):
    v = (ccs_search_configuration * count)()
    c = ct.c_size_t()
    if features is not None:
      features = features.handle
    res = ccs_tuner_ask(self.handle, features, count, v, ct.byref(c))
    Error.check(res)
    count = c.value
    return [Object.from_handle(ccs_search_configuration(v[x]), retain = False) for x in range(count)]

  def tell(self, evaluations):
    count = len(evaluations)
    v = (ccs_evaluation * count)(*[x.handle.value for x in evaluations])
    res = ccs_tuner_tell(self.handle, count, v)
    Error.check(res)

  def history_size(self, features = None):
    v = ct.c_size_t()
    if features is not None:
      features = features.handle
    res = ccs_tuner_get_history(self.handle, features, 0, None, ct.byref(v))
    Error.check(res)
    return v.value

  def history(self, features = None):
    count = self.history_size(features)
    v = (ccs_evaluation * count)()
    if features is not None:
      features = features.handle
    res = ccs_tuner_get_history(self.handle, features, count, v, None)
    Error.check(res)
    return [Evaluation.from_handle(ccs_evaluation(x)) for x in v]

  def num_optima(self, features = None):
    v = ct.c_size_t()
    if features is not None:
      features = features.handle
    res = ccs_tuner_get_optima(self.handle, features, 0, None, ct.byref(v))
    Error.check(res)
    return v.value

  def optima(self, features = None):
    count = self.num_optima(features)
    v = (ccs_evaluation * count)()
    if features is not None:
      features = features.handle
    res = ccs_tuner_get_optima(self.handle, features, count, v, None)
    Error.check(res)
    return [Evaluation.from_handle(ccs_evaluation(x)) for x in v]

  def suggest(self, features = None):
    config = ccs_search_configuration()
    if features is not None:
      features = features.handle
    res = ccs_tuner_suggest(self.handle, features, ct.byref(config))
    Error.check(res)
    return Object.from_handle(config, retain = False)

ccs_create_random_tuner = _ccs_get_function("ccs_create_random_tuner", [ct.c_char_p, ccs_objective_space, ct.POINTER(ccs_tuner)])

class RandomTuner(Tuner):
  def __init__(self, handle = None, retain = False, auto_release = True,
               name = "", objective_space = None):
    if handle is None:
      handle = ccs_tuner()
      res = ccs_create_random_tuner(str.encode(name), objective_space.handle, ct.byref(handle))
      Error.check(res)
      super().__init__(handle = handle, retain = False)
    else:
      super().__init__(handle = handle, retain = retain, auto_release = auto_release)

Tuner.Random = RandomTuner

ccs_user_defined_tuner_del_type = ct.CFUNCTYPE(Result, ccs_tuner)
ccs_user_defined_tuner_ask_type = ct.CFUNCTYPE(Result, ccs_tuner, ccs_features, ct.c_size_t, ct.POINTER(ccs_search_configuration), ct.POINTER(ct.c_size_t))
ccs_user_defined_tuner_tell_type = ct.CFUNCTYPE(Result, ccs_tuner, ct.c_size_t, ct.POINTER(ccs_evaluation))
ccs_user_defined_tuner_get_optima_type = ct.CFUNCTYPE(Result, ccs_tuner, ccs_features, ct.c_size_t, ct.POINTER(ccs_evaluation), ct.POINTER(ct.c_size_t))
ccs_user_defined_tuner_get_history_type = ct.CFUNCTYPE(Result, ccs_tuner, ccs_features, ct.c_size_t, ct.POINTER(ccs_evaluation), ct.POINTER(ct.c_size_t))
ccs_user_defined_tuner_suggest_type = ct.CFUNCTYPE(Result, ccs_tuner, ccs_features, ct.POINTER(ccs_search_configuration))
ccs_user_defined_tuner_serialize_type = ct.CFUNCTYPE(Result, ccs_tuner, ct.c_size_t, ct.c_void_p, ct.POINTER(ct.c_size_t))
ccs_user_defined_tuner_deserialize_type = ct.CFUNCTYPE(Result, ccs_objective_space, ct.c_size_t, ct.POINTER(ccs_evaluation), ct.c_size_t, ct.POINTER(ccs_evaluation), ct.c_size_t, ct.c_void_p, ct.POINTER(ct.c_void_p))

class UserDefinedTunerVector(ct.Structure):
  _fields_ = [
    ('delete', ccs_user_defined_tuner_del_type),
    ('ask', ccs_user_defined_tuner_ask_type),
    ('tell', ccs_user_defined_tuner_tell_type),
    ('get_optima', ccs_user_defined_tuner_get_optima_type),
    ('get_history', ccs_user_defined_tuner_get_history_type),
    ('suggest', ccs_user_defined_tuner_suggest_type),
    ('serialize', ccs_user_defined_tuner_serialize_type),
    ('deserialize', ccs_user_defined_tuner_deserialize_type) ]

ccs_create_user_defined_tuner = _ccs_get_function("ccs_create_user_defined_tuner", [ct.c_char_p, ccs_objective_space, ct.POINTER(UserDefinedTunerVector), ct.py_object, ct.POINTER(ccs_tuner)])
ccs_user_defined_tuner_get_tuner_data = _ccs_get_function("ccs_user_defined_tuner_get_tuner_data", [ccs_tuner, ct.POINTER(ct.c_void_p)])

def _wrap_user_defined_tuner_callbacks(delete, ask, tell, get_optima, get_history, suggest, serialize, deserialize):
  vec = UserDefinedTunerVector()
  def delete_wrapper(tun):
    try:
      tun = ct.cast(tun, ccs_tuner)
      o = Object.from_handle(tun)
      tdata = o.tuner_data
      if delete is not None:
        delete(o)
      if tdata is not None:
        ct.pythonapi.Py_DecRef(ct.py_object(tdata))
      ct.pythonapi.Py_DecRef(ct.py_object(vec))
      return Result.SUCCESS
    except Exception as e:
      return Error.set_error(e)

  def ask_wrapper(tun, features, count, p_configurations, p_count):
    try:
      tun = ct.cast(tun, ccs_tuner)
      p_confs = ct.cast(p_configurations, ct.c_void_p)
      p_c = ct.cast(p_count, ct.c_void_p)
      (configurations, count_ret) = ask(Tuner.from_handle(tun), Features.from_handle(features) if features else None, count if p_confs.value else None)
      if p_confs.value is not None and count < count_ret:
        raise Error(Result(Result.ERROR_INVALID_VALUE))
      if p_confs.value is not None:
        for i in range(len(configurations)):
          res = ccs_retain_object(configurations[i].handle)
          Error.check(res)
          p_configurations[i] = configurations[i].handle.value
        for i in range(len(configurations), count):
          p_configurations[i] = None
      if p_c.value is not None:
        p_count[0] = count_ret
      return Result.SUCCESS
    except Exception as e:
      return Error.set_error(e)

  def tell_wrapper(tun, count, p_evaluations):
    try:
      tun = ct.cast(tun, ccs_tuner)
      if count == 0:
        return Result.SUCCESS
      p_evals = ct.cast(p_evaluations, ct.c_void_p)
      if p_evals.value is None:
        raise Error(Result(Result.ERROR_INVALID_VALUE))
      evals = [Evaluation.from_handle(ccs_evaluation(p_evaluations[i])) for i in range(count)]
      tell(Tuner.from_handle(tun), evals)
      return Result.SUCCESS
    except Exception as e:
      return Error.set_error(e)

  def get_optima_wrapper(tun, features, count, p_evaluations, p_count):
    try:
      tun = ct.cast(tun, ccs_tuner)
      p_evals = ct.cast(p_evaluations, ct.c_void_p)
      p_c = ct.cast(p_count, ct.c_void_p)
      optima = get_optima(Tuner.from_handle(tun), Features.from_handle(features) if features else None)
      count_ret = len(optima)
      if p_evals.value is not None and count < count_ret:
        raise Error(Result(Result.ERROR_INVALID_VALUE))
      if p_evals.value is not None:
        for i in range(count_ret):
          p_evaluations[i] = optima[i].handle.value
        for i in range(count_ret, count):
          p_evaluations[i] = None
      if p_c.value is not None:
          p_count[0] = count_ret
      return Result.SUCCESS
    except Exception as e:
      return Error.set_error(e)

  def get_history_wrapper(tun, features, count, p_evaluations, p_count):
    try:
      tun = ct.cast(tun, ccs_tuner)
      p_evals = ct.cast(p_evaluations, ct.c_void_p)
      p_c = ct.cast(p_count, ct.c_void_p)
      history = get_history(Tuner.from_handle(tun), Features.from_handle(features) if features else None)
      count_ret = (len(history) if history else 0)
      if p_evals.value is not None and count < count_ret:
        raise Error(Result(Result.ERROR_INVALID_VALUE))
      if p_evals.value is not None:
        for i in range(count_ret):
          p_evaluations[i] = history[i].handle.value
        for i in range(count_ret, count):
          p_evaluations[i] = None
      if p_c.value is not None:
          p_count[0] = count_ret
      return Result.SUCCESS
    except Exception as e:
      return Error.set_error(e)

  if suggest is not None:
    def suggest_wrapper(tun, features, p_configuration):
      try:
        tun = ct.cast(tun, ccs_tuner)
        configuration = suggest(Tuner.from_handle(tun), Features.from_handle(features) if features else None)
        res = ccs_retain_object(configuration.handle)
        Error.check(res)
        p_configuration[0] = configuration.handle.value
        return Result.SUCCESS
      except Exception as e:
        return Error.set_error(e)
  else:
    suggest_wrapper = 0

  if serialize is not None:
    def serialize_wrapper(tun, state_size, p_state, p_state_size):
      try:
        tun = ct.cast(tun, ccs_tuner)
        p_s = ct.cast(p_state, ct.c_void_p)
        p_sz = ct.cast(p_state_size, ct.c_void_p)
        state = serialize(Tuner.from_handle(tun), True if state_size == 0 else False)
        if p_s.value is not None and state_size < ct.sizeof(state):
          raise Error(Result(Result.ERROR_INVALID_VALUE))
        if p_s.value is not None:
          ct.memmove(p_s, ct.byref(state), ct.sizeof(state))
        if p_sz.value is not None:
          p_state_size[0] = ct.sizeof(state)
        return Result.SUCCESS
      except Exception as e:
        return Error.set_error(e)
  else:
    serialize_wrapper = 0

  if deserialize is not None:
    def deserialize_wrapper(o_space, size_history, p_history, num_optima, p_optima, state_size, p_state, p_tuner_data):
      try:
        o_space = ct.cast(o_space, ccs_objective_space)
        p_h = ct.cast(p_history, ct.c_void_p)
        p_o = ct.cast(p_optima, ct.c_void_p)
        p_s = ct.cast(p_state, ct.c_void_p)
        p_t = ct.cast(p_tuner_data, ct.c_void_p)
        if p_h.value is None:
          history = []
        else:
          history = [Evaluation.from_handle(ccs_evaluation(p_h[i])) for i in range(size_history)]
        if p_o.value is None:
          optima = []
        else:
          optima = [Evaluation.from_handle(ccs_evaluation(p_o[i])) for i in range(num_optima)]
        if p_s.value is None:
          state = None
        else:
          state = ct.cast(p_s, POINTER(c_byte * state_size))
        tuner_data = deserialize(ObjectiveSpace.from_handle(o_space), history, optima, state)
        c_tuner_data = ct.py_object(tuner_data)
        p_t[0] = c_tuner_data
        ct.pythonapi.Py_IncRef(c_tuner_data)
        return Result.SUCCESS
      except Exception as e:
        return Error.set_error(e)
  else:
    deserialize_wrapper = 0

  delete_wrapper_func      = ccs_user_defined_tuner_del_type(delete_wrapper)
  ask_wrapper_func         = ccs_user_defined_tuner_ask_type(ask_wrapper)
  tell_wrapper_func        = ccs_user_defined_tuner_tell_type(tell_wrapper)
  get_optima_wrapper_func  = ccs_user_defined_tuner_get_optima_type(get_optima_wrapper)
  get_history_wrapper_func = ccs_user_defined_tuner_get_history_type(get_history_wrapper)
  suggest_wrapper_func     = ccs_user_defined_tuner_suggest_type(suggest_wrapper)
  serialize_wrapper_func   = ccs_user_defined_tuner_serialize_type(serialize_wrapper)
  deserialize_wrapper_func = ccs_user_defined_tuner_deserialize_type(deserialize_wrapper)
  vec.delete = delete_wrapper_func
  vec.ask = ask_wrapper_func
  vec.tell = tell_wrapper_func
  vec.get_optima = get_optima_wrapper_func
  vec.get_history = get_history_wrapper_func
  vec.suggest = suggest_wrapper_func
  vec.serialize = serialize_wrapper_func
  vec.deserialize = deserialize_wrapper_func

  setattr(vec, '_wrappers', (
    delete_wrapper,
    ask_wrapper,
    tell_wrapper,
    get_optima_wrapper,
    get_history_wrapper,
    suggest_wrapper,
    serialize_wrapper,
    deserialize_wrapper,
    delete_wrapper_func,
    ask_wrapper_func,
    tell_wrapper_func,
    get_optima_wrapper_func,
    get_history_wrapper_func,
    suggest_wrapper_func,
    serialize_wrapper_func,
    deserialize_wrapper_func))
  return vec


class UserDefinedTuner(Tuner):
  def __init__(self, handle = None, retain = False, auto_release = True,
               name = "", objective_space = None, delete = None, ask = None, tell = None, get_optima = None, get_history = None, suggest = None, serialize = None, deserialize = None, tuner_data = None ):
    if handle is None:
      if ask is None or tell is None or get_optima is None or get_history is None:
        raise Error(Result(Result.ERROR_INVALID_VALUE))

      vec = _wrap_user_defined_tuner_callbacks(delete, ask, tell, get_optima, get_history, suggest, serialize, deserialize)
      if tuner_data is not None:
        c_tuner_data = ct.py_object(tuner_data)
      else:
        c_tuner_data = None
      handle = ccs_tuner()
      res = ccs_create_user_defined_tuner(str.encode(name), objective_space.handle, ct.byref(vec), c_tuner_data, ct.byref(handle))
      Error.check(res)
      super().__init__(handle = handle, retain = False)
      ct.pythonapi.Py_IncRef(ct.py_object(vec))
      if c_tuner_data is not None:
        ct.pythonapi.Py_IncRef(c_tuner_data)
    else:
      super().__init__(handle = handle, retain = retain, auto_release = auto_release)

  @classmethod
  def deserialize(cls, delete, ask, tell, get_optima, get_history, suggest = None, serialize = None, deserialize = None, tuner_data = None, format = 'binary', handle_map = None, path = None, buffer = None, file_descriptor = None, callback = None, callback_data = None):
    if ask is None or tell is None or get_optima is None or get_history is None:
      raise Error(Result(Result.ERROR_INVALID_VALUE))
    vec = _wrap_user_defined_tuner_callbacks(delete, ask, tell, get_optima, get_history, suggest, serialize, deserialize)
    res = super().deserialize(format = format, handle_map = handle_map, vector = vec, data = tuner_data, path = path, buffer = buffer, file_descriptor = file_descriptor, callback = callback, callback_data = callback_data)
    ct.pythonapi.Py_IncRef(ct.py_object(vec))
    if tuner_data is not None:
      ct.pythonapi.Py_IncRef(ct.py_object(tuner_data))
    return res

  @property
  def tuner_data(self):
    if hasattr(self, "_tuner_data"):
      return self._tuner_data
    v = ct.c_void_p()
    res = ccs_user_defined_tuner_get_tuner_data(self.handle, ct.byref(v))
    Error.check(res)
    if v:
      self._tuner_data = ct.cast(v, ct.py_object).value
    else:
      self._tuner_data = None
    return self._tuner_data

Tuner.UserDefined = UserDefinedTuner
