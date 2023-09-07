import ctypes as ct
from . import libcconfigspace
from .base import Object, Error, Result, ccs_rng, ccs_tree, ccs_tree_space, ccs_tree_configuration, Datum, ccs_bool, _ccs_get_function, CEnumeration, _register_vector, _unregister_vector, ccs_retain_object
from .rng import Rng
from .tree import Tree

class TreeSpaceType(CEnumeration):
  _members_ = [
    ('STATIC', 0),
    'DYNAMIC' ]

ccs_tree_space_get_type = _ccs_get_function("ccs_tree_space_get_type", [ccs_tree_space, ct.POINTER(TreeSpaceType)])
ccs_tree_space_get_name = _ccs_get_function("ccs_tree_space_get_name", [ccs_tree_space, ct.POINTER(ct.c_char_p)])
ccs_tree_space_set_rng = _ccs_get_function("ccs_tree_space_set_rng", [ccs_tree_space, ccs_rng])
ccs_tree_space_get_rng = _ccs_get_function("ccs_tree_space_get_rng", [ccs_tree_space, ct.POINTER(ccs_rng)])
ccs_tree_space_get_tree = _ccs_get_function("ccs_tree_space_get_tree", [ccs_tree_space, ct.POINTER(ccs_tree)])
ccs_tree_space_get_node_at_position = _ccs_get_function("ccs_tree_space_get_node_at_position", [ccs_tree_space, ct.c_size_t, ct.POINTER(ct.c_size_t), ct.POINTER(ccs_tree)])
ccs_tree_space_get_values_at_position = _ccs_get_function("ccs_tree_space_get_values_at_position", [ccs_tree_space, ct.c_size_t, ct.POINTER(ct.c_size_t), ct.c_size_t, ct.POINTER(Datum)])
ccs_tree_space_check_position = _ccs_get_function("ccs_tree_space_check_position", [ccs_tree_space, ct.c_size_t, ct.POINTER(ct.c_size_t), ct.POINTER(ccs_bool)])
ccs_tree_space_check_configuration = _ccs_get_function("ccs_tree_space_check_configuration", [ccs_tree_space, ccs_tree_configuration, ct.POINTER(ccs_bool)])
ccs_tree_space_sample = _ccs_get_function("ccs_tree_space_sample", [ccs_tree_space, ct.POINTER(ccs_tree_configuration)])
ccs_tree_space_samples = _ccs_get_function("ccs_tree_space_samples", [ccs_tree_space, ct.c_size_t, ct.POINTER(ccs_tree_configuration)])

class TreeSpace(Object):

  @classmethod
  def from_handle(cls, handle, retain = True, auto_release = True):
    v = TreeSpaceType(0)
    res = ccs_tree_space_get_type(handle, ct.byref(v))
    Error.check(res)
    v = v.value
    if v == TreeSpaceType.STATIC:
      return StaticTreeSpace(handle = handle, retain = retain, auto_release = auto_release)
    elif v == TreeSpaceType.DYNAMIC:
      return DynamicTreeSpace(handle = handle, retain = retain, auto_release = auto_release)
    else:
      raise Error(Result(Result.ERROR_INVALID_TREE_SPACE))

  @property
  def type(self):
    if hasattr(self, "_type"):
      return self._type
    v = TreeSpaceType(0)
    res = ccs_tree_space_get_type(self.handle, ct.byref(v))
    Error.check(res)
    self._type = v.value
    return self._type

  @property
  def name(self):
    if hasattr(self, "_name"):
      return self._name
    v = ct.c_char_p()
    res = ccs_tree_space_get_name(self.handle, ct.byref(v))
    Error.check(res)
    self._name = v.value.decode()
    return self._name

  @property
  def rng(self):
    v = ccs_rng()
    res = ccs_tree_space_get_rng(self.handle, ct.byref(v))
    Error.check(res)
    return Rng.from_handle(v)

  @rng.setter
  def rng(self, r):
    res = ccs_tree_space_set_rng(self.handle, r.handle)
    Error.check(res)

  @property
  def tree(self):
    if hasattr(self, "_tree"):
      return self._tree
    v = ccs_tree()
    res = ccs_tree_space_get_tree(self.handle, ct.byref(v))
    Error.check(res)
    self._tree = Tree.from_handle(v)
    return self._tree

  def get_node_at_position(self, position):
    count = len(position)
    v1 = (ct.c_size_t * count)(*position)
    v2 = ccs_tree()
    res = ccs_tree_space_get_node_at_position(self.handle, count, v1, ct.byref(v2))
    Error.check(res)
    return Tree.from_handle(v2)

  def get_values_at_position(self, position):
    count = len(position)
    v1 = (ct.c_size_t * count)(*position)
    v2 = (Datum * (count + 1))()
    res = ccs_tree_space_get_values_at_position(self.handle, count, v1, count + 1, v2)
    Error.check(res)
    return [x.value for x in v2]

  def check_position(self, position):
    count = len(position)
    v = (ct.c_size_t * count)(*position)
    b = ccs_bool()
    res = ccs_tree_space_check_position(self.handle, count, v, ct.byref(b))
    Error.check(res)
    return not (b.value == 0)

  def check_configuration(self, configuration):
    b = ccs_bool()
    res = ccs_tree_space_check_configuration(self.handle, configuration.handle, ct.byref(b))
    Error.check(res)
    return not (b.value == 0)

  def sample(self):
    v = ccs_tree_configuration()
    res = ccs_tree_space_sample(self.handle, ct.byref(v))
    Error.check(res)
    return TreeConfiguration(handle = v, retain = False)

  def samples(self, count):
    if count == 0:
      return []
    v = (ccs_tree_configuration * count)()
    res = ccs_tree_space_samples(self.handle, count, v)
    Error.check(res)
    return [TreeConfiguration(handle = ccs_tree_configuration(x), retain = False) for x in v]


ccs_create_static_tree_space = _ccs_get_function("ccs_create_static_tree_space", [ct.c_char_p, ccs_tree, ct.POINTER(ccs_tree_space)])

class StaticTreeSpace(TreeSpace):

  def __init__(self, handle = None, retain = False, auto_release = True,
               name = "", tree = None):
    if handle is None:
      handle = ccs_tree_space()
      res = ccs_create_static_tree_space(str.encode(name), tree.handle, ct.byref(handle))
      Error.check(res)
      super().__init__(handle = handle, retain = False)
    else:
      super().__init__(handle = handle, retain = retain, auto_release = auto_release)

TreeSpace.Static = StaticTreeSpace

ccs_dynamic_tree_space_del_type = ct.CFUNCTYPE(Result, ccs_tree_space)
ccs_dynamic_tree_space_get_child_type = ct.CFUNCTYPE(Result, ccs_tree_space, ccs_tree, ct.c_size_t, ct.POINTER(ccs_tree))
ccs_dynamic_tree_space_serialize_type = ct.CFUNCTYPE(Result, ccs_tree_space, ct.c_size_t, ct.c_void_p, ct.POINTER(ct.c_size_t))
ccs_dynamic_tree_space_deserialize_type = ct.CFUNCTYPE(Result, ccs_tree_space, ct.c_size_t, ct.c_void_p)

class DynamicTreeSpaceVector(ct.Structure):
  _fields_ = [
    ('delete', ccs_dynamic_tree_space_del_type),
    ('get_child', ccs_dynamic_tree_space_get_child_type),
    ('serialize', ccs_dynamic_tree_space_serialize_type),
    ('deserialize', ccs_dynamic_tree_space_deserialize_type) ]

ccs_create_dynamic_tree_space = _ccs_get_function("ccs_create_dynamic_tree_space", [ct.c_char_p, ccs_tree, ct.POINTER(DynamicTreeSpaceVector), ct.py_object, ct.POINTER(ccs_tree_space)])
ccs_dynamic_tree_space_get_tree_space_data = _ccs_get_function("ccs_dynamic_tree_space_get_tree_space_data", [ccs_tree_space, ct.POINTER(ct.c_void_p)])

def _wrap_user_defined_callbacks(delete, get_child, serialize, deserialize):
  def delete_wrapper(ts):
    try:
      ts = ct.cast(ts, ccs_tree_space)
      if delete is not None:
        delete(Object.from_handle(ts))
      _unregister_vector(ts)
      return Result.SUCCESS
    except Exception as e:
      return Error.set_error(e)

  def get_child_wrapper(ts, parent, index, p_child):
    try:
      ts = ct.cast(ts, ccs_tree_space)
      parent = ct.cast(parent, ccs_tree)
      child = get_child(TreeSpace.from_handle(ts), Tree.from_handle(parent), index)
      res = ccs_retain_object(child.handle)
      Error.check(res)
      p_child[0] = child.handle.value
      return Result.SUCCESS
    except Exception as e:
      return Error.set_error(e)

  if serialize is not None:
    def serialize_wrapper(ts, state_size, p_state, p_state_size):
      try:
        ts = ct.cast(ts, ccs_tree_space)
        p_s = ct.cast(p_state, ct.c_void_p)
        p_sz = ct.cast(p_state_size, ct.c_void_p)
        state = serialize(TreeSpace.from_handle(ts), True if state_size == 0 else False)
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
    def deserialize_wrapper(ts, state_size, p_state):
      try:
        ts = ct.cast(ts, ccs_tree_space)
        p_s = ct.cast(p_state, ct.c_void_p)
        if p_s.value is None:
          state = None
        else:
          state = ct.cast(p_s, POINTER(c_byte * state_size))
        deserialize(TreeSpace.from_handle(ts), state)
        return Result.SUCCESS
      except Exception as e:
        return Error.set_error(e)
  else:
    deserialize_wrapper = 0

  return (delete_wrapper,
          get_child_wrapper,
          serialize_wrapper,
          deserialize_wrapper,
          ccs_dynamic_tree_space_del_type(delete_wrapper),
          ccs_dynamic_tree_space_get_child_type(get_child_wrapper),
          ccs_dynamic_tree_space_serialize_type(serialize_wrapper),
          ccs_dynamic_tree_space_deserialize_type(deserialize_wrapper))

class DynamicTreeSpace(TreeSpace):

  def __init__(self, handle = None, retain = False, auto_release = True,
               name = "", tree = None, delete = None, get_child = None, serialize = None, deserialize = None, tree_space_data = None):
    if handle is None:
      if get_child is None:
        raise Error(Result(Result.ERROR_INVALID_VALUE))

      (delete_wrapper,
       get_child_wrapper,
       serialize_wrapper,
       deserialize_wrapper,
       delete_wrapper_func,
       get_child_wrapper_func,
       serialize_wrapper_func,
       deserialize_wrapper_func) = _wrap_user_defined_callbacks(delete, get_child, serialize, deserialize)
      handle = ccs_tree_space()
      vec = DynamicTreeSpaceVector()
      vec.delete = delete_wrapper_func
      vec.get_child = get_child_wrapper_func
      vec.serialize = serialize_wrapper_func
      vec.deserialize = deserialize_wrapper_func
      if tree_space_data is not None:
        c_tree_space_data = ct.py_object(tree_space_data)
      else:
        c_tree_space_data = None
      res = ccs_create_dynamic_tree_space(str.encode(name), tree.handle, ct.byref(vec), c_tree_space_data, ct.byref(handle))
      Error.check(res)
      super().__init__(handle = handle, retain = False)
      _register_vector(handle, [delete_wrapper, get_child_wrapper, serialize_wrapper, deserialize_wrapper, delete_wrapper_func, get_child_wrapper_func, serialize_wrapper_func, deserialize_wrapper_func, tree_space_data])
    else:
      super().__init__(handle = handle, retain = retain, auto_release = auto_release)

  @classmethod
  def deserialize(cls, delete, get_child, serialize = None, deserialize = None, tree_space_data = None, format = 'binary', handle_map = None, path = None, buffer = None, file_descriptor = None, callback = None, callback_data = None):
    if get_child is None:
      raise Error(Result(Result.ERROR_INVALID_VALUE))
    (delete_wrapper,
     get_child_wrapper,
     serialize_wrapper,
     deserialize_wrapper,
     delete_wrapper_func,
     get_child_wrapper_func,
     serialize_wrapper_func,
     deserialize_wrapper_func) = _wrap_user_defined_callbacks(delete, get_child, serialize, deserialize)
    handle = ccs_tree_space()
    vector = DynamicTreeSpaceVector()
    vector.delete = delete_wrapper_func
    vector.get_child = get_child_wrapper_func
    vector.serialize = serialize_wrapper_func
    vector.deserialize = deserialize_wrapper_func
    res = super().deserialize(format = format, handle_map = handle_map, vector = vector, data = tree_space_data, path = path, buffer = buffer, file_descriptor = file_descriptor, callback = callback, callback_data = callback_data)
    _register_vector(res.handle, [delete_wrapper, get_child_wrapper, serialize_wrapper, deserialize_wrapper, delete_wrapper_func, get_child_wrapper_func, serialize_wrapper_func, deserialize_wrapper_func, tree_space_data])
    return res

  @property
  def tree_space_data(self):
    if hasattr(self, "_tree_space_data"):
      return self._tree_space_data
    v = ct.c_void_p()
    res = ccs_dynamic_tree_space_get_tree_space_data(self.handle, ct.byref(v))
    Error.check(res)
    if v:
      self._tree_space_data = ct.cast(v, ct.py_object).value
    else:
      self._tree_space_data = None
    return self._tree_space_data

TreeSpace.Dynamic = DynamicTreeSpace

from .tree_configuration import TreeConfiguration
