module CCS

  @hyperparameter_counter = 0
  def self.get_id
    id = @hyperparameter_counter
    @hyperparameter_counter += 1
    id
  end

  HyperparameterType = enum FFI::Type::INT32, :ccs_hyperparameter_type_t, [
    :CCS_HYPERPARAMETER_TYPE_NUMERICAL,
    :CCS_HYPERPARAMETER_TYPE_CATEGORICAL,
    :CCS_HYPERPARAMETER_TYPE_ORDINAL,
    :CCS_HYPERPARAMETER_TYPE_DISCRETE,
    :CCS_HYPERPARAMETER_TYPE_STRING
  ]
  class MemoryPointer
    def read_ccs_hyperparameter_type_t
      HyperparameterType.from_native(read_int32, nil)
    end
  end

  attach_function :ccs_hyperparameter_get_type, [:ccs_hyperparameter_t, :pointer], :ccs_error_t
  attach_function :ccs_hyperparameter_get_default_value, [:ccs_hyperparameter_t, :pointer], :ccs_error_t
  attach_function :ccs_hyperparameter_get_name, [:ccs_hyperparameter_t, :pointer], :ccs_error_t
  attach_function :ccs_hyperparameter_get_default_distribution, [:ccs_hyperparameter_t, :pointer], :ccs_error_t
  attach_function :ccs_hyperparameter_check_value, [:ccs_hyperparameter_t, :ccs_datum_t, :pointer], :ccs_error_t
  attach_function :ccs_hyperparameter_check_values, [:ccs_hyperparameter_t, :size_t, :pointer, :pointer], :ccs_error_t
  attach_function :ccs_hyperparameter_validate_value, [:ccs_hyperparameter_t, :ccs_datum_t, :pointer, :pointer], :ccs_error_t
  attach_function :ccs_hyperparameter_validate_values, [:ccs_hyperparameter_t, :size_t, :pointer, :pointer, :pointer], :ccs_error_t
  attach_function :ccs_hyperparameter_sample, [:ccs_hyperparameter_t, :ccs_distribution_t, :ccs_rng_t, :pointer], :ccs_error_t
  attach_function :ccs_hyperparameter_samples, [:ccs_hyperparameter_t, :ccs_distribution_t, :ccs_rng_t, :size_t, :pointer], :ccs_error_t

  class Hyperparameter < Object
    add_property :type, :ccs_hyperparameter_type_t, :ccs_hyperparameter_get_type, memoize:true

    def self.default_name
      "param%03d" % CCS.get_id
    end

    def self.from_handle(handle, retain: true, auto_release: true)
      ptr = MemoryPointer::new(:ccs_hyperparameter_type_t)
      CCS.error_check CCS.ccs_hyperparameter_get_type(handle, ptr)
      case ptr.read_ccs_hyperparameter_type_t
      when :CCS_HYPERPARAMETER_TYPE_NUMERICAL
        NumericalHyperparameter
      when :CCS_HYPERPARAMETER_TYPE_CATEGORICAL
        CategoricalHyperparameter
      when :CCS_HYPERPARAMETER_TYPE_ORDINAL
        OrdinalHyperparameter
      when :CCS_HYPERPARAMETER_TYPE_DISCRETE
        DiscreteHyperparameter
      when :CCS_HYPERPARAMETER_TYPE_STRING
        StringHyperparameter
      else
        raise CCSError, :CCS_INVALID_HYPERPARAMETER
      end.new(handle, retain: retain, auto_release: auto_release)
    end

    def name
      @name ||= begin
        ptr = MemoryPointer::new(:pointer)
        CCS.error_check CCS.ccs_hyperparameter_get_name(@handle, ptr)
        r = ptr.read_pointer.read_string
        r = r.sub(/^:/, "").to_sym if r.match(/^:/)
        r
      end
    end

    def default_value
      @default_value ||= begin
        ptr = MemoryPointer::new(:ccs_datum_t)
        CCS.error_check CCS.ccs_hyperparameter_get_default_value(@handle, ptr)
        d = Datum::new(ptr)
        d.value
      end
    end

    def default_distribution
      @default_distribution ||= begin
        ptr = MemoryPointer::new(:ccs_distribution_t)
        CCS.error_check CCS.ccs_hyperparameter_get_default_distribution(@handle, ptr)
        Object::from_handle(ptr.read_pointer)
      end
    end

    def check_value(v)
      ptr = MemoryPointer::new(:ccs_bool_t)
      CCS.error_check CCS.ccs_hyperparameter_check_value(@handle, Datum::from_value(v), ptr)
      ptr.read_ccs_bool_t == CCS::FALSE ? false : true
    end

    def check_values(vals)
      count = vals.size
      return [] if count == 0
      values = MemoryPointer::new(:ccs_datum_t, count)
      vals.each_with_index { |v, i| Datum::new(values[i]).set_value(v, string_store: ss) }
      ptr = MemoryPointer::new(:ccs_bool_t, count)
      CCS.error_check CCS.ccs_hyperparameter_check_values(@handle, count, values, ptr)
      count.times.collect { |i| ptr[i].read_ccs_bool_t == CCS::FALSE ? false : true }
    end

    def sample(distribution: default_distribution, rng: CCS::DefaultRng)
      value = MemoryPointer::new(:ccs_datum_t)
      CCS.error_check CCS.ccs_hyperparameter_sample(@handle, distribution, rng, value)
      Datum::new(value).value
    end

    def samples(count, distribution: default_distribution, rng: CCS::DefaultRng)
      return [] if count <= 0
      values = MemoryPointer::new(:ccs_datum_t, count)
      CCS.error_check CCS.ccs_hyperparameter_samples(@handle, distribution, rng, count, values)
      count.times.collect { |i| Datum::new(values[i]).value }
    end

    def ==(other)
      self.class == other.class && @handle == other.handle
    end
  end

  attach_function :ccs_create_numerical_hyperparameter, [:string, :ccs_numeric_type_t, :ccs_numeric_t, :ccs_numeric_t, :ccs_numeric_t, :ccs_numeric_t, :pointer], :ccs_error_t
  attach_function :ccs_numerical_hyperparameter_get_parameters, [:ccs_hyperparameter_t, :pointer, :pointer, :pointer, :pointer], :ccs_error_t
  class NumericalHyperparameter < Hyperparameter
    def initialize(handle = nil, retain: false, auto_release: true,
                   name: Hyperparameter.default_name, data_type: :CCS_NUM_FLOAT, lower: 0.0, upper: 1.0, quantization: 0.0, default: lower)
      if (handle)
        super(handle, retain: retain, auto_release: auto_release)
      else
        ptr = MemoryPointer::new(:ccs_hyperparameter_t)
        case data_type
        when :CCS_NUM_FLOAT
          lower = Numeric::from_value(lower.to_f)
          upper = Numeric::from_value(upper.to_f)
          quantization = Numeric::from_value(quantization.to_f)
          default = Numeric::from_value(default.to_f)
        when :CCS_NUM_INTEGER
          lower = Numeric::from_value(lower.to_i)
          upper = Numeric::from_value(upper.to_i)
          quantization = Numeric::from_value(quantization.to_i)
          default = Numeric::from_value(default.to_i)
        else
          raise CCSError, :CCS_INVALID_TYPE
        end
        name = name.inspect if name.kind_of?(Symbol)
        CCS.error_check CCS.ccs_create_numerical_hyperparameter(name, data_type, lower, upper, quantization, default, ptr)
        super(ptr.read_pointer, retain: false)
      end
    end

    def self.int(name: default_name, lower:, upper:, quantization: 0, default: lower)
      self.new(nil, name: name, data_type: :CCS_NUM_INTEGER, lower: lower, upper: upper, quantization: quantization, default: default)
    end
 
    def self.float(name: default_name, lower:, upper:, quantization: 0.0, default: lower)
      self.new(nil, name: name, data_type: :CCS_NUM_FLOAT, lower: lower, upper: upper, quantization: quantization, default: default)
    end

    def data_type
      @data_type ||= begin
        ptr = MemoryPointer::new(:ccs_numeric_type_t)
        CCS.error_check CCS.ccs_numerical_hyperparameter_get_parameters(@handle, ptr, nil, nil, nil)
        ptr.read_ccs_numeric_type_t
      end
    end
 
    def lower
      @lower ||= begin
        ptr = MemoryPointer::new(:ccs_numeric_t)
        CCS.error_check CCS.ccs_numerical_hyperparameter_get_parameters(@handle, nil, ptr, nil, nil)
        if data_type == :CCS_NUM_FLOAT
          ptr.read_ccs_float_t
        else
          ptr.read_ccs_int_t
        end
      end
    end

    def upper
      @upper ||= begin
        ptr = MemoryPointer::new(:ccs_numeric_t)
        CCS.error_check CCS.ccs_numerical_hyperparameter_get_parameters(@handle, nil, nil, ptr, nil)
        if data_type == :CCS_NUM_FLOAT
          ptr.read_ccs_float_t
        else
          ptr.read_ccs_int_t
        end
      end
    end

    def quantization
      @quantization ||= begin
        ptr = MemoryPointer::new(:ccs_numeric_t)
        CCS.error_check CCS.ccs_numerical_hyperparameter_get_parameters(@handle, nil, nil, nil, ptr)
        if data_type == :CCS_NUM_FLOAT
          ptr.read_ccs_float_t
        else
          ptr.read_ccs_int_t
        end
      end
    end
  end

  attach_function :ccs_create_categorical_hyperparameter, [:string, :size_t, :pointer, :size_t, :pointer],  :ccs_error_t
  attach_function :ccs_categorical_hyperparameter_get_values, [:ccs_hyperparameter_t, :size_t, :pointer, :pointer], :ccs_error_t
  class CategoricalHyperparameter < Hyperparameter
    def initialize(handle = nil, retain: false, auto_release: true,
                   name: Hyperparameter.default_name, values: [], default_index: 0)
      if handle
        super(handle, retain: retain, auto_release: auto_release)
      else
        count = values.size
        raise CCSError, :CCS_INVALID_VALUE if count == 0
        ss = []
        vals = MemoryPointer::new(:ccs_datum_t, count)
        values.each_with_index{ |v, i| Datum::new(vals[i]).set_value(v, string_store: ss) }
        ptr = MemoryPointer::new(:ccs_hyperparameter_t)
        name = name.inspect if name.kind_of?(Symbol)
        CCS.error_check CCS.ccs_create_categorical_hyperparameter(name, count, vals, default_index, ptr)
        super(ptr.read_ccs_hyperparameter_t, retain: false)
      end
    end

    def values
      @values ||= begin
        ptr = MemoryPointer::new(:size_t)
        CCS.error_check CCS.ccs_categorical_hyperparameter_get_values(@handle, 0, nil, ptr)
        count = ptr.read_size_t
        ptr = MemoryPointer::new(:ccs_datum_t, count)
        CCS.error_check CCS.ccs_categorical_hyperparameter_get_values(@handle, count, ptr, nil)
        count.times.collect { |i| Datum::new(ptr[i]).value }
      end
    end
  end

  attach_function :ccs_create_ordinal_hyperparameter, [:string, :size_t, :pointer, :size_t, :pointer],  :ccs_error_t
  attach_function :ccs_ordinal_hyperparameter_compare_values, [:ccs_hyperparameter_t, :ccs_datum_t, :ccs_datum_t, :pointer], :ccs_error_t
  attach_function :ccs_ordinal_hyperparameter_get_values, [:ccs_hyperparameter_t, :size_t, :pointer, :pointer], :ccs_error_t
  class OrdinalHyperparameter < Hyperparameter
    def initialize(handle = nil, retain: false, auto_release: true,
                   name: Hyperparameter.default_name, values: [], default_index: 0)
      if handle
        super(handle, retain: retain, auto_release: auto_release)
      else
        count = values.size
        raise CCSError, :CCS_INVALID_VALUE if count == 0
        ss = []
        vals = MemoryPointer::new(:ccs_datum_t, count)
        values.each_with_index{ |v, i| Datum::new(vals[i]).set_value(v, string_store: ss) }
        ptr = MemoryPointer::new(:ccs_hyperparameter_t)
        name = name.inspect if name.kind_of?(Symbol)
        CCS.error_check CCS.ccs_create_ordinal_hyperparameter(name, count, vals, default_index, ptr)
        super(ptr.read_ccs_hyperparameter_t, retain: false)
      end
    end

    def compare(value1, value2)
      v1 = Datum::from_value(value1)
      v2 = Datum::from_value(value2)
      ptr = MemoryPointer::new(:ccs_int_t)
      CCS.error_check CCS.ccs_ordinal_hyperparameter_compare_values(@handle, v1, v2, ptr)
      ptr.read_ccs_int_t
    end

    def values
      @values ||= begin
        ptr = MemoryPointer::new(:size_t)
        CCS.error_check CCS.ccs_ordinal_hyperparameter_get_values(@handle, 0, nil, ptr)
        count = ptr.read_size_t
        ptr = MemoryPointer::new(:ccs_datum_t, count)
        CCS.error_check CCS.ccs_ordinal_hyperparameter_get_values(@handle, count, ptr, nil)
        count.times.collect { |i| Datum::new(ptr[i]).value }
      end
    end
  end

  attach_function :ccs_create_discrete_hyperparameter, [:string, :size_t, :pointer, :size_t, :pointer],  :ccs_error_t
  attach_function :ccs_discrete_hyperparameter_get_values, [:ccs_hyperparameter_t, :size_t, :pointer, :pointer], :ccs_error_t
  class DiscreteHyperparameter < Hyperparameter
    def initialize(handle = nil, retain: false, auto_release: true,
                   name: Hyperparameter.default_name, values: [], default_index: 0)
      if handle
        super(handle, retain: retain, auto_release: auto_release)
      else
        count = values.size
        raise CCSError, :CCS_INVALID_VALUE if count == 0
        ss = []
        vals = MemoryPointer::new(:ccs_datum_t, count)
        values.each_with_index{ |v, i| Datum::new(vals[i]).set_value(v, string_store: ss) }
        ptr = MemoryPointer::new(:ccs_hyperparameter_t)
        name = name.inspect if name.kind_of?(Symbol)
        CCS.error_check CCS.ccs_create_discrete_hyperparameter(name, count, vals, default_index, ptr)
        super(ptr.read_ccs_hyperparameter_t, retain: false)
      end
    end

    def values
      @values ||= begin
        ptr = MemoryPointer::new(:size_t)
        CCS.error_check CCS.ccs_discrete_hyperparameter_get_values(@handle, 0, nil, ptr)
        count = ptr.read_size_t
        ptr = MemoryPointer::new(:ccs_datum_t, count)
        CCS.error_check CCS.ccs_discrete_hyperparameter_get_values(@handle, count, ptr, nil)
        count.times.collect { |i| Datum::new(ptr[i]).value }
      end
    end
  end

  attach_function :ccs_create_string_hyperparameter, [:string, :pointer],  :ccs_error_t
  class StringHyperparameter < Hyperparameter
    def initialize(handle = nil, retain: false, auto_release: true,
                   name: Hyperparameter.default_name)
      if handle
        super(handle, retain: retain, auto_release: auto_release)
      else
        ptr = MemoryPointer::new(:ccs_hyperparameter_t)
        name = name.inspect if name.kind_of?(Symbol)
        CCS.error_check CCS.ccs_create_string_hyperparameter(name, ptr)
        super(ptr.read_ccs_hyperparameter_t, retain: false)
      end
    end
  end

end
