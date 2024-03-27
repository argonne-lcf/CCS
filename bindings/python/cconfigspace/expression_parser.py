from parglare import Parser, Grammar
from .expression import Expression, ExpressionType, AssociativityType, ccs_expression_precedence, ccs_expression_associativity, ccs_expression_symbols, ccs_expression_arity, TerminalType, ccs_terminal_precedence, ccs_terminal_regexp

_associativity_map = {
  AssociativityType.LEFT_TO_RIGHT: "left",
  AssociativityType.RIGHT_TO_LEFT: "right"
}
ccs_grammar = "expr: '(' expr ')'\n"

for i in range(ExpressionType.OR, ExpressionType.IN):
  if ccs_expression_arity[i] == 1:
    ccs_grammar += "    | '{}' expr {{{}, {}}}\n".format(ccs_expression_symbols[i], _associativity_map[ccs_expression_associativity[i].value], ccs_expression_precedence[i])
  else:
    ccs_grammar += "    | expr '{}' expr {{{}, {}}}\n".format(ccs_expression_symbols[i], _associativity_map[ccs_expression_associativity[i].value], ccs_expression_precedence[i])

ccs_grammar += "    | expr '{}' list {{{}, {}}}".format(ccs_expression_symbols[ExpressionType.IN], _associativity_map[ccs_expression_associativity[ExpressionType.IN].value], ccs_expression_precedence[ExpressionType.IN])
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
""" % (ccs_terminal_regexp[TerminalType.NONE], ccs_terminal_precedence[TerminalType.NONE],
       ccs_terminal_regexp[TerminalType.TRUE], ccs_terminal_precedence[TerminalType.TRUE],
       ccs_terminal_regexp[TerminalType.FALSE], ccs_terminal_precedence[TerminalType.FALSE],
       ccs_terminal_regexp[TerminalType.IDENTIFIER], ccs_terminal_precedence[TerminalType.IDENTIFIER],
       ccs_terminal_regexp[TerminalType.STRING], ccs_terminal_precedence[TerminalType.STRING],
       ccs_terminal_regexp[TerminalType.INTEGER], ccs_terminal_precedence[TerminalType.INTEGER],
       ccs_terminal_regexp[TerminalType.FLOAT], ccs_terminal_precedence[TerminalType.FLOAT])

_actions = {}
_expr_actions = [ lambda _, n: n[1] ]
for i in range(ExpressionType.OR, ExpressionType.LIST):
  if ccs_expression_arity[i] == 1:
    _expr_actions.append((lambda s: lambda _, n: Expression.EXPRESSION_MAP[s](node = n[1]))(i))
  else:
    _expr_actions.append((lambda s: lambda _, n: Expression.EXPRESSION_MAP[s](left = n[0], right = n[2]))(i))
_expr_actions.append(lambda _, n: n[0])
_actions["expr"] = _expr_actions
_actions["list"] = [
  lambda _, n: Expression.List(values = n[1]),
  lambda _, n: Expression.List(values = [])
]
_actions["list_item"] = [
  lambda _, n: n[0] + [n[2]],
  lambda _, n: [n[0]]
]
_actions["none"] = lambda _, value: Expression.Literal(value = None)
_actions["true"] = lambda _, value: Expression.Literal(value = True)
_actions["false"] = lambda _, value: Expression.Literal(value = False)
_actions["identifier"] = lambda p, value: Expression.Variable(parameter = p.extra[value] if isinstance(p.extra, dict) else p.extra.parameter_by_name(value))
_actions["string"] = lambda _, value: Expression.Literal(value = eval(value))
_actions["float"] = lambda _, value: Expression.Literal(value = float(value))
_actions["integer"] = lambda _, value: Expression.Literal(value = int(value))

_g = Grammar.from_string(ccs_grammar)

parser = Parser(_g, actions=_actions)

def parse(expr):
  return parser.parse(expr)
