import ctypes as ct
from . import libcconfigspace
from .base import Object, Error, ccs_tree, ccs_tree_space, ccs_tree_configuration, Datum, ccs_bool, ccs_hash, _ccs_get_function
from .tree import Tree
from .tree_space import TreeSpace

ccs_create_tree_configuration = _ccs_get_function("ccs_create_tree_configuration", [ccs_tree_space, ct.c_size_t, ct.POINTER(ct.c_size_t), ct.POINTER(ccs_tree_configuration)])
ccs_tree_configuration_get_tree_space = _ccs_get_function("ccs_tree_configuration_get_tree_space", [ccs_tree_configuration, ct.POINTER(ccs_tree_space)])
ccs_tree_configuration_get_position = _ccs_get_function("ccs_tree_configuration_get_position", [ccs_tree_configuration, ct.c_size_t, ct.POINTER(ct.c_size_t), ct.POINTER(ct.c_size_t)])
ccs_tree_configuration_get_values = _ccs_get_function("ccs_tree_configuration_get_values", [ccs_tree_configuration, ct.c_size_t, ct.POINTER(Datum), ct.POINTER(ct.c_size_t)])
ccs_tree_configuration_get_node = _ccs_get_function("ccs_tree_configuration_get_node", [ccs_tree_configuration, ct.POINTER(ccs_tree)])
ccs_tree_configuration_check = _ccs_get_function("ccs_tree_configuration_check", [ccs_tree_configuration, ct.POINTER(ccs_bool)])
ccs_tree_configuration_hash = _ccs_get_function("ccs_tree_configuration_hash", [ccs_tree_configuration, ct.POINTER(ccs_hash)])
ccs_tree_configuration_cmp = _ccs_get_function("ccs_tree_configuration_cmp", [ccs_tree_configuration, ccs_tree_configuration, ct.POINTER(ct.c_int)])

class TreeConfiguration(Object):

  def __init__(self, handle = None, retain = False, auto_release = True,
               tree_space = None, position = None):
    if handle is None:
      handle = ccs_tree_configuration()
      count = len(position)
      v = (ct.c_size_t * count)(*position)
      res = ccs_create_tree_configuration(tree_space.handle, count, v, ct.byref(handle))
      Error.check(res)
      super().__init__(handle = handle, retain = False)
    else:
      super().__init__(handle = handle, retain = retain, auto_release = auto_release)

  @classmethod
  def from_handle(cls, handle, retain = True, auto_release = True):
    return cls(handle, retain = retain, auto_release = auto_release)

  @property
  def tree_space(self):
    if hasattr(self, "_tree_space"):
      return self._tree_space
    v = ccs_tree_space()
    res = ccs_tree_configuration_get_tree_space(self.handle, ct.byref(v))
    Error.check(res)
    self._tree_space = TreeSpace.from_handle(v)
    return self._tree_space


  @property
  def position_size(self):
    if hasattr(self, "_position_size"):
      return self._position_size
    v = ct.c_size_t()
    res = ccs_tree_configuration_get_position(self.handle, 0, None, ct.byref(v))
    Error.check(res)
    self._position_size = v.value
    return self._position_size

  @property
  def position(self):
    if hasattr(self, "_position"):
      return self._position
    count = self.position_size
    v = (ct.c_size_t * count)()
    res = ccs_tree_configuration_get_position(self.handle, count, v, None)
    Error.check(res)
    self._position = list(v)
    return self._position

  @property
  def values(self):
    if hasattr(self, "_values"):
      return self._values
    count = self.position_size + 1
    v = (Datum * count)()
    res = ccs_tree_configuration_get_values(self.handle, count, v, None)
    Error.check(res)
    self._values = [x.value for x in v]
    return self._values

  @property
  def node(self):
    if hasattr(self, "_node"):
      return self._node
    v = ccs_tree()
    res = ccs_tree_configuration_get_node(self.handle, ct.byref(v))
    Error.check(res)
    self._node = Tree.from_handle(v)
    return self._node

  def check(self):
    valid = ccs_bool()
    res = ccs_tree_configuration_check(self.handle, ct.byref(valid))
    Error.check(res)
    return not (valid.value == 0)

  @property
  def hash(self):
    if hasattr(self, "_hash"):
      return self._hash
    v = ccs_hash()
    res = ccs_tree_configuration_hash(self.handle, ct.byref(v))
    Error.check(res)
    self._hash = v.value
    return self._hash

  def __hash__(self):
    return self.hash

  def cmp(self, other):
    v = ct.c_int()
    res = ccs_tree_configuration_cmp(self.handle, other.handle, ct.byref(v))
    Error.check(res)
    return v.value

  def __lt__(self, other):
    v = ct.c_int()
    res = ccs_tree_configuration_cmp(self.handle, other.handle, ct.byref(v))
    Error.check(res)
    return v.value < 0

  def __le__(self, other):
    v = ct.c_int()
    res = ccs_tree_configuration_cmp(self.handle, other.handle, ct.byref(v))
    Error.check(res)
    return v.value <= 0

  def __gt__(self, other):
    v = ct.c_int()
    res = ccs_tree_configuration_cmp(self.handle, other.handle, ct.byref(v))
    Error.check(res)
    return v.value > 0

  def __ge__(self, other):
    v = ct.c_int()
    res = ccs_tree_configuration_cmp(self.handle, other.handle, ct.byref(v))
    Error.check(res)
    return v.value >= 0

  def __eq__(self, other):
    v = ct.c_int()
    res = ccs_tree_configuration_cmp(self.handle, other.handle, ct.byref(v))
    Error.check(res)
    return v.value == 0

  def __ne__(self, other):
    v = ct.c_int()
    res = ccs_tree_configuration_cmp(self.handle, other.handle, ct.byref(v))
    Error.check(res)
    return v.value != 0
