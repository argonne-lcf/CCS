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
    :CCS_ADD,
    :CCS_SUBSTRACT,
    :CCS_MULTIPLY,
    :CCS_DIVIDE,
    :CCS_MODULO,
    :CCS_POSITIVE,
    :CCS_NEGATIVE,
    :CCS_NOT,
    :CCS_IN,
    :CCS_LIST,
    :CCS_LITERAL,
    :CCS_VARIABLE
  ]
  class MemoryPointer
    def read_ccs_expression_type_t
      ExpressionType.from_native(read_int32, nil)
    end
  end

  AssociativityType = enum FFI::Type::INT32, :ccs_associativity_type_t, [
    :CCS_ASSOCIATIVITY_NONE, 0,
    :CCS_LEFT_TO_RIGHT,
    :CCS_RIGHT_TO_LEFT
  ]
  class MemoryPointer
    def read_ccs_associativity_type_t
      AssociativityType.from_native(read_int32, nil)
    end
  end

  attach_variable :expression_precedence, :ccs_expression_precedence, FFI::ArrayType::new(find_type(:int), ExpressionType.symbol_map.size)
  attach_variable :expression_associativity, :ccs_expression_associativity, FFI::ArrayType::new(find_type(:ccs_associativity_type_t), ExpressionType.symbol_map.size)
  attach_variable :expression_symbols, :ccs_expression_symbols, FFI::ArrayType::new(find_type(:string), ExpressionType.symbol_map.size)
  attach_variable :expression_arity, :ccs_expression_arity, FFI::ArrayType::new(find_type(:int), ExpressionType.symbol_map.size)

  ExpressionPrecedence = ExpressionType.symbol_map.collect { |k, v|
    [k, expression_precedence[v]]
  }.to_h
  ExpressionAssociativity = ExpressionType.symbol_map.collect { |k, v|
    [k, expression_associativity[v]]
  }.to_h
  ExpressionSymbols = ExpressionType.symbol_map.collect { |k, v|
    [k, expression_symbols[v]]
  }.to_h
  ExpressionArity = ExpressionType.symbol_map.collect { |k, v|
    [k, expression_arity[v]]
  }.to_h

  TerminalType = enum FFI::Type::INT32, :ccs_terminal_type_t, [
    :CCS_TERM_NONE, 0,
    :CCS_TERM_TRUE,
    :CCS_TERM_FALSE,
    :CCS_TERM_STRING,
    :CCS_TERM_IDENTIFIER,
    :CCS_TERM_INTEGER,
    :CCS_TERM_FLOAT,
  ]

  attach_variable :terminal_precedence, :ccs_terminal_precedence, FFI::ArrayType::new(find_type(:int), TerminalType.symbol_map.size)
  attach_variable :terminal_regexp, :ccs_terminal_regexp, FFI::ArrayType::new(find_type(:string), TerminalType.symbol_map.size)
  attach_variable :terminal_symbols, :ccs_terminal_symbols, FFI::ArrayType::new(find_type(:string), TerminalType.symbol_map.size)

  TerminalPrecedence = TerminalType.symbol_map.collect { |k, v|
    [k, terminal_precedence[v]]
  }.to_h
  TerminalRegexp = TerminalType.symbol_map.collect { |k, v|
    [k, terminal_regexp[v]]
  }.to_h
  TerminalSymbols = TerminalType.symbol_map.collect { |k, v|
    [k, terminal_symbols[v]]
  }.to_h

  attach_function :ccs_create_binary_expression, [:ccs_expression_type_t, :ccs_datum_t, :ccs_datum_t, :pointer], :ccs_error_t
  attach_function :ccs_create_unary_expression, [:ccs_expression_type_t, :ccs_datum_t, :pointer], :ccs_error_t
  attach_function :ccs_create_expression, [:ccs_expression_type_t, :size_t, :pointer, :pointer], :ccs_error_t
  attach_function :ccs_create_literal, [:ccs_datum_t, :pointer], :ccs_error_t
  attach_function :ccs_create_variable, [:ccs_parameter_t, :pointer], :ccs_error_t
  attach_function :ccs_expression_get_type, [:ccs_expression_t, :pointer], :ccs_error_t
  attach_function :ccs_expression_get_num_nodes, [:ccs_expression_t, :pointer], :ccs_error_t
  attach_function :ccs_expression_get_nodes, [:ccs_expression_t, :size_t, :pointer, :pointer], :ccs_error_t
  attach_function :ccs_literal_get_value, [:ccs_expression_t, :pointer], :ccs_error_t
  attach_function :ccs_variable_get_parameter, [:ccs_expression_t, :pointer], :ccs_error_t
  attach_function :ccs_expression_eval, [:ccs_expression_t, :ccs_context_t, :pointer, :pointer], :ccs_error_t
  attach_function :ccs_expression_list_eval_node, [:ccs_expression_t, :ccs_context_t, :pointer, :size_t, :pointer], :ccs_error_t
  attach_function :ccs_expression_get_parameters, [:ccs_expression_t, :size_t, :pointer, :pointer], :ccs_error_t
  attach_function :ccs_expression_check_context, [:ccs_expression_t, :ccs_context_t], :ccs_error_t

  class Expression < Object
    add_property :type, :ccs_expression_type_t, :ccs_expression_get_type, memoize: true
    add_property :num_nodes, :size_t, :ccs_expression_get_num_nodes, memoize: true
    def initialize(handle = nil, retain: false, auto_release: true,
                   type: nil, nodes: [])
      if handle
        super(handle, retain: retain)
      else
        count = nodes.size
        p_nodes = MemoryPointer::new(:ccs_datum_t, count)
        ss = []
        ptr = MemoryPointer::new(:ccs_expression_t)
        nodes.each_with_index { |n, i| Datum::new(p_nodes[i]).set_value(n, string_store: ss) }
        CCS.error_check CCS.ccs_create_expression(type, count, p_nodes, ptr)
        super(ptr.read_ccs_expression_t, retain: false)
      end
    end

    def self.from_handle(handle, retain: true, auto_release: true)
      ptr = MemoryPointer::new(:ccs_expression_type_t)
      CCS.error_check CCS.ccs_expression_get_type(handle, ptr)
      case ptr.read_ccs_expression_type_t
      when :CCS_LIST
        List
      when :CCS_LITERAL
	Literal
      when :CCS_VARIABLE
        Variable
      else
        Expression
      end.new(handle, retain: retain, auto_release: auto_release)
    end

    def self.binary(type:, left:, right:)
      ptr = MemoryPointer::new(:ccs_expression_t)
      CCS.error_check CCS.ccs_create_binary_expression(type, Datum.from_value(left), Datum.from_value(right), ptr)
      self.new(ptr.read_ccs_expression_t, retain: false)
    end

    def self.unary(type:, node:)
      ptr = MemoryPointer::new(:ccs_expression_t)
      CCS.error_check CCS.ccs_create_unary_expression(type, Datum.from_value(node), ptr)
      self.new(ptr.read_ccs_expression_t, retain: false)
    end

    def nodes
      count = num_nodes
      ptr = MemoryPointer::new(:ccs_expression_t, count)
      CCS.error_check CCS.ccs_expression_get_nodes(@handle, count, ptr, nil)
      count.times.collect { |i| Expression.from_handle(ptr[i].read_pointer) }
    end

    def eval(context: nil, values: nil)
      if values && context
        count = context.num_parameters
        raise CCSError, :CCS_INVALID_VALUES if values.size != count
        ss = []
        p_values = MemoryPointer::new(:ccs_datum_t, count)
        values.each_with_index{ |v, i| Datum::new(p_values[i]).set_value(v, string_store: ss) }
        values = p_values
      elsif values || context
        raise CCSError, :CCS_INVALID_VALUES
      end
      ptr = MemoryPointer::new(:ccs_datum_t)
      CCS.error_check CCS.ccs_expression_eval(@handle, context, values, ptr)
      Datum::new(ptr).value
    end

    def parameters
      @parameters ||= begin
        ptr = MemoryPointer::new(:size_t)
        CCS.error_check CCS.ccs_expression_get_parameters(@handle, 0, nil, ptr)
        count = ptr.read_size_t
        if count == 0
          []
        else
          ptr = MemoryPointer::new(:ccs_parameter_t, count)
          CCS.error_check CCS.ccs_expression_get_parameters(@handle, count, ptr, nil)
          count.times.collect { |i| Parameters.from_handle(ptr[i].read_ccs_parameter_t) }
        end
      end
    end

    def check_context(context)
      CCS.error_check CCS.ccs_expression_check_context(@handle, context)
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
    NONE_SYMBOL = TerminalSymbols[:CCS_TERM_NONE]
    TRUE_SYMBOL = TerminalSymbols[:CCS_TERM_TRUE]
    FALSE_SYMBOL = TerminalSymbols[:CCS_TERM_FALSE]

    def initialize(handle = nil, retain: false, auto_release: true,
                   value: nil)
      if handle
        super(handle, retain: retain)
      else
        ptr = MemoryPointer::new(:ccs_expression_t)
        CCS.error_check CCS.ccs_create_literal(Datum::from_value(value), ptr)
        super(ptr.read_ccs_expression_t, retain: false)
      end
    end

    def value
      ptr = MemoryPointer::new(:ccs_datum_t)
      CCS.error_check CCS.ccs_literal_get_value(@handle, ptr)
      Datum::new(ptr).value
    end

    def to_s
      case value
      when nil
        NONE_SYMBOL
      when true
        TRUE_SYMBOL
      when false
        FALSE_SYMBOL
      when String
        value.inspect
      else
        value.to_s
      end
    end
  end

  class Variable < Expression
    def initialize(handle = nil, retain: false, auto_release: true,
                   parameter: nil)
      if handle
        super(handle, retain: retain)
      else
        ptr = MemoryPointer::new(:ccs_expression_t)
        CCS.error_check CCS.ccs_create_variable(parameter, ptr)
        super(ptr.read_ccs_expression_t, retain: false)
      end
    end

    def parameter
      ptr = MemoryPointer::new(:ccs_parameter_t)
      CCS.error_check CCS.ccs_variable_get_parameter(@handle, ptr)
      Parameter.from_handle(ptr.read_ccs_parameter_t)
    end

    def to_s
      parameter.name
    end
  end

  class List < Expression
    def initialize(handle = nil, retain: false, auto_release: true,
                   values: nil)
      if handle
        super(handle, retain: retain)
      else
        super(nil, retain: false, type: :CCS_LIST, nodes: values)
      end
    end

    def eval(index, context: nil, values: nil)
      if values && context
        count = context.num_parameters
        raise CCSError, :CCS_INVALID_VALUES if values.size != count
        ss = []
        p_values = MemoryPointer::new(:ccs_datum_t, count)
        values.each_with_index{ |v, i| Datum::new(p_values[i]).set_value(v, string_store: ss) }
        values = p_values
      elsif values || context
        raise CCSError, :CCS_INVALID_VALUES
      end
      ptr = MemoryPointer::new(:ccs_datum_t)
      CCS.error_check CCS.ccs_expression_list_eval_node(@handle, context, values, index, ptr)
      Datum::new(ptr).value
    end

    def to_s
      "[ #{nodes.collect(&:to_s).join(", ")} ]"
    end
  end
end
