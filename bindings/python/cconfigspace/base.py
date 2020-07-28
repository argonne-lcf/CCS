import ctypes as ct
from . import libcconfigspace
from enum import IntEnum, auto

ccs_init = libcconfigspace.ccs_init
ccs_init.restype = ct.c_int

class Version(ct.Structure):
  _fields_ = [("revision", ct.c_ushort),
              ("patch",    ct.c_ushort),
              ("minor",    ct.c_ushort),
              ("major",    ct.c_ushort)]

# http://code.activestate.com/recipes/576415/
class CEnumerationType(type(ct.c_int)):
  def __new__(metacls, name, bases, dict):
    if not "_members_" in dict:
      _members_ = {}
      for key,value in dict.items():
        if not key.startswith("_"):
          _members_[key] = value
      dict["_members_"] = _members_
    cls = type(c_uint).__new__(metacls, name, bases, dict)
    for key,value in cls._members_.items():
      globals()[key] = value
    return cls

  def __contains__(self, value):
    return value in self._members_.values()

  def __repr__(self):
    return "<Enumeration %s>" % self.__name__

class CEnumeration(ct.c_int):
  __metaclass__ = CEnumerationType
  _members_ = {}
  def __init__(self, value):
    for k,v in self._members_.items():
      if v == value:
        self.name = k
        break
    else:
      raise ValueError("No enumeration member with value %r" % value)
    ct.c_int.__init__(self, value)

  @classmethod
  def from_param(cls, param):
    if isinstance(param, CEnumeration):
      if param.__class__ != cls:
        raise ValueError("Cannot mix enumeration members")
      else:
        return param
    else:
      return cls(param)

  def __repr__(self):
    return "<member %s=%d of %r>" % (self.name, self.value, self.__class__)

  def __str__(self):
    return "%s.%s" % (self.__class__.__name__, self.name)

class ObjectType(CEnumeration):
  _members_ = {
    'RNG': 0,
    'DISTRIBUTION': 1,
    'HYPERPARAMETER': 2,
    'EXPRESSION': 3,
    'CONFIGURATION_SPACE': 4,
    'CONFIGURATION': 5,
    'OBJECTIVE_SPACE': 6,
    'EVALUATION': 7,
    'TUNER': 8 }

class CTypesIntEnum(IntEnum):
  @classmethod
  def from_param(cls, obj):
    return ct.c_int(int(obj))

  def _generate_next_value_(name, start, count, last_values):
    if len(last_values) == 0:
      return 0
    return last_values[-1] + 1

#class ObjectType(CTypesIntEnum):
#  RNG = auto()
#  DISTRIBUTION = auto()
#  HYPERPARAMETER = auto()
#  EXPRESSION = auto()
#  CONFIGURATION_SPACE = auto()
#  CONFIGURATION = auto()
#  OBJECTIVE_SPACE = auto()
#  EVALUATION = auto()
#  TUNER = auto()

class Error(CTypesIntEnum):
  SUCCESS = auto()
  INVALID_OBJECT = auto()
  INVALID_VALUE = auto()
  INVALID_TYPE = auto()
  INVALID_SCALE = auto()
  INVALID_DISTRIBUTION = auto()
  INVALID_EXPRESSION = auto()
  INVALID_HYPERPARAMETER = auto()
  INVALID_CONFIGURATION = auto()
  INVALID_NAME = auto()
  INVALID_CONDITION = auto()
  INVALID_TUNER = auto()
  INVALID_GRAPH = auto()
  TYPE_NOT_COMPARABLE = auto()
  INVALID_BOUNDS = auto()
  OUT_OF_BOUNDS = auto()
  SAMPLING_UNSUCCESSFUL = auto()
  INACTIVE_HYPERPARAMETER = auto()
  OUT_OF_MEMORY = auto()
  UNSUPPORTED_OPERATION = auto()


class CCSError(Exception):
  def __init__(self, message):
    self.message = message

  @classmethod
  def check(cls, err):
    if err < 0:
      raise cls(Error(-err))

ccs_get_version = libcconfigspace.ccs_get_version
ccs_get_version.restype = Version

ccs_retain_object = libcconfigspace.ccs_retain_object
ccs_retain_object.restype = ct.c_int
ccs_retain_object.argtypes = [ct.c_void_p]

ccs_release_object = libcconfigspace.ccs_release_object
ccs_release_object.restype = ct.c_int
ccs_release_object.argtypes = [ct.c_void_p]

ccs_object_get_type = libcconfigspace.ccs_object_get_type
ccs_object_get_type.restype = ct.c_int
ccs_object_get_type.argtypes = [ct.c_void_p, ct.POINTER(ObjectType)]

ccs_object_get_refcount = libcconfigspace.ccs_object_get_refcount
ccs_object_get_refcount.restype = ct.c_int
ccs_object_get_refcount.argtypes = [ct.c_void_p, ct.POINTER(ct.c_int)]

class Object:
  def __init__(self, handle, retain = False, auto_release = True):
    if handle is None:
      raise CCSError(Error.INVALID_OBJECT)
    self.handle = handle
    self.auto_release = auto_release
    if retain:
      res = ccs_retain_object(handle)
      CCSError.check(res)

  def __del__(self):
    res = ccs_release_object(self.handle)
    CCSError.check(res)

  def object_type(self):
    t = ObjectType(0)
    res = ccs_object_get_type(self.handle, ct.byref(t))
    CCSError.check(res)
    return t

  def refcount(self):
    c = ct.c_int(0)
    res = ccs_object_get_refcount(self.handle, ct.byref(c))
    CCSError.check(res)
    return c.value
