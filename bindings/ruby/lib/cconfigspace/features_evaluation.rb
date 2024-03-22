module CCS
  attach_function :ccs_create_features_evaluation, [:ccs_objective_space_t, :ccs_configuration_t, :ccs_features_t, :ccs_evaluation_result_t, :size_t, :pointer, :pointer], :ccs_result_t
  attach_function :ccs_features_evaluation_get_configuration, [:ccs_features_evaluation_t, :pointer], :ccs_result_t
  attach_function :ccs_features_evaluation_get_features, [:ccs_features_evaluation_t, :pointer], :ccs_result_t
  class FeaturesEvaluation < EvaluationBinding
    alias objective_space context
    add_handle_property :configuration, :ccs_configuration_t, :ccs_features_evaluation_get_configuration, memoize: true
    add_handle_property :features, :ccs_features_t, :ccs_features_evaluation_get_features, memoize: true

    def initialize(handle = nil, retain: false, auto_release: true,
                   objective_space: nil, configuration: nil, features: nil, result: :CCS_RESULT_SUCCESS, values: nil)
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
        ptr = MemoryPointer::new(:ccs_features_evaluation_t)
        CCS.error_check CCS.ccs_create_features_evaluation(objective_space, configuration, features, result, count, values, ptr)
        super(ptr.read_ccs_features_evaluation_t, retain: false)
        @objective_space = objective_space
        @configuration = configuration
        @features = features
      end
    end

    def self.from_handle(handle, retain: true, auto_release: true)
      self::new(handle, retain: retain, auto_release: auto_release)
    end

  end
end
