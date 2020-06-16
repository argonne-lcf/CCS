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
  attach_function :ccs_distribution_check_oversampling, [:ccs_distribution_t, Interval.by_ref], :ccs_result_t
  attach_function :ccs_distribution_sample, [:ccs_distribution_t, :ccs_rng_t, :pointer], :ccs_result_t
  attach_function :ccs_distribution_samples, [:ccs_distribution_t, :ccs_rng_t, :size_t, :pointer], :ccs_result_t

  class Distribution < Object
    add_property :type, :ccs_distribution_type_t, :ccs_distribution_get_type, memoize: true
    add_property :data_type, :ccs_numeric_type_t, :ccs_distribution_get_data_type, memoize: true
    add_property :dimension, :size_t, :ccs_distribution_get_dimension, memoize: true
    add_property :scale_type, :ccs_scale_type_t, :ccs_distribution_get_scale_type, memoize: true

    def initialize(handle, retain: false)
      if !handle
        raise StandardError, :CCS_INVALID_OBJECT
      end
      super
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

  class UniformDistribution < Distribution
    def initialize(handle = nil, retain: false, data_type: :CCS_NUM_FLOAT, lower: 0.0, upper: 1.0, scale_type: :CCS_LINEAR, quantization: 0.0)
      if handle
        super(handle, retain: retian)
      else
        ptr = MemoryPointer::new(:ccs_distribution_t)
        res = if data_type == :CCS_NUM_FLOAT
            CCS.ccs_create_uniform_float_distribution(lower, upper, scale_type, quantization, ptr)
          else
            CCS.ccs_create_uniform_int_distribution(lower, upper, scale_type, quantization, ptr)
          end
        CCS.error_check(res)
        super(ptr.read_pointer, retain: false)
      end
    end
  end

end
