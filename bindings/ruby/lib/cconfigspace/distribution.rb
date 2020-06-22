module CCS

  DistributionType = enum FFI::Type::INT32, :ccs_distribution_type_t, [
    :CCS_UNIFORM,
    :CCS_NORMAL,
    :CCS_ROULETTE
  ]
  class MemoryPointer
    def read_ccs_distribution_type_t
      DistributionType.from_native(read_int32, nil)
    end
  end

  ScaleType = enum FFI::Type::INT32, :ccs_scale_type_t, [
    :CCS_LINEAR,
    :CCS_LOGARITHMIC
  ]
  class MemoryPointer
    def read_ccs_scale_type_t
      ScaleType.from_native(read_int32, nil)
    end
  end

  attach_function :ccs_distribution_get_type, [:ccs_distribution_t, :pointer], :ccs_result_t
  attach_function :ccs_distribution_get_data_type, [:ccs_distribution_t, :pointer], :ccs_result_t
  attach_function :ccs_distribution_get_dimension, [:ccs_distribution_t, :pointer], :ccs_result_t
  attach_function :ccs_distribution_get_scale_type, [:ccs_distribution_t, :pointer], :ccs_result_t
  attach_function :ccs_distribution_get_quantization, [:ccs_distribution_t, :pointer], :ccs_result_t
  attach_function :ccs_distribution_get_bounds, [:ccs_distribution_t, :pointer], :ccs_result_t
  attach_function :ccs_distribution_check_oversampling, [:ccs_distribution_t, Interval.by_ref, :pointer], :ccs_result_t
  attach_function :ccs_distribution_sample, [:ccs_distribution_t, :ccs_rng_t, :pointer], :ccs_result_t
  attach_function :ccs_distribution_samples, [:ccs_distribution_t, :ccs_rng_t, :size_t, :pointer], :ccs_result_t

  class Distribution < Object
    add_property :type, :ccs_distribution_type_t, :ccs_distribution_get_type, memoize: true
    add_property :data_type, :ccs_numeric_type_t, :ccs_distribution_get_data_type, memoize: true
    add_property :dimension, :size_t, :ccs_distribution_get_dimension, memoize: true
    add_property :scale, :ccs_scale_type_t, :ccs_distribution_get_scale_type, memoize: true

    def self.from_handle(handle)
      ptr = MemoryPointer::new(:ccs_distribution_type_t)
      res = CCS.ccs_distribution_get_type(handle, ptr)
      CCS.error_check(res)
      case ptr.read_ccs_distribution_type_t
      when :CCS_UNIFORM
        UniformDistribution::new(handle, retain: true)
      when :CCS_NORMAL
        NormalDistribution::new(handle, retain: true)
      when :CCS_ROULETTE
        RouletteDistribution::new(handle, retain: true)
      else
        raise CCSError, :CCS_INVALID_DISTRIBUTION
      end
    end

    def quantization
      @quantization ||= begin
        ptr = MemoryPointer::new(:ccs_numeric_t)
        res = CCS.ccs_distribution_get_quantization(@handle, ptr)
        CCS.error_check(res)
        if data_type == :CCS_NUM_FLOAT
          ptr.read_ccs_float_t
        else
          ptr.read_ccs_int_t
        end
      end
    end

    def bounds
      @bounds ||= begin
        interval = Interval::new(type: :CCS_NUM_FLOAT)
        res = CCS.ccs_distribution_get_bounds(@handle, interval)
        CCS.error_check(res)
        interval
      end
    end

    def oversampling?(interval)
      ptr = MemoryPointer::new(:ccs_bool_t)
      res = CCS.ccs_distribution_check_oversampling(@handle, interval, ptr)
      CCS.error_check(res)
      ptr.read_ccs_bool_t == CCS::FALSE ? false : true
    end

    def sample(rng)
      ptr = MemoryPointer::new(:ccs_numeric_t)
      res = CCS.ccs_distribution_sample(@handle, rng, ptr)
      CCS.error_check(res)
      if data_type == :CCS_NUM_FLOAT
        ptr.read_ccs_float_t
      else
        ptr.read_ccs_int_t
      end
    end

    def samples(rng, count)
      ptr = MemoryPointer::new(:ccs_numeric_t, count)
      res = CCS.ccs_distribution_samples(@handle, rng, count, ptr)
      CCS.error_check(res)
      if data_type == :CCS_NUM_FLOAT
        ptr.read_array_of_ccs_float_t(count)
      else
        ptr.read_array_of_ccs_int_t(count)
      end
    end

  end

  attach_function :ccs_create_uniform_distribution, [:ccs_numeric_type_t, :ccs_numeric_t, :ccs_numeric_t, :ccs_scale_type_t, :ccs_numeric_t, :pointer], :ccs_result_t
  attach_function :ccs_create_uniform_int_distribution, [:ccs_int_t, :ccs_int_t, :ccs_scale_type_t, :ccs_int_t, :pointer], :ccs_result_t
  attach_function :ccs_create_uniform_float_distribution, [:ccs_float_t, :ccs_float_t, :ccs_scale_type_t, :ccs_float_t, :pointer], :ccs_result_t
  attach_function :ccs_uniform_distribution_get_parameters, [:ccs_distribution_t, :pointer, :pointer], :ccs_result_t

  class UniformDistribution < Distribution
    def initialize(handle = nil, retain: false, data_type: :CCS_NUM_FLOAT, lower: 0.0, upper: 1.0, scale: :CCS_LINEAR, quantization: 0.0)
      if handle
        super(handle, retain: retain)
      else
        ptr = MemoryPointer::new(:ccs_distribution_t)
        res = if data_type == :CCS_NUM_FLOAT
            CCS.ccs_create_uniform_float_distribution(lower, upper, scale, quantization, ptr)
          else
            CCS.ccs_create_uniform_int_distribution(lower, upper, scale, quantization, ptr)
          end
        CCS.error_check(res)
        super(ptr.read_pointer, retain: false)
      end
    end

    def self.int(lower:, upper:, scale: :CCS_LINEAR, quantization: 0)
      self.new(nil, data_type: :CCS_NUM_INTEGER, lower: lower, upper: upper, scale: scale, quantization: quantization)
    end

    def self.float(lower:, upper:, scale: :CCS_LINEAR, quantization: 0.0)
      self.new(nil, data_type: :CCS_NUM_FLOAT, lower: lower, upper: upper, scale: scale, quantization: quantization)
    end

    def lower
      @lower ||= begin
        ptr = MemoryPointer::new(:ccs_numeric_t)
        res = CCS.ccs_uniform_distribution_get_parameters(@handle, ptr, nil)
        CCS.error_check(res)
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
        res = CCS.ccs_uniform_distribution_get_parameters(@handle, nil, ptr)
        CCS.error_check(res)
        if data_type == :CCS_NUM_FLOAT
          ptr.read_ccs_float_t
        else
          ptr.read_ccs_int_t
        end
      end
    end
  end

  attach_function :ccs_create_normal_distribution, [:ccs_numeric_type_t, :ccs_float_t, :ccs_float_t, :ccs_scale_type_t, :ccs_numeric_t, :pointer], :ccs_result_t
  attach_function :ccs_create_normal_int_distribution, [:ccs_float_t, :ccs_float_t, :ccs_scale_type_t, :ccs_int_t, :pointer], :ccs_result_t
  attach_function :ccs_create_normal_float_distribution, [:ccs_float_t, :ccs_float_t, :ccs_scale_type_t, :ccs_float_t, :pointer], :ccs_result_t
  attach_function :ccs_normal_distribution_get_parameters, [:ccs_distribution_t, :pointer, :pointer], :ccs_result_t
  class NormalDistribution < Distribution
    def initialize(handle = nil, retain: false, data_type: :CCS_NUM_FLOAT, mu: 0.0, sigma: 1.0, scale: :CCS_LINEAR, quantization: 0.0)
      if handle
        super(handle, retain: retain)
      else
        ptr = MemoryPointer::new(:ccs_distribution_t)
        res = if data_type == :CCS_NUM_FLOAT
            CCS.ccs_create_normal_float_distribution(mu, sigma, scale, quantization, ptr)
          else
            CCS.ccs_create_normal_int_distribution(mu, sigma, scale, quantization, ptr)
          end
        CCS.error_check(res)
        super(ptr.read_pointer, retain: false)
      end
    end

    def self.int(mu:, sigma:, scale: :CCS_LINEAR, quantization: 0)
      self::new(nil, retain: false, data_type: :CCS_NUM_INTEGER, mu: mu, sigma: sigma, scale: scale, quantization: quantization) 
    end

    def self.float(mu:, sigma:, scale: :CCS_LINEAR, quantization: 0)
      self::new(nil, retain: false, data_type: :CCS_NUM_FLOAT, mu: mu, sigma: sigma, scale: scale, quantization: quantization) 
    end

    def mu
      @mu ||= begin
        ptr = MemoryPointer::new(:ccs_numeric_t)
        res = CCS.ccs_normal_distribution_get_parameters(@handle, ptr, nil)
        CCS.error_check(res)
        ptr.read_ccs_float_t
      end
    end

    def sigma
      @sigma ||= begin
        ptr = MemoryPointer::new(:ccs_numeric_t)
        res = CCS.ccs_normal_distribution_get_parameters(@handle, nil, ptr)
        CCS.error_check(res)
        ptr.read_ccs_float_t
      end
    end
  end

  attach_function :ccs_create_roulette_distribution, [:size_t, :pointer, :pointer], :ccs_result_t
  attach_function :ccs_roulette_distribution_get_num_areas, [:ccs_distribution_t, :pointer], :ccs_result_t
  attach_function :ccs_roulette_distribution_get_areas, [:ccs_distribution_t, :size_t, :pointer, :pointer], :ccs_result_t
  class RouletteDistribution < Distribution
    add_property :num_areas, :size_t, :ccs_roulette_distribution_get_num_areas, memoize: true
    def initialize(handle = nil, retain: false, areas: [])
      if handle
        super(handle, retain: retain)
      else
        ptr = MemoryPointer::new(:ccs_distribution_t)
        p_areas = MemoryPointer::new(:ccs_float_t, areas.size)
        p_areas.write_array_of_ccs_float_t(areas)
        res = CCS.ccs_create_roulette_distribution(areas.size, p_areas, ptr)
        CCS.error_check(res)
        super(ptr.read_pointer, retain: false)
      end
    end

    def areas
      @areas ||= begin
        count = num_areas
        ptr = MemoryPointer::new(:ccs_float_t, count)
        res = CCS.ccs_roulette_distribution_get_areas(@handle, count, ptr, nil)
        CCS.error_check(res)
        ptr.read_array_of_ccs_float_t(count)
      end
    end
  end
end
