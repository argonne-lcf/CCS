import ctypes as ct
import pickle
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
ccs_rng                  = ccs_object
ccs_distribution         = ccs_object
ccs_parameter            = ccs_object
ccs_expression           = ccs_object
ccs_context              = ccs_object
ccs_distribution_space   = ccs_object
ccs_search_space         = ccs_object
ccs_configuration_space  = ccs_object
ccs_binding              = ccs_object
ccs_search_configuration = ccs_object
ccs_configuration        = ccs_object
ccs_feature_space        = ccs_object
ccs_features             = ccs_object
ccs_objective_space      = ccs_object
ccs_evaluation           = ccs_object
ccs_tuner                = ccs_object
ccs_map                  = ccs_object
ccs_error_stack          = ccs_object
ccs_tree                 = ccs_object
ccs_tree_space           = ccs_object
ccs_tree_configuration   = ccs_object

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
  def __init__(*args):
    ct.c_int.__init__(*args)

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
    'FEATURE_SPACE',
    'FEATURES',
    'MAP',
    'ERROR_STACK',
    'TREE',
    'TREE_SPACE',
    'TREE_CONFIGURATION',
    'DISTRIBUTION_SPACE' ]

class Result(CEnumeration):
  _members_ = [
    ('AGAIN',                              1),
    ('SUCCESS',                            0),
    ('ERROR_INVALID_OBJECT',              -1),
    ('ERROR_INVALID_VALUE',               -2),
    ('ERROR_INVALID_TYPE',                -3),
    ('ERROR_INVALID_SCALE',               -4),
    ('ERROR_INVALID_DISTRIBUTION',        -5),
    ('ERROR_INVALID_EXPRESSION',          -6),
    ('ERROR_INVALID_PARAMETER',           -7),
    ('ERROR_INVALID_CONFIGURATION',       -8),
    ('ERROR_INVALID_NAME',                -9),
    ('ERROR_INVALID_CONDITION',          -10),
    ('ERROR_INVALID_TUNER',              -11),
    ('ERROR_INVALID_GRAPH',              -12),
    ('ERROR_TYPE_NOT_COMPARABLE',        -13),
    ('ERROR_INVALID_BOUNDS',             -14),
    ('ERROR_OUT_OF_BOUNDS',              -15),
    ('ERROR_SAMPLING_UNSUCCESSFUL',      -16),
    ('ERROR_OUT_OF_MEMORY',              -17),
    ('ERROR_UNSUPPORTED_OPERATION',      -18),
    ('ERROR_INVALID_EVALUATION',         -19),
    ('ERROR_INVALID_FEATURES',           -20),
    ('ERROR_INVALID_FILE_PATH',          -21),
    ('ERROR_NOT_ENOUGH_DATA',            -22),
    ('ERROR_DUPLICATE_HANDLE',           -23),
    ('ERROR_INVALID_HANDLE',             -24),
    ('ERROR_SYSTEM',                     -25),
    ('ERROR_EXTERNAL',                   -26),
    ('ERROR_INVALID_TREE',               -27),
    ('ERROR_INVALID_TREE_SPACE',         -28),
    ('ERROR_INVALID_DISTRIBUTION_SPACE', -29) ]

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
    'MAP_HANDLES',
    'VECTOR_CALLBACK',
    'NON_BLOCKING',
    'DATA_CALLBACK'
  ]

def _ccs_get_function(method, argtypes = [], restype = Result):
  res = getattr(libcconfigspace, method)
  res.restype = restype
  res.argtypes = argtypes
  return res

ccs_init = _ccs_get_function("ccs_init")
ccs_fini = _ccs_get_function("ccs_fini")
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
ccs_object_deserialize_data_callback_type = ct.CFUNCTYPE(Result, ccs_object, ct.c_size_t, ct.c_void_p, ct.POINTER(ct.py_object))
ccs_object_deserialize_vector_callback_type = ct.CFUNCTYPE(Result, ObjectType, ct.c_char_p, ct.c_void_p, ct.POINTER(ct.c_void_p), ct.POINTER(ct.py_object))
# Variadic methods
ccs_object_serialize = getattr(libcconfigspace, "ccs_object_serialize")
ccs_object_serialize.argtypes = ccs_object, SerializeFormat, SerializeOperation,
ccs_object_serialize.restype = Result
ccs_object_deserialize = getattr(libcconfigspace, "ccs_object_deserialize")
ccs_object_deserialize.argtypes = ct.POINTER(ccs_object), SerializeFormat, SerializeOperation,
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
  def from_handle(cls, h, retain = True):
    r = ct.c_int(0)
    res = ccs_object_get_refcount(h, ct.byref(r))
    Error.check(res)
    r = r.value
    if r == 0:
      retain = False
      auto_release = False
    else:
      auto_release = True
    return cls._from_handle(h, retain, auto_release)

  def set_destroy_callback(self, callback):
    _set_destroy_callback(self.handle, callback)

  def serialize(self, format = 'binary', path = None, file_descriptor = None, callback = None):
    if format != 'binary':
      raise Error(Result(Result.ERROR_INVALID_VALUE))
    if path and file_descriptor:
      raise Error(Result(Result.ERROR_INVALID_VALUE))
    options = [SerializeOption.END]
    if callback:
      cb_wrapper = _get_serialize_callback_wrapper(callback)
      cb_wrapper_func = ccs_object_serialize_callback_type(cb_wrapper)
      options = [SerializeOption.CALLBACK, cb_wrapper_func, ct.py_object()] + options
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
      v = ct.create_string_buffer(s.value)
      res = ccs_object_serialize(self.handle, SerializeFormat.BINARY, SerializeOperation.MEMORY, ct.sizeof(v), v, *options)
      Error.check(res)
      return v.raw

  @classmethod
  def deserialize(cls, format = 'binary', handle_map = None, map_handles = False, vector_callback = None, path = None, buffer = None, file_descriptor = None, callback = None):
    if format != 'binary':
      raise Error(Result(Result.ERROR_INVALID_VALUE))
    mode_count = 0;
    if path is not None:
      mode_count += 1
    if buffer is not None:
      mode_count += 1
    if file_descriptor is not None:
      mode_count += 1
    if not mode_count == 1:
      raise Error(Result(Result.ERROR_INVALID_VALUE))
    if map_handles and handle_map is None:
      raise Error(Result(Result.ERROR_INVALID_VALUE))
    o = ccs_object(0)
    options = [DeserializeOption.END]
    if map_handles:
      options = [DeserializeOption.MAP_HANDLES] + options
    if handle_map is not None:
      options = [DeserializeOption.HANDLE_MAP, handle_map.handle] + options
    if vector_callback is not None:
      vector_cb_wrapper = _get_deserialize_vector_callback_wrapper(vector_callback)
      vector_cb_wrapper_func = ccs_object_deserialize_vector_callback_type(vector_cb_wrapper)
      options = [DeserializeOption.VECTOR_CALLBACK, vector_cb_wrapper_func, ct.py_object()] + options
    if callback is not None:
      cb_wrapper = _get_deserialize_data_callback_wrapper(callback)
      cb_wrapper_func = ccs_object_deserialize_data_callback_type(cb_wrapper)
      options = [DeserializeOption.DATA_CALLBACK, cb_wrapper_func, ct.py_object()] + options
    elif _default_user_data_deserializer is not None:
      options = [DeserializeOption.DATA_CALLBACK, _default_user_data_deserializer, ct.py_object()] + options
    if buffer is not None:
      s = len(buffer)
      res = ccs_object_deserialize(ct.byref(o), SerializeFormat.BINARY, SerializeOperation.MEMORY, s, ct.create_string_buffer(buffer, s), *options)
    elif path is not None:
      p = str.encode(path)
      pp = ct.c_char_p(p)
      res = ccs_object_deserialize(ct.byref(o), SerializeFormat.BINARY, SerializeOperation.FILE, pp, *options)
    elif file_descriptor is not None:
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
      serialized = callback(Object.from_handle(obj).user_data)
      state = ct.create_string_buffer(serialized, len(serialized))
      if serialize_data and serialize_data_size < ct.sizeof(state):
        raise Error(Result(Result.ERROR_INVALID_VALUE))
      if serialize_data:
        ct.memmove(serialize_data, ct.byref(state), ct.sizeof(state))
      if serialize_data_size_ret:
        serialize_data_size_ret[0] = ct.sizeof(state)
      return Result.SUCCESS
    except Exception as e:
      return Error.set_error(e)
  return serialize_callback_wrapper

def _get_deserialize_data_callback_wrapper(callback):
  def deserialize_data_callback_wrapper(obj, serialize_data_size, p_serialize_data, cb_data):
    try:
      user_data = callback(ct.string_at(p_serialize_data, serialize_data_size))
      Object.from_handle(ccs_object(obj)).user_data = user_data
      return Result.SUCCESS
    except Exception as e:
      return Error.set_error(e)
  return deserialize_data_callback_wrapper

def _get_deserialize_vector_callback_wrapper(callback):
  def deserialize_vector_callback_wrapper(obj_type, name, callback_user_data, vector_ret, data_ret):
    try:
      (vector, data) = callback(obj_type.value, name.decode())
      c_vector = ct.py_object(vector)
      c_data = ct.py_object(data)
      vector_ret[0] = ct.cast(ct.byref(vector), ct.c_void_p)
      data_ret[0] = c_data
      ct.pythonapi.Py_IncRef(c_vector)
      ct.pythonapi.Py_IncRef(c_data)
      return Result.SUCCESS
    except Exception as e:
      return Error.set_error(e)
  return deserialize_vector_callback_wrapper

def _pickle_user_data_serializer(user_data):
  return pickle.dumps(user_data)

def _pickle_user_data_deserializer(serialized):
  return pickle.loads(serialized)

_pickle_user_data_serializer_wrap = _get_serialize_callback_wrapper(_pickle_user_data_serializer)
_pickle_user_data_serializer_func = ccs_object_serialize_callback_type(_pickle_user_data_serializer_wrap)

_pickle_user_data_deserializer_wrap = _get_deserialize_data_callback_wrapper(_pickle_user_data_deserializer)
_pickle_user_data_deserializer_func = ccs_object_deserialize_data_callback_type(_pickle_user_data_deserializer_wrap)

_default_user_data_serializer = _pickle_user_data_serializer_func
_default_user_data_deserializer = _pickle_user_data_deserializer_func

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

def deserialize(format = 'binary', handle_map = None, map_handles = False, path = None, buffer = None, file_descriptor = None, vector_callback = None, vector_callback_data = None, callback = None, callback_data = None):
  return Object.deserialize(format = format, handle_map = handle_map, map_handles = map_handles, path = path, buffer = buffer, file_descriptor = file_descriptor, vector_callback = vector_callback, callback = callback)

def _set_destroy_callback(handle, callback):
  if callback is None:
    raise Error(Result(Result.ERROR_INVALID_VALUE))
  def cb_wrapper(obj, data):
    callback(Object.from_handle(obj), data)
  cb_wrapper_func = ccs_object_destroy_callback_type(cb_wrapper)
  res = ccs_object_set_destroy_callback(handle, cb_wrapper_func, None)
  Error.check(res)
  _register_callback(handle, [callback, cb_wrapper, cb_wrapper_func])

def _set_serialize_callback(handle, callback):
  if callback is None:
    res = ccs_object_set_serialize_callback(handle, None, None)
    Error.check(res)
    _register_serialize_callback(handle, None)
  else:
    cb_wrapper = _get_serialize_callback_wrapper(callback)
    cb_wrapper_func = ccs_object_serialize_callback_type(cb_wrapper)
    res = ccs_object_set_serialize_callback(handle, cb_wrapper_func, None)
    Error.check(res)
    _register_serialize_callback(handle, [callback, cb_wrapper, cb_wrapper_func])

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
from .feature_space import FeatureSpace
from .features import Features
from .objective_space import ObjectiveSpace
from .evaluation import Evaluation
from .tuner import Tuner
from .map import Map
from .error_stack import ErrorStack, get_thread_error, set_thread_error, clear_thread_error
from .tree import Tree
from .tree_space import TreeSpace
from .tree_configuration import TreeConfiguration
from .distribution_space import DistributionSpace

setattr(Object, 'CLASS_MAP', {
  ObjectType.RNG: Rng,
  ObjectType.DISTRIBUTION: Distribution,
  ObjectType.PARAMETER: Parameter,
  ObjectType.EXPRESSION: Expression,
  ObjectType.CONFIGURATION_SPACE: ConfigurationSpace,
  ObjectType.CONFIGURATION: Configuration,
  ObjectType.FEATURE_SPACE: FeatureSpace,
  ObjectType.FEATURES: Features,
  ObjectType.OBJECTIVE_SPACE: ObjectiveSpace,
  ObjectType.EVALUATION: Evaluation,
  ObjectType.TUNER: Tuner,
  ObjectType.MAP: Map,
  ObjectType.ERROR_STACK: ErrorStack,
  ObjectType.TREE: Tree,
  ObjectType.TREE_SPACE: TreeSpace,
  ObjectType.TREE_CONFIGURATION: TreeConfiguration,
  ObjectType.DISTRIBUTION_SPACE: DistributionSpace
})


