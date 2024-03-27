module CCS

  attach_function :ccs_create_distribution_space, [:ccs_configuration_space_t, :pointer], :ccs_result_t
  attach_function :ccs_distribution_space_get_configuration_space, [:ccs_distribution_space_t, :pointer], :ccs_result_t
  attach_function :ccs_distribution_space_set_distribution, [:ccs_distribution_space_t, :ccs_distribution_t, :pointer], :ccs_result_t
  attach_function :ccs_distribution_space_get_parameter_distribution, [:ccs_distribution_space_t, :size_t, :pointer, :pointer], :ccs_result_t

  class DistributionSpace < Object
    add_handle_property :configuration_space, :ccs_configuration_space_t, :ccs_distribution_space_get_configuration_space, memoize: true

    def initialize(handle = nil, retain: false, auto_release: true,
                   name: "", configuration_space: nil)
      if handle
        super(handle, retain: retain, auto_release: auto_release)
      else
        ptr = MemoryPointer::new(:ccs_distribution_space_t)
        CCS.error_check CCS.ccs_create_distribution_space(configuration_space, ptr)
        super(ptr.read_ccs_distribution_space_t, retain:false)
      end
    end

    def self.from_handle(handle, retain: true, auto_release: true)
      self.new(handle, retain: retain, auto_release: auto_release)
    end

    def set_distribution(distribution, parameters)
      count = distribution.dimension
      raise CCSError, :CCS_RESULT_ERROR_INVALID_VALUE if count != parameters.size
      parameters = parameters.collect { |h|
        case h
        when Parameter
          configuration_space.parameter_index(h)
        when String, Symbol
          configuration_space.parameter_index_by_name(parameter)
        else
          h
        end
      }
      p_parameters = MemoryPointer::new(:size_t, count)
      p_parameters.write_array_of_size_t(parameters)
      CCS.error_check CCS.ccs_distribution_space_set_distribution(@handle, distribution, p_parameters)
      self
    end

    def get_parameter_distribution(parameter)
      case parameter
      when Parameter
        parameter = configuration_space.parameter_index(parameter);
      when String, Symbol
        parameter = configuration_space.parameter_index_by_name(parameter);
      end
      p_distribution = MemoryPointer::new(:ccs_distribution_t)
      p_indx = MemoryPointer::new(:size_t)
      CCS.error_check CCS.ccs_distribution_space_get_parameter_distribution(@handle, parameter, p_distribution, p_indx)
      [CCS::Distribution.from_handle(p_distribution.read_ccs_distribution_t), p_indx.read_size_t]
    end

  end

end
