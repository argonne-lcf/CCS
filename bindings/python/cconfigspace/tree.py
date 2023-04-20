import ctypes as ct
from . import libcconfigspace
from .base import Object, Error, ccs_rng, ccs_tree, Datum, DatumFix, ccs_bool, ccs_float, _ccs_get_function

ccs_create_tree = _ccs_get_function("ccs_create_tree", [ ct.c_size_t, DatumFix, ct.POINTER(ccs_tree)])
ccs_tree_get_value = _ccs_get_function("ccs_tree_get_value", [ccs_tree, ct.POINTER(Datum)])
ccs_tree_get_arity = _ccs_get_function("ccs_tree_get_arity", [ccs_tree, ct.POINTER(ct.c_size_t)])
ccs_tree_set_child = _ccs_get_function("ccs_tree_set_child", [ccs_tree, ct.c_size_t, ccs_tree])
ccs_tree_get_child = _ccs_get_function("ccs_tree_get_child", [ccs_tree, ct.c_size_t, ct.POINTER(ccs_tree)])
ccs_tree_get_children = _ccs_get_function("ccs_tree_get_children", [ccs_tree, ct.c_size_t, ct.POINTER(ccs_tree), ct.POINTER(ct.c_size_t)])
ccs_tree_get_parent = _ccs_get_function("ccs_tree_get_parent", [ccs_tree, ct.POINTER(ccs_tree), ct.POINTER(ct.c_size_t)])
ccs_tree_get_position = _ccs_get_function("ccs_tree_get_position", [ccs_tree, ct.c_size_t, ct.POINTER(ct.c_size_t), ct.POINTER(ct.c_size_t)])
ccs_tree_get_values = _ccs_get_function("ccs_tree_get_values", [ccs_tree, ct.c_size_t, ct.POINTER(Datum), ct.POINTER(ct.c_size_t)])
ccs_tree_position_is_valid = _ccs_get_function("ccs_tree_position_is_valid", [ccs_tree, ct.c_size_t, ct.POINTER(ct.c_size_t), ct.POINTER(ccs_bool)])
ccs_tree_get_values_at_position = _ccs_get_function("ccs_tree_get_values_at_position", [ccs_tree, ct.c_size_t, ct.POINTER(ct.c_size_t), ct.c_size_t, ct.POINTER(Datum)])
ccs_tree_get_node_at_position = _ccs_get_function("ccs_tree_get_node_at_position", [ccs_tree, ct.c_size_t, ct.POINTER(ct.c_size_t), ct.POINTER(ccs_tree)])
ccs_tree_get_weight = _ccs_get_function("ccs_tree_get_weight", [ccs_tree, ct.POINTER(ccs_float)])
ccs_tree_set_weight = _ccs_get_function("ccs_tree_set_weight", [ccs_tree, ccs_float])
ccs_tree_get_bias = _ccs_get_function("ccs_tree_get_bias", [ccs_tree, ct.POINTER(ccs_float)])
ccs_tree_set_bias = _ccs_get_function("ccs_tree_set_bias", [ccs_tree, ccs_float])
ccs_tree_sample = _ccs_get_function("ccs_tree_sample", [ccs_tree, ccs_rng, ct.POINTER(ct.c_size_t)])
ccs_tree_samples = _ccs_get_function("ccs_tree_samples", [ccs_tree, ccs_rng, ct.c_size_t, ct.POINTER(ct.c_size_t)])

class Tree(Object):

  def __init__(self, handle = None, retain = False, auto_release = True,
               value = None, arity = None):
    if handle is None:
      handle = ccs_tree(0)
      pv = Datum(value)
      v = DatumFix(pv)
      res = ccs_create_tree(arity, v, ct.byref(handle))
      Error.check(res)
      super().__init__(handle = handle, retain = False)
    else:
      super().__init__(handle = handle, retain = retain, auto_release = auto_release)

  @classmethod
  def from_handle(cls, handle, retain = True, auto_release = True):
    return cls(handle, retain = retain, auto_release = auto_release)

  @property
  def value(self):
    if hasattr(self, "_value"):
      return self._value
    v = Datum()
    res = ccs_tree_get_value(self.handle, ct.byref(v))
    Error.check(res)
    self._value = v.value
    return self._value

  @property
  def arity(self):
    if hasattr(self, "_arity"):
      return self._arity
    v = ct.c_size_t()
    res = ccs_tree_get_arity(self.handle, ct.byref(v))
    Error.check(res)
    self._arity = v.value
    return self._arity

  @property
  def weight(self):
    v = ccs_float()
    res = ccs_tree_get_weight(self.handle, ct.byref(v))
    Error.check(res)
    return v.value

  @weight.setter
  def weight(self, w):
    res = ccs_tree_set_weight(self.handle, w)
    Error.check(res)

  @property
  def bias(self):
    v = ccs_float()
    res = ccs_tree_get_bias(self.handle, ct.byref(v))
    Error.check(res)
    return v.value

  @bias.setter
  def bias(self, b):
    res = ccs_tree_set_bias(self.handle, b)
    Error.check(res)

  def set_child(self, index, child):
    res = ccs_tree_set_child(self.handle, index, child.handle)
    Error.check(res)

  def get_child(self, index):
    v = ccs_tree()
    res = ccs_tree_get_child(self.handle, index, ct.byref(v))
    Error.check(res)
    return Tree.from_handle(v) if v else None

  @property
  def children(self):
    count = self.arity
    if count == 0:
      return []
    v = (ccs_tree * count)()
    res = ccs_tree_get_children(self.handle, count, v, None)
    Error.check(res)
    return [Tree.from_handle(ccs_tree(x)) if x else None for x in v]

  @property
  def parent(self):
    v = ccs_tree()
    res = ccs_tree_get_parent(self.handle, ct.byref(v), None)
    Error.check(res)
    return Tree.from_handle(v) if v else None

  @property
  def index(self):
    v1 = ccs_tree()
    v2 = ct.c_size_t()
    res = ccs_tree_get_parent(self.handle, ct.byref(v1), ct.byref(v2))
    Error.check(res)
    return v2.value if v1 else None


  @property
  def depth(self):
    v = ct.c_size_t()
    res = ccs_tree_get_position(self.handle, 0, None, ct.byref(v))
    Error.check(res)
    return v.value

  @property
  def position(self):
    count = self.depth
    v = (ct.c_size_t * count)()
    res = ccs_tree_get_position(self.handle, count, v, None)
    Error.check(res)
    return list(v)

  @property
  def values(self):
    count = self.depth + 1
    v = (Datum * count)()
    res = ccs_tree_get_values(self.handle, count, v, None)
    Error.check(res)
    return [x.value for x in v]

  def position_is_valid(self, position):
    count = len(position)
    v = (ct.c_size_t * count)(*position)
    b = ccs_bool()
    res = ccs_tree_position_is_valid(self.handle, count, v, ct.byref(b))
    Error.check(res)
    return not (b.value == 0)

  def get_values_at_position(self, position):
    count = len(position)
    v1 = (ct.c_size_t * count)(*position)
    v2 = (Datum * (count + 1))()
    res = ccs_tree_get_values_at_position(self.handle, count, v1, count + 1, v2)
    Error.check(res)
    return [x.value for x in v2]

  def get_node_at_position(self, position):
    count = len(position)
    v1 = (ct.c_size_t * count)(*position)
    v2 = ccs_tree()
    res = ccs_tree_get_node_at_position(self.handle, count, v1, ct.byref(v2))
    Error.check(res)
    return Tree.from_handle(v2)

  def sample(self, rng):
    v = ct.c_size_t()
    res = ccs_tree_sample(self.handle, rng.handle, ct.byref(v))
    Error.check(res)
    return v.value if v.value != self.arity else None

  def samples(self, rng, count):
    v = (ct.c_size_t * count)()
    res = ccs_tree_samples(self.handle, rng.handle, count, v)
    Error.check(res)
    arity = self.arity
    return [x if x != arity else None for x in v]
