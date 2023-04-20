module CCS

  TreeSpaceType = enum FFI::Type::INT32, :ccs_tree_space_type_t, [
    :CCS_TREE_SPACE_TYPE_STATIC,
    :CCS_TREE_SPACE_TYPE_DYNAMIC
  ]
  class MemoryPointer
    def read_ccs_tree_space_type_t
      TreeSpaceType.from_native(read_int32, nil)
    end
  end

  attach_function :ccs_tree_space_get_type, [:ccs_tree_space_t, :pointer], :ccs_result_t
  attach_function :ccs_tree_space_get_name, [:ccs_tree_space_t, :pointer], :ccs_result_t
  attach_function :ccs_tree_space_set_rng, [:ccs_tree_space_t, :ccs_rng_t], :ccs_result_t
  attach_function :ccs_tree_space_get_rng, [:ccs_tree_space_t, :pointer], :ccs_result_t
  attach_function :ccs_tree_space_get_tree, [:ccs_tree_space_t, :pointer], :ccs_result_t
  attach_function :ccs_tree_space_get_node_at_position, [:ccs_tree_space_t, :size_t, :pointer, :pointer], :ccs_result_t
  attach_function :ccs_tree_space_get_values_at_position, [:ccs_tree_space_t, :size_t, :pointer, :size_t, :pointer], :ccs_result_t
  attach_function :ccs_tree_space_check_position, [:ccs_tree_space_t, :size_t, :pointer, :pointer], :ccs_result_t
  attach_function :ccs_tree_space_check_configuration, [:ccs_tree_space_t, :ccs_tree_configuration_t, :pointer], :ccs_result_t
  attach_function :ccs_tree_space_sample, [:ccs_tree_space_t, :pointer], :ccs_result_t
  attach_function :ccs_tree_space_samples, [:ccs_tree_space_t, :size_t, :pointer], :ccs_result_t

  class TreeSpace < Object
    add_property :type, :ccs_tree_space_type_t, :ccs_tree_space_get_type, memoize: true
    add_handle_property :rng, :ccs_rng_t, :ccs_tree_space_get_rng, memoize: false
    add_handle_property :tree, :ccs_tree_t, :ccs_tree_space_get_tree, memoize: true

    def self.from_handle(handle, retain: true, auto_release: true)
      ptr = MemoryPointer::new(:ccs_tuner_type_t)
      CCS.error_check CCS.ccs_tree_space_get_type(handle, ptr)
      case ptr.read_ccs_tree_space_type_t
      when :CCS_TREE_SPACE_TYPE_STATIC
        StaticTreeSpace
      when :CCS_TREE_SPACE_TYPE_DYNAMIC
        DynamicTreeSpace
      else
        raise CCSError, :CCS_RESULT_ERROR_INVALID_TREE_SPACE
      end.new(handle, retain: retain, auto_release: auto_release)
    end

    def name
      @name ||= begin
        ptr = MemoryPointer::new(:pointer)
        CCS.error_check CCS.ccs_tree_space_get_name(@handle, ptr)
        ptr.read_pointer.read_string
      end
    end

    def rng=(rng)
      CCS.error_check CCS.ccs_tree_space_set_rng(@handle, rng)
      rng
    end

    def get_node_at_position(position)
      count = position.size
      ptr1 = MemoryPointer::new(:size_t, count)
      ptr1.write_array_of_size_t(position)
      ptr2 = MemoryPointer::new(:ccs_tree_t)
      CCS.error_check CCS.ccs_tree_space_get_node_at_position(@handle, count, ptr1, ptr2)
      Tree.from_handle(ptr2.read_ccs_tree_t)
    end

    def get_values_at_position(position)
      count1 = position.size
      ptr1 = MemoryPointer::new(:size_t, count1)
      ptr1.write_array_of_size_t(position)
      count2 = count1 + 1
      ptr2 = MemoryPointer::new(:ccs_datum_t, count2)
      CCS.error_check CCS.ccs_tree_space_get_values_at_position(@handle, count1, ptr1, count2, ptr2)
      count2.times.collect { |i| Datum::new(ptr2[i]).value }
    end

    def check_position(position)
      count = position.size
      ptr1 = MemoryPointer::new(:size_t, count)
      ptr1.write_array_of_size_t(position)
      ptr2 = MemoryPointer::new(:ccs_bool_t)
      CCS.error_check CCS.ccs_tree_space_check_position(@handle, count, ptr1, ptr2)
      ptr2.read_ccs_bool_t == CCS::FALSE ? false : true
    end

    def check_configuration(configuration)
      ptr = MemoryPointer::new(:ccs_bool_t)
      CCS.error_check CCS.ccs_tree_space_check_configuration(@handle, configuration, ptr)
      ptr.read_ccs_bool_t == CCS::FALSE ? false : true
    end

    def sample
      ptr = MemoryPointer::new(:ccs_tree_configuration_t)
      CCS.error_check CCS.ccs_tree_space_sample(@handle, ptr)
      TreeConfiguration::new(ptr.read_ccs_tree_configuration_t, retain: false)
    end

    def samples(count)
      return [] if count == 0
      ptr = MemoryPointer::new(:ccs_tree_configuration_t, count)
      CCS.error_check CCS.ccs_tree_space_samples(@handle, count, ptr)
      count.times.collect { |i| TreeConfiguration::new(ptr[i].read_pointer, retain: false) }
    end

  end

  attach_function :ccs_create_static_tree_space, [:string, :ccs_tree_t, :pointer], :ccs_result_t

  class StaticTreeSpace < TreeSpace

    def initialize(handle = nil, retain: false, auto_release: true,
                   name: nil, tree: nil)
      if handle
        super(handle, retain: retain, auto_release: auto_release)
      else
        ptr = MemoryPointer::new(:ccs_tree_space_t)
        CCS.error_check CCS.ccs_create_static_tree_space(name, tree, ptr)
        super(ptr.read_ccs_tree_space_t, retain: false)
      end
    end

  end

  callback :ccs_dynamic_tree_space_del, [:ccs_tree_space_t], :ccs_result_t
  callback :ccs_dynamic_tree_space_get_child, [:ccs_tree_space_t, :ccs_tree_t, :size_t, :pointer], :ccs_result_t
  callback :ccs_dynamic_tree_space_serialize, [:ccs_tree_space_t, :size_t, :pointer, :pointer], :ccs_result_t
  callback :ccs_dynamic_tree_space_deserialize, [:ccs_tree_space_t, :size_t, :pointer], :ccs_result_t

  class DynamicTreeSpaceVector < FFI::Struct
    layout :del, :ccs_dynamic_tree_space_del,
           :get_child, :ccs_dynamic_tree_space_get_child,
           :serialize, :ccs_dynamic_tree_space_serialize,
           :deserialize, :ccs_dynamic_tree_space_deserialize
  end
  typedef DynamicTreeSpaceVector.by_value, :ccs_dynamic_tree_space_vector_t

  def self.wrap_dynamic_tree_space_callbacks(del, get_child, serialize, deserialize)
    delwrapper = lambda { |ts|
      begin
        del.call(CCS::Object.from_handle(ts)) if del
        CCS.unregister_vector(ts)
        CCSError.to_native(:CCS_RESULT_SUCCESS)
      rescue => e
        CCS.set_error(e)
      end
    }
    get_childwrapper = lambda { |ts, parent, index, p_child|
      begin
        child = get_child.call(TreeSpace.from_handle(ts), Tree.from_handle(parent), index)
        CCS.error_check CCS.ccs_retain_object(child.handle)
        Pointer.new(p_child).write_pointer(child.handle)
        CCSError.to_native(:CCS_RESULT_SUCCESS)
      rescue => e
        CCS.set_error(e)
      end
    }
    serializewrapper =
      if serialize
        lambda { |ts, state_size, p_state, p_state_size|
          begin
            state = serialize(TreeSpace.from_handle(ts), state_size == 0 ? true : false)
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
        lambda { |ts, state_size, p_state|
          begin
            state = p_state.null? ? nil : p_state.slice(0, state_size)
            deserialize(TreeSpace.from_handle(ts), state)
            CCSError.to_native(:CCS_RESULT_SUCCESS)
          rescue => e
            CCS.set_error(e)
          end
        }
      else
        nil
      end
    return [delwrapper, get_childwrapper, serializewrapper, deserializewrapper]
  end

  attach_function :ccs_create_dynamic_tree_space, [:string, :ccs_tree_t, DynamicTreeSpaceVector.by_ref, :value, :pointer], :ccs_result_t
  attach_function :ccs_dynamic_tree_space_get_tree_space_data, [:ccs_tree_space_t, :pointer], :ccs_result_t

  class DynamicTreeSpace < TreeSpace
    add_property :tree_space_data, :value, :ccs_dynamic_tree_space_get_tree_space_data, memoize: true

    def initialize(handle = nil, retain: false, auto_release: true,
                   name: nil, tree: nil, del: nil, get_child: nil, serialize: nil, deserialize: nil, tree_space_data: nil)
      if handle
        super(handle, retain: retain, auto_release: auto_release)
      else
        raise CCSError, :CCS_RESULT_ERROR_INVALID_VALUE if get_child.nil?
        delwrapper, get_childwrapper, serializewrapper, deserializewrapper =
          CCS.wrap_dynamic_tree_space_callbacks(del, get_child, serialize, deserialize)
        vector = DynamicTreeSpaceVector::new
        vector[:del] = delwrapper
        vector[:get_child] = get_childwrapper
        vector[:serialize] = serializewrapper
        vector[:deserialize] = deserializewrapper
        ptr = MemoryPointer::new(:ccs_tree_space_t)
        CCS.error_check CCS.ccs_create_dynamic_tree_space(name, tree, vector, tree_space_data, ptr)
        h = ptr.read_ccs_tree_space_t
        super(h, retain: false)
        CCS.register_vector(h, [delwrapper, get_childwrapper, serializewrapper, deserializewrapper, tree_space_data])
      end
    end

    def self.deserialize(del: nil, get_child: nil, serialize: nil, deserialize: nil, tree_space_data: nil, format: :binary, handle_map: nil, path: nil, buffer: nil, file_descriptor: nil, callback: nil, callback_data: nil)
      raise CCSError, :CCS_RESULT_ERROR_INVALID_VALUE if get_child.nil?
      delwrapper, get_childwrapper, serializewrapper, deserializewrapper =
        CCS.wrap_dynamic_tree_space_callbacks(del, get_child, serialize, deserialize)
      vector = DynamicTreeSpaceVector::new
      vector[:del] = delwrapper
      vector[:get_child] = get_childwrapper
      vector[:serialize] = serializewrapper
      vector[:deserialize] = deserializewrapper
      res = super(format: format, handle_map: handle_map, vector: vector.to_ptr, data: tree_space_data, path: path, buffer: buffer, file_descriptor: file_descriptor, callback: callback, callback_data: callback_data)
      CCS.register_vector(res.handle, [delwrapper, get_childwrapper, serializewrapper, deserializewrapper, tree_space_data])
      res
    end

  end

end
