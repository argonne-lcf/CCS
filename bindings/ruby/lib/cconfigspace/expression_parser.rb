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
    end
    def initialize(context = nil)
      @context = context
    end
    def parse(*args)
      self.class.context = @context
      super
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

    rule(:none => Regexp.new(TerminalRegexp[:CCS_TERM_NONE])).as { |b|
      Literal::new(value: nil) }
    rule(:true => Regexp.new(TerminalRegexp[:CCS_TERM_TRUE])).as { |b|
      Literal::new(value: true) }
    rule(:false => Regexp.new(TerminalRegexp[:CCS_TERM_FALSE])).as { |b|
      Literal::new(value: false) }
    rule(:float => Regexp.new(TerminalRegexp[:CCS_TERM_FLOAT])).as {|num|
      Literal::new(value: Float(num)) }
    rule(:integer => Regexp.new(TerminalRegexp[:CCS_TERM_INTEGER])).as { |num|
      Literal::new(value: Integer(num)) }
    rule(:identifier => /[:a-zA-Z_][a-zA-Z_0-9]*/).as { |identifier|
      Variable::new(parameter: context.parameter_by_name(identifier)) }
    rule(:string => Regexp.new(TerminalRegexp[:CCS_TERM_STRING])).as { |str|
      Literal::new(value: eval(str)) }

    rule(:value) do |r|
      r[:none]
      r[:true]
      r[:false]
      r[:string]
      r[:identifier]
      r[:float]
      r[:integer]
    end

    rule(:list_item) do |r|
      r[:list_item, ",", :value].as { |l, _, e| l.push e }
      r[:value].as { |e| [e] }
    end

    rule(:list) do |r|
      r["[", :list_item, "]"].as { |_, l, _| List::new(values: l) }
      r["[", "]"].as { |_, _| List::new(values: []) }
    end

    rule(:expr) do |r|
      r["(", :expr, ")"].as { |_, exp, _| exp }
      ExpressionSymbols.reverse_each { |k, v|
        next unless v
        next if v == "#"
        arity = ExpressionArity[k]
        if arity == 1
          r[v, :expr].as { |_, a| Expression.unary(type: k, node: a) }
        else
          r[:expr, v, :expr].as { |a, _, b| Expression.binary(type: k, left: a, right: b) }
        end
      }
      r[:expr, "#", :list].as { |v, _, l| Expression.binary(type: :CCS_EXPRESSION_TYPE_IN, left: v, right: l) }
      r[:value]
    end

    start(:expr)
  end

end
