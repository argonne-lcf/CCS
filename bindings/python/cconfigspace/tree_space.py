import ctypes as ct
from . import libcconfigspace
from .base import Object, Error, ccs_rng, ccs_tree, ccs_tree_space, ccs_tree_configuration, ccs_datum, ccs_bool, _ccs_get_function, CEnumeration
from .rng import Rng
from .tree import Tree

class ccs_tree_space_type(CEnumeration):
  _members_ = [
    ('STATIC', 0),
    'DYNAMIC' ]

ccs_tree_space_get_type = _ccs_get_function("ccs_tree_space_get_type", [ccs_tree_space, ct.POINTER(ccs_tree_space_type)])
ccs_tree_space_get_name = _ccs_get_function("ccs_tree_space_get_name", [ccs_tree_space, ct.POINTER(ct.c_char_p)])
ccs_tree_space_set_rng = _ccs_get_function("ccs_tree_space_set_rng", [ccs_tree_space, ccs_rng])
ccs_tree_space_get_rng = _ccs_get_function("ccs_tree_space_get_rng", [ccs_tree_space, ct.POINTER(ccs_rng)])
ccs_tree_space_get_tree = _ccs_get_function("ccs_tree_space_get_tree", [ccs_tree_space, ct.POINTER(ccs_tree)])
ccs_tree_space_get_node_at_position = _ccs_get_function("ccs_tree_space_get_node_at_position", [ccs_tree_space, ct.c_size_t, ct.POINTER(ct.c_size_t), ct.POINTER(ccs_tree)])
ccs_tree_space_get_values_at_position = _ccs_get_function("ccs_tree_space_get_values_at_position", [ccs_tree_space, ct.c_size_t, ct.POINTER(ct.c_size_t), ct.c_size_t, ct.POINTER(ccs_datum)])
ccs_tree_space_check_position = _ccs_get_function("ccs_tree_space_check_position", [ccs_tree_space, ct.c_size_t, ct.POINTER(ct.c_size_t), ct.POINTER(ccs_bool)])
ccs_tree_space_check_configuration = _ccs_get_function("ccs_tree_space_check_configuration", [ccs_tree_space, ccs_tree_configuration, ct.POINTER(ccs_bool)])
ccs_tree_space_sample = _ccs_get_function("ccs_tree_space_sample", [ccs_tree_space, ct.POINTER(ccs_tree_configuration)])
ccs_tree_space_samples = _ccs_get_function("ccs_tree_space_samples", [ccs_tree_space, ct.c_size_t, ct.POINTER(ccs_tree_configuration)])

class TreeSpace(Object):

  @classmethod
  def from_handle(cls, handle, retain = True, auto_release = True):
    v = ccs_tree_space_type(0)
    res = ccs_tree_space_get_type(handle, ct.byref(v))
    Error.check(res)
    v = v.value
    if v == ccs_tree_space_type.STATIC:
      return StaticTreeSpace(handle = handle, retain = retain, auto_release = auto_release)
    elif v == ccs_tree_space_type.DYNAMIC:
      return DynamicTreeSpace(handle = handle, retain = retain, auto_release = auto_release)
    else:
      raise Error(ccs_error(ccs_error.INVALID_TREE_SPACE))

  @property
  def type(self):
    if hasattr(self, "_type"):
      return self._type
    v = ccs_tree_space_type(0)
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
    v2 = (ccs_datum * (count + 1))()
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
               name = None, tree = None):
    if handle is None:
      handle = ccs_tree_space()
      res = ccs_create_static_tree_space(str.encode(name), tree.handle, ct.byref(handle))
      Error.check(res)
      super().__init__(handle = handle, retain = False)
    else:
      super().__init__(handle = handle, retain = retain, auto_release = auto_release)

from .tree_configuration import TreeConfiguration
