module CCS
  attach_function :ccs_create_configuration_space, [:string, :pointer, :pointer], :ccs_result_t
  attach_function :ccs_configuration_space_get_name, [:ccs_configuration_space_t, :pointer], :ccs_result_t
  attach_function :ccs_configuration_space_get_user_data, [:ccs_configuration_space_t, :pointer], :ccs_result_t
  attach_function :ccs_configuration_space_set_rng, [:ccs_configuration_space_t, :ccs_rng_t], :ccs_result_t
  attach_function :ccs_configuration_space_get_rng, [:ccs_configuration_space_t, :pointer], :ccs_result_t
  attach_function :ccs_configuration_space_add_hyperparameter, [:ccs_configuration_space_t, :ccs_hyperparameter_t, :ccs_distribution_t], :ccs_result_t
  attach_function :ccs_configuration_space_add_hyperparameters, [:ccs_configuration_space_t, :size_t, :pointer, :pointer], :ccs_result_t
  attach_function :ccs_configuration_space_get_num_hyperparameters, [:ccs_configuration_space_t, :pointer], :ccs_result_t
  attach_function :ccs_configuration_space_get_hyperparameter, [:ccs_configuration_space_t, :size_t, :pointer], :ccs_result_t
  attach_function :ccs_configuration_space_get_hyperparameter_by_name, [:ccs_configuration_space_t, :string, :pointer], :ccs_result_t
  attach_function :ccs_configuration_space_get_hyperparameter_index_by_name, [:ccs_configuration_space_t, :string, :pointer], :ccs_result_t
  attach_function :ccs_configuration_space_get_hyperparameter_index, [:ccs_configuration_space_t, :ccs_hyperparameter_t, :pointer], :ccs_result_t
  attach_function :ccs_configuration_space_get_hyperparameter_indexes, [:ccs_configuration_space_t, :size_t, :pointer, :pointer], :ccs_result_t
  attach_function :ccs_configuration_space_get_hyperparameters, [:ccs_configuration_space_t, :size_t, :pointer, :pointer], :ccs_result_t
  attach_function :ccs_configuration_space_set_condition, [:ccs_configuration_space_t, :size_t, :ccs_expression_t], :ccs_result_t
  attach_function :ccs_configuration_space_get_condition, [:ccs_configuration_space_t, :size_t, :pointer], :ccs_result_t
  attach_function :ccs_configuration_space_get_conditions, [:ccs_configuration_space_t, :size_t, :pointer, :pointer], :ccs_result_t
  attach_function :ccs_configuration_space_add_forbidden_clause, [:ccs_configuration_space_t, :ccs_expression_t], :ccs_result_t
  attach_function :ccs_configuration_space_add_forbidden_clauses, [:ccs_configuration_space_t, :size_t, :ccs_expression_t], :ccs_result_t
  attach_function :ccs_configuration_space_get_forbidden_clause, [:ccs_configuration_space_t, :size_t, :pointer], :ccs_result_t
  attach_function :ccs_configuration_space_get_forbidden_clauses, [:ccs_configuration_space_t, :size_t, :pointer, :pointer], :ccs_result_t
  attach_function :ccs_configuration_space_check_configuration, [:ccs_configuration_space_t, :ccs_configuration_t], :ccs_result_t
  attach_function :ccs_configuration_space_check_configuration_values, [:ccs_configuration_space_t, :size_t, :pointer], :ccs_result_t
  attach_function :ccs_configuration_space_get_default_configuration, [:ccs_configuration_space_t, :pointer], :ccs_result_t
  attach_function :ccs_configuration_space_sample, [:ccs_configuration_space_t, :ccs_configuration_t], :ccs_result_t
  attach_function :ccs_configuration_space_samples, [:ccs_configuration_space_t, :size_t, :ccs_configuration_t], :ccs_result_t

  class ConfigurationSpace < Context
    add_property :user_data, :pointer, :ccs_configuration_space_get_user_data, memoize: true
    add_property :num_hyperparameters, :size_t, :ccs_configuration_space_get_num_hyperparameters, memoize: false

    def initialize(handle = nil, retain: false, name: "", user_data: nil)
      if handle
        super(handle, retain: retain)
      else
        ptr = MemoryPointer::new(:ccs_configuration_space_t)
        res = CCS.ccs_create_configuration_space(name, user_data, ptr)
        CCS.error_check(res)
        super(ptr.read_ccs_configuration_space_t, retain:false)
      end
    end

    def self.from_handle(handle)
      self::new(handle, retain: true)
    end

    def name
      @name ||= begin
        ptr = MemoryPointer::new(:pointer)
        res = CCS.ccs_configuration_space_get_name(@handle, ptr)
        CCS.error_check(res)
        ptr.read_pointer.read_string
      end
    end

    def rng
      ptr = MemoryPointer::new(:ccs_rng_t)
      res = CCS.ccs_configuration_space_get_rng(@handle, ptr)
      CCS.error_check(res)
      Rng::from_handle(ptr.read_ccs_rng_t)
    end

    def rng=(r)
      res = CCS.ccs_configuration_space_set_rng(@handle, r)
      CCS.error_check(res)
      r
    end

    def add_hyperparameter(hyperparameter, distribution: nil)
      res = CCS.ccs_configuration_space_add_hyperparameter(@handle, hyperparameter, distribution)
      CCS.error_check(res)
      self
    end

    def add_hyperparameters(hyperparameters, distributions: nil)
      count = hyperparameters.size
      return self if count == 0
      if distributions
        raise CCSError, :CCS_INVALID_VALUE if count != distributions.size
        p_dists = MemoryPointer::new(:ccs_distribution_t, count)
        p_dists.write_array_of_pointer(distributions.collect(&:handle))
        distributions = p_dist
      end
      p_hypers = MemoryPointer::new(:ccs_hyperparameter_t, count)
      p_hypers.write_array_of_pointer(hyperparameters.collect(&:handle))
      res = CCS.ccs_configuration_space_add_hyperparameters(@handle, count, p_hypers, distributions)
      CCS.error_check(res)
      self
    end

    def hyperparameter(index)
      ptr = MemoryPointer::new(:ccs_hyperparameter_t)
      res = CCS.ccs_configuration_space_get_hyperparameter(@handle, index, ptr)
      CCS.error_check(res)
      Hyperparameter.from_handle(ptr.read_ccs_hyperparameter_t)
    end

    def hyperparameter_by_name(name)
      ptr = MemoryPointer::new(:ccs_hyperparameter_t)
      res = CCS.ccs_configuration_space_get_hyperparameter_by_name(@handle, name, ptr)
      CCS.error_check(res)
      Hyperparameter.from_handle(ptr.read_ccs_hyperparameter_t)
    end

    def hyperparameter_index(hyperparameter)
      ptr = MemoryPointer::new(:size_t)
      res = CCS.ccs_configuration_space_get_hyperparameter_index(@handle, hyperparameter, ptr)
      CCS.error_check(res)
      ptr.read_size_t
    end

    def hyperparameter_index_by_name(name)
      ptr = MemoryPointer::new(:size_t)
      res = CCS.ccs_configuration_space_get_hyperparameter_index_by_name(@handle, name, ptr)
      CCS.error_check(res)
      ptr.read_size_t
    end

    def hyperparameters
      count = num_hyperparameters
      return [] if count == 0
      ptr = MemoryPointer::new(:ccs_hyperparameter_t, count)
      res = CCS.ccs_configuration_space_get_hyperparameters(@handle, count, ptr, nil)
      CCS.error_check(res)
      count.times.collect { |i| Hyperparameter.from_handle(ptr[i].read_pointer) }
    end

    def set_condition(hyperparameter, expression)
      if expression.kind_of? String
        expression = ExpressionParser::new(self).parse(expression)
      end
      case hyperparameter
      when Hyperparameter
        hyperparameter = hyperparameter_index(hyperparameter);
      when String
        hyperparameter = hyperparameter_index_by_name(hyperparameter);
      end
      res = CCS.ccs_configuration_space_set_condition(@handle, hyperparameter, expression)
      CCS.error_check(res)
      self
    end

    def condition(hyperparameter)
      case hyperparameter
      when Hyperparameter
        hyperparameter = hyperparameter_index(hyperparameter);
      when String
        hyperparameter = hyperparameter_index_by_name(hyperparameter);
      end
      ptr = MemoryPointer::new(:ccs_expression_t)
      res = CCS.ccs_configuration_space_get_condition(@handle, hyperparameter, ptr)
      CCS.error_check(res)
      handle = ptr.read_ccs_expression_t
      handle.null? ? nil : Expression.from_handle(handle)
    end

    def conditions
      count = num_hyperparameters
      ptr = MemoryPointer::new(:ccs_expression_t, count)
      res = CCS.ccs_configuration_space_get_conditions(@handle, count, ptr, nil)
      CCS.error_check(res)
      ptr.read_array_of_pointer(count).collect { |handle|
        handle.null? ? nil : Expression.from_handle(handle)
      }
    end

    def add_forbidden_clause(expression)
      if expression.kind_of? String
        expression = ExpressionParser::new(self).parse(expression)
      end
      res = CCS.ccs_configuration_space_add_forbidden_clause(@handle, expression)
      CCS.error_check(res)
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
      res = CCS.ccs_configuration_space_add_forbidden_clauses(@handle, count, ptr)
      CCS.error_check(res)
      self
    end

    def forbidden_clause(index)
      ptr = MemoryPointer::new(:ccs_expression_t)
      res = CCS.ccs_configuration_space_get_forbidden_clause(@handle, index, ptr)
      CCS.error_check(res)
      Expression.from_handle(ptr.read_ccs_expression_t)
    end

    def forbidden_clauses
      ptr = MemoryPointer::new(:size_t)
      res = CCS.ccs_configuration_space_get_forbidden_clauses(@handle, 0, nil, ptr)
      CCS.error_check(res)
      count = ptr.read_size_t
      ptr = MemoryPointer::new(:ccs_expression_t, count)
      res = CCS.ccs_configuration_space_get_forbidden_clauses(@handle, count, ptr, nil)
      CCS.error_check(res)
      count.times.collect { |i| Expression::from_handle(ptr[i].read_pointer) }
    end

    def check(configuration)
      res = CCS.ccs_configuration_space_check_configuration(@handle, configuration)
      CCS.error_check(res)
      self
    end

    def check_values(values)
      count = values.size
      raise CCSError, :CCS_INVALID_VALUE if count != num_hyperparameters
      ptr = MemoryPointer::new(:ccs_datum_t, count)
      values.each_with_index {  |v, i| Datum::new(ptr[i]).value = v }
      res = CCS.ccs_configuration_space_check_configuration_values(@handle, count, ptr)
      CCS.error_check(res)
      self
    end

    def default_configuration
      ptr = MemoryPointer::new(:ccs_configuration_t)
      res = CCS.ccs_configuration_space_get_default_configuration(@handle, ptr)
      CCS.error_check(res)
      Configuration::new(ptr.read_ccs_configuration_t, retain: false)
    end

    def sample
      ptr = MemoryPointer::new(:ccs_configuration_t)
      res = CCS.ccs_configuration_space_sample(@handle, ptr)
      CCS.error_check(res)
      Configuration::new(ptr.read_ccs_configuration_t, retain: false)
    end

    def samples(count)
      return [] if count == 0
      ptr = MemoryPointer::new(:ccs_configuration_t, count)
      res = CCS.ccs_configuration_space_samples(@handle, count, ptr)
      CCS.error_check(res)
      count.times.collect { |i| Configuration::new(ptr[i].read_pointer, retain: false) }
    end
  end

end
