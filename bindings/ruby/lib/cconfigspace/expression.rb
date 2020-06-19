module CCS
  ExpressionType = enum FFI::Type::INT32, :ccs_expression_type_t, [
    :CCS_OR, 0,
    :CCS_AND,
    :CCS_EQUAL,
    :CCS_NOT_EQUAL,
    :CCS_LESS,
    :CCS_GREATER,
    :CCS_LESS_OR_EQUAL,
    :CCS_GREATER_OR_EQUAL,
    :CCS_IN,
    :CCS_ADD,
    :CCS_SUBSTRACT,
    :CCS_MULTIPLY,
    :CCS_DIVIDE,
    :CCS_MODULO,
    :CCS_POSITIVE,
    :CCS_NEGATIVE,
    :CCS_NOT,
    :CCS_LIST,
    :CCS_LITERAL,
    :CCS_VARIABLE
  ]
  class MemoryPointer
    def read_ccs_expression_type_t
      ExpressionType.from_native(read_int32, nil)
    end
  end

  attach_variable :expression_precedence, :ccs_expression_precedence, FFI::ArrayType::new(find_type(:int), ExpressionType.symbol_map.size)
  attach_variable :expression_symbols, :ccs_expression_symbols, FFI::ArrayType::new(find_type(:string), ExpressionType.symbol_map.size)
  attach_variable :expression_arity, :ccs_expression_arity, FFI::ArrayType::new(find_type(:int), ExpressionType.symbol_map.size)

  ExpressionPrecedence = ExpressionType.symbol_map.collect { |k, v|
    [k, expression_precedence[v]]
  }.to_h
  ExpressionSymbols = ExpressionType.symbol_map.collect { |k, v|
    p = expression_symbols[v]
    [k, p]
  }.to_h

  attach_function :ccs_create_binary_expression, [:ccs_expression_type_t, :ccs_datum_t, :ccs_datum_t, :pointer], :ccs_result_t
  attach_function :ccs_create_unary_expression, [:ccs_expression_type_t, :ccs_datum_t, :pointer], :ccs_result_t
  attach_function :ccs_create_expression, [:ccs_expression_type_t, :size_t, :pointer, :pointer], :ccs_result_t
  attach_function :ccs_create_literal, [:ccs_datum_t, :pointer], :ccs_result_t
  attach_function :ccs_create_variable, [:ccs_hyperparameter_t, :pointer], :ccs_result_t
  attach_function :ccs_expression_get_type, [:ccs_expression_t, :pointer], :ccs_result_t
  attach_function :ccs_expression_get_num_nodes, [:ccs_expression_t, :pointer], :ccs_result_t
  attach_function :ccs_expression_get_nodes, [:ccs_expression_t, :size_t, :pointer, :pointer], :ccs_result_t
  attach_function :ccs_literal_get_value, [:ccs_expression_t, :pointer], :ccs_result_t
  attach_function :ccs_variable_get_hyperparameter, [:ccs_expression_t, :pointer], :ccs_result_t
  attach_function :ccs_expression_eval, [:ccs_expression_t, :ccs_context_t, :pointer, :pointer], :ccs_result_t
  attach_function :ccs_expression_list_eval_node, [:ccs_expression_t, :ccs_context_t, :pointer, :size_t, :pointer], :ccs_result_t
  attach_function :ccs_expression_get_hyperparameters, [:ccs_expression_t, :size_t, :pointer, :pointer], :ccs_result_t
  attach_function :ccs_expression_check_context, [:ccs_expression_t, :ccs_context_t], :ccs_result_t

  class Expression < Object
    add_property :type, :ccs_expression_type_t, :ccs_expression_get_type, memoize: true
    add_property :num_nodes, :size_t, :ccs_expression_get_num_nodes, memoize: true
    def initialize(handle = nil, retain: false, type: nil, nodes: [])
      if handle
        super(handle, retain: retain)
      else
        count = nodes.size
        p_nodes = MemoryPointer::new(:ccs_datum_t, count)
        ptr = MemoryPointer::new(:ccs_expression_t)
        nodes.each_with_index { |n, i| Datum::new(p_nodes[i]).value = n }
        res = CCS.ccs_create_expression(type, count, p_nodes, ptr)
        CCS.error_check(res)
        super(ptr.read_ccs_expression_t, retain: false)
      end
    end

    def self.from_handle(handle)
      ptr = MemoryPointer::new(:ccs_expression_type_t)
      res = CCS.ccs_expression_get_type(handle, ptr)
      CCS.error_check(res)
      case ptr.read_ccs_expression_type_t
      when :CCS_LIST
        List::new(handle, retain: true)
      when :CCS_LITERAL
	Literal::new(handle, retain: true)
      when :CCS_VARIABLE
        Variable::new(handle, retain: true)
      else
        Expression::new(handle, retain: true)
      end
    end

    def self.binary(type:, left:, right:)
      ptr = MemoryPointer::new(:ccs_expression_t)
      res = CCS.ccs_create_binary_expression(type, Datum.from_value(left), Datum.from_value(right), ptr)
      CCS.error_check(res)
      self.new(ptr.read_ccs_expression_t, retain: false)
    end

    def self.unary(type:, node:)
      ptr = MemoryPointer::new(:ccs_expression_t)
      res = CCS.ccs_create_unary_expression(type, Datum.from_value(node), ptr)
      CCS.error_check(res)
      self.new(ptr.read_ccs_expression_t, retain: false)
    end

    def nodes
      count = num_nodes
      ptr = MemoryPointer::new(:ccs_expression_t, count)
      res = CCS.ccs_expression_get_nodes(@handle, count, ptr, nil)
      CCS.error_check(res)
      count.times.collect { |i| Expression.from_handle(ptr[i].read_pointer) }
    end

    def eval(context: nil, values: nil)
      if values && context
        count = context.num_hyperparameters
        raise StandardError, :CCS_INVALID_VALUES if values.size != count
        p_values = MemoryPointer::new(:ccs_datum_t, count)
        values.each_with_index{ |v, i| Datum::new(p_values[i]).value = v }
        values = p_values
      end
      ptr = MemoryPointer::new(:ccs_datum_t)
      res = CCS.ccs_expression_eval(@handle, context, values, ptr)
      CCS.error_check(res)
      Datum::new(ptr).value
    end

    def hyperparameters
      @hyperparameters ||= begin
        ptr = MemoryPointer::new(:size_t)
        res = CCS.ccs_expression_get_hyperparameters(@handle, 0, nil, ptr)
        CCS.error_check(res)
        count = ptr.read_size_t
        if count == 0
          []
        else
          ptr = MemoryPointer::new(:ccs_hyperparameter_t, count)
          res = CCS.ccs_expression_get_hyperparameters(@handle, count, ptr, nil)
          CCS.error_check(res)
          count.times.collect { |i| Hyperparameters.from_handle(ptr[i].read_ccs_hyperparameter_t) }
        end
      end
    end

    def check_context(context)
      res = CCS.ccs_expression_check_context(@handle, context)
      CCS.error_check(res)
      self
    end

    def to_s
      t = type
      symbol = ExpressionSymbols[t]
      prec = ExpressionPrecedence[t]
      nds = nodes.collect { |n|
        nprec = ExpressionPrecedence[n.type]
        if nprec < prec
          "(#{n})"
        else
          n.to_s
        end
      }
      if nds.length == 1
        "#{symbol}#{nds[0]}"
      else
        "#{nds[0]} #{symbol} #{nds[1]}"
      end
    end
  end

  class Literal < Expression
    def initialize(handle = nil, retain: false, value: nil)
      if handle
        super(handle, retain: retain)
      else
        ptr = MemoryPointer::new(:ccs_expression_t)
        res = CCS.ccs_create_literal(Datum::from_value(value), ptr)
        CCS.error_check(res)
        super(ptr.read_ccs_expression_t, retain: false)
      end
    end

    def value
      ptr = MemoryPointer::new(:ccs_datum_t)
      res = CCS.ccs_literal_get_value(@handle, ptr)
      CCS.error_check(res)
      Datum::new(ptr).value
    end

    def to_s
      case value
      when String
        value.inspect
      else
        value.to_s
      end
    end
  end

  class Variable < Expression
    def initialize(handle = nil, retain: false, hyperparameter: nil)
      if handle
        super(handle, retain: retain)
      else
        ptr = MemoryPointer::new(:ccs_expression_t)
        res = CCS.ccs_create_variable(hyperparameter, ptr)
        CCS.error_check(res)
        super(ptr.read_ccs_expression_t, retain: false)
      end
    end

    def hyperparameter
      ptr = MemoryPointer::new(:ccs_hyperparameter_t)
      res = CCS.ccs_variable_get_hyperparameter(@handle, ptr)
      CCS.error_check(res)
      Hyperparameter.from_handle(ptr.read_ccs_hyperparameter_t)
    end

    def to_s
      hyperparameter.name
    end
  end

  class List < Expression
    def initialize(handle = nil, retain: false, values: nil)
      if handle
        super(handle, retain: retain)
      else
        super(nil, retain: false, type: :CCS_LIST, nodes: values)
      end
    end

    def eval(index, context: nil, values: nil)
      if values && context
        count = context.num_hyperparameters
        raise StandardError, :CCS_INVALID_VALUES if values.size != count
        p_values = MemoryPointer::new(:ccs_datum_t, count)
        values.each_with_index{ |v, i| Datum::new(p_values[i]).value = v }
        values = p_values
      end
      ptr = MemoryPointer::new(:ccs_datum_t)
      res = CCS.ccs_expression_list_eval_node(@handle, context, values, index, ptr)
      CCS.error_check(res)
      Datum::new(ptr).value
    end

    def to_s
      "[ #{nodes.collect(&:to_s).join(", ")} ]"
    end
  end
end
