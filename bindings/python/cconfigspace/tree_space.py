import ctypes as ct
from . import libcconfigspace
from .base import Object, Error, Result, ccs_rng, ccs_tree, ccs_tree_space, ccs_feature_space, ccs_features, ccs_tree_configuration, Datum, ccs_bool, _ccs_get_function, CEnumeration, ccs_retain_object
from .rng import Rng
from .tree import Tree
from .feature_space import FeatureSpace

class TreeSpaceType(CEnumeration):
  _members_ = [
    ('STATIC', 0),
    'DYNAMIC' ]

ccs_tree_space_get_type = _ccs_get_function("ccs_tree_space_get_type", [ccs_tree_space, ct.POINTER(TreeSpaceType)])
ccs_tree_space_get_name = _ccs_get_function("ccs_tree_space_get_name", [ccs_tree_space, ct.POINTER(ct.c_char_p)])
ccs_tree_space_get_rng = _ccs_get_function("ccs_tree_space_get_rng", [ccs_tree_space, ct.POINTER(ccs_rng)])
ccs_tree_space_get_feature_space = _ccs_get_function("ccs_tree_space_get_feature_space", [ccs_tree_space, ct.POINTER(ccs_feature_space)])
ccs_tree_space_get_tree = _ccs_get_function("ccs_tree_space_get_tree", [ccs_tree_space, ct.POINTER(ccs_tree)])
ccs_tree_space_get_node_at_position = _ccs_get_function("ccs_tree_space_get_node_at_position", [ccs_tree_space, ct.c_size_t, ct.POINTER(ct.c_size_t), ct.POINTER(ccs_tree)])
ccs_tree_space_get_values_at_position = _ccs_get_function("ccs_tree_space_get_values_at_position", [ccs_tree_space, ct.c_size_t, ct.POINTER(ct.c_size_t), ct.c_size_t, ct.POINTER(Datum)])
ccs_tree_space_sample = _ccs_get_function("ccs_tree_space_sample", [ccs_tree_space, ccs_features, ccs_rng, ct.POINTER(ccs_tree_configuration)])
ccs_tree_space_samples = _ccs_get_function("ccs_tree_space_samples", [ccs_tree_space, ccs_features, ccs_rng, ct.c_size_t, ct.POINTER(ccs_tree_configuration)])

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
    if hasattr(self, "_rng"):
      return self._rng
    v = ccs_rng()
    res = ccs_tree_space_get_rng(self.handle, ct.byref(v))
    Error.check(res)
    self._rng = Rng.from_handle(v)
    return self._rng

  @property
  def feature_space(self):
    if hasattr(self, "_feature_space"):
      return self._feature_space
    v = ccs_feature_space()
    res = ccs_tree_space_get_feature_space(self.handle, ct.byref(v))
    Error.check(res)
    if bool(v):
      self._feature_space = Rng.from_handle(v)
    else:
      self._feature_space = None
    return self._feature_space

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

  def sample(self, features = None, rng = None):
    if features is not None:
      features = features.handle
    if rng is not None:
      rng = rng.handle
    v = ccs_tree_configuration()
    res = ccs_tree_space_sample(self.handle, features, rng, ct.byref(v))
    Error.check(res)
    return TreeConfiguration(handle = v, retain = False)

  def samples(self, count, features = None, rng = None):
    if count == 0:
      return []
    if features is not None:
      features = features.handle
    if rng is not None:
      rng = rng.handle
    v = (ccs_tree_configuration * count)()
    res = ccs_tree_space_samples(self.handle, features, rng, count, v)
    Error.check(res)
    return [TreeConfiguration(handle = ccs_tree_configuration(x), retain = False) for x in v]


ccs_create_static_tree_space = _ccs_get_function("ccs_create_static_tree_space", [ct.c_char_p, ccs_tree, ccs_feature_space, ccs_rng, ct.POINTER(ccs_tree_space)])

class StaticTreeSpace(TreeSpace):

  def __init__(self, handle = None, retain = False, auto_release = True,
               name = "", tree = None, feature_space = None, rng = None):
    if handle is None:
      if rng is not None:
        rng = rng.handle
      if feature_space is not None:
        feature_space = feature_space.handle
      handle = ccs_tree_space()
      res = ccs_create_static_tree_space(str.encode(name), tree.handle, feature_space, rng, ct.byref(handle))
      Error.check(res)
      super().__init__(handle = handle, retain = False)
    else:
      super().__init__(handle = handle, retain = retain, auto_release = auto_release)

TreeSpace.Static = StaticTreeSpace

ccs_dynamic_tree_space_del_type = ct.CFUNCTYPE(Result, ccs_tree_space)
ccs_dynamic_tree_space_get_child_type = ct.CFUNCTYPE(Result, ccs_tree_space, ccs_tree, ct.c_size_t, ct.POINTER(ccs_tree))
ccs_dynamic_tree_space_serialize_type = ct.CFUNCTYPE(Result, ccs_tree_space, ct.c_size_t, ct.c_void_p, ct.POINTER(ct.c_size_t))
ccs_dynamic_tree_space_deserialize_type = ct.CFUNCTYPE(Result, ccs_tree, ccs_feature_space, ct.c_size_t, ct.c_void_p, ct.POINTER(ct.py_object))

class DynamicTreeSpaceVector(ct.Structure):
  _fields_ = [
    ('delete', ccs_dynamic_tree_space_del_type),
    ('get_child', ccs_dynamic_tree_space_get_child_type),
    ('serialize', ccs_dynamic_tree_space_serialize_type),
    ('deserialize', ccs_dynamic_tree_space_deserialize_type) ]

ccs_create_dynamic_tree_space = _ccs_get_function("ccs_create_dynamic_tree_space", [ct.c_char_p, ccs_tree, ccs_feature_space, ccs_rng, ct.POINTER(DynamicTreeSpaceVector), ct.py_object, ct.POINTER(ccs_tree_space)])
ccs_dynamic_tree_space_get_tree_space_data = _ccs_get_function("ccs_dynamic_tree_space_get_tree_space_data", [ccs_tree_space, ct.POINTER(ct.c_void_p)])

class DynamicTreeSpace(TreeSpace):

  def __init__(self, handle = None, retain = False, auto_release = True,
               name = "", tree = None, feature_space = None, rng = None, delete = None, get_child = None, serialize = None, deserialize = None, tree_space_data = None):
    if handle is None:
      if get_child is None:
        raise Error(Result(Result.ERROR_INVALID_VALUE))

      vec = self.get_vector(delete, get_child, serialize, deserialize)
      if tree_space_data is not None:
        c_tree_space_data = ct.py_object(tree_space_data)
      else:
        c_tree_space_data = None
      if rng is not None:
        rng = rng.handle
      if feature_space is not None:
        feature_space = feature_space.handle
      handle = ccs_tree_space()
      res = ccs_create_dynamic_tree_space(str.encode(name), tree.handle, feature_space, rng, ct.byref(vec), c_tree_space_data, ct.byref(handle))
      Error.check(res)
      super().__init__(handle = handle, retain = False)
      ct.pythonapi.Py_IncRef(ct.py_object(vec))
      if c_tree_space_data is not None:
        ct.pythonapi.Py_IncRef(c_tree_space_data)
    else:
      super().__init__(handle = handle, retain = retain, auto_release = auto_release)

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

  @classmethod
  def get_vector(self, delete = None, get_child = None, serialize = None, deserialize = None):
    if get_child is None:
      raise Error(Result(Result.ERROR_INVALID_VALUE))
    vec = DynamicTreeSpaceVector()
    def delete_wrapper(ts):
      try:
        ts = ct.cast(ts, ccs_tree_space)
        o = Object.from_handle(ts)
        tsdata = o.tree_space_data
        if delete is not None:
          delete(o)
        if tsdata is not None:
          ct.pythonapi.Py_DecRef(ct.py_object(tsdata))
        ct.pythonapi.Py_DecRef(ct.py_object(vec))
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
          serialized = serialize(TreeSpace.from_handle(ts))
          state = ct.create_string_buffer(serialized, len(serialized))
          if p_state and state_size < ct.sizeof(state):
            raise Error(Result(Result.ERROR_INVALID_VALUE))
          if p_state:
            ct.memmove(p_state, ct.byref(state), ct.sizeof(state))
          if p_state_size:
            p_state_size[0] = ct.sizeof(state)
          return Result.SUCCESS
        except Exception as e:
          return Error.set_error(e)
    else:
      serialize_wrapper = 0

    if deserialize is not None:
      def deserialize_wrapper(tree, feature_space, state_size, p_state, p_tree_space_data):
        try:
          tree_space_data = deserialize(Tree.from_handle(tree), FeatureSpace.from_handle(feature_space) if feature_space else None, ct.string_at(p_state, state_size))
          c_tree_space_data = ct.py_object(tree_space_data)
          p_tree_space_data[0] = c_tree_space_data
          ct.pythonapi.Py_IncRef(c_tree_space_data)
          return Result.SUCCESS
        except Exception as e:
          return Error.set_error(e)
    else:
      deserialize_wrapper = 0

    delete_wrapper_func      = ccs_dynamic_tree_space_del_type(delete_wrapper)
    get_child_wrapper_func   = ccs_dynamic_tree_space_get_child_type(get_child_wrapper)
    serialize_wrapper_func   = ccs_dynamic_tree_space_serialize_type(serialize_wrapper)
    deserialize_wrapper_func = ccs_dynamic_tree_space_deserialize_type(deserialize_wrapper)
    vec.delete = delete_wrapper_func
    vec.get_child = get_child_wrapper_func
    vec.serialize = serialize_wrapper_func
    vec.deserialize = deserialize_wrapper_func

    setattr(vec, '_wrappers', (
      delete_wrapper,
      get_child_wrapper,
      serialize_wrapper,
      deserialize_wrapper,
      delete_wrapper_func,
      get_child_wrapper_func,
      serialize_wrapper_func,
      deserialize_wrapper_func))
    return vec

TreeSpace.Dynamic = DynamicTreeSpace

from .tree_configuration import TreeConfiguration
