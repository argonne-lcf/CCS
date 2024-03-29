module CCS

  DistributionType = enum FFI::Type::INT32, :ccs_distribution_type_t, [
    :CCS_DISTRIBUTION_TYPE_UNIFORM,
    :CCS_DISTRIBUTION_TYPE_NORMAL,
    :CCS_DISTRIBUTION_TYPE_ROULETTE,
    :CCS_DISTRIBUTION_TYPE_MIXTURE,
    :CCS_DISTRIBUTION_TYPE_MULTIVARIATE
  ]
  class MemoryPointer
    def read_ccs_distribution_type_t
      DistributionType.from_native(read_int32, nil)
    end
  end

  ScaleType = enum FFI::Type::INT32, :ccs_scale_type_t, [
    :CCS_SCALE_TYPE_LINEAR,
    :CCS_SCALE_TYPE_LOGARITHMIC
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
      CCS.error_check CCS.ccs_distribution_get_type(handle, ptr)
      case ptr.read_ccs_distribution_type_t
      when :CCS_DISTRIBUTION_TYPE_UNIFORM
        return UniformDistribution.from_handle(handle, retain: retain, auto_release: auto_release)
      when :CCS_DISTRIBUTION_TYPE_NORMAL
        return NormalDistribution.from_handle(handle, retain: retain, auto_release: auto_release)
      when :CCS_DISTRIBUTION_TYPE_ROULETTE
        RouletteDistribution
      when :CCS_DISTRIBUTION_TYPE_MIXTURE
        MixtureDistribution
      when :CCS_DISTRIBUTION_TYPE_MULTIVARIATE
        MultivariateDistribution
      else
        raise CCSError, :CCS_RESULT_ERROR_INVALID_DISTRIBUTION
      end.new(handle, retain: retain, auto_release: auto_release)
    end

    def data_types
      @data_types ||= begin
        ptr = MemoryPointer::new(:ccs_numeric_type_t, dimension)
        CCS.error_check CCS.ccs_distribution_get_data_types(@handle, ptr)
        ptr.read_array_of_int32(dimension).collect { |i| NumericType.from_native(i, nil) }
      end
    end

    def bounds
      @bounds ||= begin
        interval = Interval::new(type: :CCS_NUMERIC_TYPE_FLOAT)
        CCS.error_check CCS.ccs_distribution_get_bounds(@handle, interval)
        interval
      end
    end

    def oversampling?(interval)
      ptr = MemoryPointer::new(:ccs_bool_t)
      CCS.error_check CCS.ccs_distribution_check_oversampling(@handle, interval, ptr)
      ptr.read_ccs_bool_t == CCS::FALSE ? false : true
    end

    def sample(rng)
      dim = dimension
      ptr = MemoryPointer::new(:ccs_numeric_t, dim)
      CCS.error_check CCS.ccs_distribution_sample(@handle, rng, ptr)
      if dim == 1
        if data_types.first == :CCS_NUMERIC_TYPE_FLOAT
          ptr.read_ccs_float_t
        else
          ptr.read_ccs_int_t
        end
      else
        data_types.each_with_index.collect { |t, i|
          if t == :CCS_NUMERIC_TYPE_FLOAT
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
      CCS.error_check CCS.ccs_distribution_samples(@handle, rng, count, ptr)
      if dim == 1
        if data_types.first == :CCS_NUMERIC_TYPE_FLOAT
          ptr.read_array_of_ccs_float_t(count)
        else
          ptr.read_array_of_ccs_int_t(count)
        end
      else
        sz = CCS.find_type(:ccs_numeric_t).size
        count.times.collect { |j|
          data_types.each_with_index.collect { |t, i|
            if t == :CCS_NUMERIC_TYPE_FLOAT
              ptr.get_ccs_float_t((j*dim + i)*sz)
            else
              ptr.get_ccs_int_t((j*dim + i)*sz)
            end
          }
        }
      end
    end

  end

  attach_function :ccs_create_uniform_int_distribution, [:ccs_int_t, :ccs_int_t, :ccs_scale_type_t, :ccs_int_t, :pointer], :ccs_result_t
  attach_function :ccs_create_uniform_float_distribution, [:ccs_float_t, :ccs_float_t, :ccs_scale_type_t, :ccs_float_t, :pointer], :ccs_result_t
  attach_function :ccs_uniform_distribution_get_properties, [:ccs_distribution_t, :pointer, :pointer, :pointer, :pointer], :ccs_result_t

  class UniformDistribution < Distribution

    def self.from_handle(handle, retain: true, auto_release: true)
      ptr = MemoryPointer::new(:ccs_numeric_type_t)
      CCS.error_check CCS.ccs_distribution_get_data_types(handle, ptr)
      case ptr.read_ccs_numeric_type_t
      when :CCS_NUMERIC_TYPE_FLOAT
        Float
      when :CCS_NUMERIC_TYPE_INT
        Int
      else
        raise CCSError, :CCS_RESULT_ERROR_INVALID_DISTRIBUTION
      end.new(handle, retain: retain, auto_release: auto_release)
    end

    class Float < UniformDistribution

      def initialize(handle = nil, retain: false, auto_release: true,
                     lower: 0.0, upper: 1.0, scale: :CCS_SCALE_TYPE_LINEAR, quantization: 0.0)
        if handle
          super(handle, retain: retain, auto_release: auto_release)
        else
          ptr = MemoryPointer::new(:ccs_distribution_t)
          CCS.error_check CCS.ccs_create_uniform_float_distribution(lower, upper, scale, quantization, ptr)
          super(ptr.read_pointer, retain: false)
        end
      end

    end

    class Int < UniformDistribution

      def initialize(handle = nil, retain: false, auto_release: true,
                     lower: 0, upper: 100, scale: :CCS_SCALE_TYPE_LINEAR, quantization: 0)
        if handle
          super(handle, retain: retain, auto_release: auto_release)
        else
          ptr = MemoryPointer::new(:ccs_distribution_t)
          CCS.error_check CCS.ccs_create_uniform_int_distribution(lower, upper, scale, quantization, ptr)
          super(ptr.read_pointer, retain: false)
        end
      end

    end

    def data_type
      @data_type ||= data_types.first
    end

    def lower
      @lower ||= begin
        ptr = MemoryPointer::new(:ccs_numeric_t)
        CCS.error_check CCS.ccs_uniform_distribution_get_properties(@handle, ptr, nil, nil, nil)
        if data_type == :CCS_NUMERIC_TYPE_FLOAT
          ptr.read_ccs_float_t
        else
          ptr.read_ccs_int_t
        end
      end
    end

    def upper
      @upper ||= begin
        ptr = MemoryPointer::new(:ccs_numeric_t)
        CCS.error_check CCS.ccs_uniform_distribution_get_properties(@handle, nil, ptr, nil, nil)
        if data_type == :CCS_NUMERIC_TYPE_FLOAT
          ptr.read_ccs_float_t
        else
          ptr.read_ccs_int_t
        end
      end
    end

    def scale
      @scale ||= begin
        ptr = MemoryPointer::new(:ccs_scale_type_t)
        CCS.error_check CCS.ccs_uniform_distribution_get_properties(@handle, nil, nil, ptr, nil)
        ptr.read_ccs_scale_type_t
      end
    end

    def quantization
      @quantization ||= begin
        ptr = MemoryPointer::new(:ccs_numeric_t)
        CCS.error_check CCS.ccs_uniform_distribution_get_properties(@handle, nil, nil, nil, ptr)
        if data_type == :CCS_NUMERIC_TYPE_FLOAT
          ptr.read_ccs_float_t
        else
          ptr.read_ccs_int_t
        end
      end
    end
  end

  Distribution::Uniform = UniformDistribution

  attach_function :ccs_create_normal_int_distribution, [:ccs_float_t, :ccs_float_t, :ccs_scale_type_t, :ccs_int_t, :pointer], :ccs_result_t
  attach_function :ccs_create_normal_float_distribution, [:ccs_float_t, :ccs_float_t, :ccs_scale_type_t, :ccs_float_t, :pointer], :ccs_result_t
  attach_function :ccs_normal_distribution_get_properties, [:ccs_distribution_t, :pointer, :pointer, :pointer, :pointer], :ccs_result_t

  class NormalDistribution < Distribution

    def self.from_handle(handle, retain: true, auto_release: true)
      ptr = MemoryPointer::new(:ccs_numeric_type_t)
      CCS.error_check CCS.ccs_distribution_get_data_types(handle, ptr)
      case ptr.read_ccs_numeric_type_t
      when :CCS_NUMERIC_TYPE_FLOAT
        Float
      when :CCS_NUMERIC_TYPE_INT
        Int
      else
        raise CCSError, :CCS_RESULT_ERROR_INVALID_DISTRIBUTION
      end.new(handle, retain: retain, auto_release: auto_release)
    end

    class Float < NormalDistribution

      def initialize(handle = nil, retain: false, auto_release: true,
                     mu: 0.0, sigma: 1.0, scale: :CCS_SCALE_TYPE_LINEAR, quantization: 0.0)
        if handle
          super(handle, retain: retain, auto_release: auto_release)
        else
          ptr = MemoryPointer::new(:ccs_distribution_t)
          CCS.error_check CCS.ccs_create_normal_float_distribution(mu, sigma, scale, quantization, ptr)
          super(ptr.read_pointer, retain: false)
        end
      end

    end

    class Int < NormalDistribution

      def initialize(handle = nil, retain: false, auto_release: true,
                     mu: 0.0, sigma: 1.0, scale: :CCS_SCALE_TYPE_LINEAR, quantization: 0)
        if handle
          super(handle, retain: retain, auto_release: auto_release)
        else
          ptr = MemoryPointer::new(:ccs_distribution_t)
          CCS.error_check CCS.ccs_create_normal_int_distribution(mu, sigma, scale, quantization, ptr)
          super(ptr.read_pointer, retain: false)
        end
      end

    end

    def data_type
      @data_type ||= data_types.first
    end

    def mu
      @mu ||= begin
        ptr = MemoryPointer::new(:ccs_numeric_t)
        CCS.error_check CCS.ccs_normal_distribution_get_properties(@handle, ptr, nil, nil, nil)
        ptr.read_ccs_float_t
      end
    end

    def sigma
      @sigma ||= begin
        ptr = MemoryPointer::new(:ccs_numeric_t)
        CCS.error_check CCS.ccs_normal_distribution_get_properties(@handle, nil, ptr, nil, nil)
        ptr.read_ccs_float_t
      end
    end

    def scale
      @scale ||= begin
        ptr = MemoryPointer::new(:ccs_scale_type_t)
        CCS.error_check CCS.ccs_normal_distribution_get_properties(@handle, nil, nil, ptr, nil)
        ptr.read_ccs_scale_type_t
      end
    end

    def quantization
      @quantization ||= begin
        ptr = MemoryPointer::new(:ccs_numeric_t)
        CCS.error_check CCS.ccs_normal_distribution_get_properties(@handle, nil, nil, nil, ptr)
        if data_type == :CCS_NUMERIC_TYPE_FLOAT
          ptr.read_ccs_float_t
        else
          ptr.read_ccs_int_t
        end
      end
    end
  end

  Distribution::Normal = NormalDistribution

  attach_function :ccs_create_roulette_distribution, [:size_t, :pointer, :pointer], :ccs_result_t
  attach_function :ccs_roulette_distribution_get_num_areas, [:ccs_distribution_t, :pointer], :ccs_result_t
  attach_function :ccs_roulette_distribution_get_areas, [:ccs_distribution_t, :size_t, :pointer, :pointer], :ccs_result_t
  class RouletteDistribution < Distribution
    add_property :num_areas, :size_t, :ccs_roulette_distribution_get_num_areas, memoize: true
    def initialize(handle = nil, retain: false, auto_release: true,
                   areas: [])
      if handle
        super(handle, retain: retain, auto_release: auto_release)
      else
        ptr = MemoryPointer::new(:ccs_distribution_t)
        p_areas = MemoryPointer::new(:ccs_float_t, areas.size)
        p_areas.write_array_of_ccs_float_t(areas)
        CCS.error_check CCS.ccs_create_roulette_distribution(areas.size, p_areas, ptr)
        super(ptr.read_pointer, retain: false)
      end
    end

    def data_type
      @data_type ||= data_types.first
    end

    def areas
      count = num_areas
      ptr = MemoryPointer::new(:ccs_float_t, count)
      CCS.error_check CCS.ccs_roulette_distribution_get_areas(@handle, count, ptr, nil)
      ptr.read_array_of_ccs_float_t(count)
    end
  end

  Distribution::Roulette = RouletteDistribution

  attach_function :ccs_create_mixture_distribution, [:size_t, :pointer, :pointer, :pointer], :ccs_result_t
  attach_function :ccs_mixture_distribution_get_num_distributions, [:ccs_distribution_t, :pointer], :ccs_result_t
  attach_function :ccs_mixture_distribution_get_distributions, [:ccs_distribution_t, :size_t, :pointer, :pointer], :ccs_result_t
  attach_function :ccs_mixture_distribution_get_weights, [:ccs_distribution_t, :size_t, :pointer, :pointer], :ccs_result_t
  class MixtureDistribution < Distribution
    add_property :num_distributions, :size_t, :ccs_mixture_distribution_get_num_distributions, memoize: true
    def initialize(handle = nil, retain: false, auto_release: true,
                   distributions: [], weights: nil)
      if handle
        super(handle, retain: retain, auto_release: auto_release)
      else
        ptr = MemoryPointer::new(:ccs_distribution_t)
        p_distributions = MemoryPointer::new(:ccs_distribution_t, distributions.length)
        if weights
          raise CCSError, :CCS_RESULT_ERROR_INVALID_VALUE if distributions.length != weights.length
        else
          weights = [1.0]*distributions.length
        end
        p_weights = MemoryPointer::new(:ccs_float_t, weights.length)
        p_distributions.write_array_of_pointer(distributions.collect(&:handle))
        p_weights.write_array_of_ccs_float_t(weights)
        CCS.error_check CCS.ccs_create_mixture_distribution(distributions.length, p_distributions, p_weights, ptr)
        super(ptr.read_pointer, retain: false)
      end
    end

    def weights
      @weights ||= begin
        count = num_distributions
        ptr = MemoryPointer::new(:ccs_float_t, count)
        CCS.error_check CCS.ccs_mixture_distribution_get_weights(@handle, count, ptr, nil)
        ptr.read_array_of_ccs_float_t(count)
      end
    end

    def distributions
      @distributions ||= begin
        count = num_distributions
        ptr = MemoryPointer::new(:ccs_distribution_t, count)
        CCS.error_check CCS.ccs_mixture_distribution_get_distributions(@handle, count, ptr, nil)
        count.times.collect { |i| Distribution.from_handle(ptr[i].read_pointer) }
      end
    end
  end

  Distribution::Mixture = MixtureDistribution

  attach_function :ccs_create_multivariate_distribution, [:size_t, :pointer, :pointer], :ccs_result_t
  attach_function :ccs_multivariate_distribution_get_num_distributions, [:ccs_distribution_t, :pointer], :ccs_result_t
  attach_function :ccs_multivariate_distribution_get_distributions, [:ccs_distribution_t, :size_t, :pointer, :pointer], :ccs_result_t
  class MultivariateDistribution < Distribution
    add_property :num_distributions, :size_t, :ccs_multivariate_distribution_get_num_distributions, memoize: true
    def initialize(handle = nil, retain: false, auto_release: true,
                   distributions: [])
      if handle
        super(handle, retain: retain, auto_release: auto_release)
      else
        ptr = MemoryPointer::new(:ccs_distribution_t)
        p_distributions = MemoryPointer::new(:ccs_distribution_t, distributions.length)
        p_distributions.write_array_of_pointer(distributions.collect(&:handle))
        CCS.error_check CCS.ccs_create_multivariate_distribution(distributions.length, p_distributions, ptr)
        super(ptr.read_pointer, retain: false)
      end
    end

    def distributions
      @distributions ||= begin
        count = num_distributions
        ptr = MemoryPointer::new(:ccs_distribution_t, count)
        CCS.error_check CCS.ccs_multivariate_distribution_get_distributions(@handle, count, ptr, nil)
        count.times.collect { |i| Distribution.from_handle(ptr[i].read_pointer) }
      end
    end
  end

  Distribution::Multivariate = MultivariateDistribution

end
