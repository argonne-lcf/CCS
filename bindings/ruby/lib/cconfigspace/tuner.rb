module CCS

  TunerType = enum FFI::Type::INT32, :ccs_tuner_type_t, [
    :CCS_TUNER_TYPE_RANDOM,
    :CCS_TUNER_TYPE_USER_DEFINED
  ]
  class MemoryPointer
    def read_ccs_tuner_type_t
      TunerType.from_native(read_int32, nil)
    end
  end

  attach_function :ccs_tuner_get_type, [:ccs_tuner_t, :pointer], :ccs_result_t
  attach_function :ccs_tuner_get_name, [:ccs_tuner_t, :pointer], :ccs_result_t
  attach_function :ccs_tuner_get_search_space, [:ccs_tuner_t, :pointer], :ccs_result_t
  attach_function :ccs_tuner_get_objective_space, [:ccs_tuner_t, :pointer], :ccs_result_t
  attach_function :ccs_tuner_get_feature_space, [:ccs_tuner_t, :pointer], :ccs_result_t
  attach_function :ccs_tuner_ask, [:ccs_tuner_t, :ccs_features_t, :size_t, :pointer, :pointer], :ccs_result_t
  attach_function :ccs_tuner_tell, [:ccs_tuner_t, :size_t, :pointer], :ccs_result_t
  attach_function :ccs_tuner_get_optima, [:ccs_tuner_t, :ccs_features_t, :size_t, :pointer, :pointer], :ccs_result_t
  attach_function :ccs_tuner_get_history, [:ccs_tuner_t, :ccs_features_t, :size_t, :pointer, :pointer], :ccs_result_t
  attach_function :ccs_tuner_suggest, [:ccs_tuner_t, :ccs_features_t, :pointer], :ccs_result_t

  class Tuner < Object
    add_property :type, :ccs_tuner_type_t, :ccs_tuner_get_type, memoize: true
    add_handle_property :search_space, :ccs_search_space_t, :ccs_tuner_get_search_space, memoize: true
    add_handle_property :feature_space, :ccs_feature_space_t, :ccs_tuner_get_feature_space, memoize: true
    add_handle_property :objective_space, :ccs_objective_space_t, :ccs_tuner_get_objective_space, memoize: true

    def self.from_handle(handle, retain: true, auto_release: true)
      ptr = MemoryPointer::new(:ccs_tuner_type_t)
      CCS.error_check CCS.ccs_tuner_get_type(handle, ptr)
      case ptr.read_ccs_tuner_type_t
      when :CCS_TUNER_TYPE_RANDOM
        RandomTuner
      when :CCS_TUNER_TYPE_USER_DEFINED
        UserDefinedTuner
      else
        raise CCSError, :CCS_RESULT_ERROR_INVALID_TUNER
      end.new(handle, retain: retain, auto_release: auto_release)
    end

    def name
      @name ||= begin
        ptr = MemoryPointer::new(:pointer)
        CCS.error_check CCS.ccs_tuner_get_name(@handle, ptr)
        ptr.read_pointer.read_string
      end
    end

    def ask(count = 1, features: nil)
      p_confs = MemoryPointer::new(:ccs_search_configuration_t, count)
      p_num = MemoryPointer::new(:size_t)
      CCS.error_check CCS.ccs_tuner_ask(@handle, features, count, p_confs, p_num)
      count = p_num.read_size_t
      count.times.collect { |i| Object::from_handle(p_confs[i].read_pointer, retain: false) }
    end

    def tell(evaluations)
      count = evaluations.size
      p_evals = MemoryPointer::new(:ccs_evaluation_t, count)
      p_evals.write_array_of_pointer(evaluations.collect(&:handle))
      CCS.error_check CCS.ccs_tuner_tell(@handle, count, p_evals)
      self
    end

    def history_size(features: nil)
      p_count = MemoryPointer::new(:size_t)
      CCS.error_check CCS.ccs_tuner_get_history(@handle, features, 0, nil, p_count)
      return p_count.read_size_t
    end

    def history(features: nil)
      count = history_size(features: features)
      p_evals = MemoryPointer::new(:ccs_evaluation_t, count)
      CCS.error_check CCS.ccs_tuner_get_history(@handle, features, count, p_evals, nil)
      count.times.collect { |i| Evaluation::from_handle(p_evals[i].read_pointer) }
    end

    def num_optima(features: nil)
      p_count = MemoryPointer::new(:size_t)
      CCS.error_check CCS.ccs_tuner_get_optima(@handle, features, 0, nil, p_count)
      return p_count.read_size_t
    end

    def optima(features: nil)
      count = num_optima(features: features)
      p_evals = MemoryPointer::new(:ccs_evaluation_t, count)
      CCS.error_check CCS.ccs_tuner_get_optima(@handle, features, count, p_evals, nil)
      count.times.collect { |i| Evaluation::from_handle(p_evals[i].read_pointer) }
    end

    def suggest(features: nil)
      p_conf = MemoryPointer::new(:ccs_search_configuration_t)
      CCS.error_check CCS.ccs_tuner_suggest(@handle, features, p_conf)
      Object.from_handle(p_conf.read_pointer, retain: false)
    end

  end

  attach_function :ccs_create_random_tuner, [:string, :ccs_objective_space_t, :pointer], :ccs_result_t

  class RandomTuner < Tuner
    def initialize(handle = nil, retain: false, auto_release: true,
                   name: "", objective_space: nil)
      if handle
        super(handle, retain: retain, auto_release: auto_release)
      else
        ptr = MemoryPointer::new(:ccs_tuner_t)
        CCS.error_check CCS.ccs_create_random_tuner(name, objective_space, ptr)
        super(ptr.read_ccs_tuner_t, retain: false)
      end
    end
  end

  Tuner::Random = RandomTuner

  callback :ccs_user_defined_tuner_del, [:ccs_tuner_t], :ccs_result_t
  callback :ccs_user_defined_tuner_ask, [:ccs_tuner_t, :ccs_features_t, :size_t, :pointer, :pointer], :ccs_result_t
  callback :ccs_user_defined_tuner_tell, [:ccs_tuner_t, :size_t, :pointer], :ccs_result_t
  callback :ccs_user_defined_tuner_get_optima, [:ccs_tuner_t, :ccs_features_t, :size_t, :pointer, :pointer], :ccs_result_t
  callback :ccs_user_defined_tuner_get_history, [:ccs_tuner_t, :ccs_features_t, :size_t, :pointer, :pointer], :ccs_result_t
  callback :ccs_user_defined_tuner_suggest, [:ccs_tuner_t, :ccs_features_t, :pointer], :ccs_result_t
  callback :ccs_user_defined_tuner_serialize, [:ccs_tuner_t, :size_t, :pointer, :pointer], :ccs_result_t
  callback :ccs_user_defined_tuner_deserialize, [:ccs_tuner_t, :size_t, :pointer, :size_t, :pointer, :size_t, :pointer], :ccs_result_t

  class UserDefinedTunerVector < FFI::Struct
    layout :del, :ccs_user_defined_tuner_del,
           :ask, :ccs_user_defined_tuner_ask,
           :tell, :ccs_user_defined_tuner_tell,
           :get_optima, :ccs_user_defined_tuner_get_optima,
           :get_history, :ccs_user_defined_tuner_get_history,
           :suggest, :ccs_user_defined_tuner_suggest,
           :serialize, :ccs_user_defined_tuner_serialize,
           :deserialize, :ccs_user_defined_tuner_deserialize
  end
  typedef UserDefinedTunerVector.by_value, :ccs_user_defined_tuner_vector_t

  def self.wrap_user_defined_tuner_callbacks(del, ask, tell, get_optima, get_history, suggest, serialize, deserialize)
    delwrapper = lambda { |tun|
      begin
        o = CCS::Object.from_handle(tun)
        tdata = o.tuner_data
        del.call(o) if del
        FFI.dec_ref(tdata) unless tdata.nil?
        CCS.unregister_vector(tun)
        CCSError.to_native(:CCS_RESULT_SUCCESS)
      rescue => e
        CCS.set_error(e)
      end
    }
    askwrapper = lambda { |tun, features, count, p_configurations, p_count|
      begin
        configurations, count_ret = ask.call(Tuner.from_handle(tun), features.null? ? nil : Features.from_handle(features), p_configurations.null? ? nil : count)
        raise CCSError, :CCS_RESULT_ERROR_INVALID_VALUE if !p_configurations.null? && count < count_ret
        if !p_configurations.null?
          configurations.each_with_index { |c, i|
            err = CCS.ccs_retain_object(c.handle)
            CCS.error_check(err)
            p_configurations.put_pointer(i*8, c.handle)
          }
          (count_ret...count).each { |i| p_configurations[i].put_pointer(i*8, 0) }
        end
        Pointer.new(p_count).write_size_t(count_ret) unless p_count.null?
        CCSError.to_native(:CCS_RESULT_SUCCESS)
      rescue => e
        CCS.set_error(e)
      end
    }
    tellwrapper = lambda { |tun, count, p_evaluations|
      begin
        if count > 0
          evals = count.times.collect { |i| Evaluation::from_handle(p_evaluations.get_pointer(i*8)) }
          tell.call(Tuner.from_handle(tun), evals)
        end
        CCSError.to_native(:CCS_RESULT_SUCCESS)
      rescue => e
        CCS.set_error(e)
      end
    }
    get_optimawrapper = lambda { |tun, features, count, p_evaluations, p_count|
      begin
        optima = get_optima.call(Tuner.from_handle(tun), features.null? ? nil : Features.from_handle(features))
        raise CCSError, :CCS_RESULT_ERROR_INVALID_VALUE if !p_evaluations.null? && count < optima.size
        unless p_evaluations.null?
          optima.each_with_index { |o, i|
            p_evaluations.put_pointer(8*i, o.handle)
          }
          ((optima.size)...count).each { |i| p_evaluations.put_pointer(8*i, 0) }
        end
        Pointer.new(p_count).write_size_t(optima.size) unless p_count.null?
        CCSError.to_native(:CCS_RESULT_SUCCESS)
      rescue => e
        CCS.set_error(e)
      end
    }
    get_historywrapper = lambda { |tun, features, count, p_evaluations, p_count|
      begin
        history = get_history.call(Tuner.from_handle(tun), features.null? ? nil : Features.from_handle(features))
        raise CCSError, :CCS_RESULT_ERROR_INVALID_VALUE if !p_evaluations.null? && count < history.size
        unless p_evaluations.null?
          history.each_with_index { |e, i|
            p_evaluations.put_pointer(8*i, e.handle)
          }
          ((history.size)...count).each { |i| p_evaluations.put_pointer(8*i, 0) }
        end
        Pointer.new(p_count).write_size_t(history.size) unless p_count.null?
        CCSError.to_native(:CCS_RESULT_SUCCESS)
      rescue => e
        CCS.set_error(e)
      end
    }
    suggestwrapper =
      if suggest
        lambda { |tun, features, p_configuration|
          begin
            configuration = suggest.call(Tuner.from_handle(tun), features.null? ? nil : Features.from_handle(features))
            err = CCS.ccs_retain_object(configuration.handle)
            CCS.error_check(err)
            p_configuration.write_pointer(configuration.handle)
            CCSError.to_native(:CCS_RESULT_SUCCESS)
          rescue => e
            CCS.set_error(e)
          end
        }
      else
        nil
      end
    serializewrapper =
      if serialize
        lambda { |tun, state_size, p_state, p_state_size|
          begin
            state = serialize(Tuner.from_handle(tun), state_size == 0 ? true : false)
            raise CCSError, :CCS_RESULT_ERROR_INVALID_VALUE if !p_state.null? && state_size < state.size
            p_state.write_bytes(state.read_bytes(state.size)) unless p_state.null?
            Pointer.new(p_state_size).write_size_t(state.size) unless p_state_size.null?
            CCSError.to_native(:CCS_RESULT_SUCCESS)
          rescue => e
            CCS.set_error(e)
          end
        }
      else
        nil
      end
    deserializewrapper =
      if deserialize
        lambda { |tun, history_size, p_history, num_optima, p_optima, state_size, p_state|
          begin
            history = p_history.null? ? [] : history_size.times.collect { |i| Evaluation::from_handle(p_p_history.get_pointer(i*8)) }
            optima = p_optima.null? ? [] : num_optima.times.collect { |i| Evaluation::from_handle(p_optima.get_pointer(i*8)) }
            state = p_state.null? ? nil : p_state.slice(0, state_size)
            deserialize(Tuner.from_handle(tun), history, optima, state)
            CCSError.to_native(:CCS_RESULT_SUCCESS)
          rescue => e
            CCS.set_error(e)
          end
        }
      else
        nil
      end
    return [delwrapper, askwrapper, tellwrapper, get_optimawrapper, get_historywrapper, suggestwrapper, serializewrapper, deserializewrapper]
  end

  attach_function :ccs_create_user_defined_tuner, [:string, :ccs_objective_space_t, UserDefinedTunerVector.by_ref, :value, :pointer], :ccs_result_t
  attach_function :ccs_user_defined_tuner_get_tuner_data, [:ccs_tuner_t, :pointer], :ccs_result_t
  class UserDefinedTuner < Tuner
    add_property :tuner_data, :value, :ccs_user_defined_tuner_get_tuner_data, memoize: true

    def initialize(handle = nil, retain: false, auto_release: true,
                   name: "", objective_space: nil,
                   del: nil, ask: nil, tell: nil, get_optima: nil, get_history: nil, suggest: nil, serialize: nil, deserialize: nil, tuner_data: nil)
      if handle
        super(handle, retain: retain, auto_release: auto_release)
      else
        raise CCSError, :CCS_RESULT_ERROR_INVALID_VALUE if ask.nil? || tell.nil? || get_optima.nil? || get_history.nil?
        wrappers = CCS.wrap_user_defined_tuner_callbacks(del, ask, tell, get_optima, get_history, suggest, serialize, deserialize)
        delwrapper, askwrapper, tellwrapper, get_optimawrapper, get_historywrapper, suggestwrapper, serializewrapper, deserializewrapper = wrappers
        vector = UserDefinedTunerVector::new
        vector[:del] = delwrapper
        vector[:ask] = askwrapper
        vector[:tell] = tellwrapper
        vector[:get_optima] = get_optimawrapper
        vector[:get_history] = get_historywrapper
        vector[:suggest] = suggestwrapper
        vector[:serialize] = serializewrapper
        vector[:deserialize] = deserializewrapper
        ptr = MemoryPointer::new(:ccs_tuner_t)
        CCS.error_check CCS.ccs_create_user_defined_tuner(name, objective_space, vector, tuner_data, ptr)
        handle = ptr.read_ccs_tuner_t
        super(handle, retain: false)
        CCS.register_vector(handle, wrappers)
        FFI.inc_ref(tuner_data) unless tuner_data.nil?
      end
    end

    def self.deserialize(del: nil, ask: nil, tell: nil, get_optima: nil, get_history: nil, suggest: nil, serialize: nil, deserialize: nil, tuner_data: nil, format: :binary, handle_map: nil, path: nil, buffer: nil, file_descriptor: nil, callback: nil, callback_data: nil)
      raise CCSError, :CCS_RESULT_ERROR_INVALID_VALUE if ask.nil? || tell.nil? || get_optima.nil? || get_history.nil?
      wrappers = CCS.wrap_user_defined_tuner_callbacks(del, ask, tell, get_optima, get_history, suggest, serialize, deserialize)
      delwrapper, askwrapper, tellwrapper, get_optimawrapper, get_historywrapper, suggestwrapper, serializewrapper, deserializewrapper = wrappers
      vector = UserDefinedTunerVector::new
      vector[:del] = delwrapper
      vector[:ask] = askwrapper
      vector[:tell] = tellwrapper
      vector[:get_optima] = get_optimawrapper
      vector[:get_history] = get_historywrapper
      vector[:suggest] = suggestwrapper
      vector[:serialize] = serializewrapper
      vector[:deserialize] = deserializewrapper
      res = super(format: format, handle_map: handle_map, vector: vector.to_ptr, data: tuner_data, path: path, buffer: buffer, file_descriptor: file_descriptor, callback: callback, callback_data: callback_data)
      CCS.register_vector(res.handle, wrappers)
      FFI.inc_ref(tuner_data) unless tuner_data.nil?
      res
    end
  end

  Tuner::UserDefined = UserDefinedTuner

end
