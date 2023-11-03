module CCS
  attach_function :ccs_create_configuration_space, [:string, :size_t, :pointer, :pointer, :size_t, :pointer, :pointer], :ccs_result_t
  attach_function :ccs_configuration_space_set_rng, [:ccs_configuration_space_t, :ccs_rng_t], :ccs_result_t
  attach_function :ccs_configuration_space_get_rng, [:ccs_configuration_space_t, :pointer], :ccs_result_t
  attach_function :ccs_configuration_space_get_condition, [:ccs_configuration_space_t, :size_t, :pointer], :ccs_result_t
  attach_function :ccs_configuration_space_get_conditions, [:ccs_configuration_space_t, :size_t, :pointer, :pointer], :ccs_result_t
  attach_function :ccs_configuration_space_get_forbidden_clause, [:ccs_configuration_space_t, :size_t, :pointer], :ccs_result_t
  attach_function :ccs_configuration_space_get_forbidden_clauses, [:ccs_configuration_space_t, :size_t, :pointer, :pointer], :ccs_result_t
  attach_function :ccs_configuration_space_check_configuration, [:ccs_configuration_space_t, :ccs_configuration_t, :pointer], :ccs_result_t
  attach_function :ccs_configuration_space_check_configuration_values, [:ccs_configuration_space_t, :size_t, :pointer, :pointer], :ccs_result_t
  attach_function :ccs_configuration_space_get_default_configuration, [:ccs_configuration_space_t, :pointer], :ccs_result_t
  attach_function :ccs_configuration_space_sample, [:ccs_configuration_space_t, :ccs_distribution_space_t, :pointer], :ccs_result_t
  attach_function :ccs_configuration_space_samples, [:ccs_configuration_space_t, :ccs_distribution_space_t, :size_t, :pointer], :ccs_result_t

  class ConfigurationSpace < Context

    def initialize(handle = nil, retain: false, auto_release: true,
                   name: "", parameters: nil, conditions: nil, forbidden_clauses: nil)
      if handle
        super(handle, retain: retain, auto_release: auto_release)
      else
        count = parameters.size
        p_parameters = MemoryPointer::new(:ccs_parameter_t, count)
        p_parameters.write_array_of_pointer(parameters.collect(&:handle))
        ptr = MemoryPointer::new(:ccs_configuration_space_t)

        if forbidden_clauses
          ctx = parameters.map { |p| [p.name, p] }.to_h
          p = ExpressionParser::new(ctx)
          forbidden_clauses = forbidden_clauses.collect { |e| e.kind_of?(String) ? p.parse(e) : e }
          fccount = forbidden_clauses.size
          fcptr = MemoryPointer::new(:ccs_expression_t, fccount)
          fcptr.write_array_of_pointer(forbidden_clauses.collect(&:handle))
        else
          fccount = 0
          fcptr = nil
        end

        if conditions
          ctx = parameters.map { |p| [p.name, p] }.to_h
          indexdict = parameters.each_with_index.to_h
          p = ExpressionParser::new(ctx)
          conditions = conditions.transform_values { |v| v.kind_of?(String) ? p.parse(v) : v }
          cond_handles = [0]*count
          conditions.each do |k, v|
            index = case k
              when Parameter
                indexdict[k]
              when String, Symbol
                indexdict[ctx[k]]
              else
                k
              end
              cond_handles[index] = v.handle
          end
          cptr = MemoryPointer::new(:ccs_expression_t, count)
          cptr.write_array_of_pointer(cond_handles)
        else
          cptr = nil
        end

        CCS.error_check CCS.ccs_create_configuration_space(name, count, p_parameters, cptr, fccount, fcptr, ptr)
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

    def forbidden_clause(index)
      ptr = MemoryPointer::new(:ccs_expression_t)
      CCS.error_check CCS.ccs_configuration_space_get_forbidden_clause(@handle, index, ptr)
      Expression.from_handle(ptr.read_ccs_expression_t)
    end

    def num_forbidden_clauses
      @num_forbidden_clauses ||= begin
        ptr = MemoryPointer::new(:size_t)
        CCS.error_check CCS.ccs_configuration_space_get_forbidden_clauses(@handle, 0, nil, ptr)
        ptr.read_size_t
      end
    end

    def forbidden_clauses
      @forbidden_clauses ||= begin
        count = num_forbidden_clauses
        ptr = MemoryPointer::new(:ccs_expression_t, count)
        CCS.error_check CCS.ccs_configuration_space_get_forbidden_clauses(@handle, count, ptr, nil)
        count.times.collect { |i| Expression::from_handle(ptr[i].read_pointer) }.freeze
      end
    end

    def check(configuration)
      ptr = MemoryPointer::new(:ccs_bool_t)
      CCS.error_check CCS.ccs_configuration_space_check_configuration(@handle, configuration, ptr)
      return ptr.read_ccs_bool_t == CCS::FALSE ? false : true
    end

    def check_values(values)
      count = values.size
      raise CCSError, :CCS_RESULT_ERROR_INVALID_VALUE if count != num_parameters
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

    def sample(distribution_space: nil)
      ptr = MemoryPointer::new(:ccs_configuration_t)
      CCS.error_check CCS.ccs_configuration_space_sample(@handle, distribution_space ? distribution_space.handle : nil, ptr)
      Configuration::new(ptr.read_ccs_configuration_t, retain: false)
    end

    def samples(count, distribution_space: nil)
      ptr = MemoryPointer::new(:ccs_configuration_t, count)
      CCS.error_check CCS.ccs_configuration_space_samples(@handle, distribution_space ? distribution_space.handle : nil, count, ptr)
      count.times.collect { |i| Configuration::new(ptr[i].read_pointer, retain: false) }
    end
  end
end
