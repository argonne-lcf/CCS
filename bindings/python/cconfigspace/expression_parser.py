from parglare import Parser, Grammar
from .expression import Expression, Literal, Variable, List, ccs_expression_type, ccs_associativity_type, ccs_expression_precedence, ccs_expression_associativity, ccs_expression_symbols, ccs_expression_arity, ccs_terminal_type, ccs_terminal_precedence, ccs_terminal_regexp

_associativity_map = {
  ccs_associativity_type.LEFT_TO_RIGHT: "left",
  ccs_associativity_type.RIGHT_TO_LEFT: "right"
}
ccs_grammar = "expr: '(' expr ')'\n"

for i in range(ccs_expression_type.OR, ccs_expression_type.IN):
  if ccs_expression_arity[i] == 1:
    ccs_grammar += "    | '{}' expr {{{}, {}}}\n".format(ccs_expression_symbols[i], _associativity_map[ccs_expression_associativity[i].value], ccs_expression_precedence[i])
  else:
    ccs_grammar += "    | expr '{}' expr {{{}, {}}}\n".format(ccs_expression_symbols[i], _associativity_map[ccs_expression_associativity[i].value], ccs_expression_precedence[i])

ccs_grammar += "    | expr '{}' list {{{}, {}}}".format(ccs_expression_symbols[ccs_expression_type.IN], _associativity_map[ccs_expression_associativity[ccs_expression_type.IN].value], ccs_expression_precedence[ccs_expression_type.IN])
ccs_grammar += r"""
    | value;
list: '[' list_item ']'
    | '[' ']';
list_item: list_item ',' value
         | value;
value: none
     | true
     | false
     | string
     | identifier
     | integer
     | float;

terminals
none: /%s/ {%d};
true: /%s/ {%d};
false: /%s/ {%d};
identifier: /%s/ {%d};
string: /%s/ {%d};
integer: /%s/ {%d};
float: /%s/ {%d};
""" % (ccs_terminal_regexp[ccs_terminal_type.NONE], ccs_terminal_precedence[ccs_terminal_type.NONE],
       ccs_terminal_regexp[ccs_terminal_type.TRUE], ccs_terminal_precedence[ccs_terminal_type.TRUE],
       ccs_terminal_regexp[ccs_terminal_type.FALSE], ccs_terminal_precedence[ccs_terminal_type.FALSE],
       ccs_terminal_regexp[ccs_terminal_type.IDENTIFIER], ccs_terminal_precedence[ccs_terminal_type.IDENTIFIER],
       ccs_terminal_regexp[ccs_terminal_type.STRING], ccs_terminal_precedence[ccs_terminal_type.STRING],
       ccs_terminal_regexp[ccs_terminal_type.INTEGER], ccs_terminal_precedence[ccs_terminal_type.INTEGER],
       ccs_terminal_regexp[ccs_terminal_type.FLOAT], ccs_terminal_precedence[ccs_terminal_type.FLOAT])

_actions = {}
_expr_actions = [ lambda _, n: n[1] ]
for i in range(ccs_expression_type.OR, ccs_expression_type.LIST):
  if ccs_expression_arity[i] == 1:
    _expr_actions.append((lambda s: lambda _, n: Expression.unary(t = s, node = n[1]))(i))
  else:
    _expr_actions.append((lambda s: lambda _, n: Expression.binary(t = s, left = n[0], right = n[2]))(i))
_expr_actions.append(lambda _, n: n[0])
_actions["expr"] = _expr_actions
_actions["list"] = [
  lambda _, n: List(values = n[1]),
  lambda _, n: List(values = [])
]
_actions["list_item"] = [
  lambda _, n: n[0] + [n[2]],
  lambda _, n: [n[0]]
]
_actions["none"] = lambda _, value: Literal(value = None)
_actions["true"] = lambda _, value: Literal(value = True)
_actions["false"] = lambda _, value: Literal(value = False)
_actions["identifier"] = lambda p, value: Variable(parameter = p.extra.parameter_by_name(value))
_actions["string"] = lambda _, value: Literal(value = eval(value))
_actions["float"] = lambda _, value: Literal(value = float(value))
_actions["integer"] = lambda _, value: Literal(value = int(value))

_g = Grammar.from_string(ccs_grammar)

ccs_parser = Parser(_g, actions=_actions)

