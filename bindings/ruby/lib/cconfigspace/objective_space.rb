module CCS
  ObjectiveType = enum FFI::Type::INT32, :ccs_objective_type_t, [
    :CCS_OBJECTIVE_TYPE_MINIMIZE,
    :CCS_OBJECTIVE_TYPE_MAXIMIZE
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

  attach_function :ccs_create_objective_space, [:string, :size_t, :pointer, :size_t, :pointer, :pointer, :size_t, :pointer, :pointer], :ccs_result_t
  attach_function :ccs_objective_space_get_objective, [:ccs_objective_space_t, :size_t, :pointer, :pointer], :ccs_result_t
  attach_function :ccs_objective_space_get_objectives, [:ccs_objective_space_t, :size_t, :pointer, :pointer, :pointer], :ccs_result_t
  attach_function :ccs_objective_space_check_evaluation_values, [:ccs_objective_space_t, :size_t, :pointer, :pointer], :ccs_result_t
  attach_function :ccs_objective_space_get_contexts, [:ccs_objective_space_t, :size_t, :pointer, :pointer], :ccs_result_t

  class ObjectiveSpace < Context

    def initialize(handle = nil, retain: false, auto_release: true,
                   name: "", parameters: [], objectives: [], types: nil, contexts: nil)
      if handle
        super(handle, retain: retain, auto_release: auto_release)
      else
        count = parameters.size
        ctx_params = parameters
        if contexts
          ctx_params = contexts.reduce(ctx_params, :+)
        end
        ctx = ctx_params.map { |p| [p.name, p] }.to_h
        if objectives.kind_of? Hash
          types = objectives.values
          objectives = objectives.keys
        end
        p = ExpressionParser::new(ctx)
        objectives = objectives.collect { |e|
          if e.kind_of? String
            e = p.parse(e)
          else
            e
          end
        }
        ocount = objectives.length
        if types
          raise CCSError, :CCS_RESULT_ERROR_INVALID_VALUE if types.size != ocount
        else
          types = [:CCS_OBJECTIVE_TYPE_MINIMIZE] * ocount
        end
        ccount = 0
        p_contexts = nil
        if contexts
          ccount = contexts.size
          p_contexts = MemoryPointer::new(:ccs_context_t, ccount)
          p_contexts.write_array_of_context_t(contexts)
        end
        p_types = MemoryPointer::new(:ccs_objective_type_t, ocount)
        p_types.write_array_of_ccs_objective_type_t(types)
        p_objs = MemoryPointer::new(:ccs_expression_t, ocount)
        p_objs.write_array_of_pointer(objectives.collect(&:handle))
        p_parameters = MemoryPointer::new(:ccs_parameter_t, count)
        p_parameters.write_array_of_pointer(parameters.collect(&:handle))
        ptr = MemoryPointer::new(:ccs_objective_space_t)
        CCS.error_check CCS.ccs_create_objective_space(name, count, p_parameters, ocount, p_objs, p_types, ccount, p_contexts, ptr)
        super(ptr.read_ccs_objective_space_t, retain: false)
      end
    end

    def self.from_handle(handle, retain: true, auto_release: true)
      self::new(handle, retain: retain, auto_release: auto_release)
    end

    def get_objective(index)
      p_type = MemoryPointer::new(:ccs_objective_type_t)
      p_expr = MemoryPointer::new(:ccs_expression_t)
      CCS.error_check CCS.ccs_objective_space_get_objective(@handle, index, p_expr, p_type)
      return [Expression::from_handle(p_expr.read_ccs_expression_t), p_type.read_ccs_objective_type_t]
    end

    def num_objectives
      @num_objectives ||= begin
        ptr = MemoryPointer::new(:size_t)
        CCS.error_check CCS.ccs_objective_space_get_objectives(@handle, 0, nil, nil, ptr)
        ptr.read_size_t
      end
    end

    def objectives
      @objectives ||= begin
        count   = num_objectives
        p_exprs = MemoryPointer::new(:ccs_expression_t, count)
        p_types = MemoryPointer::new(:ccs_objective_type_t, count)
        CCS.error_check CCS.ccs_objective_space_get_objectives(@handle, count, p_exprs, p_types, nil)
        exprs = p_exprs.read_array_of_pointer(count).collect { |p| Expression.from_handle(p) }
        types = p_types.read_array_of_ccs_objective_type_t(count)
        exprs.zip(types).freeze
      end
    end

    def num_contexts
      @num_contexts ||= begin
        ptr = MemoryPointer::new(:size_t)
        CCS.error_check CCS.ccs_objective_space_get_contexts(@handle, 0, nil, ptr)
        ptr.read_size_t
      end
    end

    def contexts
      @contexts ||= begin
        count  = num_contexts
        p_ctxs = MemoryPointer::new(:ccs_context_t, count)
        CCS.error_check CCS.ccs_objective_space_get_contexts(@handle, count, p_ctxs, nil)
        p_ctxs.read_array_of_pointer(count).collect { |p| Context.from_handle(p) }.freeze
      end
    end

    def check_values(values)
      count = values.size
      raise CCSError, :CCS_RESULT_ERROR_INVALID_VALUE if count != num_parameters
      ss = []
      ptr = MemoryPointer::new(:ccs_datum_t, count)
      values.each_with_index {  |v, i| Datum::new(ptr[i]).set_value(v, string_store: ss) }
      ptr2 = MemoryPointer::new(:ccs_bool_t)
      CCS.error_check CCS.ccs_objective_space_check_evaluation_values(@handle, count, ptr, ptr2)
      return ptr2.read_ccs_bool_t == CCS::FALSE ? false : true
    end
  end
end
