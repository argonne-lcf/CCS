import ctypes as ct
from .base import Object, Error, CEnumeration, Result, _ccs_get_function, ccs_context, ccs_parameter, ccs_tree_space, ccs_tree_configuration, Datum, ccs_objective_space, ccs_tree_evaluation, ccs_tree_tuner, ccs_retain_object, _register_vector, _unregister_vector
from .context import Context
from .parameter import Parameter
from .tree_space import TreeSpace
from .tree_configuration import TreeConfiguration
from .objective_space import ObjectiveSpace
from .tree_evaluation import TreeEvaluation

class TreeTunerType(CEnumeration):
  _members_ = [
    ('RANDOM',0),
    'USER_DEFINED' ]

ccs_tree_tuner_get_type = _ccs_get_function("ccs_tree_tuner_get_type", [ccs_tree_tuner, ct.POINTER(TreeTunerType)])
ccs_tree_tuner_get_name = _ccs_get_function("ccs_tree_tuner_get_name", [ccs_tree_tuner, ct.POINTER(ct.c_char_p)])
ccs_tree_tuner_get_tree_space = _ccs_get_function("ccs_tree_tuner_get_tree_space", [ccs_tree_tuner, ct.POINTER(ccs_tree_space)])
ccs_tree_tuner_get_objective_space = _ccs_get_function("ccs_tree_tuner_get_objective_space", [ccs_tree_tuner, ct.POINTER(ccs_objective_space)])
ccs_tree_tuner_ask = _ccs_get_function("ccs_tree_tuner_ask", [ccs_tree_tuner, ct.c_size_t, ct.POINTER(ccs_tree_configuration), ct.POINTER(ct.c_size_t)])
ccs_tree_tuner_tell = _ccs_get_function("ccs_tree_tuner_tell", [ccs_tree_tuner, ct.c_size_t, ct.POINTER(ccs_tree_evaluation)])
ccs_tree_tuner_get_optima = _ccs_get_function("ccs_tree_tuner_get_optima", [ccs_tree_tuner, ct.c_size_t, ct.POINTER(ccs_tree_evaluation), ct.POINTER(ct.c_size_t)])
ccs_tree_tuner_get_history = _ccs_get_function("ccs_tree_tuner_get_history", [ccs_tree_tuner, ct.c_size_t, ct.POINTER(ccs_tree_evaluation), ct.POINTER(ct.c_size_t)])
ccs_tree_tuner_suggest = _ccs_get_function("ccs_tree_tuner_suggest", [ccs_tree_tuner, ct.POINTER(ccs_tree_configuration)])

class TreeTuner(Object):
  @classmethod
  def from_handle(cls, handle, retain = True, auto_release = True):
    v = TreeTunerType(0)
    res = ccs_tree_tuner_get_type(handle, ct.byref(v))
    Error.check(res)
    v = v.value
    if v == TreeTunerType.RANDOM:
      return RandomTreeTuner(handle = handle, retain = retain, auto_release = auto_release)
    elif v == TreeTunerType.USER_DEFINED:
      return UserDefinedTreeTuner(handle = handle, retain = retain, auto_release = auto_release)
    else:
      raise Error(Result(Result.ERROR_INVALID_TREE_TUNER))

  @property
  def type(self):
    if hasattr(self, "_type"):
      return self._type
    v = TreeTunerType(0)
    res = ccs_tree_tuner_get_type(self.handle, ct.byref(v))
    Error.check(res)
    self._type = v.value
    return self._type

  @property
  def name(self):
    if hasattr(self, "_name"):
      return self._name
    v = ct.c_char_p()
    res = ccs_tree_tuner_get_name(self.handle, ct.byref(v))
    Error.check(res)
    self._name = v.value.decode()
    return self._name

  @property
  def objective_space(self):
    if hasattr(self, "_objective_space"):
      return self._objective_space
    v = ccs_objective_space()
    res = ccs_tree_tuner_get_objective_space(self.handle, ct.byref(v))
    Error.check(res)
    self._objective_space = ObjectiveSpace.from_handle(v)
    return self._objective_space

  @property
  def tree_space(self):
    if hasattr(self, "_tree_space"):
      return self._tree_space
    v = ccs_tree_space()
    res = ccs_tree_tuner_get_tree_space(self.handle, ct.byref(v))
    Error.check(res)
    self._tree_space = TreeSpace.from_handle(v)
    return self._tree_space

  def ask(self, count = 1):
    v = (ccs_tree_configuration * count)()
    c = ct.c_size_t()
    res = ccs_tree_tuner_ask(self.handle, count, v, ct.byref(c))
    Error.check(res)
    count = c.value
    return [TreeConfiguration(handle = ccs_tree_configuration(v[x]), retain = False) for x in range(count)]

  def tell(self, evaluations):
    count = len(evaluations)
    v = (ccs_tree_evaluation * count)(*[x.handle.value for x in evaluations])
    res = ccs_tree_tuner_tell(self.handle, count, v)
    Error.check(res)

  @property
  def history_size(self):
    v = ct.c_size_t()
    res = ccs_tree_tuner_get_history(self.handle, 0, None, ct.byref(v))
    Error.check(res)
    return v.value

  @property
  def history(self):
    count = self.history_size
    v = (ccs_tree_evaluation * count)()
    res = ccs_tree_tuner_get_history(self.handle, count, v, None)
    Error.check(res)
    return [TreeEvaluation.from_handle(ccs_tree_evaluation(x)) for x in v]

  @property
  def num_optima(self):
    v = ct.c_size_t()
    res = ccs_tree_tuner_get_optima(self.handle, 0, None, ct.byref(v))
    Error.check(res)
    return v.value

  @property
  def optima(self):
    count = self.num_optima
    v = (ccs_tree_evaluation * count)()
    res = ccs_tree_tuner_get_optima(self.handle, count, v, None)
    Error.check(res)
    return [TreeEvaluation.from_handle(ccs_tree_evaluation(x)) for x in v]

  @property
  def suggest(self):
    config = ccs_tree_configuration()
    res = ccs_tree_tuner_suggest(self.handle, ct.byref(config))
    Error.check(res)
    return TreeConfiguration(handle = config, retain = False)

ccs_create_random_tree_tuner = _ccs_get_function("ccs_create_random_tree_tuner", [ct.c_char_p, ccs_tree_space, ccs_objective_space, ct.POINTER(ccs_tree_tuner)])

class RandomTreeTuner(TreeTuner):
  def __init__(self, handle = None, retain = False, auto_release = True,
               name = "", tree_space = None, objective_space = None):
    if handle is None:
      handle = ccs_tree_tuner()
      res = ccs_create_random_tree_tuner(str.encode(name), tree_space.handle, objective_space.handle, ct.byref(handle))
      Error.check(res)
      super().__init__(handle = handle, retain = False)
    else:
      super().__init__(handle = handle, retain = retain, auto_release = auto_release)

TreeTuner.Random = RandomTreeTuner

ccs_user_defined_tree_tuner_del_type = ct.CFUNCTYPE(Result, ccs_tree_tuner)
ccs_user_defined_tree_tuner_ask_type = ct.CFUNCTYPE(Result, ccs_tree_tuner, ct.c_size_t, ct.POINTER(ccs_tree_configuration), ct.POINTER(ct.c_size_t))
ccs_user_defined_tree_tuner_tell_type = ct.CFUNCTYPE(Result, ccs_tree_tuner, ct.c_size_t, ct.POINTER(ccs_tree_evaluation))
ccs_user_defined_tree_tuner_get_optima_type = ct.CFUNCTYPE(Result, ccs_tree_tuner, ct.c_size_t, ct.POINTER(ccs_tree_evaluation), ct.POINTER(ct.c_size_t))
ccs_user_defined_tree_tuner_get_history_type = ct.CFUNCTYPE(Result, ccs_tree_tuner, ct.c_size_t, ct.POINTER(ccs_tree_evaluation), ct.POINTER(ct.c_size_t))
ccs_user_defined_tree_tuner_suggest_type = ct.CFUNCTYPE(Result, ccs_tree_tuner, ct.POINTER(ccs_tree_configuration))
ccs_user_defined_tree_tuner_serialize_type = ct.CFUNCTYPE(Result, ccs_tree_tuner, ct.c_size_t, ct.c_void_p, ct.POINTER(ct.c_size_t))
ccs_user_defined_tree_tuner_deserialize_type = ct.CFUNCTYPE(Result, ccs_tree_tuner, ct.c_size_t, ct.POINTER(ccs_tree_evaluation), ct.c_size_t, ct.POINTER(ccs_tree_evaluation), ct.c_size_t, ct.c_void_p)

class UserDefinedTreeTunerVector(ct.Structure):
  _fields_ = [
    ('delete', ccs_user_defined_tree_tuner_del_type),
    ('ask', ccs_user_defined_tree_tuner_ask_type),
    ('tell', ccs_user_defined_tree_tuner_tell_type),
    ('get_optima', ccs_user_defined_tree_tuner_get_optima_type),
    ('get_history', ccs_user_defined_tree_tuner_get_history_type),
    ('suggest', ccs_user_defined_tree_tuner_suggest_type),
    ('serialize', ccs_user_defined_tree_tuner_serialize_type),
    ('deserialize', ccs_user_defined_tree_tuner_deserialize_type) ]

ccs_create_user_defined_tree_tuner = _ccs_get_function("ccs_create_user_defined_tree_tuner", [ct.c_char_p, ccs_tree_space, ccs_objective_space, ct.POINTER(UserDefinedTreeTunerVector), ct.py_object, ct.POINTER(ccs_tree_tuner)])
ccs_user_defined_tree_tuner_get_tuner_data = _ccs_get_function("ccs_user_defined_tree_tuner_get_tuner_data", [ccs_tree_tuner, ct.POINTER(ct.c_void_p)])

def _wrap_user_defined_tree_tuner_callbacks(delete, ask, tell, get_optima, get_history, suggest, serialize, deserialize):
  def delete_wrapper(tun):
    try:
      tun = ct.cast(tun, ccs_tree_tuner)
      if delete is not None:
        delete(Object.from_handle(tun))
      _unregister_vector(tun)
      return Result.SUCCESS
    except Exception as e:
      return Error.set_error(e)

  def ask_wrapper(tun, count, p_configurations, p_count):
    try:
      tun = ct.cast(tun, ccs_tree_tuner)
      p_confs = ct.cast(p_configurations, ct.c_void_p)
      p_c = ct.cast(p_count, ct.c_void_p)
      (configurations, count_ret) = ask(TreeTuner.from_handle(tun), count if p_confs.value else None)
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
      tun = ct.cast(tun, ccs_tree_tuner)
      if count == 0:
        return Result.SUCCESS
      p_evals = ct.cast(p_evaluations, ct.c_void_p)
      if p_evals.value is None:
        raise Error(Result(Result.ERROR_INVALID_VALUE))
      evals = [TreeEvaluation.from_handle(ccs_tree_evaluation(p_evaluations[i])) for i in range(count)]
      tell(TreeTuner.from_handle(tun), evals)
      return Result.SUCCESS
    except Exception as e:
      return Error.set_error(e)

  def get_optima_wrapper(tun, count, p_evaluations, p_count):
    try:
      tun = ct.cast(tun, ccs_tree_tuner)
      p_evals = ct.cast(p_evaluations, ct.c_void_p)
      p_c = ct.cast(p_count, ct.c_void_p)
      optima = get_optima(TreeTuner.from_handle(tun))
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

  def get_history_wrapper(tun, count, p_evaluations, p_count):
    try:
      tun = ct.cast(tun, ccs_tree_tuner)
      p_evals = ct.cast(p_evaluations, ct.c_void_p)
      p_c = ct.cast(p_count, ct.c_void_p)
      history = get_history(TreeTuner.from_handle(tun))
      count_ret = len(history)
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
    def suggest_wrapper(tun, p_configuration):
      try:
        tun = ct.cast(tun, ccs_tree_tuner)
        configuration = suggest(TreeTuner.from_handle(tun))
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
        tun = ct.cast(tun, ccs_tree_tuner)
        p_s = ct.cast(p_state, ct.c_void_p)
        p_sz = ct.cast(p_state_size, ct.c_void_p)
        state = serialize(TreeTuner.from_handle(tun), True if state_size == 0 else False)
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
    def deserialize_wrapper(tun, size_history, p_history, num_optima, p_optima, state_size, p_state):
      try:
        tun = ct.cast(tun, ccs_tree_tuner)
        p_h = ct.cast(p_history, ct.c_void_p)
        p_o = ct.cast(p_optima, ct.c_void_p)
        p_s = ct.cast(p_state, ct.c_void_p)
        if p_h.value is None:
          history = []
        else:
          history = [TreeEvaluation.from_handle(ccs_tree_evaluation(p_h[i])) for i in range(size_history)]
        if p_o.value is None:
          optima = []
        else:
          optima = [TreeEvaluation.from_handle(ccs_tree_evaluation(p_o[i])) for i in range(num_optima)]
        if p_s.value is None:
          state = None
        else:
          state = ct.cast(p_s, POINTER(c_byte * state_size))
        deserialize(TreeTuner.from_handle(tun), history, optima, state)
        return Result.SUCCESS
      except Exception as e:
        return Error.set_error(e)
  else:
    deserialize_wrapper = 0

  return (delete_wrapper,
          ask_wrapper,
          tell_wrapper,
          get_optima_wrapper,
          get_history_wrapper,
          suggest_wrapper,
          serialize_wrapper,
          deserialize_wrapper,
          ccs_user_defined_tree_tuner_del_type(delete_wrapper),
          ccs_user_defined_tree_tuner_ask_type(ask_wrapper),
          ccs_user_defined_tree_tuner_tell_type(tell_wrapper),
          ccs_user_defined_tree_tuner_get_optima_type(get_optima_wrapper),
          ccs_user_defined_tree_tuner_get_history_type(get_history_wrapper),
          ccs_user_defined_tree_tuner_suggest_type(suggest_wrapper),
          ccs_user_defined_tree_tuner_serialize_type(serialize_wrapper),
          ccs_user_defined_tree_tuner_deserialize_type(deserialize_wrapper))


class UserDefinedTreeTuner(TreeTuner):
  def __init__(self, handle = None, retain = False, auto_release = True,
               name = "", tree_space = None, objective_space = None, delete = None, ask = None, tell = None, get_optima = None, get_history = None, suggest = None, serialize = None, deserialize = None, tuner_data = None ):
    if handle is None:
      if ask is None or tell is None or get_optima is None or get_history is None:
        raise Error(Result(Result.ERROR_INVALID_VALUE))

      (delete_wrapper,
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
       deserialize_wrapper_func) = _wrap_user_defined_tree_tuner_callbacks(delete, ask, tell, get_optima, get_history, suggest, serialize, deserialize)
      handle = ccs_tree_tuner()
      vec = UserDefinedTreeTunerVector()
      vec.delete = delete_wrapper_func
      vec.ask = ask_wrapper_func
      vec.tell = tell_wrapper_func
      vec.get_optima = get_optima_wrapper_func
      vec.get_history = get_history_wrapper_func
      vec.suggest = suggest_wrapper_func
      vec.serialize = serialize_wrapper_func
      vec.deserialize = deserialize_wrapper_func
      if tuner_data is not None:
        c_tuner_data = ct.py_object(tuner_data)
      else:
        c_tuner_data = None
      res = ccs_create_user_defined_tree_tuner(str.encode(name), tree_space.handle, objective_space.handle, ct.byref(vec), c_tuner_data, ct.byref(handle))
      Error.check(res)
      super().__init__(handle = handle, retain = False)
      _register_vector(handle, [delete_wrapper, ask_wrapper, tell_wrapper, get_optima_wrapper, get_history_wrapper, suggest_wrapper, serialize_wrapper, deserialize_wrapper, delete_wrapper_func, ask_wrapper_func, tell_wrapper_func, get_optima_wrapper_func, get_history_wrapper_func, suggest_wrapper_func, serialize_wrapper_func, deserialize_wrapper_func, tuner_data])
    else:
      super().__init__(handle = handle, retain = retain, auto_release = auto_release)

  @classmethod
  def deserialize(cls, delete, ask, tell, get_optima, get_history, suggest = None, serialize = None, deserialize = None, tuner_data = None, format = 'binary', handle_map = None, path = None, buffer = None, file_descriptor = None, callback = None, callback_data = None):
    if ask is None or tell is None or get_optima is None or get_history is None:
      raise Error(Result(Result.ERROR_INVALID_VALUE))
    (delete_wrapper,
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
     deserialize_wrapper_func) = _wrap_user_defined_tree_tuner_callbacks(delete, ask, tell, get_optima, get_history, suggest, serialize, deserialize)
    vector = UserDefinedTreeTunerVector()
    vector.delete = delete_wrapper_func
    vector.ask = ask_wrapper_func
    vector.tell = tell_wrapper_func
    vector.get_optima = get_optima_wrapper_func
    vector.get_history = get_history_wrapper_func
    vector.suggest = suggest_wrapper_func
    vector.serialize = serialize_wrapper_func
    vector.deserialize = deserialize_wrapper_func
    res = Object.deserialize(format = format, handle_map = handle_map, vector = vector, data = tuner_data, path = path, buffer = buffer, file_descriptor = file_descriptor, callback = callback, callback_data = callback_data)
    _register_vector(res.handle, [delete_wrapper, ask_wrapper, tell_wrapper, get_optima_wrapper, get_history_wrapper, suggest_wrapper, serialize_wrapper, deserialize_wrapper, delete_wrapper_func, ask_wrapper_func, tell_wrapper_func, get_optima_wrapper_func, get_history_wrapper_func, suggest_wrapper_func, serialize_wrapper_func, deserialize_wrapper_func, tuner_data])
    return res

  @property
  def tuner_data(self):
    if hasattr(self, "_tuner_data"):
      return self._tuner_data
    v = ct.c_void_p()
    res = ccs_user_defined_tree_tuner_get_tuner_data(self.handle, ct.byref(v))
    Error.check(res)
    if v:
      self._tuner_data = ct.cast(v, ct.py_object).value
    else:
      self._tuner_data = None
    return self._tuner_data

TreeTuner.UserDefined = UserDefinedTreeTuner
