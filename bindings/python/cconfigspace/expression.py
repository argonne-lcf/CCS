import ctypes as ct
from . import libcconfigspace
from .base import Object, Error, Result, CEnumeration, _ccs_get_function, ccs_expression, Datum, DatumFix, ccs_parameter, ccs_context, ccs_binding
from .parameter import Parameter

class ExpressionType(CEnumeration):
  _members_ = [
    ('OR', 0),
    'AND',
    'EQUAL',
    'NOT_EQUAL',
    'LESS',
    'GREATER',
    'LESS_OR_EQUAL',
    'GREATER_OR_EQUAL',
    'ADD',
    'SUBSTRACT',
    'MULTIPLY',
    'DIVIDE', 
    'MODULO', 
    'POSITIVE',
    'NEGATIVE',
    'NOT',
    'IN',
    'LIST',
    'LITERAL',
    'VARIABLE',
    'USER_DEFINED' ]

class AssociativityType(CEnumeration):
  _members_ = [
    ('ASSOCIATIVITY_NONE', 0),
    'LEFT_TO_RIGHT',
    'RIGHT_TO_LEFT' ]

_sz_expr = len(ExpressionType._members_)
ccs_expression_precedence = (ct.c_int * _sz_expr).in_dll(libcconfigspace, "ccs_expression_precedence")
ccs_expression_associativity = (AssociativityType * _sz_expr).in_dll(libcconfigspace, "ccs_expression_associativity")
ccs_expression_symbols = [x.decode() if x else x for x in (ct.c_char_p * _sz_expr).in_dll(libcconfigspace, "ccs_expression_symbols")]
ccs_expression_arity = (ct.c_int * _sz_expr).in_dll(libcconfigspace, "ccs_expression_arity")

class TerminalType(CEnumeration):
  _members_ = [
    ('NONE', 0),
    'TRUE',
    'FALSE',
    'STRING',
    'IDENTIFIER',
    'INTEGER',
    'FLOAT' ]

_sz_term = len(TerminalType._members_)
ccs_terminal_precedence = (ct.c_int * _sz_term).in_dll(libcconfigspace, "ccs_terminal_precedence")
ccs_terminal_regexp = [x.decode() if x else x for x in (ct.c_char_p * _sz_term).in_dll(libcconfigspace, "ccs_terminal_regexp")]
ccs_terminal_symbols = [x.decode() if x else x for x in (ct.c_char_p * _sz_term).in_dll(libcconfigspace, "ccs_terminal_symbols")]

ccs_create_binary_expression = _ccs_get_function("ccs_create_binary_expression", [ExpressionType, DatumFix, DatumFix, ct.POINTER(ccs_expression)])
ccs_create_unary_expression = _ccs_get_function("ccs_create_unary_expression", [ExpressionType, DatumFix, ct.POINTER(ccs_expression)])
ccs_create_expression = _ccs_get_function("ccs_create_expression", [ExpressionType, ct.c_size_t, ct.POINTER(Datum), ct.POINTER(ccs_expression)])
ccs_create_literal = _ccs_get_function("ccs_create_literal", [DatumFix, ct.POINTER(ccs_expression)])
ccs_create_variable = _ccs_get_function("ccs_create_variable", [ccs_parameter, ct.POINTER(ccs_expression)])
ccs_expression_get_type = _ccs_get_function("ccs_expression_get_type", [ccs_expression, ct.POINTER(ExpressionType)])
ccs_expression_get_nodes = _ccs_get_function("ccs_expression_get_nodes", [ccs_expression, ct.c_size_t, ct.POINTER(ccs_expression), ct.POINTER(ct.c_size_t)])
ccs_literal_get_value = _ccs_get_function("ccs_literal_get_value", [ccs_expression, ct.POINTER(Datum)])
ccs_variable_get_parameter = _ccs_get_function("ccs_variable_get_parameter", [ccs_expression, ct.POINTER(ccs_parameter)])
ccs_expression_eval = _ccs_get_function("ccs_expression_eval", [ccs_expression, ct.c_size_t, ct.POINTER(ccs_binding), ct.POINTER(Datum)])
ccs_expression_list_eval_node = _ccs_get_function("ccs_expression_list_eval_node", [ccs_expression, ct.c_size_t, ct.POINTER(ccs_binding), ct.c_size_t, ct.POINTER(Datum)])
ccs_expression_get_parameters = _ccs_get_function("ccs_expression_get_parameters", [ccs_expression, ct.c_size_t, ct.POINTER(ccs_parameter), ct.POINTER(ct.c_size_t)])
ccs_expression_check_context = _ccs_get_function("ccs_expression_check_contexts", [ccs_expression, ct.c_size_t, ct.POINTER(ccs_context)])

class Expression(Object):

  @classmethod
  def from_handle(cls, handle, retain = True, auto_release = True):
    v = ExpressionType(0)
    res = ccs_expression_get_type(handle, ct.byref(v))
    Error.check(res)
    v = v.value
    klass = cls.EXPRESSION_MAP[v]
    if klass is None:
      raise Error(Result(Result.ERROR_INVALID_EXPRESSION))
    return klass(handle = handle, retain = retain, auto_release = auto_release)

  def _create_binary(self, t, left, right):
    pvleft = Datum(left)
    pvright = Datum(right)
    vleft = DatumFix(pvleft)
    vright = DatumFix(pvright)
    handle = ccs_expression()
    res = ccs_create_binary_expression(t, vleft, vright, ct.byref(handle))
    Error.check(res)
    return handle

  def _create_unary(self, t, node):
    pvnode = Datum(node)
    vnode = DatumFix(pvnode)
    handle = ccs_expression()
    res = ccs_create_unary_expression(t, vnode, ct.byref(handle))
    Error.check(res)
    return handle

  @property
  def type(self):
    if hasattr(self, "_type"):
      return self._type
    v = ExpressionType(0)
    res = ccs_expression_get_type(self.handle, ct.byref(v))
    Error.check(res)
    self._type = v.value
    return self._type

  @property
  def num_nodes(self):
    if hasattr(self, "_num_nodes"):
      return self._num_nodes
    v = ct.c_size_t(0)
    res = ccs_expression_get_nodes(self.handle, 0, None, ct.byref(v))
    Error.check(res)
    self._num_nodes = v.value
    return self._num_nodes

  @property
  def nodes(self):
    if hasattr(self, "_nodes"):
      return self._nodes
    sz = self.num_nodes
    v = (ccs_expression * sz)()
    res = ccs_expression_get_nodes(self.handle, sz, v, None)
    Error.check(res)
    self._nodes = tuple(Expression.from_handle(handle = ccs_expression(x)) for x in v)
    return self._nodes

  @property
  def parameters(self):
    if hasattr(self, "_parameters"):
      return self._parameters
    sz = ct.c_size_t()
    res = ccs_expression_get_parameters(self.handle, 0, None, ct.byref(sz))
    Error.check(res)
    sz = sz.value
    v = (ccs_parameter * sz)()
    res = ccs_expression_get_parameters(self.handle, sz, v, None)
    Error.check(res)
    self._parameters = tuple(Parameter.from_handle(ccs_parameter(x)) for x in v)
    return self._parameters

  def eval(self, bindings = None):
    if bindings is not None:
      num_bindings = len(bindings)
      bindingsv = (ccs_binding * num_bindings)(*[x.handle.value for x in bindings])
    else:
      num_bindings = 0
      bindingsv = None
    v = Datum()
    res = ccs_expression_eval(self.handle, num_bindings, bindingsv, ct.byref(v))
    Error.check(res)
    return v.value

  def check_context(self, contexts):
    num_contexts = len(contexts)
    contextsv = (ccs_context * num_context)(*[x.handle.value for x in contexts])
    res = ccs_expression_check_context(self.handle, num_contexts, contextsv)
    Error.check(res)

  def __str__(self):
    t = self.type
    symbol = ccs_expression_symbols[t]
    prec = ccs_expression_precedence[t]
    nds = ["({})".format(n) if ccs_expression_precedence[n.type] < prec else n.__str__() for n in self.nodes]
    if len(nds) == 1:
      return "{}{}".format(symbol, nds[0])
    else:
      return "{} {} {}".format(nds[0], symbol, nds[1])

  @classmethod
  def get_function_vector_data(cls, name, binding = {}):
    proc = binding[name]
    def evaluate(expr, *args):
      return proc(*args)
    return (Expression.UserDefined.get_vector(evaluate = evaluate), None)

class ExpressionOr(Expression):

  def __init__(self, handle = None, retain = False, auto_release = True,
               left = None, right = None):
    if handle is None:
      handle = self._create_binary(ExpressionType.OR, left, right)
      super().__init__(handle = handle, retain = False)
    else:
      super().__init__(handle = handle, retain = retain, auto_release = auto_release)

Expression.Or = ExpressionOr

class ExpressionAnd(Expression):

  def __init__(self, handle = None, retain = False, auto_release = True,
               left = None, right = None):
    if handle is None:
      handle = self._create_binary(ExpressionType.AND, left, right)
      super().__init__(handle = handle, retain = False)
    else:
      super().__init__(handle = handle, retain = retain, auto_release = auto_release)

Expression.And = ExpressionAnd

class ExpressionEqual(Expression):

  def __init__(self, handle = None, retain = False, auto_release = True,
               left = None, right = None):
    if handle is None:
      handle = self._create_binary(ExpressionType.EQUAL, left, right)
      super().__init__(handle = handle, retain = False)
    else:
      super().__init__(handle = handle, retain = retain, auto_release = auto_release)

Expression.Equal = ExpressionEqual

class ExpressionNotEqual(Expression):

  def __init__(self, handle = None, retain = False, auto_release = True,
               left = None, right = None):
    if handle is None:
      handle = self._create_binary(ExpressionType.NOT_EQUAL, left, right)
      super().__init__(handle = handle, retain = False)
    else:
      super().__init__(handle = handle, retain = retain, auto_release = auto_release)

Expression.NotEqual = ExpressionNotEqual

class ExpressionLess(Expression):

  def __init__(self, handle = None, retain = False, auto_release = True,
               left = None, right = None):
    if handle is None:
      handle = self._create_binary(ExpressionType.LESS, left, right)
      super().__init__(handle = handle, retain = False)
    else:
      super().__init__(handle = handle, retain = retain, auto_release = auto_release)

Expression.Less = ExpressionLess

class ExpressionGreater(Expression):

  def __init__(self, handle = None, retain = False, auto_release = True,
               left = None, right = None):
    if handle is None:
      handle = self._create_binary(ExpressionType.GREATER, left, right)
      super().__init__(handle = handle, retain = False)
    else:
      super().__init__(handle = handle, retain = retain, auto_release = auto_release)

Expression.Greater = ExpressionGreater

class ExpressionLessOrEqual(Expression):

  def __init__(self, handle = None, retain = False, auto_release = True,
               left = None, right = None):
    if handle is None:
      handle = self._create_binary(ExpressionType.LESS_OR_EQUAL, left, right)
      super().__init__(handle = handle, retain = False)
    else:
      super().__init__(handle = handle, retain = retain, auto_release = auto_release)

Expression.LessOrEqual = ExpressionLessOrEqual

class ExpressionGreaterOrEqual(Expression):

  def __init__(self, handle = None, retain = False, auto_release = True,
               left = None, right = None):
    if handle is None:
      handle = self._create_binary(ExpressionType.GREATER_OR_EQUAL, left, right)
      super().__init__(handle = handle, retain = False)
    else:
      super().__init__(handle = handle, retain = retain, auto_release = auto_release)

Expression.GreaterOrEqual = ExpressionGreaterOrEqual

class ExpressionAdd(Expression):

  def __init__(self, handle = None, retain = False, auto_release = True,
               left = None, right = None):
    if handle is None:
      handle = self._create_binary(ExpressionType.ADD, left, right)
      super().__init__(handle = handle, retain = False)
    else:
      super().__init__(handle = handle, retain = retain, auto_release = auto_release)

Expression.Add = ExpressionAdd

class ExpressionSubstract(Expression):

  def __init__(self, handle = None, retain = False, auto_release = True,
               left = None, right = None):
    if handle is None:
      handle = self._create_binary(ExpressionType.SUBSTRACT, left, right)
      super().__init__(handle = handle, retain = False)
    else:
      super().__init__(handle = handle, retain = retain, auto_release = auto_release)

Expression.Substract = ExpressionSubstract

class ExpressionMultiply(Expression):

  def __init__(self, handle = None, retain = False, auto_release = True,
               left = None, right = None):
    if handle is None:
      handle = self._create_binary(ExpressionType.MULTIPLY, left, right)
      super().__init__(handle = handle, retain = False)
    else:
      super().__init__(handle = handle, retain = retain, auto_release = auto_release)

Expression.Multiply = ExpressionMultiply

class ExpressionDivide(Expression):

  def __init__(self, handle = None, retain = False, auto_release = True,
               left = None, right = None):
    if handle is None:
      handle = self._create_binary(ExpressionType.DIVIDE, left, right)
      super().__init__(handle = handle, retain = False)
    else:
      super().__init__(handle = handle, retain = retain, auto_release = auto_release)

Expression.Divide = ExpressionDivide

class ExpressionModulo(Expression):

  def __init__(self, handle = None, retain = False, auto_release = True,
               left = None, right = None):
    if handle is None:
      handle = self._create_binary(ExpressionType.MODULO, left, right)
      super().__init__(handle = handle, retain = False)
    else:
      super().__init__(handle = handle, retain = retain, auto_release = auto_release)

Expression.Modulo = ExpressionModulo

class ExpressionPositive(Expression):

  def __init__(self, handle = None, retain = False, auto_release = True,
               node = None):
    if handle is None:
      handle = self._create_unary(ExpressionType.POSITIVE, node)
      super().__init__(handle = handle, retain = False)
    else:
      super().__init__(handle = handle, retain = retain, auto_release = auto_release)

Expression.Positive = ExpressionPositive

class ExpressionNegative(Expression):

  def __init__(self, handle = None, retain = False, auto_release = True,
               node = None):
    if handle is None:
      handle = self._create_unary(ExpressionType.NEGATIVE, node)
      super().__init__(handle = handle, retain = False)
    else:
      super().__init__(handle = handle, retain = retain, auto_release = auto_release)

Expression.Negative = ExpressionNegative

class ExpressionNot(Expression):

  def __init__(self, handle = None, retain = False, auto_release = True,
               node = None):
    if handle is None:
      handle = self._create_unary(ExpressionType.NOT, node)
      super().__init__(handle = handle, retain = False)
    else:
      super().__init__(handle = handle, retain = retain, auto_release = auto_release)

Expression.Not = ExpressionNot

class ExpressionIn(Expression):

  def __init__(self, handle = None, retain = False, auto_release = True,
               left = None, right = None):
    if handle is None:
      handle = self._create_binary(ExpressionType.IN, left, right)
      super().__init__(handle = handle, retain = False)
    else:
      super().__init__(handle = handle, retain = retain, auto_release = auto_release)

Expression.In = ExpressionIn

class ExpressionLiteral(Expression):
  none_symbol = ccs_terminal_symbols[TerminalType.NONE]
  true_aymbol = ccs_terminal_symbols[TerminalType.TRUE]
  false_symbol = ccs_terminal_symbols[TerminalType.FALSE]

  def __init__(self, handle = None, retain = False, auto_release = True,
               value = None):
    if handle is None:
      handle = ccs_expression()
      pv = Datum(value)
      v = DatumFix(pv)
      res = ccs_create_literal(v, ct.byref(handle))
      Error.check(res)
      super().__init__(handle = handle, retain = False)
    else:
      super().__init__(handle = handle, retain = retain, auto_release = auto_release)
 
  @property 
  def value(self):
    if hasattr(self, "_value"):
      return self._value
    v = Datum()
    res = ccs_literal_get_value(self.handle, ct.byref(v))
    Error.check(res)
    self._value = v.value
    return self._value
  
  def __str__(self):
    v = self.value
    if isinstance(v, str):
      return repr(v)
    elif v is None:
      return ExpressionLiteral.none_symbol
    elif v is True:
      return ExpressionLiteral.true_aymbol
    elif v is False:
      return ExpressionLiteral.false_symbol
    else:
      return "{}".format(v)

Expression.Literal = ExpressionLiteral

class ExpressionVariable(Expression):
  def __init__(self, handle = None, retain = False, auto_release = True,
               parameter = None):
    if handle is None:
      handle = ccs_expression()
      res = ccs_create_variable(parameter.handle, ct.byref(handle))
      Error.check(res)
      super().__init__(handle = handle, retain = False)
    else:
      super().__init__(handle = handle, retain = retain, auto_release = auto_release)
  
  @property 
  def parameter(self):
    if hasattr(self, "_parameter"):
      return self._parameter
    v = ccs_parameter()
    res = ccs_variable_get_parameter(self.handle, ct.byref(v))
    Error.check(res)
    self._parameter = Parameter.from_handle(v)
    return self._parameter
  
  def __str__(self):
    return self.parameter.name

Expression.Variable = ExpressionVariable

class ExpressionList(Expression):
  def __init__(self, handle = None, retain = False, auto_release = True,
               values = []):
    if handle is None:
      sz = len(values)
      handle = ccs_expression()
      v = (Datum*sz)()
      ss = []
      for i in range(sz):
        v[i].set_value(values[i], string_store = ss)
      res = ccs_create_expression(ExpressionType.LIST, sz, v, ct.byref(handle))
      Error.check(res)
      super().__init__(handle = handle, retain = False)
    else:
      super().__init__(handle = handle, retain = retain, auto_release = auto_release)
  
  def eval(self, index, bindings = None):
    if bindings is not None:
      num_bindings = len(bindings)
      bindingsv = (ccs_binding * num_bindings)(*[x.handle.value for x in bindings])
    else:
      num_bindings = 0
      bindingsv = None
    v = Datum()
    res = ccs_expression_list_eval_node(self.handle, num_bindings, bindingsv, index, ct.byref(v))
    Error.check(res)
    return v.value

  def __str__(self):
    return "[ {} ]".format(", ".join(map(str, self.nodes)))

Expression.List = ExpressionList

ccs_user_defined_expression_del_type = ct.CFUNCTYPE(Result, ccs_expression)
ccs_user_defined_expression_eval_type = ct.CFUNCTYPE(Result, ccs_expression, ct.c_size_t, ct.POINTER(Datum), ct.POINTER(Datum))
ccs_user_defined_expression_serialize_type = ct.CFUNCTYPE(Result, ccs_expression, ct.c_size_t, ct.c_void_p, ct.POINTER(ct.c_size_t))
ccs_user_defined_expression_deserialize_type = ct.CFUNCTYPE(Result, ct.c_size_t, ct.c_void_p, ct.POINTER(ct.py_object))

class UserDefinedExpressionVector(ct.Structure):
  _fields_ = [
    ('delete', ccs_user_defined_expression_del_type),
    ('evaluate', ccs_user_defined_expression_eval_type),
    ('serialize', ccs_user_defined_expression_serialize_type),
    ('deserialize', ccs_user_defined_expression_deserialize_type) ]

ccs_create_user_defined_expression = _ccs_get_function("ccs_create_user_defined_expression", [ct.c_char_p, ct.c_size_t, ct.POINTER(Datum), ct.POINTER(UserDefinedExpressionVector), ct.py_object, ct.POINTER(ccs_expression)])
ccs_user_defined_expression_get_name = _ccs_get_function("ccs_user_defined_expression_get_name", [ccs_expression, ct.POINTER(ct.c_char_p)])
ccs_user_defined_expression_get_expression_data = _ccs_get_function("ccs_user_defined_expression_get_expression_data", [ccs_expression, ct.POINTER(ct.c_void_p)])

class ExpressionUserDefined(Expression):
  def __init__(self, handle = None, retain = False, auto_release = True,
               name = "", nodes = [], delete = None, evaluate = None, serialize = None, deserialize = None, expression_data = None):
    if handle is None:
      if evaluate is None:
        raise Error(Result(Result.ERROR_INVALID_VALUE))

      vec = self.get_vector(delete, evaluate, serialize, deserialize)
      c_expression_data = None
      if expression_data is not None:
        c_expression_data = ct.py_object(expression_data)
      handle = ccs_expression()
      sz = len(nodes)
      v = (Datum*sz)()
      ss = []
      for i in range(sz):
        v[i].set_value(nodes[i], string_store = ss)
      res = ccs_create_user_defined_expression(str.encode(name), sz, v, ct.byref(vec), c_expression_data, ct.byref(handle))
      Error.check(res)
      super().__init__(handle = handle, retain = False)
      ct.pythonapi.Py_IncRef(ct.py_object(vec))
      if c_expression_data is not None:
        ct.pythonapi.Py_IncRef(c_expression_data)
    else:
      super().__init__(handle = handle, retain = retain, auto_release = auto_release)

  @property
  def name(self):
    if hasattr(self, "_name"):
      return self._name
    v = ct.c_char_p()
    res = ccs_user_defined_expression_get_name(self.handle, ct.byref(v))
    Error.check(res)
    self._name = v.value.decode()
    return self._name

  @property
  def expression_data(self):
    if hasattr(self, "_expression_data"):
      return self._expression_data
    v = ct.c_void_p()
    res = ccs_user_defined_expression_get_expression_data(self.handle, ct.byref(v))
    Error.check(res)
    if v:
      self._expression_data = ct.cast(v, ct.py_object).value
    else:
      self._expression_data = None
    return self._expression_data

  def __str__(self):
    return "{}({})".format(self.name, ", ".join(map(str, self.nodes)))

  @classmethod
  def get_vector(self, delete = None, evaluate = None, serialize = None, deserialize = None):
    vec = UserDefinedExpressionVector()
    setattr(vec, '_string_store', list())
    setattr(vec, '_object_store', list())
    def delete_wrapper(expr):
      try:
        o = Object.from_handle(expr)
        edata = o.expression_data
        if delete is not None:
          delete(o)
        if edata is not None:
          ct.pythonapi.Py_DecRef(ct.py_object(edata))
        ct.pythonapi.Py_DecRef(ct.py_object(vec))
        return Result.SUCCESS
      except Exception as e:
        return Error.set_error(e)

    def evaluate_wrapper(expr, num_values, p_values, p_value_ret):
      try:
        if num_values == 0:
          value_ret = evaluate(Expression.from_handle(expr))
        else:
          values = tuple(p_values[i].value for i in range(num_values))
          value_ret = evaluate(Expression.from_handle(expr), *values)
        p_value_ret[0].set_value(value_ret, string_store = getattr(vec, '_string_store'), object_store = getattr(vec, '_object_store'))
        return Result.SUCCESS
      except Exception as e:
        return Error.set_error(e)

    if serialize is not None:
      def serialize_wrapper(expr, state_size, p_state, p_state_size):
        try:
          serialized = serialize(Expression.from_handle(expr))
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
      def deserialize_wrapper(state_size, p_state, p_expression_data):
        try:
          expression_data = deserialize(ct.string_at(p_state, state_size))
          c_expression_data = ct.py_object(expression_data)
          p_expression_data[0] = c_expression_data
          ct.pythonapi.Py_IncRef(c_expression_data)
          return Result.SUCCESS
        except Exception as e:
          return Error.set_error(e)
    else:
      deserialize_wrapper = 0

    delete_wrapper_func      = ccs_user_defined_expression_del_type(delete_wrapper)
    evaluate_wrapper_func    = ccs_user_defined_expression_eval_type(evaluate_wrapper)
    serialize_wrapper_func   = ccs_user_defined_expression_serialize_type(serialize_wrapper)
    deserialize_wrapper_func = ccs_user_defined_expression_deserialize_type(deserialize_wrapper)
    vec.delete = delete_wrapper_func
    vec.evaluate = evaluate_wrapper_func
    vec.serialize = serialize_wrapper_func
    vec.deserialize = deserialize_wrapper_func

    setattr(vec, '_wrappers', (
      delete_wrapper,
      evaluate_wrapper,
      serialize_wrapper,
      deserialize_wrapper,
      delete_wrapper_func,
      evaluate_wrapper_func,
      serialize_wrapper_func,
      deserialize_wrapper_func))
    return vec

Expression.UserDefined = ExpressionUserDefined

setattr(Expression, 'EXPRESSION_MAP', {
  ExpressionType.OR: ExpressionOr,
  ExpressionType.AND: ExpressionAnd,
  ExpressionType.EQUAL: ExpressionEqual,
  ExpressionType.NOT_EQUAL: ExpressionNotEqual,
  ExpressionType.LESS: ExpressionLess,
  ExpressionType.GREATER: ExpressionGreater,
  ExpressionType.LESS_OR_EQUAL: ExpressionLessOrEqual,
  ExpressionType.GREATER_OR_EQUAL: ExpressionGreaterOrEqual,
  ExpressionType.ADD: ExpressionAdd,
  ExpressionType.SUBSTRACT: ExpressionSubstract,
  ExpressionType.MULTIPLY: ExpressionMultiply,
  ExpressionType.DIVIDE: ExpressionDivide,
  ExpressionType.MODULO: ExpressionModulo,
  ExpressionType.POSITIVE: ExpressionPositive,
  ExpressionType.NEGATIVE: ExpressionNegative,
  ExpressionType.NOT: ExpressionNot,
  ExpressionType.IN: ExpressionIn,
  ExpressionType.LIST: ExpressionList,
  ExpressionType.LITERAL: ExpressionLiteral,
  ExpressionType.VARIABLE: ExpressionVariable,
  ExpressionType.USER_DEFINED: ExpressionUserDefined,
})
