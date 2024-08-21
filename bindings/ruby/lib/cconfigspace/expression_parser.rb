def silence_warnings(&block)
  warn_level = $VERBOSE
  $VERBOSE = nil
  result = block.call
  $VERBOSE = warn_level
  result
end

silence_warnings {
  require 'whittle'
}

undef silence_warnings
module CCS
  AssociativityMap = {
    :CCS_ASSOCIATIVITY_TYPE_LEFT_TO_RIGHT => :left,
    :CCS_ASSOCIATIVITY_TYPE_RIGHT_TO_LEFT => :right
  }
  class ExpressionParser < Whittle::Parser
    class << self
      attr_accessor :context
      attr_accessor :bind
      attr_accessor :mutex
      attr_reader :parser
    end

    @mutex = Mutex.new

    def parse(input, context: nil, binding: TOPLEVEL_BINDING, **options)
      self.class.mutex.synchronize {
        self.class.context = context
        self.class.bind = binding
        expr = super(input, options)
        self.class.context = nil
        self.class.bind = nil
        expr
      }
    end

    ExpressionSymbols.reverse_each { |k, v|
      next unless v
      next if k == :CCS_EXPRESSION_TYPE_POSITIVE || k == :CCS_EXPRESSION_TYPE_NEGATIVE
      associativity = AssociativityMap[ExpressionAssociativity[k]]
      precedence = ExpressionPrecedence[k]
      rule(v) % associativity ^ precedence
    }

    rule(:wsp => /\s+/).skip!
    rule("(")
    rule(")")
    rule("[")
    rule("]")
    rule(",")

    rule(:none => Regexp.new(TerminalRegexp[:CCS_TERMINAL_TYPE_NONE])).as { |b|
      Expression::Literal::new(value: nil) }
    rule(:true => Regexp.new(TerminalRegexp[:CCS_TERMINAL_TYPE_TRUE])).as { |b|
      Expression::Literal::new(value: true) }
    rule(:false => Regexp.new(TerminalRegexp[:CCS_TERMINAL_TYPE_FALSE])).as { |b|
      Expression::Literal::new(value: false) }
    rule(:float => Regexp.new(TerminalRegexp[:CCS_TERMINAL_TYPE_FLOAT])).as {|num|
      Expression::Literal::new(value: Float(num)) }
    rule(:integer => Regexp.new(TerminalRegexp[:CCS_TERMINAL_TYPE_INTEGER])).as { |num|
      Expression::Literal::new(value: Integer(num)) }
    rule(:identifier => /[:a-zA-Z_][a-zA-Z_0-9]*/)
    rule(:variable) do |r|
      r[:identifier].as { |identifier| Expression::Variable::new(parameter: context.kind_of?(Context) ? context.parameter_by_name(identifier) : context[identifier]) }
    end
    rule(:string => Regexp.new(TerminalRegexp[:CCS_TERMINAL_TYPE_STRING])).as { |str|
      Expression::Literal::new(value: eval(str)) }


    rule(:value) do |r|
      r[:none]
      r[:true]
      r[:false]
      r[:string]
      r[:user_defined]
      r[:variable]
      r[:float]
      r[:integer]
    end

    rule(:list_item) do |r|
      r[:list_item, ",", :value].as { |l, _, e| l.push e }
      r[:value].as { |e| [e] }
    end

    rule(:list) do |r|
      r["[", :list_item, "]"].as { |_, l, _| Expression::List::new(values: l) }
      r["[", "]"].as { |_, _| Expression::List::new(values: []) }
    end

    rule(:user_defined) do |r|
      r[:identifier, "(", :list_item, ")"].as do |e, _, l, _|
        m = bind.receiver.method(e.to_sym)
        evaluate = lambda { |expr, *args| m.call(*args) }
        Expression::UserDefined.new(name: e, nodes: l, eval: evaluate)
      end
      r[:identifier, "(", ")"].as do |e, _, _|
        m = bind.receiver.method(e.to_sym)
        evaluate = lambda { |expr, *args| m.call }
        Expression::UserDefined.new(name: e, eval: evaluate)
      end
    end

    rule(:expr) do |r|
      r["(", :expr, ")"].as { |_, exp, _| exp }
      ExpressionSymbols.reverse_each { |k, v|
        next unless v
        next if v == "#"
        arity = ExpressionArity[k]
        if arity == 1
          r[v, :expr].as { |_, a| Expression.expression_map[k].new(node: a) }
        else
          r[:expr, v, :expr].as { |a, _, b| Expression.expression_map[k].new(left: a, right: b) }
        end
      }
      r[:expr, "#", :list].as { |v, _, l| Expression::In.new(left: v, right: l) }
      r[:value]
    end

    start(:expr)

  end

  @parser = ExpressionParser.new

  def self.parse(input, context: nil, binding: TOPLEVEL_BINDING, **params)
    @parser.parse(input, context: context, binding: binding, **params)
  end

end
