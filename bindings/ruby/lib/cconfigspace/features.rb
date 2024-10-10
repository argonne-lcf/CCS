module CCS

  attach_function :ccs_create_features, [:ccs_feature_space_t, :size_t, :pointer, :pointer], :ccs_result_t

  class Features < Binding
    alias feature_space context
    include Comparable

    def initialize(handle = nil, retain: false, auto_release: true,
                   feature_space: nil,  values: nil)
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
        ptr = MemoryPointer::new(:ccs_features_t)
        CCS.error_check CCS.ccs_create_features(feature_space, count, values, ptr)
        super(ptr.read_ccs_features_t, retain: false)
      end
    end

    def self.from_handle(handle, retain: true, auto_release: true)
      self::new(handle, retain: retain, auto_release: auto_release)
    end

  end

end
