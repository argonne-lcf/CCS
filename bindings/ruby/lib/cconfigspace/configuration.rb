module CCS

  attach_function :ccs_create_configuration, [:ccs_configuration_space_t, :size_t, :pointer, :pointer, :pointer], :ccs_result_t
  attach_function :ccs_configuration_get_configuration_space, [:ccs_configuration_t, :pointer], :ccs_result_t
  attach_function :ccs_configuration_get_user_data, [:ccs_configuration_t, :pointer], :ccs_result_t
  attach_function :ccs_configuration_get_value, [:ccs_configuration_t, :size_t, :pointer], :ccs_result_t
  attach_function :ccs_configuration_set_value, [:ccs_configuration_t, :size_t, :ccs_datum_t], :ccs_result_t
  attach_function :ccs_configuration_get_values, [:ccs_configuration_t, :size_t, :pointer, :pointer], :ccs_result_t
  attach_function :ccs_configuration_get_value_by_name, [:ccs_configuration_t, :string, :pointer], :ccs_result_t
  attach_function :ccs_configuration_check, [:ccs_configuration_t], :ccs_result_t
  attach_function :ccs_configuration_hash, [:ccs_configuration_t, :pointer], :ccs_result_t
  attach_function :ccs_configuration_cmp, [:ccs_configuration_t, :ccs_configuration_t, :pointer], :ccs_result_t

  class Configuration < Object
    include Comparable
    add_property :user_data, :pointer, :ccs_configuration_get_user_data, memoize: true
    add_property :hash, :ccs_hash_t, :ccs_configuration_hash, memoize: false
    add_handle_property :configuration_space, :ccs_configuration_space_t, :ccs_configuration_get_configuration_space, memoize: true

    def initialize(handle = nil, retain: false, configuration_space: nil,  values: nil, user_data: nil)
      if (handle)
        super(handle, retain: retain)
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

    def self.from_handle(handle)
      self::new(handle, retain: true)
    end

    def set_value(hyperparameter, value)
      d = Datum.from_value(value)
      case hyperparameter
      when String
        hyperparameter = configuration_space.hyperparameter_index_by_name(hyperparameter)
      when Hyperparameter
        hyperparameter = configuration_space.hyperparameter_index(hyperparameter)
      end
      res = CCS.ccs_configuration_set_value(@handle, hyperparameter, d)
      CCS.error_check(res)
      self
    end

    def value(hyperparameter)
      ptr = MemoryPointer::new(:ccs_datum_t)
      case hyperparameter
      when String
        res = CCS.ccs_configuration_get_value_by_name(@handle, hyperparameter, ptr)
      when Hyperparameter
        res = CCS.ccs_configuration_get_value(@handle, configuration_space.hyperparameter_index(hyperparameter), ptr)
      when Integer
        res = CCS.ccs_configuration_get_value(@handle, hyperparameter, ptr)
      else
        raise CCSError, :CCS_INVALID_VALUE
      end
      CCS.error_check(res)
      Datum::new(ptr).value
    end

    def num_values
      @num_values ||= begin
        ptr = MemoryPointer::new(:size_t)
        res = CCS.ccs_configuration_get_values(@handle, 0, nil, ptr)
        CCS.error_check(res)
        ptr.read_size_t
      end
    end

    def values
      count = num_values
      return [] if count == 0
      values = MemoryPointer::new(:ccs_datum_t, count)
      res = CCS.ccs_configuration_get_values(@handle, count, values, nil)
      CCS.error_check(res)
      count.times.collect { |i| Datum::new(values[i]).value }
    end

    def check
      res = CCS.ccs_configuration_check(@handle)
      CCS.error_check(res)
      self
    end

    def <=>(other)
      ptr = MemoryPointer::new(:int)
      res = CCS.ccs_configuration_cmp(@handle, other, ptr)
      CCS.error_check(res)
      return ptr.read_int
    end

    def to_h
      configuration_space.hyperparameters.collect(&:name).zip(values).to_h
    end
  end

end
