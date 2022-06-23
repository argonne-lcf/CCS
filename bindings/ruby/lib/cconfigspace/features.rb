module CCS

  attach_function :ccs_create_features, [:ccs_features_space_t, :size_t, :pointer, :pointer], :ccs_error_t
  attach_function :ccs_features_check, [:ccs_features_t, :pointer], :ccs_error_t

  class Features < Binding
    alias features_space context
    include Comparable

    def initialize(handle = nil, retain: false, auto_release: true,
                   features_space: nil,  values: nil)
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
        ptr = MemoryPointer::new(:ccs_features_t)
        CCS.error_check CCS.ccs_create_features(features_space, count, values, ptr)
        super(ptr.read_ccs_features_t, retain: false)
      end
    end

    def self.from_handle(handle, retain: true, auto_release: true)
      self::new(handle, retain: retain, auto_release: auto_release)
    end

    def check
      ptr = MemoryPointer::new(:ccs_bool_t)
      CCS.error_check CCS.ccs_features_check(@handle, ptr)
      return ptr.read_ccs_bool_t == CCS::FALSE ? false : true
    end

  end

end
