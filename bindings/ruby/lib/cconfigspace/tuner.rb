module CCS

  TunerType = enum FFI::Type::INT32, :ccs_tuner_type_t, [
    :CCS_TUNER_RANDOM,
    :CCS_TUNER_USER_DEFINED
  ]
  class MemoryPointer
    def read_ccs_tuner_type_t
      TunerType.from_native(read_int32, nil)
    end
  end

  attach_function :ccs_tuner_get_type, [:ccs_tuner_t, :pointer], :ccs_result_t
  attach_function :ccs_tuner_get_name, [:ccs_tuner_t, :pointer], :ccs_result_t
  attach_function :ccs_tuner_get_user_data, [:ccs_tuner_t, :pointer], :ccs_result_t
  attach_function :ccs_tuner_get_configuration_space, [:ccs_tuner_t, :pointer], :ccs_result_t
  attach_function :ccs_tuner_get_objective_space, [:ccs_tuner_t, :pointer], :ccs_result_t
  attach_function :ccs_tuner_ask, [:ccs_tuner_t, :size_t, :pointer, :pointer], :ccs_result_t
  attach_function :ccs_tuner_tell, [:ccs_tuner_t, :size_t, :pointer], :ccs_result_t
  attach_function :ccs_tuner_get_optimums, [:ccs_tuner_t, :size_t, :pointer, :pointer], :ccs_result_t
  attach_function :ccs_tuner_get_history, [:ccs_tuner_t, :size_t, :pointer, :pointer], :ccs_result_t
  attach_function :ccs_create_random_tuner, [:string, :ccs_configuration_space_t, :ccs_objective_space_t, :pointer, :pointer], :ccs_result_t

  class Tuner < Object
    add_property :type, :ccs_tuner_type_t, :ccs_tuner_get_type, memoize: true
    add_property :user_data, :pointer, :ccs_tuner_get_user_data, memoize: true
    add_handle_property :configuration_space, :ccs_configuration_space_t, :ccs_tuner_get_configuration_space, memoize: true
    add_handle_property :objective_space, :ccs_objective_space_t, :ccs_tuner_get_objective_space, memoize: true

    def from_handle(handle)
      ptr = MemoryPointer::new(:ccs_tuner_type_t)
      res = CCS.ccs_tuner_get_type(handle, ptr)
      CCS.error_check(res)
      case ptr.read_ccs_tuner_type_t
      when :CCS_TUNER_RANDOM
	RandomTuner::new(handle, retain: true)
      when :CCS_TUNER_USER_DEFINED
        GenericTuner::new(handle, retain: true)
      else
        raise StandardError, :CCS_INVALID_TUNER
      end
    end

    def name
      @name ||= begin
        ptr = MemoryPointer::new(:pointer)
        res = CCS.ccs_tuner_get_name(@handle, ptr)
        CCS.error_check(res)
        ptr.read_pointer.read_string
      end
    end

    def ask(count = 1)
      p_confs = MemoryPointer::new(:ccs_configuration_t, count)
      p_num = MemoryPointer::new(:size_t)
      res = CCS.ccs_tuner_ask(@handle, count, p_confs, p_num)
      CCS.error_check(res)
      count = p_num.read_size_t
      count.times.collect { |i| Configuration::new(p_confs[i].read_pointer, retain: false) }
    end

    def tell(evaluations)
      count = evaluations.size
      p_evals = MemoryPointer::new(:ccs_evaluation_t, count)
      p_evals.write_array_of_pointer(evaluations.collect(&:handle))
      res = CCS.ccs_tuner_tell(@handle, count, p_evals)
      CCS.error_check(res)
      self
    end

    def history_size
      p_count = MemoryPointer::new(:size_t)
      res = CCS.ccs_tuner_get_history(@handle, 0, nil, p_count)
      CCS.error_check(res)
      return p_count.read_size_t
    end

    def history
      count = history_size
      p_evals = MemoryPointer::new(:ccs_evaluation_t, count)
      res = CCS.ccs_tuner_get_history(@handle, count, p_evals, nil)
      CCS.error_check(res)
      count.times.collect { |i| Evaluation::from_handle(p_evals[i].read_pointer) }
    end

    def num_optimums
      p_count = MemoryPointer::new(:size_t)
      res = CCS.ccs_tuner_get_optimums(@handle, 0, nil, p_count)
      CCS.error_check(res)
      return p_count.read_size_t
    end

    def optimums
      count = num_optimums
      p_evals = MemoryPointer::new(:ccs_evaluation_t, count)
      res = CCS.ccs_tuner_get_optimums(@handle, count, p_evals, nil)
      CCS.error_check(res)
      count.times.collect { |i| Evaluation::from_handle(p_evals[i].read_pointer) }
    end
  end

  class RandomTuner < Tuner
    def initialize(handle = nil, retain: false, name: nil, configuration_space: nil, objective_space: nil, user_data: nil)
      if handle
        super(handle, retain: retain)
      else
        ptr = MemoryPointer::new(:ccs_tuner_t)
        res = CCS.ccs_create_random_tuner(name, configuration_space, objective_space, user_data, ptr)
        CCS.error_check(res)
        super(ptr.read_ccs_tuner_t, retain: false)
      end
    end
  end
end
