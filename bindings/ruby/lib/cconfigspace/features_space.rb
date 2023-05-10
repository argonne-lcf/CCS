module CCS
  attach_function :ccs_create_features_space, [:string, :pointer], :ccs_result_t
  attach_function :ccs_features_space_add_parameter, [:ccs_features_space_t, :ccs_parameter_t], :ccs_result_t
  attach_function :ccs_features_space_add_parameters, [:ccs_features_space_t, :size_t, :pointer], :ccs_result_t
  attach_function :ccs_features_space_check_features, [:ccs_features_space_t, :ccs_features_t, :pointer], :ccs_result_t
  attach_function :ccs_features_space_check_features_values, [:ccs_features_space_t, :size_t, :pointer, :pointer], :ccs_result_t

  class FeaturesSpace < Context
    def initialize(handle = nil, retain: false, auto_release: true,
                   name: "")
      if handle
        super(handle, retain: retain, auto_release: auto_release)
      else
        ptr = MemoryPointer::new(:ccs_features_space_t)
        CCS.error_check CCS.ccs_create_features_space(name, ptr)
        super(ptr.read_ccs_features_space_t, retain: false)
      end
    end

    def self.from_handle(handle, retain: true, auto_release: true)
      self::new(handle, retain: retain, auto_release: auto_release)
    end

    def add_parameter(parameter)
      CCS.error_check CCS.ccs_features_space_add_parameter(@handle, parameter)
      self
    end

    def add_parameters(parameters)
      count = parameters.size
      return self if count == 0
      p_parameters = MemoryPointer::new(:ccs_parameter_t, count)
      p_parameters.write_array_of_pointer(parameters.collect(&:handle))
      CCS.error_check CCS.ccs_features_space_add_parameters(@handle, count, p_parameters)
      self
    end

    def check(features)
      ptr = MemoryPointer::new(:ccs_bool_t)
      CCS.error_check CCS.ccs_features_space_check_features(@handle, features, ptr)
      return ptr.read_ccs_bool_t == CCS::FALSE ? false : true
    end

    def check_values(values)
      count = values.size
      raise CCSError, :CCS_RESULT_ERROR_INVALID_VALUE if count != num_parameters
      ss = []
      ptr = MemoryPointer::new(:ccs_datum_t, count)
      values.each_with_index {  |v, i| Datum::new(ptr[i]).set_value(v, string_store: ss) }
      ptr2 = MemoryPointer::new(:ccs_bool_t)
      CCS.error_check CCS.ccs_features_space_check_features_values(@handle, count, ptr, ptr2)
      return ptr2.read_ccs_bool_t == CCS::FALSE ? false : true
    end
  end
end
