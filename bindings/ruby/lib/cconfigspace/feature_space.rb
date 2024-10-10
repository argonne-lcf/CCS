module CCS
  attach_function :ccs_create_feature_space, [:string, :size_t, :pointer, :pointer], :ccs_result_t
  attach_function :ccs_feature_space_get_default_features, [:ccs_feature_space_t, :pointer], :ccs_result_t

  class FeatureSpace < Context
    add_handle_property :default_features, :ccs_features_t, :ccs_feature_space_get_default_features, memoize: true

    def initialize(handle = nil, retain: false, auto_release: true,
                   name: "", parameters: nil)
      if handle
        super(handle, retain: retain, auto_release: auto_release)
      else
        count = parameters.size
	p_parameters = MemoryPointer::new(:ccs_parameter_t, count)
	p_parameters.write_array_of_pointer(parameters.collect(&:handle))
        ptr = MemoryPointer::new(:ccs_feature_space_t)
        CCS.error_check CCS.ccs_create_feature_space(name, count, p_parameters, ptr)
        super(ptr.read_ccs_feature_space_t, retain: false)
      end
    end

    def self.from_handle(handle, retain: true, auto_release: true)
      self::new(handle, retain: retain, auto_release: auto_release)
    end

  end
end
