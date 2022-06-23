module CCS
  attach_function :ccs_create_features_evaluation, [:ccs_objective_space_t, :ccs_configuration_t, :ccs_features_t, :ccs_result_t, :size_t, :pointer, :pointer], :ccs_error_t
  attach_function :ccs_features_evaluation_get_configuration, [:ccs_features_evaluation_t, :pointer], :ccs_error_t
  attach_function :ccs_features_evaluation_get_features, [:ccs_features_evaluation_t, :pointer], :ccs_error_t
  attach_function :ccs_features_evaluation_get_error, [:ccs_features_evaluation_t, :pointer], :ccs_error_t
  attach_function :ccs_features_evaluation_set_error, [:ccs_features_evaluation_t, :ccs_result_t], :ccs_error_t
  attach_function :ccs_features_evaluation_get_objective_value, [:ccs_features_evaluation_t, :size_t, :pointer], :ccs_error_t
  attach_function :ccs_features_evaluation_get_objective_values, [:ccs_features_evaluation_t, :size_t, :pointer, :pointer], :ccs_error_t
  attach_function :ccs_features_evaluation_compare, [:ccs_features_evaluation_t, :ccs_features_evaluation_t, :pointer], :ccs_error_t
  attach_function :ccs_features_evaluation_check, [:ccs_features_evaluation_t, :pointer], :ccs_error_t
  class FeaturesEvaluation < Binding
    alias objective_space context
    add_handle_property :configuration, :ccs_configuration_t, :ccs_features_evaluation_get_configuration, memoize: true
    add_handle_property :features, :ccs_features_t, :ccs_features_evaluation_get_features, memoize: true
    add_property :error, :ccs_result_t, :ccs_features_evaluation_get_error, memoize: false

    def initialize(handle = nil, retain: false, auto_release: true,
                   objective_space: nil, configuration: nil, features: nil, error: :CCS_SUCCESS, values: nil)
      if handle
        super(handle, retain: retain, auto_release: auto_release)
      else
        if values
          count = values.size
          raise CCSError, :CCS_INVALID_VALUE if count == 0
          p_values = MemoryPointer::new(:ccs_datum_t, count)
          values.each_with_index {  |v, i| Datum::new(p_values[i]).value = v }
          values = p_values
        else
          count = 0
        end
        ptr = MemoryPointer::new(:ccs_features_evaluation_t)
        CCS.error_check CCS.ccs_create_features_evaluation(objective_space, configuration, features, error, count, values, ptr)
        super(ptr.read_ccs_features_evaluation_t, retain: false)
        @objective_space = objective_space
        @configuration = configuration
        @features = features
      end
    end

    def self.from_handle(handle, retain: true, auto_release: true)
      self::new(handle, retain: retain, auto_release: auto_release)
    end

    def error=(err)
      CCS.error_check CCS.ccs_features_evaluation_set_error(@handle, err)
      err
    end

    def num_objective_values
      @num_values ||= begin
        ptr = MemoryPointer::new(:size_t)
        CCS.error_check CCS.ccs_features_evaluation_get_objective_values(@handle, 0, nil, ptr)
        ptr.read_size_t
      end
    end

    def objective_values
      count = num_values
      return [] if count == 0
      values = MemoryPointer::new(:ccs_datum_t, count)
      CCS.error_check CCS.ccs_features_evaluation_get_objective_values(@handle, count, values, nil)
      count.times.collect { |i| Datum::new(values[i]).value }
    end

    def check
      ptr = MemoryPointer::new(:ccs_bool_t)
      CCS.error_check CCS.ccs_features_evaluation_check(@handle, ptr)
      return ptr.read_ccs_bool_t == CCS::FALSE ? false : true
    end

    def compare(other)
      ptr = MemoryPointer::new(:ccs_comparison_t)
      CCS.error_check CCS.ccs_features_evaluation_compare(@handle, other, ptr)
      ptr.read_ccs_comparison_t
    end

    def <=>(other)
      ptr = MemoryPointer::new(:ccs_comparison_t)
      CCS.error_check CCS.ccs_features_evaluation_compare(@handle, other, ptr)
      r = ptr.read_int32
      r == 2 ? nil : r 
    end
  end
end
