import ctypes as ct
from . import libcconfigspace
from .base import Object, Error, Result, CEnumeration, _ccs_get_function, ccs_expression, Datum, DatumFix, ccs_parameter, ccs_context
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
    'VARIABLE' ]

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
ccs_expression_get_num_nodes = _ccs_get_function("ccs_expression_get_num_nodes", [ccs_expression, ct.POINTER(ct.c_size_t)])
ccs_expression_get_nodes = _ccs_get_function("ccs_expression_get_nodes", [ccs_expression, ct.c_size_t, ct.POINTER(ccs_expression), ct.POINTER(ct.c_size_t)])
ccs_literal_get_value = _ccs_get_function("ccs_literal_get_value", [ccs_expression, ct.POINTER(Datum)])
ccs_variable_get_parameter = _ccs_get_function("ccs_variable_get_parameter", [ccs_expression, ct.POINTER(ccs_parameter)])
ccs_expression_eval = _ccs_get_function("ccs_expression_eval", [ccs_expression, ccs_context, ct.POINTER(Datum), ct.POINTER(Datum)])
ccs_expression_list_eval_node = _ccs_get_function("ccs_expression_list_eval_node", [ccs_expression, ccs_context, ct.POINTER(Datum), ct.c_size_t, ct.POINTER(Datum)])
ccs_expression_get_parameters = _ccs_get_function("ccs_expression_get_parameters", [ccs_expression, ct.c_size_t, ct.POINTER(ccs_parameter), ct.POINTER(ct.c_size_t)])
ccs_expression_check_context = _ccs_get_function("ccs_expression_check_context", [ccs_expression, ccs_context])

class Expression(Object):
  def __init__(self, handle = None, retain = False, auto_release = True,
               t = None, nodes = []):
    if handle is None:
      sz = len(nodes)
      handle = ccs_expression()
      v = (Datum*sz)()
      ss = []
      for i in range(sz):
        v[i].set_value(nodes[i], string_store = ss)
      res = ccs_create_expression(t, sz, v, ct.byref(handle))
      Error.check(res)
      super().__init__(handle = handle, retain = False)
    else:
      super().__init__(handle = handle, retain = retain, auto_release = auto_release)

  @classmethod
  def from_handle(cls, handle, retain = True, auto_release = True):
    v = ExpressionType(0)
    res = ccs_expression_get_type(handle, ct.byref(v))
    Error.check(res)
    v = v.value
    if v == ExpressionType.LIST:
      return List(handle = handle, retain = retain, auto_release = auto_release)
    elif v == ExpressionType.LITERAL:
      return Literal(handle = handle, retain = retain, auto_release = auto_release)
    elif v == ExpressionType.VARIABLE:
      return Variable(handle = handle, retain = retain, auto_release = auto_release)
    else:
      return cls(handle = handle, retain = retain, auto_release = auto_release)

  @classmethod
  def binary(cls, t, left, right):
    pvleft = Datum(left)
    pvright = Datum(right)
    vleft = DatumFix(pvleft)
    vright = DatumFix(pvright)
    handle = ccs_expression()
    res = ccs_create_binary_expression(t, vleft, vright, ct.byref(handle))
    Error.check(res)
    return cls(handle = handle, retain = False)

  @classmethod
  def unary(cls, t, node):
    pvnode = Datum(node)
    vnode = DatumFix(pvnode)
    handle = ccs_expression()
    res = ccs_create_unary_expression(t, vnode, ct.byref(handle))
    Error.check(res)
    return cls(handle = handle, retain = False)

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
    res = ccs_expression_get_num_nodes(self.handle, ct.byref(v))
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
    self._nodes = [Expression.from_handle(handle = ccs_expression(x)) for x in v]
    return self._nodes

  @property
  def parameters(self):
    if hasattr(self, "_parameters"):
      return self._parameters
    sz = ct.c_size_t()
    res = ccs_expression_get_parameters(self.handle, 0, None, ct.byref(sz))
    Error.check(res)
    sz = sz.value
    if sz == 0:
      self._parameters = []
      return []
    v = (ccs_parameter * sz.value)()
    res = ccs_expression_get_parameters(self.handle, sz, v, None)
    Error.check(res)
    self._parameters = [Parameter.from_handle(ccs_parameter(x)) for x in v]
    return self._parameters

  def eval(self, context = None, values = None):
    if context and values:
      count = context.num_parameters
      if count != len(values):
        raise Error(Result(Result.ERROR_INVALID_VALUE))
      v = (Datum * count)()
      ss = []
      for i in range(count):
        v[i].set_value(values[i], string_store = ss)
      values = v
      context = context.handle
    elif context or values:
      raise Error(Result(Result.ERROR_INVALID_VALUE))
    v = Datum()
    res = ccs_expression_eval(self.handle, context, values, ct.byref(v))
    Error.check(res)
    return v.value

  def check_context(self, context):
    res = ccs_expression_check_context(self.handle, context.handle)
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


class Literal(Expression):
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
      return Literal.none_symbol
    elif v is True:
      return Literal.true_aymbol
    elif v is False:
      return Literal.false_symbol
    else:
      return "{}".format(v)


class Variable(Expression):
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


class List(Expression):
  def __init__(self, handle = None, retain = False, auto_release = True,
               values = []):
    if handle is None:
      super().__init__(t = ExpressionType.LIST, nodes = values)
    else:
      super().__init__(handle = handle, retain = retain, auto_release = auto_release)
  
  def eval(self, index, context = None, values = None):
    if context and values:
      count = context.num_parameters
      if count != len(values):
        raise Error(Result(Result.ERROR_INVALID_VALUE))
      v = (Datum * count)()
      ss = []
      for i in range(count):
        v[i].set_value(values[i], string_store = ss)
      values = v
      context = context.handle
    elif context or values:
      raise Error(Result(Result.ERROR_INVALID_VALUE))
    v = Datum()
    res = ccs_expression_list_eval_node(self.handle, context, values, index, ct.byref(v))
    Error.check(res)
    return v.value

  def __str__(self):
    return "[ {} ]".format(", ".join(map(str, self.nodes)))

