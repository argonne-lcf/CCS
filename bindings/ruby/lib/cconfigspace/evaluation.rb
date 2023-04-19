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

  attach_function :ccs_create_evaluation, [:ccs_objective_space_t, :ccs_configuration_t, :ccs_evaluation_result_t, :size_t, :pointer, :pointer], :ccs_error_t
  attach_function :ccs_evaluation_get_configuration, [:ccs_evaluation_t, :pointer], :ccs_error_t
  attach_function :ccs_evaluation_get_result, [:ccs_evaluation_t, :pointer], :ccs_error_t
  attach_function :ccs_evaluation_set_result, [:ccs_evaluation_t, :ccs_evaluation_result_t], :ccs_error_t
  attach_function :ccs_evaluation_get_objective_values, [:ccs_evaluation_t, :size_t, :pointer, :pointer], :ccs_error_t
  attach_function :ccs_evaluation_compare, [:ccs_evaluation_t, :ccs_evaluation_t, :pointer], :ccs_error_t
  attach_function :ccs_evaluation_check, [:ccs_evaluation_t, :pointer], :ccs_error_t

  class Evaluation < Binding
    alias objective_space context
    add_handle_property :configuration, :ccs_configuration_t, :ccs_evaluation_get_configuration, memoize: true
    add_property :result, :ccs_evaluation_result_t, :ccs_evaluation_get_result, memoize: false

    def initialize(handle = nil, retain: false, auto_release: true,
                   objective_space: nil, configuration: nil, result: :CCS_SUCCESS, values: nil)
      if handle
        super(handle, retain: retain, auto_release: auto_release)
      else
        if values
          count = values.size
          raise CCSError, :CCS_INVALID_VALUE if count == 0
          ss = []
          p_values = MemoryPointer::new(:ccs_datum_t, count)
          values.each_with_index {  |v, i| Datum::new(p_values[i]).set_value(v, string_store: ss) }
          values = p_values
        else
          count = 0
        end
        ptr = MemoryPointer::new(:ccs_evaluation_t)
        CCS.error_check CCS.ccs_create_evaluation(objective_space, configuration, result, count, values, ptr)
        super(ptr.read_ccs_evaluation_t, retain: false)
        @objective_space = objective_space
        @configuration = configuration
      end
    end

    def self.from_handle(handle, retain: true, auto_release: true)
      self::new(handle, retain: retain, auto_release: auto_release)
    end

    def result=(res)
      CCS.error_check CCS.ccs_evaluation_set_result(@handle, res)
      res
    end

    def num_objective_values
      @num_objective_values ||= begin
        ptr = MemoryPointer::new(:size_t)
        CCS.error_check CCS.ccs_evaluation_get_objective_values(@handle, 0, nil, ptr)
        ptr.read_size_t
      end
    end

    def objective_values
      count = num_objective_values
      return [] if count == 0
      values = MemoryPointer::new(:ccs_datum_t, count)
      CCS.error_check CCS.ccs_evaluation_get_objective_values(@handle, count, values, nil)
      count.times.collect { |i| Datum::new(values[i]).value }
    end

    def check
      ptr = MemoryPointer::new(:ccs_bool_t)
      CCS.error_check CCS.ccs_evaluation_check(@handle, ptr)
      return ptr.read_ccs_bool_t == CCS::FALSE ? false : true
    end

    def compare(other)
      ptr = MemoryPointer::new(:ccs_comparison_t)
      CCS.error_check CCS.ccs_evaluation_compare(@handle, other, ptr)
      ptr.read_ccs_comparison_t
    end

    def <=>(other)
      ptr = MemoryPointer::new(:ccs_comparison_t)
      CCS.error_check CCS.ccs_evaluation_compare(@handle, other, ptr)
      r = ptr.read_int32
      r == 2 ? nil : r 
    end

  end
end
