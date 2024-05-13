module CCS
  attach_function :ccs_binding_get_context, [:ccs_binding_t, :pointer], :ccs_result_t
  attach_function :ccs_binding_get_value, [:ccs_binding_t, :size_t, :pointer], :ccs_result_t
  attach_function :ccs_binding_get_values, [:ccs_binding_t, :size_t, :pointer, :pointer], :ccs_result_t
  attach_function :ccs_binding_get_value_by_name, [:ccs_binding_t, :string, :pointer, :pointer], :ccs_result_t
  attach_function :ccs_binding_get_value_by_parameter, [:ccs_binding_t, :ccs_parameter_t, :pointer, :pointer], :ccs_result_t
  attach_function :ccs_binding_hash, [:ccs_binding_t, :pointer], :ccs_result_t
  attach_function :ccs_binding_cmp, [:ccs_binding_t, :ccs_binding_t, :pointer], :ccs_result_t

  class Binding < Object
    include Comparable
    add_property :hash, :ccs_hash_t, :ccs_binding_hash, memoize: true
    add_handle_property :context, :ccs_context_t, :ccs_binding_get_context, memoize: true

    def value(parameter)
      ptr = MemoryPointer::new(:ccs_datum_t)
      case parameter
      when String
        CCS.error_check CCS.ccs_binding_get_value_by_name(@handle, parameter, nil, ptr)
      when Symbol
        name = parameter.inspect
        CCS.error_check CCS.ccs_binding_get_value_by_name(@handle, name, nil, ptr)
      when Parameter
        CCS.error_check CCS.ccs_binding_get_value_by_parameter(@handle, parameter.handle, nil, ptr)
      when Integer
        CCS.error_check CCS.ccs_binding_get_value(@handle, parameter, ptr)
      else
        raise CCSError, :CCS_RESULT_ERROR_INVALID_VALUE
      end
      Datum::new(ptr).value
    end

    def values
      @values ||= begin
        count = num_values
        values = MemoryPointer::new(:ccs_datum_t, count)
        CCS.error_check CCS.ccs_binding_get_values(@handle, count, values, nil)
        count.times.collect { |i| Datum::new(values[i]).value }.freeze
      end
    end

    def num_values
      @num_values ||= begin
        ptr = MemoryPointer::new(:size_t)
        CCS.error_check CCS.ccs_binding_get_values(@handle, 0, nil, ptr)
        ptr.read_size_t
      end
    end

    def <=>(other)
      ptr = MemoryPointer::new(:int)
      CCS.error_check CCS.ccs_binding_cmp(@handle, other, ptr)
      return ptr.read_int
    end

    def to_h
      context.parameters.collect(&:name).zip(values).to_h
    end

  end

end
