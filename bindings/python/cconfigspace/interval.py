import ctypes as ct
from . import libcconfigspace
from .base import Error, ccs_result, ccs_numeric_type, ccs_numeric, ccs_float, ccs_int, ccs_bool, ccs_false, ccs_true, _ccs_get_function

class ccs_interval(ct.Structure):
  _fields_ = [('_type', ccs_numeric_type),
              ('_lower', ccs_numeric),
              ('_upper', ccs_numeric),
              ('_lower_included', ccs_bool),
              ('_upper_included', ccs_bool)]

  def __init__(self, t = ccs_numeric_type.FLOAT, lower = 0.0, upper = 1.0, lower_included = True, upper_included = False):
    if t == ccs_numeric_type.INT:
      self._lower.i = lower
      self._upper.i = upper
    elif t == ccs_numeric_type.FLOAT:
      self._lower.f = lower
      self._upper.f = upper
    else:
      raise Error(ccs_result(ccs_result.ERROR_INVALID_VALUE))
    self._type.value = t
    if lower_included:
      self._lower_included = ccs_true
    else:
      self._lower_included = ccs_false
    if upper_included:
      self._upper_included = ccs_true
    else:
      self._upper_included = ccs_false

  @property
  def type(self):
    return self._type.value

  @type.setter
  def type(self, v):
    self._type.value = v

  @property
  def lower(self):
    t = self.type
    if t == ccs_numeric_type.INT:
      return self._lower.i
    elif t == ccs_numeric_type.FLOAT:
      return self._lower.f
    else:
      raise Error(ccs_result(ccs_result.ERROR_INVALID_VALUE))

  @lower.setter
  def lower(self, value):
    t = self.type
    if t == ccs_numeric_type.INT:
      self._lower.i = value
    elif t == ccs_numeric_type.FLOAT:
      self._lower.f = value
    else:
      raise Error(ccs_result(ccs_result.ERROR_INVALID_VALUE))

  @property
  def upper(self):
    t = self.type
    if t == ccs_numeric_type.INT:
      return self._upper.i
    elif t == ccs_numeric_type.FLOAT:
      return self._upper.f
    else:
      raise Error(ccs_result(ccs_result.ERROR_INVALID_VALUE))

  @upper.setter
  def upper(self, value):
    t = self.type
    if t == ccs_numeric_type.INT:
      self._upper.i = value
    elif t == ccs_numeric_type.FLOAT:
      self._upper.f = value
    else:
      raise Error(ccs_result(ccs_result.ERROR_INVALID_VALUE))

  @property
  def lower_included(self):
    return False if self._lower_included == ccs_false else True

  @lower_included.setter
  def lower_included(self, value):
    if value:
      self._lower_included = ccs_true
    else:
      self._lower_included = ccs_false

  @property
  def upper_included(self):
    return False if self._upper_included == ccs_false else True

  @upper_included.setter
  def upper_included(self, value):
    if value:
      self._upper_included = ccs_true
    else:
      self._upper_included = ccs_false

  @property
  def empty(self):
    v = ccs_bool(0)
    res = ccs_interval_empty(ct.byref(self), ct.byref(v))
    Error.check(res)
    return False if v.value == ccs_false else True

  def intersect(self, other):
    v = ccs_interval()
    res = ccs_interval_intersect(ct.byref(self), ct.byref(other), ct.byref(v))
    Error.check(res)
    return v

  def __eq__(self, other):
    v = ccs_bool(0)
    res = ccs_interval_equal(ct.byref(self), ct.byref(other), ct.byref(v))
    Error.check(res)
    return False if v.value == ccs_false else True

  # this works around a subtle bug in union support...
  def include(self, value):
    v = ccs_numeric()
    t = self.type
    if t == ccs_numeric_type.INT:
      v.i = value
    elif t == ccs_numeric_type.FLOAT:
      v.f = value
    res = ccs_interval_include(ct.byref(self), v.i)
    return False if res == ccs_false else True

  def __str__(self):
    s = ""
    s += "[" if  self.lower_included else "("
    s += "{}, {}".format(self.lower, self.upper)
    s += "]" if  self.upper_included else ")"
    return s

ccs_interval_empty = _ccs_get_function("ccs_interval_empty", [ct.POINTER(ccs_interval), ct.POINTER(ccs_bool)])
ccs_interval_intersect = _ccs_get_function("ccs_interval_intersect", [ct.POINTER(ccs_interval), ct.POINTER(ccs_interval), ct.POINTER(ccs_interval)])
ccs_interval_equal = _ccs_get_function("ccs_interval_equal", [ct.POINTER(ccs_interval), ct.POINTER(ccs_interval), ct.POINTER(ccs_bool)])
ccs_interval_include = _ccs_get_function("ccs_interval_include", [ct.POINTER(ccs_interval), ccs_int], ccs_bool)

