module CCS

  DistributionType = enum FFI::Type::INT32, :ccs_distribution_type_t, [
    :CCS_UNIFORM,
    :CCS_NORMAL,
    :CCS_ROULETTE,
    :CCS_MIXTURE,
    :CCS_MULTIVARIATE
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
  attach_function :ccs_distribution_get_data_types, [:ccs_distribution_t, :pointer], :ccs_result_t
  attach_function :ccs_distribution_get_dimension, [:ccs_distribution_t, :pointer], :ccs_result_t
  attach_function :ccs_distribution_get_bounds, [:ccs_distribution_t, :pointer], :ccs_result_t
  attach_function :ccs_distribution_check_oversampling, [:ccs_distribution_t, Interval.by_ref, :pointer], :ccs_result_t
  attach_function :ccs_distribution_sample, [:ccs_distribution_t, :ccs_rng_t, :pointer], :ccs_result_t
  attach_function :ccs_distribution_samples, [:ccs_distribution_t, :ccs_rng_t, :size_t, :pointer], :ccs_result_t

  class Distribution < Object
    add_property :type, :ccs_distribution_type_t, :ccs_distribution_get_type, memoize: true
    add_property :dimension, :size_t, :ccs_distribution_get_dimension, memoize: true

    def self.from_handle(handle, retain: true, auto_release: true)
      ptr = MemoryPointer::new(:ccs_distribution_type_t)
      res = CCS.ccs_distribution_get_type(handle, ptr)
      CCS.error_check(res)
      case ptr.read_ccs_distribution_type_t
      when :CCS_UNIFORM
        UniformDistribution
      when :CCS_NORMAL
        NormalDistribution
      when :CCS_ROULETTE
        RouletteDistribution
      when :CCS_MIXTURE
        MixtureDistribution
      when :CCS_MULTIVARIATE
        MultivariateDistribution
      else
        raise CCSError, :CCS_INVALID_DISTRIBUTION
      end.new(handle, retain: retain, auto_release: auto_release)
    end

    def data_types
      @data_types ||= begin
        ptr = MemoryPointer::new(:ccs_numeric_type_t, dimension)
        res = CCS.ccs_distribution_get_data_types(@handle, ptr)
        CCS.error_check(res)
        ptr.read_array_of_int32(dimension).collect { |i| NumericType.from_native(i, nil) }
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
      dim = dimension
      ptr = MemoryPointer::new(:ccs_numeric_t, dim)
      res = CCS.ccs_distribution_sample(@handle, rng, ptr)
      CCS.error_check(res)
      if dim == 1
        if data_types.first == :CCS_NUM_FLOAT
          ptr.read_ccs_float_t
        else
          ptr.read_ccs_int_t
        end
      else
        data_types.each_with_index.collect { |t, i|
          if t == :CCS_NUM_FLOAT
            ptr.get_ccs_float_t(i*8)
          else
            ptr.get_ccs_int_t(i*8)
          end
        }
      end
    end

    def samples(rng, count)
      return [] if count == 0
      dim = dimension
      ptr = MemoryPointer::new(:ccs_numeric_t, count*dim)
      res = CCS.ccs_distribution_samples(@handle, rng, count, ptr)
      CCS.error_check(res)
      if dim == 1
        if data_types.first == :CCS_NUM_FLOAT
          ptr.read_array_of_ccs_float_t(count)
        else
          ptr.read_array_of_ccs_int_t(count)
        end
      else
        sz = CCS.find_type(:ccs_numeric_t).size
        count.times.collect { |j|
          data_types.each_with_index.collect { |t, i|
            if t == :CCS_NUM_FLOAT
              ptr.get_ccs_float_t((j*dim + i)*sz)
            else
              ptr.get_ccs_int_t((j*dim + i)*sz)
            end
          }
        }
      end
    end

  end

  attach_function :ccs_create_uniform_distribution, [:ccs_numeric_type_t, :ccs_numeric_t, :ccs_numeric_t, :ccs_scale_type_t, :ccs_numeric_t, :pointer], :ccs_result_t
  attach_function :ccs_create_uniform_int_distribution, [:ccs_int_t, :ccs_int_t, :ccs_scale_type_t, :ccs_int_t, :pointer], :ccs_result_t
  attach_function :ccs_create_uniform_float_distribution, [:ccs_float_t, :ccs_float_t, :ccs_scale_type_t, :ccs_float_t, :pointer], :ccs_result_t
  attach_function :ccs_uniform_distribution_get_parameters, [:ccs_distribution_t, :pointer, :pointer, :pointer, :pointer], :ccs_result_t

  class UniformDistribution < Distribution
    def initialize(handle = nil, retain: false, auto_release: true,
                   data_type: :CCS_NUM_FLOAT, lower: 0.0, upper: 1.0, scale: :CCS_LINEAR, quantization: 0.0)
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

    def data_type
      @data_type ||= data_types.first
    end

    def lower
      @lower ||= begin
        ptr = MemoryPointer::new(:ccs_numeric_t)
        res = CCS.ccs_uniform_distribution_get_parameters(@handle, ptr, nil, nil, nil)
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
        res = CCS.ccs_uniform_distribution_get_parameters(@handle, nil, ptr, nil, nil)
        CCS.error_check(res)
        if data_type == :CCS_NUM_FLOAT
          ptr.read_ccs_float_t
        else
          ptr.read_ccs_int_t
        end
      end
    end

    def scale
      @scale ||= begin
        ptr = MemoryPointer::new(:ccs_scale_type_t)
        res = CCS.ccs_uniform_distribution_get_parameters(@handle, nil, nil, ptr, nil)
        CCS.error_check(res)
        ptr.read_ccs_scale_type_t
      end
    end

    def quantization
      @quantization ||= begin
        ptr = MemoryPointer::new(:ccs_numeric_t)
        res = CCS.ccs_uniform_distribution_get_parameters(@handle, nil, nil, nil, ptr)
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
  attach_function :ccs_normal_distribution_get_parameters, [:ccs_distribution_t, :pointer, :pointer, :pointer, :pointer], :ccs_result_t
  class NormalDistribution < Distribution
    def initialize(handle = nil, retain: false, auto_release: true,
                   data_type: :CCS_NUM_FLOAT, mu: 0.0, sigma: 1.0, scale: :CCS_LINEAR, quantization: 0.0)
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

    def data_type
      @data_type ||= data_types.first
    end

    def mu
      @mu ||= begin
        ptr = MemoryPointer::new(:ccs_numeric_t)
        res = CCS.ccs_normal_distribution_get_parameters(@handle, ptr, nil, nil, nil)
        CCS.error_check(res)
        ptr.read_ccs_float_t
      end
    end

    def sigma
      @sigma ||= begin
        ptr = MemoryPointer::new(:ccs_numeric_t)
        res = CCS.ccs_normal_distribution_get_parameters(@handle, nil, ptr, nil, nil)
        CCS.error_check(res)
        ptr.read_ccs_float_t
      end
    end

    def scale
      @scale ||= begin
        ptr = MemoryPointer::new(:ccs_scale_type_t)
        res = CCS.ccs_normal_distribution_get_parameters(@handle, nil, nil, ptr, nil)
        CCS.error_check(res)
        ptr.read_ccs_scale_type_t
      end
    end

    def quantization
      @quantization ||= begin
        ptr = MemoryPointer::new(:ccs_numeric_t)
        res = CCS.ccs_normal_distribution_get_parameters(@handle, nil, nil, nil, ptr)
        CCS.error_check(res)
        if data_type == :CCS_NUM_FLOAT
          ptr.read_ccs_float_t
        else
          ptr.read_ccs_int_t
        end
      end
    end
  end

  attach_function :ccs_create_roulette_distribution, [:size_t, :pointer, :pointer], :ccs_result_t
  attach_function :ccs_roulette_distribution_get_num_areas, [:ccs_distribution_t, :pointer], :ccs_result_t
  attach_function :ccs_roulette_distribution_get_areas, [:ccs_distribution_t, :size_t, :pointer, :pointer], :ccs_result_t
  class RouletteDistribution < Distribution
    add_property :num_areas, :size_t, :ccs_roulette_distribution_get_num_areas, memoize: true
    def initialize(handle = nil, retain: false, auto_release: true,
                   areas: [])
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

    def data_type
      @data_type ||= data_types.first
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

  attach_function :ccs_create_mixture_distribution, [:size_t, :pointer, :pointer, :pointer], :ccs_result_t
  attach_function :ccs_mixture_distribution_get_num_distributions, [:ccs_distribution_t, :pointer], :ccs_result_t
  attach_function :ccs_mixture_distribution_get_distributions, [:ccs_distribution_t, :size_t, :pointer, :pointer], :ccs_result_t
  attach_function :ccs_mixture_distribution_get_weights, [:ccs_distribution_t, :size_t, :pointer, :pointer], :ccs_result_t
  class MixtureDistribution < Distribution
    add_property :num_distributions, :size_t, :ccs_mixture_distribution_get_num_distributions, memoize: true
    def initialize(handle = nil, retain: false, auto_release: true,
                   distributions: [], weights: nil)
      if handle
        super(handle, retain: retain)
      else
        ptr = MemoryPointer::new(:ccs_distribution_t)
        p_distributions = MemoryPointer::new(:ccs_distribution_t, distributions.length)
        if weights
          raise CCSError, :CCS_INVALID_VALUE if distributions.length != weights.length
        else
          weights = [1.0]*distributions.length
        end
        p_weights = MemoryPointer::new(:ccs_float_t, weights.length)
        p_distributions.write_array_of_pointer(distributions.collect(&:handle))
        p_weights.write_array_of_ccs_float_t(weights)
        res = CCS.ccs_create_mixture_distribution(distributions.length, p_distributions, p_weights, ptr)
        CCS.error_check(res)
        super(ptr.read_pointer, retain: false)
      end
    end

    def weights
      @weights ||= begin
        count = num_distributions
        ptr = MemoryPointer::new(:ccs_float_t, count)
        res = CCS.ccs_mixture_distribution_get_weights(@handle, count, ptr, nil)
        CCS.error_check(res)
        ptr.read_array_of_ccs_float_t(count)
      end
    end

    def distributions
      @distributions ||= begin
        count = num_distributions
        ptr = MemoryPointer::new(:ccs_distribution_t, count)
        res = CCS.ccs_mixture_distribution_get_distributions(@handle, count, ptr, nil)
        CCS.error_check(res)
        count.times.collect { |i| Distribution.from_handle(ptr[i].read_pointer) }
      end
    end
  end

  attach_function :ccs_create_multivariate_distribution, [:size_t, :pointer, :pointer], :ccs_result_t
  attach_function :ccs_multivariate_distribution_get_num_distributions, [:ccs_distribution_t, :pointer], :ccs_result_t
  attach_function :ccs_multivariate_distribution_get_distributions, [:ccs_distribution_t, :size_t, :pointer, :pointer], :ccs_result_t
  class MultivariateDistribution < Distribution
    add_property :num_distributions, :size_t, :ccs_multivariate_distribution_get_num_distributions, memoize: true
    def initialize(handle = nil, retain: false, auto_release: true,
                   distributions: [])
      if handle
        super(handle, retain: retain)
      else
        ptr = MemoryPointer::new(:ccs_distribution_t)
        p_distributions = MemoryPointer::new(:ccs_distribution_t, distributions.length)
        p_distributions.write_array_of_pointer(distributions.collect(&:handle))
        res = CCS.ccs_create_multivariate_distribution(distributions.length, p_distributions, ptr)
        CCS.error_check(res)
        super(ptr.read_pointer, retain: false)
      end
    end

    def distributions
      @distributions ||= begin
        count = num_distributions
        ptr = MemoryPointer::new(:ccs_distribution_t, count)
        res = CCS.ccs_multivariate_distribution_get_distributions(@handle, count, ptr, nil)
        CCS.error_check(res)
        count.times.collect { |i| Distribution.from_handle(ptr[i].read_pointer) }
      end
    end
  end

end
