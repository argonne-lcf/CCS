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
      next if k == :CCS_POSITIVE || k == :CCS_NEGATIVE
      associativity = ExpressionAssociativity[k] == :CCS_LEFT_TO_RIGHT ? :left :
                      ExpressionAssociativity[k] == :CCS_RIGHT_TO_LEFT ? :right : nil
      precedence = ExpressionPrecedence[k]
      eval "rule('#{v}') % :#{associativity} ^ #{precedence}"
    }

    rule(:wsp => /\s+/).skip!
    rule("(")
    rule(")")
    rule("[")
    rule("]")
    rule(",")

    rule(:float => /-?[0-9]+([eE][+-]?[0-9]+|\.[0-9]+([eE][+-]?[0-9]+)?)/).as {|num|
      Literal::new(value: Float(num)) }
    rule(:integer => /-?[0-9]+/).as { |num|
      Literal::new(value: Integer(num)) }
    rule(:identifier => /[a-zA-Z_][a-zA-Z_0-9]*/).as { |identifier|
      Variable::new(hyperparameter: context.hyperparameter_by_name(identifier)) }
    rule(:string => /"([^\0\t\n\r\f"\\]|\\[0tnrf"\\])+"|'([^\0\t\n\r\f'\\]|\\[0tnrf'\\])+'/).as { |str|
      Literal::new(value: eval(str)) }

    rule(:value) do |r|
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

    rule(:in_expr) do |r|
      r[:value, "#", :list].as { |v, _, l|
        Expression.binary(type: :CCS_IN, left: v, right: l) }
    end

    rule(:expr) do |r|
      r["(", :expr, ")"].as { |_, exp, _| exp }
      ExpressionSymbols.reverse_each { |k, v|
        next unless v
        next if v == "#"
        arity = ExpressionArity[k]
        if arity == 1
          eval "r['#{v}', :expr].as { |_, a| Expression.unary(type: :#{k}, node: a) }"
        else
          eval "r[:expr, '#{v}', :expr].as { |a, _, b| Expression.binary(type: :#{k}, left: a, right: b) }"
        end
      }
      r[:in_expr]
      r[:value]
    end

    start(:expr)
  end

end
