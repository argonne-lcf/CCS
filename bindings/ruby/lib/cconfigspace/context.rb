module CCS
  attach_function :ccs_context_get_name, [:ccs_context_t, :pointer], :ccs_result_t
  attach_function :ccs_context_get_parameter, [:ccs_context_t, :size_t, :pointer], :ccs_result_t
  attach_function :ccs_context_get_parameter_by_name, [:ccs_context_t, :string, :pointer], :ccs_result_t
  attach_function :ccs_context_get_parameter_index_by_name, [:ccs_context_t, :string, :pointer, :pointer], :ccs_result_t
  attach_function :ccs_context_get_parameter_index, [:ccs_context_t, :ccs_parameter_t, :pointer, :pointer], :ccs_result_t
  attach_function :ccs_context_get_parameter_indexes, [:ccs_context_t, :size_t, :pointer, :pointer, :pointer], :ccs_result_t
  attach_function :ccs_context_get_parameters, [:ccs_context_t, :size_t, :pointer, :pointer], :ccs_result_t
  attach_function :ccs_context_validate_value, [:ccs_context_t, :size_t, :ccs_datum_t, :pointer], :ccs_result_t

  class Context < Object
    add_handle_array_property :parameters, :ccs_parameter_t, :ccs_context_get_parameters, memoize: true

    def name
      @name ||= begin
        ptr = MemoryPointer::new(:pointer)
        CCS.error_check CCS.ccs_context_get_name(@handle, ptr)
        ptr.read_pointer.read_string
      end
    end

    def parameter(index)
      ptr = MemoryPointer::new(:ccs_parameter_t)
      CCS.error_check CCS.ccs_context_get_parameter(@handle, index, ptr)
      Parameter.from_handle(ptr.read_ccs_parameter_t)
    end

    def parameter_by_name(name)
      name = name.inspect if name.kind_of?(Symbol)
      ptr = MemoryPointer::new(:ccs_parameter_t)
      CCS.error_check CCS.ccs_context_get_parameter_by_name(@handle, name, ptr)
      param = ptr.read_ccs_parameter_t
      param.null? ? nil : Parameter.from_handle(param)
    end

    def parameter_index_by_name(name)
      name = name.inspect if name.kind_of?(Symbol)
      ptr = MemoryPointer::new(:size_t)
      CCS.error_check CCS.ccs_context_get_parameter_index_by_name(@handle, name, nil, ptr)
      ptr.read_size_t
    end

    def parameter_index(parameter)
      ptr = MemoryPointer::new(:size_t)
      CCS.error_check CCS.ccs_context_get_parameter_index(@handle, parameter, nil, ptr)
      ptr.read_size_t
    end

    def validate_value(parameter, value)
      ptr = MemoryPointer::new(:ccs_datum_t)
      d = Datum.from_value(value)
      case parameter
      when String, Symbol
        parameter = parameter_index_by_name(parameter)
      when Parameter
        parameter = parameter_index(parameter)
      end
      CCS.error_check CCS.ccs_context_validate_value(@handle, parameter, d, ptr)
      Datum::new(ptr).value
    end

  end
end
