module CCS
  attach_function :ccs_context_get_name, [:ccs_context_t, :pointer], :ccs_error_t
  attach_function :ccs_context_get_num_hyperparameters, [:ccs_context_t, :pointer], :ccs_error_t
  attach_function :ccs_context_get_hyperparameter, [:ccs_context_t, :size_t, :pointer], :ccs_error_t
  attach_function :ccs_context_get_hyperparameter_by_name, [:ccs_context_t, :string, :pointer], :ccs_error_t
  attach_function :ccs_context_get_hyperparameter_index_by_name, [:ccs_context_t, :string, :pointer], :ccs_error_t
  attach_function :ccs_context_get_hyperparameter_index, [:ccs_context_t, :ccs_hyperparameter_t, :pointer], :ccs_error_t
  attach_function :ccs_context_get_hyperparameter_indexes, [:ccs_context_t, :size_t, :pointer, :pointer], :ccs_error_t
  attach_function :ccs_context_get_hyperparameters, [:ccs_context_t, :size_t, :pointer, :pointer], :ccs_error_t
  attach_function :ccs_context_validate_value, [:ccs_context_t, :size_t, :ccs_datum_t, :pointer], :ccs_error_t

  class Context < Object
    add_property :num_hyperparameters, :size_t, :ccs_context_get_num_hyperparameters, memoize: false

    def name
      @name ||= begin
        ptr = MemoryPointer::new(:pointer)
        CCS.error_check CCS.ccs_context_get_name(@handle, ptr)
        ptr.read_pointer.read_string
      end
    end

    def hyperparameter(index)
      ptr = MemoryPointer::new(:ccs_hyperparameter_t)
      CCS.error_check CCS.ccs_context_get_hyperparameter(@handle, index, ptr)
      Hyperparameter.from_handle(ptr.read_ccs_hyperparameter_t)
    end

    def hyperparameter_by_name(name)
      name = name.inspect if name.kind_of?(Symbol)
      ptr = MemoryPointer::new(:ccs_hyperparameter_t)
      CCS.error_check CCS.ccs_context_get_hyperparameter_by_name(@handle, name, ptr)
      Hyperparameter.from_handle(ptr.read_ccs_hyperparameter_t)
    end

    def hyperparameter_index_by_name(name)
      name = name.inspect if name.kind_of?(Symbol)
      ptr = MemoryPointer::new(:size_t)
      CCS.error_check CCS.ccs_context_get_hyperparameter_index_by_name(@handle, name, ptr)
      ptr.read_size_t
    end

    def hyperparameter_index(hyperparameter)
      ptr = MemoryPointer::new(:size_t)
      CCS.error_check CCS.ccs_context_get_hyperparameter_index(@handle, hyperparameter, ptr)
      ptr.read_size_t
    end

    def hyperparameters
      count = num_hyperparameters
      return [] if count == 0
      ptr = MemoryPointer::new(:ccs_hyperparameter_t, count)
      CCS.error_check CCS.ccs_context_get_hyperparameters(@handle, count, ptr, nil)
      count.times.collect { |i| Hyperparameter.from_handle(ptr[i].read_pointer) }
    end

    def validate_value(hyperparameter, value)
      ptr = MemoryPointer::new(:ccs_datum_t)
      d = Datum.from_value(value)
      case hyperparameter
      when String, Symbol
        hyperparameter = hyperparameter_index_by_name(hyperparameter)
      when Hyperparameter
        hyperparameter = hyperparameter_index(hyperparameter)
      end
      CCS.error_check CCS.ccs_context_validate_value(@handle, hyperparameter, d, ptr)
      Datum::new(ptr).value
    end

  end
end
