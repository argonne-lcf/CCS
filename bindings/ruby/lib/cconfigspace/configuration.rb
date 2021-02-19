module CCS

  attach_function :ccs_create_configuration, [:ccs_configuration_space_t, :size_t, :pointer, :pointer, :pointer], :ccs_result_t
  attach_function :ccs_configuration_check, [:ccs_configuration_t], :ccs_result_t

  class Configuration < Binding
    alias configuration_space context

    def initialize(handle = nil, retain: false, auto_release: true,
                   configuration_space: nil,  values: nil, user_data: nil)
      if (handle)
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
        ptr = MemoryPointer::new(:ccs_configuration_t)
        res = CCS.ccs_create_configuration(configuration_space, count, values, user_data, ptr)
        CCS.error_check(res)
        super(ptr.read_ccs_configuration_t, retain: false)
      end
    end

    def self.from_handle(handle, retain: true, auto_release: true)
      self::new(handle, retain: retain, auto_release: auto_release)
    end

    def check
      res = CCS.ccs_configuration_check(@handle)
      CCS.error_check(res)
      self
    end

  end

end
