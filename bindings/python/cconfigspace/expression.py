import ctypes as ct
from . import libcconfigspace
from .base import Object, Error, ccs_error, CEnumeration, _ccs_get_function, ccs_expression, ccs_datum, ccs_datum_fix, ccs_hyperparameter, ccs_context
from .hyperparameter import Hyperparameter

class ccs_expression_type(CEnumeration):
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

class ccs_associativity_type(CEnumeration):
  _members_ = [
    ('ASSOCIATIVITY_NONE', 0),
    'LEFT_TO_RIGHT',
    'RIGHT_TO_LEFT' ]

_sz_expr = len(ccs_expression_type._members_)
ccs_expression_precedence = (ct.c_int * _sz_expr).in_dll(libcconfigspace, "ccs_expression_precedence")
ccs_expression_associativity = (ccs_associativity_type * _sz_expr).in_dll(libcconfigspace, "ccs_expression_associativity")
ccs_expression_symbols = [x.decode() if x else x for x in (ct.c_char_p * _sz_expr).in_dll(libcconfigspace, "ccs_expression_symbols")]
ccs_expression_arity = (ct.c_int * _sz_expr).in_dll(libcconfigspace, "ccs_expression_arity")

ccs_create_binary_expression = _ccs_get_function("ccs_create_binary_expression", [ccs_expression_type, ccs_datum_fix, ccs_datum_fix, ct.POINTER(ccs_expression)])
ccs_create_unary_expression = _ccs_get_function("ccs_create_unary_expression", [ccs_expression_type, ccs_datum_fix, ct.POINTER(ccs_expression)])
ccs_create_expression = _ccs_get_function("ccs_create_expression", [ccs_expression_type, ct.c_size_t, ct.POINTER(ccs_datum), ct.POINTER(ccs_expression)])
ccs_create_literal = _ccs_get_function("ccs_create_literal", [ccs_datum_fix, ct.POINTER(ccs_expression)])
ccs_create_variable = _ccs_get_function("ccs_create_variable", [ccs_hyperparameter, ct.POINTER(ccs_expression)])
ccs_expression_get_type = _ccs_get_function("ccs_expression_get_type", [ccs_expression, ct.POINTER(ccs_expression_type)])
ccs_expression_get_num_nodes = _ccs_get_function("ccs_expression_get_num_nodes", [ccs_expression, ct.POINTER(ct.c_size_t)])
ccs_expression_get_nodes = _ccs_get_function("ccs_expression_get_nodes", [ccs_expression, ct.c_size_t, ct.POINTER(ccs_expression), ct.POINTER(ct.c_size_t)])
ccs_literal_get_value = _ccs_get_function("ccs_literal_get_value", [ccs_expression, ct.POINTER(ccs_datum)])
ccs_variable_get_hyperparameter = _ccs_get_function("ccs_variable_get_hyperparameter", [ccs_expression, ct.POINTER(ccs_hyperparameter)])
ccs_expression_eval = _ccs_get_function("ccs_expression_eval", [ccs_expression, ccs_context, ct.POINTER(ccs_datum), ct.POINTER(ccs_datum)])
ccs_expression_list_eval_node = _ccs_get_function("ccs_expression_list_eval_node", [ccs_expression, ccs_context, ct.POINTER(ccs_datum), ct.c_size_t, ct.POINTER(ccs_datum)])
ccs_expression_get_hyperparameters = _ccs_get_function("ccs_expression_get_hyperparameters", [ccs_expression, ct.c_size_t, ct.POINTER(ccs_hyperparameter), ct.POINTER(ct.c_size_t)])
ccs_expression_check_context = _ccs_get_function("ccs_expression_check_context", [ccs_expression, ccs_context])

class Expression(Object):
  def  __init__(self, handle = None, retain = False, t = None, nodes = []):
    if handle is None:
      sz = len(nodes)
      handle = ccs_expression()
      v = (ccs_datum*sz)()
      for i in range(sz):
        v[i].value = nodes[i]
      res = ccs_create_expression(t, sz, v, ct.byref(handle))
      Error.check(res)
      super().__init__(handle = handle, retain = False)
    else:
      super().__init__(handle = handle, retain = retain)

  @classmethod
  def from_handle(cls, handle):
    v = ccs_expression_type(0)
    res = ccs_expression_get_type(handle, ct.byref(v))
    Error.check(res)
    v = v.value
    if v == ccs_expression_type.LIST:
      return List(handle = handle, retain = True)
    elif v == ccs_expression_type.LITERAL:
      return Literal(handle = handle, retain = True)
    elif v == ccs_expression_type.VARIABLE:
      return Variable(handle = handle, retain = True)
    else:
      return cls(handle = handle, retain = True)

  @classmethod
  def binary(cls, t, left, right):
    pvleft = ccs_datum(left)
    pvright = ccs_datum(right)
    vleft = ccs_datum_fix()
    vright = ccs_datum_fix()
    vleft.value = pvleft._value.i
    vleft.type = pvleft.type
    vright.value = pvright._value.i
    vright.type = pvright.type
    handle = ccs_expression()
    res = ccs_create_binary_expression(t, vleft, vright, ct.byref(handle))
    Error.check(res)
    return cls(handle = handle, retain = False)

  @classmethod
  def unary(cls, t, node):
    pvnode = ccs_datum(node)
    vnode = ccs_datum_fix()
    vnode.value = pvnode._value.i
    vnode.type = pvnode.type
    handle = ccs_expression()
    res = ccs_create_unary_expression(t, vnode, ct.byref(handle))
    Error.check(res)
    return cls(handle = handle, retain = False)

  @property
  def type(self):
    if hasattr(self, "_type"):
      return self._type
    v = ccs_expression_type(0)
    res = ccs_expression_get_type(self.handle, ct.byref(v))
    Error.check(res)
    self._type = v
    return v

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
  def hyperparameters(self):
    if hasattr(self, "_hyperparameters"):
      return self._hyperparameters
    sz = ct.c_size_t()
    res = ccs_expression_get_hyperparameters(self.handle, 0, None, ct.byref(sz))
    Error.check(res)
    sz = sz.value
    if sz == 0:
      self._hyperparameters = []
      return []
    v = (ccs_hyperparameter * sz.value)()
    res = ccs_expression_get_hyperparameters(self.handle, sz, v, None)
    Error.check(res)
    self._hyperparameters = [Hyperparameter.from_handle(ccs_hyperparameter(x)) for x in v]
    return self._hyperparameters

  def eval(self, context = None, values = None):
    if context and values:
      count = context.num_hyperparameters
      if count != len(values):
        raise Error(ccs_error.INVALID_VALUE)
      v = (ccs_datum * count)()
      for i in range(count):
        v[i].value = values[i]
      values = v
      context = context.handle
    elif context or values:
      raise Error(ccs_error.INVALID_VALUE)
    v = ccs_datum()
    res = ccs_expression_eval(self.handle, context, values, ct.byref(v))
    Error.check(res)
    return v.value

  def check_context(self, context):
    res = ccs_expression_check_context(self.handle, context.handle)
    Error.check(res)

  def __str__(self):
    t = self.type.value
    symbol = ccs_expression_symbols[t]
    prec = ccs_expression_precedence[t]
    nds = ["({})".format(n) if ccs_expression_precedence[n.type.value] < prec else n.__str__() for n in self.nodes]
    if len(nds) == 1:
      return "{}{}".format(symbol, nds[0])
    else:
      return "{} {} {}".format(nds[0], symbol, nds[1])


class Literal(Expression):
  def __init__(self, handle = None, retain = False, value = None):
    if handle is None:
      handle = ccs_expression()
      pv = ccs_datum(value)
      v = ccs_datum_fix()
      v.value = pv._value.i
      v.type = pv.type
      res = ccs_create_literal(v, ct.byref(handle))
      Error.check(res)
      super().__init__(handle = handle, retain = False)
    else:
      super().__init__(handle = handle, retain = retain)
 
  @property 
  def value(self):
    if hasattr(self, "_value"):
      return self._value
    v = ccs_datum()
    res = ccs_literal_get_value(self.handle, ct.byref(v))
    Error.check(res)
    self._value = v.value
    return self._value
  
  def __str__(self):
    v = self.value
    if isinstance(v, str):
      return repr(v)
    else:
      return "{}".format(v)


class Variable(Expression):
  def __init__(self, handle = None, retain = False, hyperparameter = None):
    if handle is None:
      handle = ccs_expression()
      res = ccs_create_variable(hyperparameter.handle, ct.byref(handle))
      Error.check(res)
      super().__init__(handle = handle, retain = False)
    else:
      super().__init__(handle = handle, retain = retain)
  
  @property 
  def hyperparameter(self):
    if hasattr(self, "_hyperparameter"):
      return self._hyperparameter
    v = ccs_hyperparameter()
    res = ccs_variable_get_hyperparameter(self.handle, ct.byref(v))
    Error.check(res)
    self._hyperparameter = Hyperparameter.from_handle(v)
    return self._hyperparameter
  
  def __str__(self):
    return self.hyperparameter.name


class List(Expression):
  def __init__(self, handle = None, retain = False, values = []):
    if handle is None:
      super().__init__(t = ccs_expression_type.LIST, nodes = values)
    else:
      super().__init__(handle = handle, retain = retain)
  
  def eval(self, index, context = None, values = None):
    if context and values:
      count = context.num_hyperparameters
      if count != len(values):
        raise Error(ccs_error.INVALID_VALUE)
      v = (ccs_datum * count)()
      for i in range(count):
        v[i].value = values[i]
      values = v
      context = context.handle
    elif context or values:
      raise Error(ccs_error.INVALID_VALUE)
    v = ccs_datum()
    res = ccs_expression_list_eval_node(self.handle, context, values, index, ct.byref(v))
    Error.check(res)
    return v.value

  def __str__(self):
    return "[ {} ]".format(", ".join(map(str, self.nodes)))

