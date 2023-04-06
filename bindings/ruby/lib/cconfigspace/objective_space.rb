module CCS
  ObjectiveType = enum FFI::Type::INT32, :ccs_objective_type_t, [
    :CCS_MINIMIZE,
    :CCS_MAXIMIZE
  ]
  class MemoryPointer
    def read_ccs_objective_type_t
      ObjectiveType.from_native(read_int32, nil)
    end
    def write_ccs_objective_type_t(v)
      write_int32 ObjectiveType.to_native(v, nil)
    end
    def write_array_of_ccs_objective_type_t(ary)
      write_array_of_int32( ary.collect { |v| ObjectiveType.to_native(v, nil) } )
    end
    def read_array_of_ccs_objective_type_t(count)
      read_array_of_int32(count).collect { |v| ObjectiveType.from_native(v, nil) }
    end
  end

  attach_function :ccs_create_objective_space, [:string, :pointer], :ccs_error_t
  attach_function :ccs_objective_space_add_parameter, [:ccs_objective_space_t, :ccs_parameter_t], :ccs_error_t
  attach_function :ccs_objective_space_add_parameters, [:ccs_objective_space_t, :size_t, :pointer], :ccs_error_t
  attach_function :ccs_objective_space_add_objective, [:ccs_objective_space_t, :ccs_expression_t, :ccs_objective_type_t], :ccs_error_t
  attach_function :ccs_objective_space_add_objectives, [:ccs_objective_space_t, :size_t, :pointer, :pointer], :ccs_error_t
  attach_function :ccs_objective_space_get_objective, [:ccs_objective_space_t, :size_t, :pointer, :pointer], :ccs_error_t
  attach_function :ccs_objective_space_get_objectives, [:ccs_objective_space_t, :size_t, :pointer, :pointer, :pointer], :ccs_error_t
  attach_function :ccs_objective_space_check_evaluation_values, [:ccs_objective_space_t, :size_t, :pointer, :pointer], :ccs_error_t

  class ObjectiveSpace < Context

    def initialize(handle = nil, retain: false, auto_release: true,
                   name: "")
      if handle
        super(handle, retain: retain, auto_release: auto_release)
      else
        ptr = MemoryPointer::new(:ccs_objective_space_t)
        CCS.error_check CCS.ccs_create_objective_space(name, ptr)
        super(ptr.read_ccs_objective_space_t, retain: false)
      end
    end

    def self.from_handle(handle, retain: true, auto_release: true)
      self::new(handle, retain: retain, auto_release: auto_release)
    end

    def add_parameter(parameter)
      CCS.error_check CCS.ccs_objective_space_add_parameter(@handle, parameter)
      self
    end

    def add_parameters(parameters)
      count = parameters.size
      return self if count == 0
      p_parameters = MemoryPointer::new(:ccs_parameter_t, count)
      p_parameters.write_array_of_pointer(parameters.collect(&:handle))
      CCS.error_check CCS.ccs_objective_space_add_parameters(@handle, count, p_parameters)
      self
    end

    def add_objective(expression, type: :CCS_MINIMIZE)
      if expression.kind_of? String
        expression = ExpressionParser::new(self).parse(expression)
      end
      CCS.error_check CCS.ccs_objective_space_add_objective(@handle, expression, type)
      self
    end

    def add_objectives(expressions, types: nil)
      if expressions.kind_of? Hash
        types = expressions.values
        expressions = expressions.keys
      end
      expressions = expressions.collect { |e|
        p = ExpressionParser::new(self)
        if e.kind_of? String
          e = p.parse(e)
        else
          e
        end
      }
      count = expressions.length
      return self if count == 0
      if types
        raise CCSError, :CCS_INVALID_VALUE if types.size != count
      else
        types = [:CCS_MINIMIZE] * count
      end
      p_types = MemoryPointer::new(:ccs_objective_type_t, count)
      p_types.write_array_of_ccs_objective_type_t(types)
      p_exprs = MemoryPointer::new(:ccs_expression_t, count)
      p_exprs.write_array_of_pointer(expressions.collect(&:handle))
      CCS.error_check CCS.ccs_objective_space_add_objectives(@handle, count, p_exprs, p_types)
      self
    end

    def get_objective(index)
      p_type = MemoryPointer::new(:ccs_objective_type_t)
      p_expr = MemoryPointer::new(:ccs_expression_t)
      CCS.error_check CCS.ccs_objective_space_get_objective(@handle, index, p_expr, p_type)
      return [Expression::from_handle(p_expr.read_ccs_expression_t), p_type.read_ccs_objective_type_t]
    end

    def num_objectives
      ptr = MemoryPointer::new(:size_t)
      CCS.error_check CCS.ccs_objective_space_get_objectives(@handle, 0, nil, nil, ptr)
      ptr.read_size_t
    end

    def objectives
      count  = num_objectives
      return [] if count == 0
      p_exprs = MemoryPointer::new(:ccs_expression_t, count)
      p_types = MemoryPointer::new(:ccs_objective_type_t, count)
      CCS.error_check CCS.ccs_objective_space_get_objectives(@handle, count, p_exprs, p_types, nil)
      exprs = p_exprs.read_array_of_pointer(count).collect { |p| Expression.from_handle(p) }
      types = p_types.read_array_of_ccs_objective_type_t(count)
      exprs.zip types
    end

    def check_values(values)
      count = values.size
      raise CCSError, :CCS_INVALID_VALUE if count != num_parameters
      ss = []
      ptr = MemoryPointer::new(:ccs_datum_t, count)
      values.each_with_index {  |v, i| Datum::new(ptr[i]).set_value(v, string_store: ss) }
      ptr2 = MemoryPointer::new(:ccs_bool_t)
      CCS.error_check CCS.ccs_objective_space_check_evaluation_values(@handle, count, ptr, ptr2)
      return ptr2.read_ccs_bool_t == CCS::FALSE ? false : true
    end
  end
end
