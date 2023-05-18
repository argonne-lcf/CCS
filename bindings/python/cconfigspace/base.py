import ctypes as ct
import json
import sys
import traceback
from . import libcconfigspace

class Version(ct.Structure):
  _fields_ = [("revision", ct.c_ushort),
              ("patch",    ct.c_ushort),
              ("minor",    ct.c_ushort),
              ("major",    ct.c_ushort)]

  def __str__(self):
    return "{}.{}.{}.{}".format(self.major, self.minor, self.patch, self.revision)

# Base types
ccs_float             = ct.c_double
ccs_int               = ct.c_longlong
ccs_bool              = ct.c_int32
ccs_evaluation_result = ct.c_int32
ccs_hash              = ct.c_uint32
ccs_object            = ct.c_void_p

# Objects
ccs_rng                 = ccs_object
ccs_distribution        = ccs_object
ccs_parameter           = ccs_object
ccs_expression          = ccs_object
ccs_context             = ccs_object
ccs_configuration_space = ccs_object
ccs_binding             = ccs_object
ccs_configuration       = ccs_object
ccs_features_space      = ccs_object
ccs_features            = ccs_object
ccs_objective_space     = ccs_object
ccs_evaluation          = ccs_object
ccs_features_evaluation = ccs_object
ccs_tuner               = ccs_object
ccs_features_tuner      = ccs_object
ccs_map                 = ccs_object
ccs_error_stack         = ccs_object
ccs_tree                = ccs_object
ccs_tree_space          = ccs_object
ccs_tree_configuration  = ccs_object
ccs_tree_evaluation     = ccs_object
ccs_tree_tuner          = ccs_object

ccs_false = 0
ccs_true = 1

# https://www.python-course.eu/python3_metaclasses.php
class Singleton(type):
  _instances = {}
  def __call__(cls, *args, **kwargs):
    if cls not in cls._instances:
      cls._instances[cls] = super(Singleton, cls).__call__(*args, **kwargs)
    return cls._instances[cls]

class Inactive(metaclass=Singleton):
  def __str__(self):
    return "Inactive"

  def __repr__(self):
    return "Inactive"

inactive = Inactive()

# derived and adapted from http://code.activestate.com/recipes/576415/
class CEnumerationType(type(ct.c_int)):
  def __new__(metacls, name, bases, dict):
    if not "_members_" in dict:
      raise ValueError("CEnumeration must define a _members_ attribute")
    scope = False
    if "_scope_" in dict:
      scope = dict["_scope_"]
    last = -1
    if isinstance(dict["_members_"], list):
      _members_ = {}
      for item in dict["_members_"]:
        if isinstance(item, tuple):
          (i, v) = item
          _members_[i] = v
          last = v
        else:
          last += 1
          _members_[item] = last
      dict["_members_"] = _members_
    _reverse_members_ = {}
    for key,value in dict["_members_"].items():
      dict[key] = value
      _reverse_members_[value] = key
    dict["_reverse_members_"] = _reverse_members_
    cls = type(ct.c_int).__new__(metacls, name, bases, dict)
    for key,value in cls._members_.items():
      if scope:
        cls_name = name.upper()
        if name.startswith("ccs_"):
          cls_name = cls_name[4:]
        globals()[cls_name + "_" + key] = value
    return cls

  def __contains__(self, value):
    return value in self._members_.values()

  def __repr__(self):
    return "<Enumeration %s>" % self.__name__

class CEnumeration(ct.c_int, metaclass=CEnumerationType):
  _members_ = {}
  def __init__(self, value):
    ct.c_int.__init__(self, value)

  @property
  def name(self):
    if self.value in self._reverse_members_:
      return self._reverse_members_[self.value]
    else:
      raise ValueError("No enumeration member with value %r" % value)

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
    return "<member %s(%d) of %r>" % (self.name, self.value, self.__class__)

  def __str__(self):
    return "%s.%s" % (self.__class__.__name__, self.name)

class ObjectType(CEnumeration):
  _members_ = [
    ('RNG', 0),
    'DISTRIBUTION',
    'PARAMETER',
    'EXPRESSION',
    'CONFIGURATION_SPACE',
    'CONFIGURATION',
    'OBJECTIVE_SPACE',
    'EVALUATION',
    'TUNER',
    'FEATURES_SPACE',
    'FEATURES',
    'FEATURES_EVALUATION',
    'FEATURES_TUNER',
    'MAP',
    'ERROR_STACK',
    'TREE',
    'TREE_SPACE',
    'TREE_CONFIGURATION',
    'TREE_EVALUATION',
    'TREE_TUNER' ]

class Result(CEnumeration):
  _members_ = [
    ('AGAIN',                          1),
    ('SUCCESS',                        0),
    ('ERROR_INVALID_OBJECT',          -1),
    ('ERROR_INVALID_VALUE',           -2),
    ('ERROR_INVALID_TYPE',            -3),
    ('ERROR_INVALID_SCALE',           -4),
    ('ERROR_INVALID_DISTRIBUTION',    -5),
    ('ERROR_INVALID_EXPRESSION',      -6),
    ('ERROR_INVALID_PARAMETER',       -7),
    ('ERROR_INVALID_CONFIGURATION',   -8),
    ('ERROR_INVALID_NAME',            -9),
    ('ERROR_INVALID_CONDITION',      -10),
    ('ERROR_INVALID_TUNER',          -11),
    ('ERROR_INVALID_GRAPH',          -12),
    ('ERROR_TYPE_NOT_COMPARABLE',    -13),
    ('ERROR_INVALID_BOUNDS',         -14),
    ('ERROR_OUT_OF_BOUNDS',          -15),
    ('ERROR_SAMPLING_UNSUCCESSFUL',  -16),
    ('ERROR_OUT_OF_MEMORY',          -17),
    ('ERROR_UNSUPPORTED_OPERATION',  -18),
    ('ERROR_INVALID_EVALUATION',     -19),
    ('ERROR_INVALID_FEATURES',       -20),
    ('ERROR_INVALID_FEATURES_TUNER', -21),
    ('ERROR_INVALID_FILE_PATH',      -22),
    ('ERROR_NOT_ENOUGH_DATA',        -23),
    ('ERROR_DUPLICATE_HANDLE',       -24),
    ('ERROR_INVALID_HANDLE',         -25),
    ('ERROR_SYSTEM',                 -26),
    ('ERROR_EXTERNAL',               -27),
    ('ERROR_INVALID_TREE',           -28),
    ('ERROR_INVALID_TREE_SPACE',     -29),
    ('ERROR_INVALID_TREE_TUNER',     -30) ]

class DataType(CEnumeration):
  _members_ = [
    ('NONE', 0),
    'INT',
    'FLOAT',
    'BOOL',
    'STRING',
    'INACTIVE',
    'OBJECT' ]

class DatumFlag(CEnumeration):
  _members_ = [
    ('DEFAULT', 0),
    ('TRANSIENT', (1 << 0)),
    ('UNPOOLED', (1 << 1)),
    ('ID', (1 << 2))]

DatumFlags = ct.c_uint

class NumericType(CEnumeration):
  _members_ = [
    ('INT', DataType.INT),
    ('FLOAT', DataType.FLOAT) ]

class Numeric(ct.Union):
  _fields_ = [('f', ccs_float),
              ('i', ccs_int)]

  def __init__(self, v = 0):
    super().__init__()
    self.set_value(v)

  def get_value(self, t):
    if t == NumericType.INT:
      return self.i
    elif t == NumericType.FLOAT:
      return self.f
    else:
      raise Error(Result(Result.ERROR_INVALID_VALUE))

  def set_value(self, v):
    if isinstance(v, int):
      self.i = v
    elif isinstance(v, float):
      self.f = v
    else:
      raise Error(Result(Result.ERROR_INVALID_VALUE))

class Value(ct.Union):
  _fields_ = [('f', ccs_float),
              ('i', ccs_int),
              ('s', ct.c_char_p),
              ('o', ccs_object)]

class DatumFix(ct.Structure):
  _fields_ = [('value', ccs_int),
              ('type', DataType),
              ('flags', DatumFlags)]

  def __init__(self, v = None):
    super().__init__()
    if v:
      self.value = v._value.i
      self.type = v._type
      self.flags = v.flags

class Datum(ct.Structure):
  _fields_ = [('_value', Value),
              ('_type', DataType),
              ('flags', DatumFlags)]

  def __init__(self, v = None):
    super().__init__()
    self.value = v

  @property
  def type(self):
    return self._type.value

  @type.setter
  def type(self, v):
    self._type.value = v

  @property
  def value(self):
    t = self.type
    if t == DataType.NONE:
      return None
    elif t == DataType.INT:
      return self._value.i
    elif t == DataType.FLOAT:
      return self._value.f
    elif t == DataType.BOOL:
      return False if self._value.i == ccs_false else True
    elif t == DataType.STRING:
      return self._value.s.decode()
    elif t == DataType.INACTIVE:
      return inactive
    elif t == DataType.OBJECT:
      if self.flags == DatumFlag.ID:
        return Object(ct.c_void_p(self._value.o), retain = False, auto_release = False)
      else:
        return Object.from_handle(ct.c_void_p(self._value.o))
    else:
      raise Error(Result(Result.ERROR_INVALID_VALUE))

  def set_value(self, v, string_store = None, object_store = None):
    if v is None:
      self.type = DataType.NONE
      self._value.i = 0
      self.flags = 0
    elif isinstance(v, bool):
      self.type = DataType.BOOL
      self._value.i = 1 if v else 0
      self.flags = 0
    elif isinstance(v, int):
      self.type = DataType.INT
      self._value.i = v
      self.flags = 0
    elif isinstance(v, float):
      self.type = DataType.FLOAT
      self._value.f = v
      self.flags = 0
    elif isinstance(v, str):
      self.type = DataType.STRING
      s = ct.c_char_p(str.encode(v))
      if string_store:
        string_store.append(s)
      else:
        self._string = s
      self._value.s = s
      self.flags = DatumFlag.TRANSIENT
    elif v is inactive:
      self.type = DataType.INACTIVE
      self._value.i = 0
      self.flags = 0
    elif isinstance(v, Object):
      self.type = DataType.OBJECT
      self._value.o = v.handle
      if type(v) == Object:
        self.flags = DatumFlag.ID
      else:
        self._object = v
        self.flags = DatumFlag.TRANSIENT
    else:
      raise Error(Result(Result.ERROR_INVALID_VALUE))

  @value.setter
  def value(self, v):
    self.set_value(v)

class Error(Exception):
  def __init__(self, code):
    # see: https://stackoverflow.com/a/54653137 for ideas
    self._code = code
    self._stack = get_thread_error()
    if self._stack:
      elems = '\n'.join( [f"  File \"{e.file.decode()}\", line {e.line}, in {e.func.decode()}" for e in self._stack.elems()] )
      if elems:
        msg = f"{code}: {self._stack.message}\n{elems}"
      else:
        msg = f"{code}: {self._stack.message}"
      super().__init__(msg)
    else:
      super().__init__(code)

  @classmethod
  def check(cls, err):
    if err.value < 0:
      raise cls(err)

  @classmethod
  def set_error(cls, exc):
    if isinstance(exc, Error):
      if exc._stack:
        stack = exc._stack
      else:
        stack = ErrorStack(error = exc._code)
    else:
      stack = ErrorStack(error = Result.ERROR_EXTERNAL, message = str(exc))
    for s in traceback.extract_tb(sys.exc_info()[2]):
      stack.push(s.filename, s.lineno, s.name)
    set_thread_error(stack)
    return stack.code.value

class SerializeFormat(CEnumeration):
  _members_ = [
    ('BINARY', 0)
  ]

class SerializeOperation(CEnumeration):
  _members_ = [
    ('SIZE', 0),
    'MEMORY',
    'FILE',
    'FILE_DESCRIPTOR'
  ]

class SerializeOption(CEnumeration):
  _members_ = [
    ('END', 0),
    'NON_BLOCKING',
    'CALLBACK'
  ]

class DeserializeOption(CEnumeration):
  _members_ = [
    ('END', 0),
    'HANDLE_MAP',
    'VECTOR',
    'DATA',
    'NON_BLOCKING',
    'CALLBACK'
  ]

def _ccs_get_function(method, argtypes = [], restype = Result):
  res = getattr(libcconfigspace, method)
  res.restype = restype
  res.argtypes = argtypes
  return res

ccs_init = _ccs_get_function("ccs_init")
ccs_get_result_name = _ccs_get_function("ccs_get_result_name", [Result, ct.POINTER(ct.c_char_p)])
ccs_get_version = _ccs_get_function("ccs_get_version", restype = Version)
ccs_get_version_string = _ccs_get_function("ccs_get_version_string", restype = ct.c_char_p)
ccs_retain_object = _ccs_get_function("ccs_retain_object", [ccs_object])
ccs_release_object = _ccs_get_function("ccs_release_object", [ccs_object])
ccs_object_get_type = _ccs_get_function("ccs_object_get_type", [ccs_object, ct.POINTER(ObjectType)])
ccs_object_get_refcount = _ccs_get_function("ccs_object_get_refcount", [ccs_object, ct.POINTER(ct.c_int)])
ccs_object_destroy_callback_type = ct.CFUNCTYPE(None, ccs_object, ct.c_void_p)
ccs_object_set_destroy_callback = _ccs_get_function("ccs_object_set_destroy_callback", [ccs_object, ccs_object_destroy_callback_type, ct.c_void_p])
ccs_object_set_user_data = _ccs_get_function("ccs_object_set_user_data", [ccs_object, ct.py_object])
ccs_object_get_user_data = _ccs_get_function("ccs_object_get_user_data", [ccs_object, ct.POINTER(ct.c_void_p)])
ccs_object_serialize_callback_type = ct.CFUNCTYPE(Result, ccs_object, ct.c_size_t, ct.c_void_p, ct.POINTER(ct.c_size_t), ct.c_void_p)
ccs_object_set_serialize_callback = _ccs_get_function("ccs_object_set_serialize_callback", [ccs_object, ccs_object_serialize_callback_type, ct.c_void_p])
ccs_object_deserialize_callback_type = ct.CFUNCTYPE(Result, ccs_object, ct.c_size_t, ct.c_void_p, ct.c_void_p)
# Variadic methods
ccs_object_serialize = getattr(libcconfigspace, "ccs_object_serialize")
ccs_object_serialize.restype = Result
ccs_object_deserialize = getattr(libcconfigspace, "ccs_object_deserialize")
ccs_object_deserialize.restype = Result

_res = ccs_init()
Error.check(_res)

version = ccs_get_version()

version_string = ccs_get_version_string().decode()

class Object:

  def __init__(self, handle, retain = False, auto_release = True):
    if handle is None:
      raise Error(Result(Result.ERROR_INVALID_OBJECT))
    self._handle = handle
    self.auto_release = auto_release
    if retain:
      res = ccs_retain_object(handle)
      Error.check(res)

  def __del__(self):
    if self.auto_release:
      res = ccs_release_object(self._handle)
      Error.check(res)

  @property
  def handle(self):
    return self._handle

  @property
  def object_type(self):
    if hasattr(self, "_object_type"):
      return self._object_type
    t = ObjectType(0)
    res = ccs_object_get_type(self.handle, ct.byref(t))
    Error.check(res)
    self._object_type = t.value
    return self._object_type

  @property
  def refcount(self):
    c = ct.c_int(0)
    res = ccs_object_get_refcount(self.handle, ct.byref(c))
    Error.check(res)
    return c.value

  @property
  def user_data(self):
    v = ct.c_void_p()
    res = ccs_object_get_user_data(self.handle, ct.byref(v))
    Error.check(res)
    if v:
      v = ct.cast(v, ct.py_object).value
    else:
      v = None
    return v

  @user_data.setter
  def user_data(self, ud):
    if ud is not None:
      c_ud = ct.py_object(ud)
    else:
      c_ud = None
    res = ccs_object_set_user_data(self.handle, c_ud)
    Error.check(res)
    _register_user_data(self._handle, ud)

  @classmethod
  def _from_handle(cls, h, retain, auto_release):
    t = ObjectType(0)
    res = ccs_object_get_type(h, ct.byref(t))
    Error.check(res)
    v = t.value
    klass = cls.CLASS_MAP[v]
    if klass is None:
      raise Error(Result(Result.ERROR_INVALID_OBJECT))
    return klass.from_handle(h, retain = retain, auto_release = auto_release)

  @classmethod
  def from_handle(cls, h):
    r = ct.c_int(0)
    res = ccs_object_get_refcount(h, ct.byref(r))
    Error.check(res)
    r = r.value
    if r == 0:
      retain = False
      auto_release = False
    else:
      retain = True
      auto_release = True
    return cls._from_handle(h, retain, auto_release)

  def set_destroy_callback(self, callback, user_data = None):
    _set_destroy_callback(self.handle, callback, user_data = user_data)

  def serialize(self, format = 'binary', path = None, file_descriptor = None, callback = None, callback_data = None):
    if format != 'binary':
      raise Error(Result(Result.ERROR_INVALID_VALUE))
    if path and file_descriptor:
      raise Error(Result(Result.ERROR_INVALID_VALUE))
    options = [SerializeOption.END]
    if callback:
      cb_wrapper = _get_serialize_callback_wrapper(callback)
      cb_wrapper_func = ccs_object_serialize_callback_type(cb_wrapper)
      options = [SerializeOption.CALLBACK, cb_wrapper_func, ct.py_object(callback_data)] + options
    elif _default_user_data_serializer:
      options = [SerializeOption.CALLBACK, _default_user_data_serializer, ct.py_object()] + options
    if path:
      p = str.encode(path)
      pp = ct.c_char_p(p)
      res = ccs_object_serialize(self.handle, SerializeFormat.BINARY, SerializeOperation.FILE, pp, *options)
      Error.check(res)
      return None
    elif file_descriptor:
      fd = ct.c_int(file_descriptor)
      res = ccs_object_serialize(self.handle, SerializeFormat.BINARY, SerializeOperation.FILE_DESCRIPTOR, fd, *options)
      Error.check(res)
      return None
    else:
      s = ct.c_size_t(0)
      res = ccs_object_serialize(self.handle, SerializeFormat.BINARY, SerializeOperation.SIZE, ct.byref(s), *options)
      Error.check(res)
      v = (ct.c_byte * s.value)()
      res = ccs_object_serialize(self.handle, SerializeFormat.BINARY, SerializeOperation.MEMORY, ct.sizeof(v), v, *options)
      Error.check(res)
      return v

  @classmethod
  def deserialize(cls, format = 'binary', handle_map = None, vector = None, data = None, path = None, buffer = None, file_descriptor = None, callback = None, callback_data = None):
    if format != 'binary':
      raise Error(Result(Result.ERROR_INVALID_VALUE))
    mode_count = 0;
    if path:
      mode_count += 1
    if buffer:
      mode_count += 1
    if file_descriptor:
      mode_count += 1
    if not mode_count == 1:
      raise Error(Result(Result.ERROR_INVALID_VALUE))
    o = ccs_object(0)
    options = [DeserializeOption.END]
    if handle_map:
      options = [DeserializeOption.HANDLE_MAP, handle_map.handle] + options
    if vector:
      options = [DeserializeOption.VECTOR, ct.byref(vector)] + options
    if data:
      options = [DeserializeOption.DATA, ct.py_object(data)] + options
    if callback:
      cb_wrapper = _get_deserialize_callback_wrapper(callback)
      cb_wrapper_func = ccs_object_deserialize_callback_type(cb_wrapper)
      options = [DeserializeOption.CALLBACK, cb_wrapper_func, ct.py_object(callback_data)] + options
    elif _default_user_data_deserializer:
      options = [DeserializeOption.CALLBACK, _default_user_data_deserializer, ct.py_object()] + options
    if buffer:
      s = ct.c_size_t(ct.sizeof(buffer))
      res = ccs_object_deserialize(ct.byref(o), SerializeFormat.BINARY, SerializeOperation.MEMORY, s, buffer, *options)
    elif path:
      p = str.encode(path)
      pp = ct.c_char_p(p)
      res = ccs_object_deserialize(ct.byref(o), SerializeFormat.BINARY, SerializeOperation.FILE, pp, *options)
    elif file_descriptor:
      fd = ct.c_int(file_descriptor)
      res = ccs_object_deserialize(ct.byref(o), SerializeFormat.BINARY, SerializeOperation.FILE_DESCRIPTOR, fd, *options)
    else:
      raise Error(Result(Result.ERROR_INVALID_VALUE))
    Error.check(res)
    return cls._from_handle(o, False, True)

_data_store = {}

def _get_serialize_callback_wrapper(callback):
  def serialize_callback_wrapper(obj, serialize_data_size, serialize_data, serialize_data_size_ret, cb_data):
    try:
      p_sd = ct.cast(serialize_data, ct.c_void_p)
      p_sdsz = ct.cast(serialize_data_size_ret, ct.POINTER(ct.c_size_t))
      cb_data = ct.cast(cb_data, ct.c_void_p)
      if cb_data:
        cb_data = ct.cast(cb_data, ct.py_object).value
      else:
        cb_data = None
      serialized = callback(Object.from_handle(ccs_object(obj)), cb_data, True if serialize_data_size == 0 else False)
      if p_sd and serialize_data_size < ct.sizeof(serialized):
        raise Error(Result(Result.ERROR_INVALID_VALUE))
      if p_sd:
        ct.memmove(p_sd, ct.byref(serialized), ct.sizeof(serialized))
      if p_sdsz:
        p_sdsz[0] = ct.sizeof(serialized)
      return Result.SUCCESS
    except Exception as e:
      return Error.set_error(e)
  return serialize_callback_wrapper

def _get_deserialize_callback_wrapper(callback):
  def deserialize_callback_wrapper(obj, serialize_data_size, serialize_data, cb_data):
    try:
      p_sd = ct.cast(serialize_data, ct.c_void_p)
      cb_data = ct.cast(cb_data, ct.c_void_p)
      if cb_data:
        cb_data = ct.cast(cb_data, ct.py_object).value
      else:
        cb_data = None
      if p_sd:
        serialized = ct.cast(p_sd, ct.POINTER(ct.c_byte * serialize_data_size))
      else:
        serialized = None
      callback(Object.from_handle(ccs_object(obj)), serialized, cb_data)
      return Result.SUCCESS
    except Exception as e:
      return Error.set_error(e)
  return deserialize_callback_wrapper

def _json_user_data_serializer(obj, data, size):
  string = json.dumps(obj.user_data).encode("ascii")
  return ct.create_string_buffer(string)

def _json_user_data_deserializer(obj, serialized, data):
  serialized = ct.cast(serialized, ct.c_char_p)
  obj.user_data = json.loads(serialized.value)

_json_user_data_serializer_wrap = _get_serialize_callback_wrapper(_json_user_data_serializer)
_json_user_data_serializer_func = ccs_object_serialize_callback_type(_json_user_data_serializer_wrap)

_json_user_data_deserializer_wrap = _get_deserialize_callback_wrapper(_json_user_data_deserializer)
_json_user_data_deserializer_func = ccs_object_deserialize_callback_type(_json_user_data_deserializer_wrap)

_default_user_data_serializer = _json_user_data_serializer_func
_default_user_data_deserializer = _json_user_data_deserializer_func

# Delete wrappers are responsible for deregistering the object data_store
def _register_vector(handle, vector_data):
  value = handle.value
  if value in _data_store:
    raise Error(Result(Result.ERROR_INVALID_VALUE))
  _data_store[value] = dict.fromkeys(['callbacks', 'user_data', 'serialize_calback', 'strings'])
  _data_store[value]['callbacks'] = vector_data
  _data_store[value]['strings'] = []

def _unregister_vector(handle):
  value = handle.value
  del _data_store[value]

# If objects don't have a user-defined del operation, then the first time a
# data needs to be registered a destruction callback is attached.
def _register_destroy_callback(handle):
  value = handle.value
  def cb(obj, data):
    del _data_store[value]
  cb_func = ccs_object_destroy_callback_type(cb)
  res = ccs_object_set_destroy_callback(handle, cb_func, None)
  Error.check(res)
  _data_store[value] = dict.fromkeys(['callbacks', 'user_data', 'serialize_calback', 'strings'])
  _data_store[value]['callbacks'] = [ [ cb, cb_func ] ]
  _data_store[value]['strings'] = []

def _register_callback(handle, callback_data):
  value = handle.value
  if value not in _data_store:
    _register_destroy_callback(handle)
  _data_store[value]['callbacks'].append( callback_data )

def _register_user_data(handle, user_data):
  value = handle.value
  if value not in _data_store:
    _register_destroy_callback(handle)
  _data_store[value]['user_data'] = user_data

def _register_string(handle, string):
  value = handle.value
  if value not in _data_store:
    _register_destroy_callback(handle)
  _data_store[value]['strings'].append( string )

def _register_serialize_callback(handle, callback_data):
  value = handle.value
  if value not in _data_store:
    _register_destroy_callback(handle)
  _data_store[value]['serialize_calback'] = callback_data

def deserialize(format = 'binary', handle_map = None, path = None, buffer = None, callback = None, callback_data = None):
  return Object.deserialize(format = format, handle_map = handle_map, path = path, buffer = buffer, callback = callback, callback_data = callback_data)

def _set_destroy_callback(handle, callback, user_data = None):
  if callback is None:
    raise Error(Result(Result.ERROR_INVALID_VALUE))
  def cb_wrapper(obj, data):
    callback(Object.from_handle(obj), data)
  cb_wrapper_func = ccs_object_destroy_callback_type(cb_wrapper)
  res = ccs_object_set_destroy_callback(handle, cb_wrapper_func, user_data)
  Error.check(res)
  _register_callback(handle, [callback, cb_wrapper, cb_wrapper_func, user_data])

def _set_serialize_callback(handle, callback, user_data = None):
  if callback is None:
    res = ccs_object_set_serialize_callback(handle, None, None)
    Error.check(res)
    _register_serialize_callback(handle, None)
  else:
    cb_wrapper = _get_serialize_callback_wrapper(callback)
    cb_wrapper_func = ccs_object_serialize_callback_type(cb_wrapper)
    res = ccs_object_set_serialize_callback(handle, cb_wrapper_func, user_data)
    Error.check(res)
    _register_serialize_callback(handle, [callback, cb_wrapper, cb_wrapper_func, user_data])

_ccs_id = 0
def _ccs_get_id():
  global _ccs_id
  res = _ccs_id
  _ccs_id += 1
  return res

from .rng import Rng
from .distribution import Distribution
from .parameter import Parameter
from .expression import Expression
from .configuration_space import ConfigurationSpace
from .configuration import Configuration
from .features_space import FeaturesSpace
from .features import Features
from .objective_space import ObjectiveSpace
from .evaluation import Evaluation
from .features_evaluation import FeaturesEvaluation
from .tuner import Tuner
from .features_tuner import FeaturesTuner
from .map import Map
from .error_stack import ErrorStack, get_thread_error, set_thread_error, clear_thread_error
from .tree import Tree
from .tree_space import TreeSpace
from .tree_configuration import TreeConfiguration
from .tree_evaluation import TreeEvaluation
from .tree_tuner import TreeTuner

setattr(Object, 'CLASS_MAP', {
  ObjectType.RNG: Rng,
  ObjectType.DISTRIBUTION: Distribution,
  ObjectType.PARAMETER: Parameter,
  ObjectType.EXPRESSION: Expression,
  ObjectType.CONFIGURATION_SPACE: ConfigurationSpace,
  ObjectType.CONFIGURATION: Configuration,
  ObjectType.FEATURES_SPACE: ConfigurationSpace,
  ObjectType.FEATURES: Features,
  ObjectType.OBJECTIVE_SPACE: ObjectiveSpace,
  ObjectType.EVALUATION: Evaluation,
  ObjectType.FEATURES_EVALUATION: FeaturesEvaluation,
  ObjectType.TUNER: Tuner,
  ObjectType.FEATURES_TUNER: FeaturesTuner,
  ObjectType.MAP: Map,
  ObjectType.ERROR_STACK: ErrorStack,
  ObjectType.TREE: Tree,
  ObjectType.TREE_SPACE: TreeSpace,
  ObjectType.TREE_CONFIGURATION: TreeConfiguration,
  ObjectType.TREE_EVALUATION: TreeEvaluation,
  ObjectType.TREE_TUNER: TreeTuner
})


