module CCS

  attach_function :ccs_create_configuration, [:ccs_configuration_space_t, :ccs_features_t, :size_t, :pointer, :pointer], :ccs_result_t
  attach_function :ccs_configuration_get_features, [:ccs_configuration_t, :pointer], :ccs_result_t
  attach_function :ccs_configuration_check, [:ccs_configuration_t, :pointer], :ccs_result_t

  class Configuration < Binding
    alias configuration_space context
    add_optional_handle_property :features, :ccs_features_t, :ccs_configuration_get_features, memoize: true

    def initialize(handle = nil, retain: false, auto_release: true,
                   configuration_space: nil, features: nil, values: nil)
      if (handle)
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
        ptr = MemoryPointer::new(:ccs_configuration_t)
        CCS.error_check CCS.ccs_create_configuration(configuration_space, features, count, values, ptr)
        super(ptr.read_ccs_configuration_t, retain: false)
      end
    end

    def self.from_handle(handle, retain: true, auto_release: true)
      self::new(handle, retain: retain, auto_release: auto_release)
    end

    def check
      ptr = MemoryPointer::new(:ccs_bool_t)
      CCS.error_check CCS.ccs_configuration_check(@handle, ptr)
      return ptr.read_ccs_bool_t == CCS::FALSE ? false : true
    end

  end

end
