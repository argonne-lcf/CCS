module CCS
  attach_function :ccs_context_get_name, [:ccs_context_t, :pointer], :ccs_result_t
  attach_function :ccs_context_get_user_data, [:ccs_context_t, :pointer], :ccs_result_t
  attach_function :ccs_context_get_num_hyperparameters, [:ccs_context_t, :pointer], :ccs_result_t
  attach_function :ccs_context_get_hyperparameter, [:ccs_context_t, :size_t, :pointer], :ccs_result_t
  attach_function :ccs_context_get_hyperparameter_by_name, [:ccs_context_t, :string, :pointer], :ccs_result_t
  attach_function :ccs_context_get_hyperparameter_index_by_name, [:ccs_context_t, :string, :pointer], :ccs_result_t
  attach_function :ccs_context_get_hyperparameter_index, [:ccs_context_t, :ccs_hyperparameter_t, :pointer], :ccs_result_t
  attach_function :ccs_context_get_hyperparameter_indexes, [:ccs_context_t, :size_t, :pointer, :pointer], :ccs_result_t
  attach_function :ccs_context_get_hyperparameters, [:ccs_context_t, :size_t, :pointer, :pointer], :ccs_result_t
  class Context < Object
    add_property :user_data, :pointer, :ccs_context_get_user_data, memoize: true
    add_property :num_hyperparameters, :size_t, :ccs_context_get_num_hyperparameters, memoize: false

    def name
      @name ||= begin
        ptr = MemoryPointer::new(:pointer)
        res = CCS.ccs_context_get_name(@handle, ptr)
        CCS.error_check(res)
        ptr.read_pointer.read_string
      end
    end

    def hyperparameter(index)
      ptr = MemoryPointer::new(:ccs_hyperparameter_t)
      res = CCS.ccs_context_get_hyperparameter(@handle, index, ptr)
      CCS.error_check(res)
      Hyperparameter.from_handle(ptr.read_ccs_hyperparameter_t)
    end

    def hyperparameter_by_name(name)
      name = name.inspect if name.kind_of?(Symbol)
      ptr = MemoryPointer::new(:ccs_hyperparameter_t)
      res = CCS.ccs_context_get_hyperparameter_by_name(@handle, name, ptr)
      CCS.error_check(res)
      Hyperparameter.from_handle(ptr.read_ccs_hyperparameter_t)
    end

    def hyperparameter_index_by_name(name)
      name = name.inspect if name.kind_of?(Symbol)
      ptr = MemoryPointer::new(:size_t)
      res = CCS.ccs_context_get_hyperparameter_index_by_name(@handle, name, ptr)
      CCS.error_check(res)
      ptr.read_size_t
    end

    def hyperparameter_index(hyperparameter)
      ptr = MemoryPointer::new(:size_t)
      res = CCS.ccs_context_get_hyperparameter_index(@handle, hyperparameter, ptr)
      CCS.error_check(res)
      ptr.read_size_t
    end

    def hyperparameters
      count = num_hyperparameters
      return [] if count == 0
      ptr = MemoryPointer::new(:ccs_hyperparameter_t, count)
      res = CCS.ccs_context_get_hyperparameters(@handle, count, ptr, nil)
      CCS.error_check(res)
      count.times.collect { |i| Hyperparameter.from_handle(ptr[i].read_pointer) }
    end

  end
end
