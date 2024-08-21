module CCS
  ExpressionType = enum FFI::Type::INT32, :ccs_expression_type_t, [
    :CCS_EXPRESSION_TYPE_OR, 0,
    :CCS_EXPRESSION_TYPE_AND,
    :CCS_EXPRESSION_TYPE_EQUAL,
    :CCS_EXPRESSION_TYPE_NOT_EQUAL,
    :CCS_EXPRESSION_TYPE_LESS,
    :CCS_EXPRESSION_TYPE_GREATER,
    :CCS_EXPRESSION_TYPE_LESS_OR_EQUAL,
    :CCS_EXPRESSION_TYPE_GREATER_OR_EQUAL,
    :CCS_EXPRESSION_TYPE_ADD,
    :CCS_EXPRESSION_TYPE_SUBSTRACT,
    :CCS_EXPRESSION_TYPE_MULTIPLY,
    :CCS_EXPRESSION_TYPE_DIVIDE,
    :CCS_EXPRESSION_TYPE_MODULO,
    :CCS_EXPRESSION_TYPE_POSITIVE,
    :CCS_EXPRESSION_TYPE_NEGATIVE,
    :CCS_EXPRESSION_TYPE_NOT,
    :CCS_EXPRESSION_TYPE_IN,
    :CCS_EXPRESSION_TYPE_LIST,
    :CCS_EXPRESSION_TYPE_LITERAL,
    :CCS_EXPRESSION_TYPE_VARIABLE,
    :CCS_EXPRESSION_TYPE_USER_DEFINED,
  ]
  module MemoryAccessor
    def read_ccs_expression_type_t
      ExpressionType.from_native(read_int32, nil)
    end
  end

  AssociativityType = enum FFI::Type::INT32, :ccs_associativity_type_t, [
    :CCS_ASSOCIATIVITY_TYPE_NONE, 0,
    :CCS_ASSOCIATIVITY_TYPE_LEFT_TO_RIGHT,
    :CCS_ASSOCIATIVITY_TYPE_RIGHT_TO_LEFT
  ]
  module MemoryAccessor
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
    :CCS_TERMINAL_TYPE_NONE, 0,
    :CCS_TERMINAL_TYPE_TRUE,
    :CCS_TERMINAL_TYPE_FALSE,
    :CCS_TERMINAL_TYPE_STRING,
    :CCS_TERMINAL_TYPE_IDENTIFIER,
    :CCS_TERMINAL_TYPE_INTEGER,
    :CCS_TERMINAL_TYPE_FLOAT,
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

  attach_function :ccs_create_binary_expression, [:ccs_expression_type_t, :ccs_datum_t, :ccs_datum_t, :pointer], :ccs_result_t
  attach_function :ccs_create_unary_expression, [:ccs_expression_type_t, :ccs_datum_t, :pointer], :ccs_result_t
  attach_function :ccs_create_expression, [:ccs_expression_type_t, :size_t, :pointer, :pointer], :ccs_result_t
  attach_function :ccs_expression_get_type, [:ccs_expression_t, :pointer], :ccs_result_t
  attach_function :ccs_expression_get_nodes, [:ccs_expression_t, :size_t, :pointer, :pointer], :ccs_result_t
  attach_function :ccs_expression_eval, [:ccs_expression_t, :size_t, :pointer, :pointer], :ccs_result_t
  attach_function :ccs_expression_get_parameters, [:ccs_expression_t, :size_t, :pointer, :pointer], :ccs_result_t
  attach_function :ccs_expression_check_contexts, [:ccs_expression_t, :size_t, :pointer], :ccs_result_t

  class Expression < Object
    add_property :type, :ccs_expression_type_t, :ccs_expression_get_type, memoize: true
    add_handle_array_property :nodes, :ccs_expression_t, :ccs_expression_get_nodes, memoize: true
    add_handle_array_property :parameters, :ccs_parameter_t, :ccs_expression_get_parameters, memoize: true

    def self.expression_map
      @expression_map ||= {
        CCS_EXPRESSION_TYPE_OR: Or,
        CCS_EXPRESSION_TYPE_AND: And,
        CCS_EXPRESSION_TYPE_EQUAL: Equal,
        CCS_EXPRESSION_TYPE_NOT_EQUAL: NotEqual,
        CCS_EXPRESSION_TYPE_LESS: Less,
        CCS_EXPRESSION_TYPE_GREATER: Greater,
        CCS_EXPRESSION_TYPE_LESS_OR_EQUAL: LessOrEqual,
        CCS_EXPRESSION_TYPE_GREATER_OR_EQUAL: GreaterOrEqual,
        CCS_EXPRESSION_TYPE_ADD: Add,
        CCS_EXPRESSION_TYPE_SUBSTRACT: Substract,
        CCS_EXPRESSION_TYPE_MULTIPLY: Multiply,
        CCS_EXPRESSION_TYPE_DIVIDE: Divide,
        CCS_EXPRESSION_TYPE_MODULO: Modulo,
        CCS_EXPRESSION_TYPE_POSITIVE: Positive,
        CCS_EXPRESSION_TYPE_NEGATIVE: Negative,
        CCS_EXPRESSION_TYPE_NOT: Not,
        CCS_EXPRESSION_TYPE_IN: In,
        CCS_EXPRESSION_TYPE_LIST: List,
        CCS_EXPRESSION_TYPE_LITERAL: Literal,
        CCS_EXPRESSION_TYPE_VARIABLE: Variable,
        CCS_EXPRESSION_TYPE_USER_DEFINED: UserDefined,
      }
    end

    def self.from_handle(handle, retain: true, auto_release: true)
      ptr = MemoryPointer::new(:ccs_expression_type_t)
      CCS.error_check CCS.ccs_expression_get_type(handle, ptr)
      klass = expression_map[ptr.read_ccs_expression_type_t]
      raise CCSError, :CCS_RESULT_ERROR_INVALID_EXPRESSION unless klass
      klass.new(handle, retain: retain, auto_release: auto_release)
    end

    def create_binary(type, left, right)
      ptr = MemoryPointer::new(:ccs_expression_t)
      CCS.error_check CCS.ccs_create_binary_expression(type, Datum.from_value(left), Datum.from_value(right), ptr)
      return ptr.read_ccs_expression_t
    end

    private :create_binary

    def create_unary(type, node)
      ptr = MemoryPointer::new(:ccs_expression_t)
      CCS.error_check CCS.ccs_create_unary_expression(type, Datum.from_value(node), ptr)
      return ptr.read_ccs_expression_t
    end

    private :create_unary

    def eval(bindings: nil)
      count = 0
      p_bindings = nil
      if bindings
        count = bindings.size
        p_bindings = MemoryPointer::new(:ccs_binding_t, count)
        p_bindings.write_array_of_pointer(bindings.collect(&:handle))
      end
      ptr = MemoryPointer::new(:ccs_datum_t)
      CCS.error_check CCS.ccs_expression_eval(@handle, count, p_bindings, ptr)
      Datum::new(ptr).value
    end

    def check_contexts(contexts)
      count = contexts.size
      p_contexts = MemoryPointer::new(:ccs_context_t, count)
      p_contexts.write_array_of_pointer(contexts.collect(&:handle))
      CCS.error_check CCS.ccs_expression_check_contexts(@handle, count, p_contexts)
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

    def self.get_function_vector_data(name, binding: TOPLEVEL_BINDING)
      m = binding.receiver.method(name.to_sym)
      evaluate = lambda { |expr, *args| m.call(*args) }
      [CCS::Expression::UserDefined.get_vector(eval: evaluate), nil]
    end

  end

  class ExpressionOr < Expression

    def initialize(handle = nil, retain: false, auto_release: true,
                   left: nil, right: nil)
      if handle
        super(handle, retain: retain, auto_release: auto_release)
      else
        handle = create_binary(:CCS_EXPRESSION_TYPE_OR, left, right)
        super(handle, retain: false)
      end
    end

  end

  Expression::Or = ExpressionOr

  class ExpressionAnd < Expression

    def initialize(handle = nil, retain: false, auto_release: true,
                   left: nil, right: nil)
      if handle
        super(handle, retain: retain, auto_release: auto_release)
      else
        handle = create_binary(:CCS_EXPRESSION_TYPE_AND, left, right)
        super(handle, retain: false)
      end
    end

  end

  Expression::And = ExpressionAnd

  class ExpressionEqual < Expression

    def initialize(handle = nil, retain: false, auto_release: true,
                   left: nil, right: nil)
      if handle
        super(handle, retain: retain, auto_release: auto_release)
      else
        handle = create_binary(:CCS_EXPRESSION_TYPE_EQUAL, left, right)
        super(handle, retain: false)
      end
    end

  end

  Expression::Equal = ExpressionEqual

  class ExpressionNotEqual < Expression

    def initialize(handle = nil, retain: false, auto_release: true,
                   left: nil, right: nil)
      if handle
        super(handle, retain: retain, auto_release: auto_release)
      else
        handle = create_binary(:CCS_EXPRESSION_TYPE_NOT_EQUAL, left, right)
        super(handle, retain: false)
      end
    end

  end

  Expression::NotEqual = ExpressionNotEqual

  class ExpressionLess < Expression

    def initialize(handle = nil, retain: false, auto_release: true,
                   left: nil, right: nil)
      if handle
        super(handle, retain: retain, auto_release: auto_release)
      else
        handle = create_binary(:CCS_EXPRESSION_TYPE_LESS, left, right)
        super(handle, retain: false)
      end
    end

  end

  Expression::Less = ExpressionLess

  class ExpressionGreater < Expression

    def initialize(handle = nil, retain: false, auto_release: true,
                   left: nil, right: nil)
      if handle
        super(handle, retain: retain, auto_release: auto_release)
      else
        handle = create_binary(:CCS_EXPRESSION_TYPE_GREATER, left, right)
        super(handle, retain: false)
      end
    end

  end

  Expression::Greater = ExpressionGreater

  class ExpressionLessOrEqual < Expression

    def initialize(handle = nil, retain: false, auto_release: true,
                   left: nil, right: nil)
      if handle
        super(handle, retain: retain, auto_release: auto_release)
      else
        handle = create_binary(:CCS_EXPRESSION_TYPE_LESS_OR_EQUAL, left, right)
        super(handle, retain: false)
      end
    end

  end

  Expression::LessOrEqual = ExpressionLessOrEqual

  class ExpressionGreaterOrEqual < Expression

    def initialize(handle = nil, retain: false, auto_release: true,
                   left: nil, right: nil)
      if handle
        super(handle, retain: retain, auto_release: auto_release)
      else
        handle = create_binary(:CCS_EXPRESSION_TYPE_GREATER_OR_EQUAL, left, right)
        super(handle, retain: false)
      end
    end

  end

  Expression::GreaterOrEqual = ExpressionGreaterOrEqual

  class ExpressionAdd < Expression

    def initialize(handle = nil, retain: false, auto_release: true,
                   left: nil, right: nil)
      if handle
        super(handle, retain: retain, auto_release: auto_release)
      else
        handle = create_binary(:CCS_EXPRESSION_TYPE_ADD, left, right)
        super(handle, retain: false)
      end
    end

  end

  Expression::Add = ExpressionAdd

  class ExpressionSubstract < Expression

    def initialize(handle = nil, retain: false, auto_release: true,
                   left: nil, right: nil)
      if handle
        super(handle, retain: retain, auto_release: auto_release)
      else
        handle = create_binary(:CCS_EXPRESSION_TYPE_SUBSTRACT, left, right)
        super(handle, retain: false)
      end
    end

  end

  Expression::Substract = ExpressionSubstract

  class ExpressionMultiply < Expression

    def initialize(handle = nil, retain: false, auto_release: true,
                   left: nil, right: nil)
      if handle
        super(handle, retain: retain, auto_release: auto_release)
      else
        handle = create_binary(:CCS_EXPRESSION_TYPE_MULTIPLY, left, right)
        super(handle, retain: false)
      end
    end

  end

  Expression::Multiply = ExpressionMultiply

  class ExpressionDivide < Expression

    def initialize(handle = nil, retain: false, auto_release: true,
                   left: nil, right: nil)
      if handle
        super(handle, retain: retain, auto_release: auto_release)
      else
        handle = create_binary(:CCS_EXPRESSION_TYPE_DIVIDE, left, right)
        super(handle, retain: false)
      end
    end

  end

  Expression::Divide = ExpressionDivide

  class ExpressionModulo < Expression

    def initialize(handle = nil, retain: false, auto_release: true,
                   left: nil, right: nil)
      if handle
        super(handle, retain: retain, auto_release: auto_release)
      else
        handle = create_binary(:CCS_EXPRESSION_TYPE_MODULO, left, right)
        super(handle, retain: false)
      end
    end

  end

  Expression::Modulo = ExpressionModulo

  class ExpressionIn < Expression

    def initialize(handle = nil, retain: false, auto_release: true,
                   left: nil, right: nil)
      if handle
        super(handle, retain: retain, auto_release: auto_release)
      else
        handle = create_binary(:CCS_EXPRESSION_TYPE_IN, left, right)
        super(handle, retain: false)
      end
    end

  end

  Expression::In = ExpressionIn

  class ExpressionPositive < Expression

    def initialize(handle = nil, retain: false, auto_release: true,
                   node: nil)
      if handle
        super(handle, retain: retain, auto_release: auto_release)
      else
        handle = create_unary(:CCS_EXPRESSION_TYPE_POSITIVE, node)
        super(handle, retain: false)
      end
    end

  end

  Expression::Positive = ExpressionPositive

  class ExpressionNegative < Expression

    def initialize(handle = nil, retain: false, auto_release: true,
                   node: nil)
      if handle
        super(handle, retain: retain, auto_release: auto_release)
      else
        handle = create_unary(:CCS_EXPRESSION_TYPE_NEGATIVE, node)
        super(handle, retain: false)
      end
    end

  end

  Expression::Negative = ExpressionNegative

  class ExpressionNot < Expression

    def initialize(handle = nil, retain: false, auto_release: true,
                   node: nil)
      if handle
        super(handle, retain: retain, auto_release: auto_release)
      else
        handle = create_unary(:CCS_EXPRESSION_TYPE_NOT, node)
        super(handle, retain: false)
      end
    end

  end

  Expression::Not = ExpressionNot

  attach_function :ccs_create_literal, [:ccs_datum_t, :pointer], :ccs_result_t
  attach_function :ccs_literal_get_value, [:ccs_expression_t, :pointer], :ccs_result_t
  class ExpressionLiteral < Expression
    add_property :value, :ccs_datum_t, :ccs_literal_get_value, memoize: true

    NONE_SYMBOL = TerminalSymbols[:CCS_TERMINAL_TYPE_NONE]
    TRUE_SYMBOL = TerminalSymbols[:CCS_TERMINAL_TYPE_TRUE]
    FALSE_SYMBOL = TerminalSymbols[:CCS_TERMINAL_TYPE_FALSE]

    def initialize(handle = nil, retain: false, auto_release: true,
                   value: nil)
      if handle
        super(handle, retain: retain, auto_release: auto_release)
      else
        ptr = MemoryPointer::new(:ccs_expression_t)
        CCS.error_check CCS.ccs_create_literal(Datum::from_value(value), ptr)
        super(ptr.read_ccs_expression_t, retain: false)
      end
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

  Expression::Literal = ExpressionLiteral

  attach_function :ccs_create_variable, [:ccs_parameter_t, :pointer], :ccs_result_t
  attach_function :ccs_variable_get_parameter, [:ccs_expression_t, :pointer], :ccs_result_t
  class ExpressionVariable < Expression
    add_handle_property :parameter, :ccs_parameter_t, :ccs_variable_get_parameter, memoize: true

    def initialize(handle = nil, retain: false, auto_release: true,
                   parameter: nil)
      if handle
        super(handle, retain: retain, auto_release: auto_release)
      else
        ptr = MemoryPointer::new(:ccs_expression_t)
        CCS.error_check CCS.ccs_create_variable(parameter, ptr)
        super(ptr.read_ccs_expression_t, retain: false)
      end
    end

    def to_s
      parameter.name
    end
  end

  Expression::Variable = ExpressionVariable

  attach_function :ccs_expression_list_eval_node, [:ccs_expression_t, :size_t, :pointer, :size_t, :pointer], :ccs_result_t
  class ExpressionList < Expression
    def initialize(handle = nil, retain: false, auto_release: true,
                   values: nil)
      if handle
        super(handle, retain: retain, auto_release: auto_release)
      else
        count = values.size
        p_values = MemoryPointer::new(:ccs_datum_t, count)
        ss = []
        ptr = MemoryPointer::new(:ccs_expression_t)
        values.each_with_index { |n, i| Datum::new(p_values[i]).set_value(n, string_store: ss) }
        CCS.error_check CCS.ccs_create_expression(:CCS_EXPRESSION_TYPE_LIST, count, p_values, ptr)
        super(ptr.read_ccs_expression_t, retain: false)
      end
    end

    def eval(index, bindings = nil)
      count = 0
      p_bindings = nil
      if bindings
        count = bindings.size
        p_bindings = MemoryPointer::new(:ccs_binding_t, count)
        p_bindings.write_array_of_pointer(bindings.collect(&:handle))
      end
      ptr = MemoryPointer::new(:ccs_datum_t)
      CCS.error_check CCS.ccs_expression_list_eval_node(@handle, count, p_bindings, index, ptr)
      Datum::new(ptr).value
    end

    def to_s
      "[ #{nodes.collect(&:to_s).join(", ")} ]"
    end
  end

  Expression::List = ExpressionList

  callback :ccs_user_defined_expression_del, [:ccs_expression_t], :ccs_result_t
  callback :ccs_user_defined_expression_eval, [:ccs_expression_t, :size_t, :pointer, :pointer], :ccs_result_t
  callback :ccs_user_defined_expression_serialize, [:ccs_expression_t, :size_t, :pointer, :pointer], :ccs_result_t
  callback :ccs_user_defined_expression_deserialize, [:size_t, :pointer, :pointer], :ccs_result_t

  class UserDefinedExpressionVector < FFI::Struct
    attr_accessor :wrappers
    attr_accessor :string_store
    attr_accessor :object_store
    layout :del, :ccs_user_defined_expression_del,
           :eval, :ccs_user_defined_expression_eval,
           :serialize, :ccs_user_defined_expression_serialize,
           :deserialize, :ccs_user_defined_expression_deserialize
  end
  typedef UserDefinedExpressionVector.by_value, :ccs_user_defined_expression_vector_t

  attach_function :ccs_create_user_defined_expression, [:string, :size_t, :pointer, UserDefinedExpressionVector.by_ref, :value, :pointer], :ccs_result_t
  attach_function :ccs_user_defined_expression_get_expression_data, [:ccs_expression_t, :pointer], :ccs_result_t
  attach_function :ccs_user_defined_expression_get_name, [:ccs_expression_t, :pointer], :ccs_result_t
  class ExpressionUserDefined < Expression
    add_property :expression_data, :value, :ccs_user_defined_expression_get_expression_data, memoize: true

    def initialize(handle = nil, retain: false, auto_release: true,
                   name: nil, nodes: [], del: nil, eval: nil, serialize: nil, deserialize: nil, expression_data: nil)
      if handle
        super(handle, retain: retain, auto_release: auto_release)
      else
        raise CCSError, :CCS_RESULT_ERROR_INVALID_VALUE if name.nil? || eval.nil?
        vector = ExpressionUserDefined.get_vector(del: del, eval: eval, serialize: serialize, deserialize: deserialize)
        count = nodes.size
        p_values = MemoryPointer::new(:ccs_datum_t, count)
        ss = []
        os = []
        ptr = MemoryPointer::new(:ccs_expression_t)
        nodes.each_with_index { |n, i| Datum::new(p_values[i]).set_value(n, string_store: ss, object_store: os) }
        CCS.error_check CCS.ccs_create_user_defined_expression(name, count, p_values, vector, expression_data, ptr)
        handle = ptr.read_ccs_expression_t
        super(handle, retain: false)
        FFI.inc_ref(vector)
        FFI.inc_ref(expression_data) unless expression_data.nil?
      end
    end

    def name
      @name ||= begin
        ptr = MemoryPointer::new(:pointer)
        CCS.error_check CCS.ccs_user_defined_expression_get_name(@handle, ptr)
        ptr.read_pointer.read_string
      end
    end

    def self.get_vector(del: nil, eval: nil, serialize: nil, deserialize: nil)
      vector = UserDefinedExpressionVector::new
      vector.string_store = []
      vector.object_store = []
      delwrapper = lambda { |expr|
        begin
          o = CCS::Object.from_handle(expr)
          edata = o.expression_data
          del.call(o) if del
          FFI.dec_ref(edata) unless edata.nil?
          FFI.dec_ref(vector)
          CCSError.to_native(:CCS_RESULT_SUCCESS)
        rescue => e
          CCS.set_error(e)
        end
      }
      evalwrapper = lambda { |expr, num_values, p_values, p_value_ret|
        begin
          values = Pointer.new(p_values).read_array_of_ccs_datum_t(num_values)
          value_ret = eval.call(Expression.from_handle(expr), *values)
          Datum::new(p_value_ret).set_value(value_ret, string_store: vector.string_store, object_store: vector.object_store)
          CCSError.to_native(:CCS_RESULT_SUCCESS)
        rescue => e
          CCS.set_error(e)
        end
      }
      serializewrapper =
        if serialize
          lambda { |tun, state_size, p_state, p_state_size|
            begin
              state = serialize.call(Expression.from_handle(tun))
              raise CCSError, :CCS_RESULT_ERROR_INVALID_VALUE if !state.kind_of?(String)
              raise CCSError, :CCS_RESULT_ERROR_INVALID_VALUE if !p_state.null? && state_size < state.bytesize
              p_state.write_bytes(state, 0, state.bytesize) unless p_state.null?
              Pointer.new(p_state_size).write_size_t(state.bytesize) unless p_state_size.null?
              CCSError.to_native(:CCS_RESULT_SUCCESS)
            rescue => e
              CCS.set_error(e)
            end
          }
        else
          nil
        end
      deserializewrapper =
        if deserialize
          lambda { |state_size, p_state, p_expression_data|
            begin
              state = p_state.null? ? nil : p_state.read_bytes(state_size)
              expression_data = deserialize.call(state)
              p_expression_data.write_value(expression_data)
              FFI.inc_ref(expression_data)
              CCSError.to_native(:CCS_RESULT_SUCCESS)
            rescue => e
              CCS.set_error(e)
            end
          }
        else
          nil
        end
      vector[:del] = delwrapper
      vector[:eval] = evalwrapper
      vector[:serialize] = serializewrapper
      vector[:deserialize] = deserializewrapper
      vector.wrappers = [delwrapper, evalwrapper, serializewrapper, deserializewrapper]
      vector
    end

    def to_s
      "#{name}(#{nodes.collect(&:to_s).join(", ")})"
    end

  end

  Expression::UserDefined = ExpressionUserDefined

end
