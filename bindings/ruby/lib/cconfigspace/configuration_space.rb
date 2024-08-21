module CCS
  attach_function :ccs_create_configuration_space, [:string, :size_t, :pointer, :pointer, :size_t, :pointer, :ccs_feature_space_t, :ccs_rng_t, :pointer], :ccs_result_t
  attach_function :ccs_configuration_space_get_rng, [:ccs_configuration_space_t, :pointer], :ccs_result_t
  attach_function :ccs_configuration_space_get_feature_space, [:ccs_configuration_space_t, :pointer], :ccs_result_t
  attach_function :ccs_configuration_space_get_condition, [:ccs_configuration_space_t, :size_t, :pointer], :ccs_result_t
  attach_function :ccs_configuration_space_get_conditions, [:ccs_configuration_space_t, :size_t, :pointer, :pointer], :ccs_result_t
  attach_function :ccs_configuration_space_get_forbidden_clause, [:ccs_configuration_space_t, :size_t, :pointer], :ccs_result_t
  attach_function :ccs_configuration_space_get_forbidden_clauses, [:ccs_configuration_space_t, :size_t, :pointer, :pointer], :ccs_result_t
  attach_function :ccs_configuration_space_check_configuration, [:ccs_configuration_space_t, :ccs_configuration_t, :pointer], :ccs_result_t
  attach_function :ccs_configuration_space_get_default_configuration, [:ccs_configuration_space_t, :ccs_features_t, :pointer], :ccs_result_t
  attach_function :ccs_configuration_space_sample, [:ccs_configuration_space_t, :ccs_distribution_space_t, :ccs_features_t, :ccs_rng_t, :pointer], :ccs_result_t
  attach_function :ccs_configuration_space_samples, [:ccs_configuration_space_t, :ccs_distribution_space_t, :ccs_features_t, :ccs_rng_t, :size_t, :pointer], :ccs_result_t

  class ConfigurationSpace < Context
    add_handle_property :feature_space, :ccs_feature_space_t, :ccs_configuration_space_get_feature_space, memoize: true
    add_handle_property :rng, :ccs_rng_t, :ccs_configuration_space_get_rng, memoize: true
    add_handle_array_property :conditions, :ccs_expression_t, :ccs_configuration_space_get_conditions, memoize: true
    add_handle_array_property :forbidden_clauses, :ccs_expression_t, :ccs_configuration_space_get_forbidden_clauses, memoize: true

    def initialize(handle = nil, retain: false, auto_release: true,
                   name: "", parameters: nil, conditions: nil, forbidden_clauses: nil, feature_space: nil, rng: nil, binding: nil)
      if handle
        super(handle, retain: retain, auto_release: auto_release)
      else
        count = parameters.size
        p_parameters = MemoryPointer::new(:ccs_parameter_t, count)
        p_parameters.write_array_of_pointer(parameters.collect(&:handle))
        ptr = MemoryPointer::new(:ccs_configuration_space_t)

        ctx_params = parameters
        ctx_params += feature_space.parameters if feature_space
        ctx = ctx_params.map { |p| [p.name, p] }.to_h

        if forbidden_clauses
          forbidden_clauses = forbidden_clauses.collect { |e| e.kind_of?(String) ? CCS.parse(e, context: ctx, binding: binding) : e }
          fccount = forbidden_clauses.size
          fcptr = MemoryPointer::new(:ccs_expression_t, fccount)
          fcptr.write_array_of_pointer(forbidden_clauses.collect(&:handle))
        else
          fccount = 0
          fcptr = nil
        end

        if conditions
          indexdict = parameters.each_with_index.to_h
          conditions = conditions.transform_values { |v| v.kind_of?(String) ? CCS.parse(v, context: ctx, binding: binding) : v }
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

        CCS.error_check CCS.ccs_create_configuration_space(name, count, p_parameters, cptr, fccount, fcptr, feature_space, rng, ptr)
        super(ptr.read_ccs_configuration_space_t, retain:false)
      end
    end

    def self.from_handle(handle, retain: true, auto_release: true)
      self.new(handle, retain: retain, auto_release: auto_release)
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

    def conditional_parameters
      @conditional_parameters ||= begin
        hps = parameters
        conds = conditions
        hps.each_with_index.select { |h, i| conds[i] }.collect { |h, i| h }.to_a.freeze
      end
    end

    def unconditional_parameters
      @unconditional_parameters ||= begin
        hps = parameters
        conds = conditions
        hps.each_with_index.select { |h, i| !conds[i] }.collect { |h, i| h }.to_a.freeze
      end
    end

    def forbidden_clause(index)
      ptr = MemoryPointer::new(:ccs_expression_t)
      CCS.error_check CCS.ccs_configuration_space_get_forbidden_clause(@handle, index, ptr)
      Expression.from_handle(ptr.read_ccs_expression_t)
    end

    def check(configuration)
      ptr = MemoryPointer::new(:ccs_bool_t)
      CCS.error_check CCS.ccs_configuration_space_check_configuration(@handle, configuration.handle, ptr)
      return ptr.read_ccs_bool_t == CCS::FALSE ? false : true
    end

    def default_configuration(features: nil)
      ptr = MemoryPointer::new(:ccs_configuration_t)
      CCS.error_check CCS.ccs_configuration_space_get_default_configuration(@handle, features, ptr)
      Configuration::new(ptr.read_ccs_configuration_t, retain: false)
    end

    def sample(distribution_space: nil, features: nil, rng: nil)
      ptr = MemoryPointer::new(:ccs_configuration_t)
      CCS.error_check CCS.ccs_configuration_space_sample(@handle, distribution_space, features, rng, ptr)
      Configuration::new(ptr.read_ccs_configuration_t, retain: false)
    end

    def samples(count, distribution_space: nil, features: nil, rng: nil)
      ptr = MemoryPointer::new(:ccs_configuration_t, count)
      CCS.error_check CCS.ccs_configuration_space_samples(@handle, distribution_space, features, rng, count, ptr)
      count.times.collect { |i| Configuration::new(ptr[i].read_pointer, retain: false) }
    end
  end
end
