module CCS
  attach_function :ccs_create_configuration_space, [:string, :pointer], :ccs_result_t
  attach_function :ccs_configuration_space_set_rng, [:ccs_configuration_space_t, :ccs_rng_t], :ccs_result_t
  attach_function :ccs_configuration_space_get_rng, [:ccs_configuration_space_t, :pointer], :ccs_result_t
  attach_function :ccs_configuration_space_add_parameter, [:ccs_configuration_space_t, :ccs_parameter_t, :ccs_distribution_t], :ccs_result_t
  attach_function :ccs_configuration_space_add_parameters, [:ccs_configuration_space_t, :size_t, :pointer, :pointer], :ccs_result_t
  attach_function :ccs_configuration_space_set_distribution, [:ccs_configuration_space_t, :ccs_distribution_t, :pointer], :ccs_result_t
  attach_function :ccs_configuration_space_get_parameter_distribution, [:ccs_configuration_space_t, :size_t, :pointer, :pointer], :ccs_result_t
  attach_function :ccs_configuration_space_set_condition, [:ccs_configuration_space_t, :size_t, :ccs_expression_t], :ccs_result_t
  attach_function :ccs_configuration_space_get_condition, [:ccs_configuration_space_t, :size_t, :pointer], :ccs_result_t
  attach_function :ccs_configuration_space_get_conditions, [:ccs_configuration_space_t, :size_t, :pointer, :pointer], :ccs_result_t
  attach_function :ccs_configuration_space_add_forbidden_clause, [:ccs_configuration_space_t, :ccs_expression_t], :ccs_result_t
  attach_function :ccs_configuration_space_add_forbidden_clauses, [:ccs_configuration_space_t, :size_t, :ccs_expression_t], :ccs_result_t
  attach_function :ccs_configuration_space_get_forbidden_clause, [:ccs_configuration_space_t, :size_t, :pointer], :ccs_result_t
  attach_function :ccs_configuration_space_get_forbidden_clauses, [:ccs_configuration_space_t, :size_t, :pointer, :pointer], :ccs_result_t
  attach_function :ccs_configuration_space_check_configuration, [:ccs_configuration_space_t, :ccs_configuration_t, :pointer], :ccs_result_t
  attach_function :ccs_configuration_space_check_configuration_values, [:ccs_configuration_space_t, :size_t, :pointer, :pointer], :ccs_result_t
  attach_function :ccs_configuration_space_get_default_configuration, [:ccs_configuration_space_t, :pointer], :ccs_result_t
  attach_function :ccs_configuration_space_sample, [:ccs_configuration_space_t, :pointer], :ccs_result_t
  attach_function :ccs_configuration_space_samples, [:ccs_configuration_space_t, :size_t, :pointer], :ccs_result_t

  class ConfigurationSpace < Context

    def initialize(handle = nil, retain: false, auto_release: true,
                   name: "")
      if handle
        super(handle, retain: retain, auto_release: auto_release)
      else
        ptr = MemoryPointer::new(:ccs_configuration_space_t)
        CCS.error_check CCS.ccs_create_configuration_space(name, ptr)
        super(ptr.read_ccs_configuration_space_t, retain:false)
      end
    end

    def self.from_handle(handle, retain: true, auto_release: true)
      self.new(handle, retain: retain, auto_release: auto_release)
    end

    def rng
      ptr = MemoryPointer::new(:ccs_rng_t)
      CCS.error_check CCS.ccs_configuration_space_get_rng(@handle, ptr)
      Rng::from_handle(ptr.read_ccs_rng_t)
    end

    def rng=(r)
      CCS.error_check CCS.ccs_configuration_space_set_rng(@handle, r)
      r
    end

    def add_parameter(parameter, distribution: nil)
      CCS.error_check CCS.ccs_configuration_space_add_parameter(@handle, parameter, distribution)
      self
    end

    def set_distribution(distribution, parameters )
      count = distribution.dimension
      raise CCSError, :CCS_INVALID_VALUE if count != parameters.size
      parameters = parameters.collect { |h|
        case h
        when Parameter
          parameter_index(h)
        when String, Symbol
          parameter_index_by_name(parameter)
        else
          h
        end
      }
      p_parameters = MemoryPointer::new(:size_t, count)
      p_parameters.write_array_of_size_t(parameters)
      CCS.error_check CCS.ccs_configuration_space_set_distribution(@handle, distribution, p_parameters)
      self
    end

    def get_parameter_distribution(parameter)
      case parameter
      when Parameter
        parameter = parameter_index(parameter);
      when String, Symbol
        parameter = parameter_index_by_name(parameter);
      end
      p_distribution = MemoryPointer::new(:ccs_distribution_t)
      p_indx = MemoryPointer::new(:size_t)
      CCS.error_check CCS.ccs_configuration_space_get_parameter_distribution(@handle, parameter, p_distribution, p_indx)
      [CCS::Distribution.from_handle(p_distribution.read_ccs_distribution_t), p_indx.read_size_t]
    end

    def add_parameters(parameters, distributions: nil)
      count = parameters.size
      return self if count == 0
      if distributions
        raise CCSError, :CCS_INVALID_VALUE if count != distributions.size
        p_dists = MemoryPointer::new(:ccs_distribution_t, count)
        p_dists.write_array_of_pointer(distributions.collect(&:handle))
      else
        p_dists = nil
      end
      p_parameters = MemoryPointer::new(:ccs_parameter_t, count)
      p_parameters.write_array_of_pointer(parameters.collect(&:handle))
      CCS.error_check CCS.ccs_configuration_space_add_parameters(@handle, count, p_parameters, p_dists)
      self
    end

    def set_condition(parameter, expression)
      if expression.kind_of? String
        expression = ExpressionParser::new(self).parse(expression)
      end
      case parameter
      when Parameter
        parameter = parameter_index(parameter);
      when String, Symbol
        parameter = parameter_index_by_name(parameter);
      end
      CCS.error_check CCS.ccs_configuration_space_set_condition(@handle, parameter, expression)
      self
    end

    def condition(parameter)
      case parameter
      when Parameter
        parameter = parameter_index(parameter);
      when String, Symbol
        parameter = parameter_index_by_name(parameter);
      end
      ptr = MemoryPointer::new(:ccs_expression_t)
      CCS.error_check CCS.ccs_configuration_space_get_condition(@handle, parameter, ptr)
      handle = ptr.read_ccs_expression_t
      handle.null? ? nil : Expression.from_handle(handle)
    end

    def conditions
      count = num_parameters
      ptr = MemoryPointer::new(:ccs_expression_t, count)
      CCS.error_check CCS.ccs_configuration_space_get_conditions(@handle, count, ptr, nil)
      ptr.read_array_of_pointer(count).collect { |handle|
        handle.null? ? nil : Expression.from_handle(handle)
      }
    end

    def conditional_parameters
      hps = parameters
      conds = conditions
      hps.each_with_index.select { |h, i| conds[i] }.collect { |h, i| h }.to_a
    end

    def unconditional_parameters
      hps = parameters
      conds = conditions
      hps.each_with_index.select { |h, i| !conds[i] }.collect { |h, i| h }.to_a
    end

    def add_forbidden_clause(expression)
      if expression.kind_of? String
        expression = ExpressionParser::new(self).parse(expression)
      end
      CCS.error_check CCS.ccs_configuration_space_add_forbidden_clause(@handle, expression)
      self
    end

    def add_forbidden_clauses(expressions)
      expressions = expressions.collect { |e|
        p = ExpressionParser::new(self)
        if e.kind_of? String
          e = p.parse(e)
        else
          e
        end
      }
      count = expressions.size
      return self if count == 0
      ptr = MemoryPointer::new(:ccs_expression_t, count)
      ptr.write_array_of_pointer(expressions.collect(&:handle))
      CCS.error_check CCS.ccs_configuration_space_add_forbidden_clauses(@handle, count, ptr)
      self
    end

    def forbidden_clause(index)
      ptr = MemoryPointer::new(:ccs_expression_t)
      CCS.error_check CCS.ccs_configuration_space_get_forbidden_clause(@handle, index, ptr)
      Expression.from_handle(ptr.read_ccs_expression_t)
    end

    def num_forbidden_clauses
      ptr = MemoryPointer::new(:size_t)
      CCS.error_check CCS.ccs_configuration_space_get_forbidden_clauses(@handle, 0, nil, ptr)
      ptr.read_size_t
    end

    def forbidden_clauses
      count = num_forbidden_clauses
      ptr = MemoryPointer::new(:ccs_expression_t, count)
      CCS.error_check CCS.ccs_configuration_space_get_forbidden_clauses(@handle, count, ptr, nil)
      count.times.collect { |i| Expression::from_handle(ptr[i].read_pointer) }
    end

    def check(configuration)
      ptr = MemoryPointer::new(:ccs_bool_t)
      CCS.error_check CCS.ccs_configuration_space_check_configuration(@handle, configuration, ptr)
      return ptr.read_ccs_bool_t == CCS::FALSE ? false : true
    end

    def check_values(values)
      count = values.size
      raise CCSError, :CCS_INVALID_VALUE if count != num_parameters
      ss = []
      ptr = MemoryPointer::new(:ccs_datum_t, count)
      values.each_with_index {  |v, i| Datum::new(ptr[i]).set_value(v, string_store: ss) }
      ptr2 = MemoryPointer::new(:ccs_bool_t)
      CCS.error_check CCS.ccs_configuration_space_check_configuration_values(@handle, count, ptr, ptr2)
      return ptr2.read_ccs_bool_t == CCS::FALSE ? false : true
    end

    def default_configuration
      ptr = MemoryPointer::new(:ccs_configuration_t)
      CCS.error_check CCS.ccs_configuration_space_get_default_configuration(@handle, ptr)
      Configuration::new(ptr.read_ccs_configuration_t, retain: false)
    end

    def sample
      ptr = MemoryPointer::new(:ccs_configuration_t)
      CCS.error_check CCS.ccs_configuration_space_sample(@handle, ptr)
      Configuration::new(ptr.read_ccs_configuration_t, retain: false)
    end

    def samples(count)
      return [] if count == 0
      ptr = MemoryPointer::new(:ccs_configuration_t, count)
      CCS.error_check CCS.ccs_configuration_space_samples(@handle, count, ptr)
      count.times.collect { |i| Configuration::new(ptr[i].read_pointer, retain: false) }
    end
  end
end
