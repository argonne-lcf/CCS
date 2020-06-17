module CCS

  HyperparameterType = enum FFI::Type::INT32, :ccs_hyperparameter_type_t, [
    :CCS_NUMERICAL,
    :CCS_CATEGORICAL,
    :CCS_ORDINAL
  ]
  class MemoryPointer
    def read_ccs_hyperparameter_type_t
      HyperparameterType.from_native(read_int32, nil)
    end
  end

  attach_function :ccs_hyperparameter_get_type, [:ccs_hyperparameter_t, :pointer], :ccs_result_t
  attach_function :ccs_hyperparameter_get_default_value, [:ccs_hyperparameter_t, :pointer], :ccs_result_t
  attach_function :ccs_hyperparameter_get_name, [:ccs_hyperparameter_t, :pointer], :ccs_result_t
  attach_function :ccs_hyperparameter_get_user_data, [:ccs_hyperparameter_t, :pointer], :ccs_result_t
  attach_function :ccs_hyperparameter_get_default_distribution, [:ccs_hyperparameter_t, :pointer], :ccs_result_t
  attach_function :ccs_hyperparameter_check_value, [:ccs_hyperparameter_t, :ccs_datum_t, :pointer], :ccs_result_t
  attach_function :ccs_hyperparameter_check_values, [:ccs_hyperparameter_t, :size_t, :pointer, :pointer], :ccs_result_t

  class Hyperparameter < Object
    add_property :type, :ccs_distribution_type_t, :ccs_hyperparameter_get_type, memoize:true
    add_property :user_data, :pointer, :ccs_hyperparameter_get_user_data, memoize: true
    def initialize(handle, retain: false)
      if !handle
        raise StandardError, :CCS_INVALID_OBJECT
      end
      super
    end

    def self.from_handle(handle)
      ptr = MemoryPointer::new(:ccs_hyperparameter_type_t)
      res = CCS.ccs_hyperparameter_get_type(handle, ptr)
      CCS.error_check(res)
      case ptr.read_ccs_hyperparameter_type_t
      when :CCS_NUMERICAL
        NumericalHyperparameter::new(handle, retain: true)
      when :CCS_CATEGORICAL
        CategoricalHyperparameter::new(handle, retain: true)
      when :CCS_ORDINAL
        OrdinalHyperparameter::new(handle, retain: true)
      else
        raise StandardError, :CCS_INVALID_HYPERPARAMETER
      end
    end

    def name
      @name ||= begin
        ptr = MemoryPointer::new(:pointer)
        res = CCS.ccs_hyperparameter_get_name(@handle, ptr)
        CCS.error_check(res)
        ptr.read_pointer.read_string
      end
    end

    def default_value
      @default_value ||= begin
        ptr = MemoryPointer::new(:ccs_datum_t)
        res = CCS.ccs_hyperparameter_get_default_value(handle, ptr)
        CCS.error_check(res)
        d = Datum::new(ptr)
        d.value
      end
    end

    def default_distribution
      @default_distribution ||= begin
        ptr = MemoryPointer::new(:ccs_distribution_t)
        res = CCS.ccs_hyperparameter_get_default_distribution(handle, ptr)
        CCS.error_check(res)
        Object::from_handle(ptr.read_pointer)
      end
    end
  end
end
