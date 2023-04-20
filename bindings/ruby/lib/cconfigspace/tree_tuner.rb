module CCS

  TreeTunerType = enum FFI::Type::INT32, :ccs_tree_tuner_type_t, [
    :CCS_TREE_TUNER_TYPE_RANDOM,
    :CCS_TREE_TUNER_TYPE_USER_DEFINED
  ]
  class MemoryPointer
    def read_ccs_tree_tuner_type_t
      TreeTunerType.from_native(read_int32, nil)
    end
  end

  attach_function :ccs_tree_tuner_get_type, [:ccs_tree_tuner_t, :pointer], :ccs_result_t
  attach_function :ccs_tree_tuner_get_name, [:ccs_tree_tuner_t, :pointer], :ccs_result_t
  attach_function :ccs_tree_tuner_get_tree_space, [:ccs_tree_tuner_t, :pointer], :ccs_result_t
  attach_function :ccs_tree_tuner_get_objective_space, [:ccs_tree_tuner_t, :pointer], :ccs_result_t
  attach_function :ccs_tree_tuner_ask, [:ccs_tree_tuner_t, :size_t, :pointer, :pointer], :ccs_result_t
  attach_function :ccs_tree_tuner_tell, [:ccs_tree_tuner_t, :size_t, :pointer], :ccs_result_t
  attach_function :ccs_tree_tuner_get_optima, [:ccs_tree_tuner_t, :size_t, :pointer, :pointer], :ccs_result_t
  attach_function :ccs_tree_tuner_get_history, [:ccs_tree_tuner_t, :size_t, :pointer, :pointer], :ccs_result_t
  attach_function :ccs_tree_tuner_suggest, [:ccs_tree_tuner_t, :pointer], :ccs_result_t

  class TreeTuner < Object
    add_property :type, :ccs_tree_tuner_type_t, :ccs_tree_tuner_get_type, memoize: true
    add_handle_property :tree_space, :ccs_tree_space_t, :ccs_tree_tuner_get_tree_space, memoize: true
    add_handle_property :objective_space, :ccs_objective_space_t, :ccs_tree_tuner_get_objective_space, memoize: true

    def self.from_handle(handle, retain: true, auto_release: true)
      ptr = MemoryPointer::new(:ccs_tuner_type_t)
      CCS.error_check CCS.ccs_tree_tuner_get_type(handle, ptr)
      case ptr.read_ccs_tree_tuner_type_t
      when :CCS_TREE_TUNER_TYPE_RANDOM
	RandomTreeTuner
      when :CCS_TREE_TUNER_TYPE_USER_DEFINED
        UserDefinedTreeTuner
      else
        raise CCSError, :CCS_RESULT_ERROR_INVALID_TREE_TUNER
      end.new(handle, retain: retain, auto_release: auto_release)
    end

    def name
      @name ||= begin
        ptr = MemoryPointer::new(:pointer)
        CCS.error_check CCS.ccs_tree_tuner_get_name(@handle, ptr)
        ptr.read_pointer.read_string
      end
    end

    def ask(count = 1)
      p_confs = MemoryPointer::new(:ccs_tree_configuration_t, count)
      p_num = MemoryPointer::new(:size_t)
      CCS.error_check CCS.ccs_tree_tuner_ask(@handle, count, p_confs, p_num)
      count = p_num.read_size_t
      count.times.collect { |i| TreeConfiguration::new(p_confs[i].read_pointer, retain: false) }
    end

    def tell(evaluations)
      count = evaluations.size
      p_evals = MemoryPointer::new(:ccs_tree_evaluation_t, count)
      p_evals.write_array_of_pointer(evaluations.collect(&:handle))
      CCS.error_check CCS.ccs_tree_tuner_tell(@handle, count, p_evals)
      self
    end

    def history_size
      p_count = MemoryPointer::new(:size_t)
      CCS.error_check CCS.ccs_tree_tuner_get_history(@handle, 0, nil, p_count)
      return p_count.read_size_t
    end

    def history
      count = history_size
      p_evals = MemoryPointer::new(:ccs_tree_evaluation_t, count)
      CCS.error_check CCS.ccs_tree_tuner_get_history(@handle, count, p_evals, nil)
      count.times.collect { |i| TreeEvaluation::from_handle(p_evals[i].read_pointer) }
    end

    def num_optima
      p_count = MemoryPointer::new(:size_t)
      CCS.error_check CCS.ccs_tree_tuner_get_optima(@handle, 0, nil, p_count)
      return p_count.read_size_t
    end

    def optima
      count = num_optima
      p_evals = MemoryPointer::new(:ccs_tree_evaluation_t, count)
      CCS.error_check CCS.ccs_tree_tuner_get_optima(@handle, count, p_evals, nil)
      count.times.collect { |i| TreeEvaluation::from_handle(p_evals[i].read_pointer) }
    end

    def suggest
      p_conf = MemoryPointer::new(:ccs_tree_configuration_t)
      CCS.error_check CCS.ccs_tree_tuner_suggest(@handle, p_conf)
      TreeConfiguration::new(p_conf.read_pointer, retain: false)
    end

  end

  attach_function :ccs_create_random_tree_tuner, [:string, :ccs_tree_space_t, :ccs_objective_space_t, :pointer], :ccs_result_t

  class RandomTreeTuner < TreeTuner
    def initialize(handle = nil, retain: false, auto_release: true,
                   name: "", tree_space: nil, objective_space: nil)
      if handle
        super(handle, retain: retain, auto_release: auto_release)
      else
        ptr = MemoryPointer::new(:ccs_tree_tuner_t)
        CCS.error_check CCS.ccs_create_random_tree_tuner(name, tree_space, objective_space, ptr)
        super(ptr.read_ccs_tree_tuner_t, retain: false)
      end
    end
  end

  callback :ccs_user_defined_tree_tuner_del, [:ccs_tree_tuner_t], :ccs_result_t
  callback :ccs_user_defined_tree_tuner_ask, [:ccs_tree_tuner_t, :size_t, :pointer, :pointer], :ccs_result_t
  callback :ccs_user_defined_tree_tuner_tell, [:ccs_tree_tuner_t, :size_t, :pointer], :ccs_result_t
  callback :ccs_user_defined_tree_tuner_get_optima, [:ccs_tree_tuner_t, :size_t, :pointer, :pointer], :ccs_result_t
  callback :ccs_user_defined_tree_tuner_get_history, [:ccs_tree_tuner_t, :size_t, :pointer, :pointer], :ccs_result_t
  callback :ccs_user_defined_tree_tuner_suggest, [:ccs_tree_tuner_t, :pointer], :ccs_result_t
  callback :ccs_user_defined_tree_tuner_serialize, [:ccs_tree_tuner_t, :size_t, :pointer, :pointer], :ccs_result_t
  callback :ccs_user_defined_tree_tuner_deserialize, [:ccs_tree_tuner_t, :size_t, :pointer, :size_t, :pointer, :size_t, :pointer], :ccs_result_t

  class UserDefinedTreeTunerVector < FFI::Struct
    layout :del, :ccs_user_defined_tree_tuner_del,
           :ask, :ccs_user_defined_tree_tuner_ask,
           :tell, :ccs_user_defined_tree_tuner_tell,
           :get_optima, :ccs_user_defined_tree_tuner_get_optima,
           :get_history, :ccs_user_defined_tree_tuner_get_history,
           :suggest, :ccs_user_defined_tree_tuner_suggest,
           :serialize, :ccs_user_defined_tree_tuner_serialize,
           :deserialize, :ccs_user_defined_tree_tuner_deserialize
  end
  typedef UserDefinedTreeTunerVector.by_value, :ccs_user_definedi_tree_tuner_vector_t

  def self.wrap_user_defined_tree_tuner_callbacks(del, ask, tell, get_optima, get_history, suggest, serialize, deserialize)
    delwrapper = lambda { |tun|
      begin
        del.call(CCS::Object.from_handle(tun)) if del
        CCS.unregister_vector(tun)
        CCSError.to_native(:CCS_RESULT_SUCCESS)
      rescue => e
        CCS.set_error(e)
      end
    }
    askwrapper = lambda { |tun, count, p_configurations, p_count|
      begin
        configurations, count_ret = ask.call(TreeTuner.from_handle(tun), p_configurations.null? ? nil : count)
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
          evals = count.times.collect { |i| TreeEvaluation::from_handle(p_evaluations.get_pointer(i*8)) }
          tell.call(TreeTuner.from_handle(tun), evals)
        end
        CCSError.to_native(:CCS_RESULT_SUCCESS)
      rescue => e
        CCS.set_error(e)
      end
    }
    get_optimawrapper = lambda { |tun, count, p_evaluations, p_count|
      begin
        optima = get_optima.call(TreeTuner.from_handle(tun))
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
    get_historywrapper = lambda { |tun, count, p_evaluations, p_count|
      begin
        history = get_history.call(TreeTuner.from_handle(tun))
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
        lambda { |tun, p_configuration|
          begin
            configuration = suggest.call(TreeTuner.from_handle(tun))
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
            state = serialize(TreeTuner.from_handle(tun), state_size == 0 ? true : false)
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
            history = p_history.null? ? [] : history_size.times.collect { |i| TreeEvaluation::from_handle(p_p_history.get_pointer(i*8)) }
            optima = p_optima.null? ? [] : num_optima.times.collect { |i| TreeEvaluation::from_handle(p_optima.get_pointer(i*8)) }
            state = p_state.null? ? nil : p_state.slice(0, state_size)
            deserialize(TreeTuner.from_handle(tun), history, optima, state)
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

  attach_function :ccs_create_user_defined_tree_tuner, [:string, :ccs_tree_space_t, :ccs_objective_space_t, UserDefinedTreeTunerVector.by_ref, :value, :pointer], :ccs_result_t
  attach_function :ccs_user_defined_tree_tuner_get_tuner_data, [:ccs_tree_tuner_t, :pointer], :ccs_result_t

  class UserDefinedTreeTuner < TreeTuner
    add_property :tuner_data, :value, :ccs_user_defined_tree_tuner_get_tuner_data, memoize: true

    def initialize(handle = nil, retain: false, auto_release: true,
                   name: "", tree_space: nil, objective_space: nil,
                   del: nil, ask: nil, tell: nil, get_optima: nil, get_history: nil, suggest: nil, serialize: nil, deserialize: nil, tuner_data: nil)
      if handle
        super(handle, retain: retain, auto_release: auto_release)
      else
        raise CCSError, :CCS_RESULT_ERROR_INVALID_VALUE if ask.nil? || tell.nil? || get_optima.nil? || get_history.nil?
        delwrapper, askwrapper, tellwrapper, get_optimawrapper, get_historywrapper, suggestwrapper, serializewrapper, deserializewrapper =
          CCS.wrap_user_defined_tree_tuner_callbacks(del, ask, tell, get_optima, get_history, suggest, serialize, deserialize)
        vector = UserDefinedTreeTunerVector::new
        vector[:del] = delwrapper
        vector[:ask] = askwrapper
        vector[:tell] = tellwrapper
        vector[:get_optima] = get_optimawrapper
        vector[:get_history] = get_historywrapper
        vector[:suggest] = suggestwrapper
        vector[:serialize] = serializewrapper
        vector[:deserialize] = deserializewrapper
        ptr = MemoryPointer::new(:ccs_tree_tuner_t)
        CCS.error_check CCS.ccs_create_user_defined_tree_tuner(name, tree_space, objective_space, vector, tuner_data, ptr)
        handle = ptr.read_ccs_tree_tuner_t
        super(handle, retain: false)
        CCS.register_vector(handle, [delwrapper, askwrapper, tellwrapper, get_optimawrapper, get_historywrapper, suggestwrapper, serializewrapper, deserializewrapper, tuner_data])
      end
    end

    def self.deserialize(del: nil, ask: nil, tell: nil, get_optima: nil, get_history: nil, suggest: nil, serialize: nil, deserialize: nil, tuner_data: nil, format: :binary, handle_map: nil, path: nil, buffer: nil, file_descriptor: nil, callback: nil, callback_data: nil)
      raise CCSError, :CCS_RESULT_ERROR_INVALID_VALUE if ask.nil? || tell.nil? || get_optima.nil? || get_history.nil?
      delwrapper, askwrapper, tellwrapper, get_optimawrapper, get_historywrapper, suggestwrapper, serializewrapper, deserializewrapper =
        CCS.wrap_user_defined_tree_tuner_callbacks(del, ask, tell, get_optima, get_history, suggest, serialize, deserialize)
      vector = UserDefinedTreeTunerVector::new
      vector[:del] = delwrapper
      vector[:ask] = askwrapper
      vector[:tell] = tellwrapper
      vector[:get_optima] = get_optimawrapper
      vector[:get_history] = get_historywrapper
      vector[:suggest] = suggestwrapper
      vector[:serialize] = serializewrapper
      vector[:deserialize] = deserializewrapper
      res = super(format: format, handle_map: handle_map, vector: vector.to_ptr, data: tuner_data, path: path, buffer: buffer, file_descriptor: file_descriptor, callback: callback, callback_data: callback_data)
      CCS.register_vector(res.handle, [delwrapper, askwrapper, tellwrapper, get_optimawrapper, get_historywrapper, suggestwrapper, serializewrapper, deserializewrapper, tuner_data])
      res
    end

  end
   
end
