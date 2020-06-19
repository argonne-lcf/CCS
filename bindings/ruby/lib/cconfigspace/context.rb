module CCS
  attach_function :ccs_context_get_hyperparameter_index, [:ccs_context_t, :ccs_hyperparameter_t, :pointer], :ccs_result_t
  class Context < Object
    def hyperparameter_index(hyperparameter)
      ptr = MemoryPointer::new(:size_t)
      res = CCS.ccs_context_get_hyperparameter_index(@handle, hyperparameter, ptr)
      CCS.error_check(res)
      ptr.read_size_t
    end
  end
end
