import ctypes as ct
import json
import sys
import traceback
from . import libcconfigspace

class ccs_version(ct.Structure):
  _fields_ = [("revision", ct.c_ushort),
              ("patch",    ct.c_ushort),
              ("minor",    ct.c_ushort),
              ("major",    ct.c_ushort)]

  def __str__(self):
    return "{}.{}.{}.{}".format(self.major, self.minor, self.patch, self.revision)

# Base types
ccs_float  = ct.c_double
ccs_int    = ct.c_longlong
ccs_bool   = ct.c_int
ccs_result = ct.c_int
ccs_hash   = ct.c_uint
ccs_object = ct.c_void_p

# Objects
ccs_rng                 = ccs_object
ccs_distribution        = ccs_object
ccs_hyperparameter      = ccs_object
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

ccs_inactive = Inactive()

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
      else:
        globals()[key] = value
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

class ccs_object_type(CEnumeration):
  _members_ = [
    ('RNG', 0),
    'DISTRIBUTION',
    'HYPERPARAMETER',
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
    'TREE_CONFIGURATION' ]

class ccs_error(CEnumeration):
  _members_ = [
    ('AGAIN',                    1),
    ('SUCCESS',                  0),
    ('INVALID_OBJECT',          -1),
    ('INVALID_VALUE',           -2),
    ('INVALID_TYPE',            -3),
    ('INVALID_SCALE',           -4),
    ('INVALID_DISTRIBUTION',    -5),
    ('INVALID_EXPRESSION',      -6),
    ('INVALID_HYPERPARAMETER',  -7),
    ('INVALID_CONFIGURATION',   -8),
    ('INVALID_NAME',            -9),
    ('INVALID_CONDITION',      -10),
    ('INVALID_TUNER',          -11),
    ('INVALID_GRAPH',          -12),
    ('TYPE_NOT_COMPARABLE',    -13),
    ('INVALID_BOUNDS',         -14),
    ('OUT_OF_BOUNDS',          -15),
    ('SAMPLING_UNSUCCESSFUL',  -16),
    ('OUT_OF_MEMORY',          -17),
    ('UNSUPPORTED_OPERATION',  -18),
    ('INVALID_EVALUATION',     -19),
    ('INVALID_FEATURES',       -20),
    ('INVALID_FEATURES_TUNER', -21),
    ('INVALID_FILE_PATH',      -22),
    ('NOT_ENOUGH_DATA',        -23),
    ('HANDLE_DUPLICATE',       -24),
    ('INVALID_HANDLE',         -25),
    ('SYSTEM_ERROR',           -26),
    ('EXTERNAL_ERROR',         -27),
    ('INVALID_TREE',           -28) ]

class ccs_data_type(CEnumeration):
  _members_ = [
    ('NONE', 0),
    'INTEGER',
    'FLOAT',
    'BOOLEAN',
    'STRING',
    'INACTIVE',
    'OBJECT' ]

class ccs_datum_flag(CEnumeration):
  _members_ = [
    ('DEFAULT', 0),
    ('TRANSIENT', (1 << 0)),
    ('UNPOOLED', (1 << 1)),
    ('ID', (1 << 2))]

ccs_datum_flags = ct.c_uint

class ccs_numeric_type(CEnumeration):
  _members_ = [
    ('NUM_INTEGER', ccs_data_type.INTEGER),
    ('NUM_FLOAT', ccs_data_type.FLOAT) ]

class ccs_numeric(ct.Union):
  _fields_ = [('f', ccs_float),
              ('i', ccs_int)]

  def __init__(self, v = 0):
    super().__init__()
    self.set_value(v)

  def get_value(self, t):
    if t == ccs_numeric_type.NUM_INTEGER:
      return self.i
    elif t == ccs_numeric_type.NUM_FLOAT:
      return self.f
    else:
      raise Error(ccs_error(ccs_error.INVALID_VALUE))

  def set_value(self, v):
    if isinstance(v, int):
      self.i = v
    elif isinstance(v, float):
      self.f = v
    else:
      raise Error(ccs_error(ccs_error.INVALID_VALUE))

class ccs_value(ct.Union):
  _fields_ = [('f', ccs_float),
              ('i', ccs_int),
              ('s', ct.c_char_p),
              ('o', ccs_object)]

class ccs_datum_fix(ct.Structure):
  _fields_ = [('value', ccs_int),
              ('type', ccs_data_type),
              ('flags', ccs_datum_flags)]

  def __init__(self, v = None):
    super().__init__()
    if v:
      self.value = v._value.i
      self.type = v._type
      self.flags = v.flags

class ccs_datum(ct.Structure):
  _fields_ = [('_value', ccs_value),
              ('_type', ccs_data_type),
              ('flags', ccs_datum_flags)]

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
    if t == ccs_data_type.NONE:
      return None
    elif t == ccs_data_type.INTEGER:
      return self._value.i
    elif t == ccs_data_type.FLOAT:
      return self._value.f
    elif t == ccs_data_type.BOOLEAN:
      return False if self._value.i == ccs_false else True
    elif t == ccs_data_type.STRING:
      return self._value.s.decode()
    elif t == ccs_data_type.INACTIVE:
      return ccs_inactive
    elif t == ccs_data_type.OBJECT:
      if self.flags == ccs_datum_flag.ID:
        return Object(ct.c_void_p(self._value.o), retain = False, auto_release = False)
      else:
        return Object.from_handle(ct.c_void_p(self._value.o))
    else:
      raise Error(ccs_error(ccs_error.INVALID_VALUE))

  def set_value(self, v, string_store = None, object_store = None):
    if v is None:
      self.type = ccs_data_type.NONE
      self._value.i = 0
      self.flags = 0
    elif isinstance(v, bool):
      self.type = ccs_data_type.BOOLEAN
      self._value.i = 1 if v else 0
      self.flags = 0
    elif isinstance(v, int):
      self.type = ccs_data_type.INTEGER
      self._value.i = v
      self.flags = 0
    elif isinstance(v, float):
      self.type = ccs_data_type.FLOAT
      self._value.f = v
      self.flags = 0
    elif isinstance(v, str):
      self.type = ccs_data_type.STRING
      s = ct.c_char_p(str.encode(v))
      if string_store:
        string_store.append(s)
      else:
        self._string = s
      self._value.s = s
      self.flags = ccs_datum_flag.TRANSIENT
    elif v is ccs_inactive:
      self.type = ccs_data_type.INACTIVE
      self._value.i = 0
      self.flags = 0
    elif isinstance(v, Object):
      self.type = ccs_data_type.OBJECT
      self._value.o = v.handle
      if type(v) == Object:
        self.flags = ccs_datum_flag.ID
      else:
        self._object = v
        self.flags = ccs_datum_flag.TRANSIENT
    else:
      raise Error(ccs_error(ccs_error.INVALID_VALUE))

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
      stack = ErrorStack(error = ccs_error.EXTERNAL_ERROR, message = str(exc))
    for s in traceback.extract_tb(sys.exc_info()[2]):
      stack.push(s.filename, s.lineno, s.name)
    set_thread_error(stack)
    return stack.code.value

class ccs_serialize_format(CEnumeration):
  _members_ = [
    ('BINARY', 0)
  ]

class ccs_serialize_operation(CEnumeration):
  _members_ = [
    ('SIZE', 0),
    'MEMORY',
    'FILE',
    'FILE_DESCRIPTOR'
  ]

class ccs_serialize_option(CEnumeration):
  _members_ = [
    ('END', 0),
    'NON_BLOCKING',
    'CALLBACK'
  ]

class ccs_deserialize_option(CEnumeration):
  _members_ = [
    ('END', 0),
    'HANDLE_MAP',
    'VECTOR',
    'DATA',
    'NON_BLOCKING',
    'CALLBACK'
  ]

def _ccs_get_function(method, argtypes = [], restype = ccs_error):
  res = getattr(libcconfigspace, method)
  res.restype = restype
  res.argtypes = argtypes
  return res

ccs_init = _ccs_get_function("ccs_init")
ccs_fini = _ccs_get_function("ccs_fini")
ccs_get_error_name = _ccs_get_function("ccs_get_error_name", [ccs_error, ct.POINTER(ct.c_char_p)])
ccs_get_version = _ccs_get_function("ccs_get_version", restype = ccs_version)
ccs_retain_object = _ccs_get_function("ccs_retain_object", [ccs_object])
ccs_release_object = _ccs_get_function("ccs_release_object", [ccs_object])
ccs_object_get_type = _ccs_get_function("ccs_object_get_type", [ccs_object, ct.POINTER(ccs_object_type)])
ccs_object_get_refcount = _ccs_get_function("ccs_object_get_refcount", [ccs_object, ct.POINTER(ct.c_int)])
ccs_object_destroy_callback_type = ct.CFUNCTYPE(None, ccs_object, ct.c_void_p)
ccs_object_set_destroy_callback = _ccs_get_function("ccs_object_set_destroy_callback", [ccs_object, ccs_object_destroy_callback_type, ct.c_void_p])
ccs_object_set_user_data = _ccs_get_function("ccs_object_set_user_data", [ccs_object, ct.py_object])
ccs_object_get_user_data = _ccs_get_function("ccs_object_get_user_data", [ccs_object, ct.POINTER(ct.c_void_p)])
ccs_object_serialize_callback_type = ct.CFUNCTYPE(ccs_error, ccs_object, ct.c_size_t, ct.c_void_p, ct.POINTER(ct.c_size_t), ct.c_void_p)
ccs_object_set_serialize_callback = _ccs_get_function("ccs_object_set_serialize_callback", [ccs_object, ccs_object_serialize_callback_type, ct.c_void_p])
ccs_object_deserialize_callback_type = ct.CFUNCTYPE(ccs_error, ccs_object, ct.c_size_t, ct.c_void_p, ct.c_void_p)
# Variadic methods
ccs_object_serialize = getattr(libcconfigspace, "ccs_object_serialize")
ccs_object_serialize.restype = ccs_error
ccs_object_deserialize = getattr(libcconfigspace, "ccs_object_deserialize")
ccs_object_deserialize.restype = ccs_error

_res = ccs_init()
Error.check(_res)

class Object:

  def __init__(self, handle, retain = False, auto_release = True):
    if handle is None:
      raise Error(ccs_error(ccs_error.INVALID_OBJECT))
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
    t = ccs_object_type(0)
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
    t = ccs_object_type(0)
    res = ccs_object_get_type(h, ct.byref(t))
    Error.check(res)
    v = t.value
    klass = cls.CLASS_MAP[v]
    if klass is None:
      raise Error(ccs_error(ccs_error.INVALID_OBJECT))
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
      raise Error(ccs_error(ccs_error.INVALID_VALUE))
    if path and file_descriptor:
      raise Error(ccs_error(ccs_error.INVALID_VALUE))
    options = [ccs_serialize_option.END]
    if callback:
      cb_wrapper = _get_serialize_callback_wrapper(callback)
      cb_wrapper_func = ccs_object_serialize_callback_type(cb_wrapper)
      options = [ccs_serialize_option.CALLBACK, cb_wrapper_func, ct.py_object(callback_data)] + options
    elif _default_user_data_serializer:
      options = [ccs_serialize_option.CALLBACK, _default_user_data_serializer, ct.py_object()] + options
    if path:
      p = str.encode(path)
      pp = ct.c_char_p(p)
      res = ccs_object_serialize(self.handle, ccs_serialize_format.BINARY, ccs_serialize_operation.FILE, pp, *options)
      Error.check(res)
      return None
    elif file_descriptor:
      fd = ct.c_int(file_descriptor)
      res = ccs_object_serialize(self.handle, ccs_serialize_format.BINARY, ccs_serialize_operation.FILE_DESCRIPTOR, fd, *options)
      Error.check(res)
      return None
    else:
      s = ct.c_size_t(0)
      res = ccs_object_serialize(self.handle, ccs_serialize_format.BINARY, ccs_serialize_operation.SIZE, ct.byref(s), *options)
      Error.check(res)
      v = (ct.c_byte * s.value)()
      res = ccs_object_serialize(self.handle, ccs_serialize_format.BINARY, ccs_serialize_operation.MEMORY, ct.sizeof(v), v, *options)
      Error.check(res)
      return v

  @classmethod
  def deserialize(cls, format = 'binary', handle_map = None, vector = None, data = None, path = None, buffer = None, file_descriptor = None, callback = None, callback_data = None):
    if format != 'binary':
      raise Error(ccs_error(ccs_error.INVALID_VALUE))
    mode_count = 0;
    if path:
      mode_count += 1
    if buffer:
      mode_count += 1
    if file_descriptor:
      mode_count += 1
    if not mode_count == 1:
      raise Error(ccs_error(ccs_error.INVALID_VALUE))
    o = ccs_object(0)
    options = [ccs_deserialize_option.END]
    if handle_map:
      options = [ccs_deserialize_option.HANDLE_MAP, handle_map.handle] + options
    if vector:
      options = [ccs_deserialize_option.VECTOR, ct.byref(vector)] + options
    if data:
      options = [ccs_deserialize_option.DATA, ct.py_object(data)] + options
    if callback:
      cb_wrapper = _get_deserialize_callback_wrapper(callback)
      cb_wrapper_func = ccs_object_deserialize_callback_type(cb_wrapper)
      options = [ccs_deserialize_option.CALLBACK, cb_wrapper_func, ct.py_object(callback_data)] + options
    elif _default_user_data_deserializer:
      options = [ccs_deserialize_option.CALLBACK, _default_user_data_deserializer, ct.py_object()] + options
    if buffer:
      s = ct.c_size_t(ct.sizeof(buffer))
      res = ccs_object_deserialize(ct.byref(o), ccs_serialize_format.BINARY, ccs_serialize_operation.MEMORY, s, buffer, *options)
    elif path:
      p = str.encode(path)
      pp = ct.c_char_p(p)
      res = ccs_object_deserialize(ct.byref(o), ccs_serialize_format.BINARY, ccs_serialize_operation.FILE, pp, *options)
    elif file_descriptor:
      fd = ct.c_int(file_descriptor)
      res = ccs_object_deserialize(ct.byref(o), ccs_serialize_format.BINARY, ccs_serialize_operation.FILE_DESCRIPTOR, fd, *options)
    else:
      raise Error(ccs_error(ccs_error.INVALID_VALUE))
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
        raise Error(ccs_error(ccs_error.INVALID_VALUE))
      if p_sd:
        ct.memmove(p_sd, ct.byref(serialized), ct.sizeof(serialized))
      if p_sdsz:
        p_sdsz[0] = ct.sizeof(serialized)
      return ccs_error.SUCCESS
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
      return ccs_error.SUCCESS
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
    raise Error(ccs_error(ccs_error.INVALID_VALUE))
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
    raise Error(ccs_error(ccs_error.INVALID_VALUE))
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
from .hyperparameter import Hyperparameter
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

setattr(Object, 'CLASS_MAP', {
  ccs_object_type.RNG: Rng,
  ccs_object_type.DISTRIBUTION: Distribution,
  ccs_object_type.HYPERPARAMETER: Hyperparameter,
  ccs_object_type.EXPRESSION: Expression,
  ccs_object_type.CONFIGURATION_SPACE: ConfigurationSpace,
  ccs_object_type.CONFIGURATION: Configuration,
  ccs_object_type.FEATURES_SPACE: ConfigurationSpace,
  ccs_object_type.FEATURES: Features,
  ccs_object_type.OBJECTIVE_SPACE: ObjectiveSpace,
  ccs_object_type.EVALUATION: Evaluation,
  ccs_object_type.FEATURES_EVALUATION: FeaturesEvaluation,
  ccs_object_type.TUNER: Tuner,
  ccs_object_type.FEATURES_TUNER: FeaturesTuner,
  ccs_object_type.MAP: Map,
  ccs_object_type.ERROR_STACK: ErrorStack,
  ccs_object_type.TREE: Tree
})


