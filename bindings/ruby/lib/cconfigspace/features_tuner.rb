module CCS

  FeaturesTunerType = enum FFI::Type::INT32, :ccs_features_tuner_type_t, [
    :CCS_FEATURES_TUNER_RANDOM,
    :CCS_FEATURES_TUNER_USER_DEFINED
  ]
  class MemoryPointer
    def read_ccs_features_tuner_type_t
      FeaturesTunerType.from_native(read_int32, nil)
    end
  end

  attach_function :ccs_features_tuner_get_type, [:ccs_features_tuner_t, :pointer], :ccs_result_t
  attach_function :ccs_features_tuner_get_name, [:ccs_features_tuner_t, :pointer], :ccs_result_t
  attach_function :ccs_features_tuner_get_configuration_space, [:ccs_features_tuner_t, :pointer], :ccs_result_t
  attach_function :ccs_features_tuner_get_objective_space, [:ccs_features_tuner_t, :pointer], :ccs_result_t
  attach_function :ccs_features_tuner_get_features_space, [:ccs_features_tuner_t, :pointer], :ccs_result_t
  attach_function :ccs_features_tuner_ask, [:ccs_features_tuner_t, :ccs_features_t, :size_t, :pointer, :pointer], :ccs_result_t
  attach_function :ccs_features_tuner_tell, [:ccs_features_tuner_t, :size_t, :pointer], :ccs_result_t
  attach_function :ccs_features_tuner_get_optimums, [:ccs_features_tuner_t, :ccs_features_t, :size_t, :pointer, :pointer], :ccs_result_t
  attach_function :ccs_features_tuner_get_history, [:ccs_features_tuner_t, :ccs_features_t, :size_t, :pointer, :pointer], :ccs_result_t
  attach_function :ccs_features_tuner_suggest, [:ccs_features_tuner_t, :ccs_features_t, :pointer], :ccs_result_t
  attach_function :ccs_create_random_features_tuner, [:string, :ccs_configuration_space_t, :ccs_features_space_t, :ccs_objective_space_t, :pointer, :pointer], :ccs_result_t

  class FeaturesTuner < Object
    add_property :type, :ccs_features_tuner_type_t, :ccs_features_tuner_get_type, memoize: true
    add_handle_property :configuration_space, :ccs_configuration_space_t, :ccs_features_tuner_get_configuration_space, memoize: true
    add_handle_property :features_space, :ccs_features_space_t, :ccs_features_tuner_get_features_space, memoize: true
    add_handle_property :objective_space, :ccs_objective_space_t, :ccs_features_tuner_get_objective_space, memoize: true

    def self.from_handle(handle, retain: true, auto_release: true)
      ptr = MemoryPointer::new(:ccs_features_tuner_type_t)
      res = CCS.ccs_features_tuner_get_type(handle, ptr)
      CCS.error_check(res)
      case ptr.read_ccs_features_tuner_type_t
      when :CCS_FEATURES_TUNER_RANDOM
	RandomFeaturesTuner
      when :CCS_FEATURES_TUNER_USER_DEFINED
        UserDefinedFeaturesTuner
      else
        raise CCSError, :CCS_INVALID_TUNER
      end.new(handle, retain: retain, auto_release: auto_release)
    end

    def name
      @name ||= begin
        ptr = MemoryPointer::new(:pointer)
        res = CCS.ccs_features_tuner_get_name(@handle, ptr)
        CCS.error_check(res)
        ptr.read_pointer.read_string
      end
    end

    def ask(features, count = 1)
      p_confs = MemoryPointer::new(:ccs_configuration_t, count)
      p_num = MemoryPointer::new(:size_t)
      res = CCS.ccs_features_tuner_ask(@handle, features, count, p_confs, p_num)
      CCS.error_check(res)
      count = p_num.read_size_t
      count.times.collect { |i| Configuration::new(p_confs[i].read_pointer, retain: false) }
    end

    def tell(evaluations)
      count = evaluations.size
      p_evals = MemoryPointer::new(:ccs_features_evaluation_t, count)
      p_evals.write_array_of_pointer(evaluations.collect(&:handle))
      res = CCS.ccs_features_tuner_tell(@handle, count, p_evals)
      CCS.error_check(res)
      self
    end

    def history_size(features: nil)
      p_count = MemoryPointer::new(:size_t)
      res = CCS.ccs_features_tuner_get_history(@handle, features, 0, nil, p_count)
      CCS.error_check(res)
      return p_count.read_size_t
    end

    def history(features: nil)
      count = history_size(features: features)
      p_evals = MemoryPointer::new(:ccs_features_evaluation_t, count)
      res = CCS.ccs_features_tuner_get_history(@handle, features, count, p_evals, nil)
      CCS.error_check(res)
      count.times.collect { |i| FeaturesEvaluation::from_handle(p_evals[i].read_pointer) }
    end

    def num_optimums(features: nil)
      p_count = MemoryPointer::new(:size_t)
      res = CCS.ccs_features_tuner_get_optimums(@handle, features, 0, nil, p_count)
      CCS.error_check(res)
      return p_count.read_size_t
    end

    def optimums(features: nil)
      count = num_optimums(features: features)
      p_evals = MemoryPointer::new(:ccs_features_evaluation_t, count)
      res = CCS.ccs_features_tuner_get_optimums(@handle, features, count, p_evals, nil)
      CCS.error_check(res)
      count.times.collect { |i| FeaturesEvaluation::from_handle(p_evals[i].read_pointer) }
    end

    def suggest(features)
      p_conf = MemoryPointer::new(:ccs_configuration_t)
      res = CCS.ccs_features_tuner_suggest(@handle, features, p_conf)
      CCS.error_check(res)
      Configuration::new(p_conf.read_pointer, retain: false)
    end

  end

  class RandomFeaturesTuner < FeaturesTuner
    def initialize(handle = nil, retain: false, auto_release: true,
                   name: nil, configuration_space: nil, features_space: nil, objective_space: nil, user_data: nil)
      if handle
        super(handle, retain: retain, auto_release: auto_release)
      else
        ptr = MemoryPointer::new(:ccs_features_tuner_t)
        res = CCS.ccs_create_random_features_tuner(name, configuration_space, features_space, objective_space, user_data, ptr)
        CCS.error_check(res)
        super(ptr.read_ccs_features_tuner_t, retain: false)
      end
    end
  end

  callback :ccs_user_defined_features_tuner_del, [:ccs_features_tuner_t], :ccs_result_t
  callback :ccs_user_defined_features_tuner_ask, [:ccs_features_tuner_t, :ccs_features_t, :size_t, :pointer, :pointer], :ccs_result_t
  callback :ccs_user_defined_features_tuner_tell, [:ccs_features_tuner_t, :size_t, :pointer], :ccs_result_t
  callback :ccs_user_defined_features_tuner_get_optimums, [:ccs_features_tuner_t, :ccs_features_t, :size_t, :pointer, :pointer], :ccs_result_t
  callback :ccs_user_defined_features_tuner_get_history, [:ccs_features_tuner_t, :ccs_features_t, :size_t, :pointer, :pointer], :ccs_result_t
  callback :ccs_user_defined_features_tuner_suggest, [:ccs_features_tuner_t, :ccs_features_t, :pointer], :ccs_result_t

  class UserDefinedFeaturesTunerVector < FFI::Struct
    layout :del, :ccs_user_defined_features_tuner_del,
           :ask, :ccs_user_defined_features_tuner_ask,
           :tell, :ccs_user_defined_features_tuner_tell,
           :get_optimums, :ccs_user_defined_features_tuner_get_optimums,
           :get_history, :ccs_user_defined_features_tuner_get_history,
           :suggest, :ccs_user_defined_features_tuner_suggest
  end
  typedef UserDefinedFeaturesTunerVector.by_value, :ccs_user_defined_features_tuner_vector_t

  def self.wrap_user_defined_features_tuner_callbacks(del, ask, tell, get_optimums, get_history, suggest)
    delwrapper = lambda { |tun|
      begin
        del.call(CCS::Object.from_handle(tun))
        @@callbacks.delete(delwrapper)
        CCSError.to_native(:CCS_SUCCESS)
      rescue CCSError => e
        e.to_native
      end
    }
    askwrapper = lambda { |tun, features, count, p_configurations, p_count|
      begin
        configurations, count_ret = ask.call(FeaturesTuner.from_handle(tun), Features.from_handle(features), p_configurations.null? ? nil : count)
        raise CCSError, :CCS_INVALID_VALUE if !p_configurations.null? && count < count_ret
        if !p_configurations.null?
          configurations.each_with_index { |c, i|
            err = CCS.ccs_retain_object(c.handle)
            CCS.error_check(err)
            p_configurations.put_pointer(i*8, c.handle)
          }
          (count_ret...count).each { |i| p_configurations[i].put_pointer(i*8, 0) }
        end
        p_count.write_uint64(count_ret) unless p_count.null?
        CCSError.to_native(:CCS_SUCCESS)
      rescue CCSError => e
        e.to_native
      end
    }
    tellwrapper = lambda { |tun, count, p_evaluations|
      begin
        if count > 0
          evals = count.times.collect { |i| FeaturesEvaluation::from_handle(p_evaluations.get_pointer(i*8)) }
          tell.call(FeaturesTuner.from_handle(tun), evals)
        end
        CCSError.to_native(:CCS_SUCCESS)
      rescue CCSError => e
        e.to_native
      end
    }
    get_optimumswrapper = lambda { |tun, features, count, p_evaluations, p_count|
      begin
        optimums = get_optimums.call(FeaturesTuner.from_handle(tun), features.null? ? nil : Features.from_handle(features))
        raise CCSError, :CCS_INVALID_VALUE if !p_evaluations.null? && count < optimums.size
        unless p_evaluations.null?
          optimums.each_with_index { |o, i|
            p_evaluations.put_pointer(8*i, o.handle)
          }
          ((optimums.size)...count).each { |i| p_evaluations.put_pointer(8*i, 0) }
        end
        p_count.write_uint64(optimums.size) unless p_count.null?
        CCSError.to_native(:CCS_SUCCESS)
      rescue CCSError => e
        e.to_native
      end
    }
    get_historywrapper = lambda { |tun, features, count, p_evaluations, p_count|
      begin
        history = get_history.call(FeaturesTuner.from_handle(tun), features.null? ? nil : Features.from_handle(features))
        raise CCSError, :CCS_INVALID_VALUE if !p_evaluations.null? && count < history.size
        unless p_evaluations.null?
          history.each_with_index { |e, i|
            p_evaluations.put_pointer(8*i, e.handle)
          }
          ((history.size)...count).each { |i| p_evaluations.put_pointer(8*i, 0) }
        end
        p_count.write_uint64(history.size) unless p_count.null?
        CCSError.to_native(:CCS_SUCCESS)
      rescue CCSError => e
        e.to_native
      end
    }
    suggestwrapper =
      if suggest
        lambda { |tun, features, p_configuration|
          begin
            configuration = suggest.call(FeaturesTuner.from_handle(tun), Features.from_handle(features))
            err = CCS.ccs_retain_object(configuration.handle)
            CCS.error_check(err)
            p_configuration.write_pointer(configuration.handle)
            CCSError.to_native(:CCS_SUCCESS)
          rescue CCSError => e
            e.to_native
          end
        }
      else
        nil
      end
    return [delwrapper, askwrapper, tellwrapper, get_optimumswrapper, get_historywrapper, suggestwrapper]
  end

  attach_function :ccs_create_user_defined_features_tuner, [:string, :ccs_configuration_space_t, :ccs_features_space_t, :ccs_objective_space_t, :pointer, UserDefinedFeaturesTunerVector.by_ref, :pointer, :pointer], :ccs_result_t
  attach_function :ccs_user_defined_features_tuner_get_tuner_data, [:ccs_features_tuner_t, :pointer], :ccs_result_t
  class UserDefinedFeaturesTuner < FeaturesTuner
    add_property :tuner_data, :pointer, :ccs_user_defined_features_tuner_get_tuner_data, memoize: true
    class << self
      attr_reader :callbacks
    end

    def initialize(handle = nil, retain: false, auto_release: true,
                   name: nil, configuration_space: nil, features_space: nil, objective_space: nil, user_data: nil, del: nil, ask: nil, tell: nil, get_optimums: nil, get_history: nil, suggest: nil, tuner_data: nil)
      if handle
        super(handle, retain: retain, auto_release: auto_release)
      else
	raise CCSError, :CCS_INVALID_VALUE if del.nil? || ask.nil? || tell.nil? || get_optimums.nil? || get_history.nil?
        delwrapper, askwrapper, tellwrapper, get_optimumswrapper, get_historywrapper, suggestwrapper =
          CCS.wrap_user_defined_features_tuner_callbacks(del, ask, tell, get_optimums, get_history, suggest)
        vector = UserDefinedFeaturesTunerVector::new
        vector[:del] = delwrapper
        vector[:ask] = askwrapper
        vector[:tell] = tellwrapper
        vector[:get_optimums] = get_optimumswrapper
        vector[:get_history] = get_historywrapper
        vector[:suggest] = suggestwrapper
        ptr = MemoryPointer::new(:ccs_features_tuner_t)
        res = CCS.ccs_create_user_defined_features_tuner(name, configuration_space, features_space, objective_space, user_data, vector, tuner_data, ptr)
        CCS.error_check(res)
        super(ptr.read_ccs_features_tuner_t, retain: false)
        CCS.class_variable_get(:@@callbacks)[delwrapper] = [askwrapper, tellwrapper, get_optimumswrapper, get_historywrapper, suggestwrapper, user_data, tuner_data]
      end
    end
  end
end
