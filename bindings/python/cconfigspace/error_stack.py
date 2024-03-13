import ctypes as ct
from .base import Object, Error, ccs_retain_object, Result, ccs_error_stack, _ccs_get_function, _register_string

class ErrorStackElem(ct.Structure):
  _fields_ = [('file', ct.c_char_p),
              ('line', ct.c_int),
              ('func', ct.c_char_p)]

ccs_get_thread_error = _ccs_get_function("ccs_get_thread_error", [], ccs_error_stack)
ccs_set_thread_error = _ccs_get_function("ccs_set_thread_error", [ccs_error_stack])
ccs_clear_thread_error = _ccs_get_function("ccs_clear_thread_error", [], None)
ccs_create_error_stack = _ccs_get_function("ccs_create_error_stack", [ct.POINTER(ccs_error_stack), Result, ct.c_char_p])
ccs_error_stack_push = _ccs_get_function("ccs_error_stack_push", [ccs_error_stack, ct.c_char_p, ct.c_int, ct.c_char_p])
ccs_error_stack_get_message = _ccs_get_function("ccs_error_stack_get_message", [ccs_error_stack, ct.POINTER(ct.c_char_p)])
ccs_error_stack_get_code = _ccs_get_function("ccs_error_stack_get_code", [ccs_error_stack, ct.POINTER(Result)])
ccs_error_stack_get_elems = _ccs_get_function("ccs_error_stack_get_elems", [ccs_error_stack, ct.c_size_t, ct.POINTER(ErrorStackElem), ct.POINTER(ct.c_size_t)])

class ErrorStack(Object):
  def __init__(self, handle = None, retain = False, auto_release = True,
               error = None, message = None):
    if handle is None:
      if message:
        message = str.encode(message.replace("%", "%%"))
      msg = ct.c_char_p(message)
      handle = ccs_error_stack()
      res = ccs_create_error_stack(ct.byref(handle), error, msg)
      Error.check(res)
      super().__init__(handle = handle, retain = False)
      if message:
        _register_string(handle, msg)
    else:
      super().__init__(handle = handle, retain = retain, auto_release = auto_release)

  def push(self, file, line, func):
    if file:
      file = str.encode(file)
    if func:
      func = str.encode(func)
    fil = ct.c_char_p(file)
    fun = ct.c_char_p(func)
    res = ccs_error_stack_push(self.handle, fil, line, fun);
    Error.check(res)
    if file:
      _register_string(self.handle, fil)
    if func:
      _register_string(self.handle, fun)

  @property
  def code(self):
    if hasattr(self, "_code"):
      return self._code
    v = Result(0)
    res = ccs_error_stack_get_code(self.handle, ct.byref(v))
    Error.check(res)
    self._code = v
    return v

  @property
  def message(self):
    if hasattr(self, "_message"):
      return self._message
    v = ct.c_char_p()
    res = ccs_error_stack_get_message(self.handle, ct.byref(v))
    Error.check(res)
    self._message = v.value.decode()
    return self._message

  def elems(self):
    v1 = ct.c_size_t()
    res = ccs_error_stack_get_elems(self.handle, 0, None, ct.byref(v1))
    Error.check(res)
    sz = v1.value
    v2 = (ErrorStackElem * sz)()
    res = ccs_error_stack_get_elems(self.handle, sz, v2, None)
    Error.check(res)
    return [x for x in v2]

def get_thread_error():
  handle = ccs_error_stack(ccs_get_thread_error())
  if handle:
    return ErrorStack(handle = handle)
  else:
    return None

def set_thread_error(error):
  res = ccs_set_thread_error(error.handle)
  Error.check(res)
  res = ccs_retain_object(error.handle)
  Error.check(res)

def clear_thread_error():
  ccs_clear_thread_error()
