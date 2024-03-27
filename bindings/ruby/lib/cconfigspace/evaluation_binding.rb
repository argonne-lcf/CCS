module CCS
  Comparison = enum FFI::Type::INT32, :ccs_comparison_t, [
    :CCS_COMPARISON_BETTER, -1,
    :CCS_COMPARISON_EQUIVALENT, 0,
    :CCS_COMPARISON_WORSE, 1,
    :CCS_COMPARISON_NOT_COMPARABLE, 2
  ]
  class MemoryPointer
    def read_ccs_comparison_t
      Comparison.from_native(read_int32, nil)
    end
  end

  attach_function :ccs_evaluation_binding_get_result, [:ccs_evaluation_binding_t, :pointer], :ccs_result_t
  attach_function :ccs_evaluation_binding_get_objective_values, [:ccs_evaluation_binding_t, :size_t, :pointer, :pointer], :ccs_result_t
  attach_function :ccs_evaluation_binding_compare, [:ccs_evaluation_binding_t, :ccs_evaluation_binding_t, :pointer], :ccs_result_t
  attach_function :ccs_evaluation_binding_check, [:ccs_evaluation_binding_t, :pointer], :ccs_result_t

  class EvaluationBinding < Binding
    alias objective_space context
    add_property :result, :ccs_evaluation_binding_result_t, :ccs_evaluation_binding_get_result, memoize: true

    def num_objective_values
      @num_objective_values ||= begin
        ptr = MemoryPointer::new(:size_t)
        CCS.error_check CCS.ccs_evaluation_binding_get_objective_values(@handle, 0, nil, ptr)
        ptr.read_size_t
      end
    end

    def objective_values
      @objective_values ||= begin
        count = num_objective_values
        values = MemoryPointer::new(:ccs_datum_t, count)
        CCS.error_check CCS.ccs_evaluation_binding_get_objective_values(@handle, count, values, nil)
        count.times.collect { |i| Datum::new(values[i]).value }.freeze
      end
    end

    def check
      ptr = MemoryPointer::new(:ccs_bool_t)
      CCS.error_check CCS.ccs_evaluation_binding_check(@handle, ptr)
      return ptr.read_ccs_bool_t == CCS::FALSE ? false : true
    end

    def compare(other)
      ptr = MemoryPointer::new(:ccs_comparison_t)
      CCS.error_check CCS.ccs_evaluation_binding_compare(@handle, other, ptr)
      ptr.read_ccs_comparison_t
    end

    def <=>(other)
      ptr = MemoryPointer::new(:ccs_comparison_t)
      CCS.error_check CCS.ccs_evaluation_binding_compare(@handle, other, ptr)
      r = ptr.read_int32
      r == 2 ? nil : r 
    end

  end
end
