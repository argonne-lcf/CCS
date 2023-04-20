module CCS

  attach_function :ccs_create_tree_evaluation, [:ccs_objective_space_t, :ccs_tree_configuration_t, :ccs_evaluation_result_t, :size_t, :pointer, :pointer], :ccs_result_t
  attach_function :ccs_tree_evaluation_get_configuration, [:ccs_tree_evaluation_t, :pointer], :ccs_result_t
  attach_function :ccs_tree_evaluation_get_result, [:ccs_tree_evaluation_t, :pointer], :ccs_result_t
  attach_function :ccs_tree_evaluation_set_result, [:ccs_tree_evaluation_t, :ccs_evaluation_result_t], :ccs_result_t
  attach_function :ccs_tree_evaluation_get_objective_values, [:ccs_tree_evaluation_t, :size_t, :pointer, :pointer], :ccs_result_t
  attach_function :ccs_tree_evaluation_compare, [:ccs_tree_evaluation_t, :ccs_tree_evaluation_t, :pointer], :ccs_result_t
  attach_function :ccs_tree_evaluation_check, [:ccs_tree_evaluation_t, :pointer], :ccs_result_t

  class TreeEvaluation < Binding
    alias objective_space context
    add_handle_property :configuration, :ccs_tree_configuration_t, :ccs_tree_evaluation_get_configuration, memoize: true
    add_property :result, :ccs_evaluation_result_t, :ccs_tree_evaluation_get_result, memoize: false

    def initialize(handle = nil, retain: false, auto_release: true,
                   objective_space: nil, configuration: nil, result: :CCS_RESULT_SUCCESS, values: nil)
      if handle
         super(handle, retain: retain, auto_release: auto_release)
      else
        if values
          count = values.size
          raise CCSError, :CCS_RESULT_ERROR_INVALID_VALUE if count == 0
          ss = []
          p_values = MemoryPointer::new(:ccs_datum_t, count)
          values.each_with_index {  |v, i| Datum::new(p_values[i]).set_value(v, string_store: ss) }
          values = p_values
        else
          count = 0
        end
        ptr = MemoryPointer::new(:ccs_tree_evaluation_t)
        CCS.error_check CCS.ccs_create_tree_evaluation(objective_space, configuration, result, count, values, ptr)
        super(ptr.read_ccs_tree_evaluation_t, retain: false)
        @objective_space = objective_space
        @configuration = configuration
      end
    end

    def self.from_handle(handle, retain: true, auto_release: true)
      self::new(handle, retain: retain, auto_release: auto_release)
    end

    def result=(res)
      CCS.error_check CCS.ccs_tree_evaluation_set_result(@handle, res)
      res
    end

    def num_objective_values
      @num_objective_values ||= begin
        ptr = MemoryPointer::new(:size_t)
        CCS.error_check CCS.ccs_tree_evaluation_get_objective_values(@handle, 0, nil, ptr)
        ptr.read_size_t
      end
    end

    def objective_values
      count = num_objective_values
      return [] if count == 0
      values = MemoryPointer::new(:ccs_datum_t, count)
      CCS.error_check CCS.ccs_tree_evaluation_get_objective_values(@handle, count, values, nil)
      count.times.collect { |i| Datum::new(values[i]).value }
    end

    def check
      ptr = MemoryPointer::new(:ccs_bool_t)
      CCS.error_check CCS.ccs_tree_evaluation_check(@handle, ptr)
      return ptr.read_ccs_bool_t == CCS::FALSE ? false : true
    end

    def compare(other)
      ptr = MemoryPointer::new(:ccs_comparison_t)
      CCS.error_check CCS.ccs_tree_evaluation_compare(@handle, other, ptr)
      ptr.read_ccs_comparison_t
    end

    def <=>(other)
      ptr = MemoryPointer::new(:ccs_comparison_t)
      CCS.error_check CCS.ccs_tree_evaluation_compare(@handle, other, ptr)
      r = ptr.read_int32
      r == 2 ? nil : r 
    end

  end

end
