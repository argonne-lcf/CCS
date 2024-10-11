module CCS

  TreeSpaceType = enum FFI::Type::INT32, :ccs_tree_space_type_t, [
    :CCS_TREE_SPACE_TYPE_STATIC,
    :CCS_TREE_SPACE_TYPE_DYNAMIC
  ]
  module MemoryAccessor
    def read_ccs_tree_space_type_t
      TreeSpaceType.from_native(read_int32, nil)
    end
  end

  attach_function :ccs_tree_space_get_type, [:ccs_tree_space_t, :pointer], :ccs_result_t
  attach_function :ccs_tree_space_get_name, [:ccs_tree_space_t, :pointer], :ccs_result_t
  attach_function :ccs_tree_space_get_rng, [:ccs_tree_space_t, :pointer], :ccs_result_t
  attach_function :ccs_tree_space_get_feature_space, [:ccs_tree_space_t, :pointer], :ccs_result_t
  attach_function :ccs_tree_space_get_tree, [:ccs_tree_space_t, :pointer], :ccs_result_t
  attach_function :ccs_tree_space_get_node_at_position, [:ccs_tree_space_t, :size_t, :pointer, :pointer], :ccs_result_t
  attach_function :ccs_tree_space_get_values_at_position, [:ccs_tree_space_t, :size_t, :pointer, :size_t, :pointer], :ccs_result_t
  attach_function :ccs_tree_space_sample, [:ccs_tree_space_t, :ccs_features_t, :ccs_rng_t, :pointer], :ccs_result_t
  attach_function :ccs_tree_space_samples, [:ccs_tree_space_t, :ccs_features_t, :ccs_rng_t, :size_t, :pointer], :ccs_result_t

  class TreeSpace < Object
    add_property :type, :ccs_tree_space_type_t, :ccs_tree_space_get_type, memoize: true
    add_handle_property :rng, :ccs_rng_t, :ccs_tree_space_get_rng, memoize: true
    add_handle_property :feature_space, :ccs_feature_space_t, :ccs_tree_space_get_feature_space, memoize: true
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

    def sample(features: nil, rng: nil)
      ptr = MemoryPointer::new(:ccs_tree_configuration_t)
      CCS.error_check CCS.ccs_tree_space_sample(@handle, features, rng, ptr)
      TreeConfiguration::new(ptr.read_ccs_tree_configuration_t, retain: false)
    end

    def samples(count, features: nil, rng: nil)
      return [] if count == 0
      ptr = MemoryPointer::new(:ccs_tree_configuration_t, count)
      CCS.error_check CCS.ccs_tree_space_samples(@handle, features, rng, count, ptr)
      count.times.collect { |i| TreeConfiguration::new(ptr[i].read_pointer, retain: false) }
    end

  end

  attach_function :ccs_create_static_tree_space, [:string, :ccs_tree_t, :ccs_feature_space_t, :ccs_rng_t, :pointer], :ccs_result_t

  class StaticTreeSpace < TreeSpace

    def initialize(handle = nil, retain: false, auto_release: true,
                   name: "", tree: nil, feature_space: nil, rng: nil)
      if handle
        super(handle, retain: retain, auto_release: auto_release)
      else
        ptr = MemoryPointer::new(:ccs_tree_space_t)
        CCS.error_check CCS.ccs_create_static_tree_space(name, tree, feature_space, rng, ptr)
        super(ptr.read_ccs_tree_space_t, retain: false)
      end
    end

  end

  TreeSpace::Static = StaticTreeSpace

  callback :ccs_dynamic_tree_space_del, [:ccs_tree_space_t], :ccs_result_t
  callback :ccs_dynamic_tree_space_get_child, [:ccs_tree_space_t, :ccs_tree_t, :size_t, :pointer], :ccs_result_t
  callback :ccs_dynamic_tree_space_serialize, [:ccs_tree_space_t, :size_t, :pointer, :pointer], :ccs_result_t
  callback :ccs_dynamic_tree_space_deserialize, [:ccs_tree_t, :ccs_feature_space_t, :size_t, :pointer, :pointer], :ccs_result_t

  class DynamicTreeSpaceVector < FFI::Struct
    attr_accessor :wrappers
    layout :del, :ccs_dynamic_tree_space_del,
           :get_child, :ccs_dynamic_tree_space_get_child,
           :serialize, :ccs_dynamic_tree_space_serialize,
           :deserialize, :ccs_dynamic_tree_space_deserialize
  end
  typedef DynamicTreeSpaceVector.by_value, :ccs_dynamic_tree_space_vector_t

  attach_function :ccs_create_dynamic_tree_space, [:string, :ccs_tree_t, :ccs_feature_space_t, :ccs_rng_t, DynamicTreeSpaceVector.by_ref, :value, :pointer], :ccs_result_t
  attach_function :ccs_dynamic_tree_space_get_tree_space_data, [:ccs_tree_space_t, :pointer], :ccs_result_t

  class DynamicTreeSpace < TreeSpace
    add_property :tree_space_data, :value, :ccs_dynamic_tree_space_get_tree_space_data, memoize: true

    def initialize(handle = nil, retain: false, auto_release: true,
                   name: "", tree: nil, feature_space: nil, rng: nil,
                   del: nil, get_child: nil, serialize: nil, deserialize: nil, tree_space_data: nil)
      if handle
        super(handle, retain: retain, auto_release: auto_release)
      else
        raise CCSError, :CCS_RESULT_ERROR_INVALID_VALUE if get_child.nil?
        vector = DynamicTreeSpace.get_vector(del: del, get_child: get_child, serialize: serialize, deserialize: deserialize)
        ptr = MemoryPointer::new(:ccs_tree_space_t)
        CCS.error_check CCS.ccs_create_dynamic_tree_space(name, tree, feature_space, rng, vector, tree_space_data, ptr)
        h = ptr.read_ccs_tree_space_t
        super(h, retain: false)
        FFI.inc_ref(vector)
        FFI.inc_ref(tree_space_data) unless tree_space_data.nil?
      end
    end

    def self.get_vector(del: nil, get_child: nil, serialize: nil, deserialize: nil)
      vector = DynamicTreeSpaceVector::new
      delwrapper = lambda { |ts|
        begin
          o = CCS::Object.from_handle(ts)
          tsdata = o.tree_space_data
          del.call(o) if del
          FFI.dec_ref(tsdata) unless tsdata.nil?
          FFI.dec_ref(vector)
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
              state = serialize.call(TreeSpace.from_handle(ts))
              raise CCSError, :CCS_RESULT_ERROR_INVALID_VALUE if !state.kind_of?(String)
              raise CCSError, :CCS_RESULT_ERROR_INVALID_VALUE if !p_state.null? && state_size < state.bytesize
              p_state.write_bytes(state, 0, state.bytesize) unless p_state.null?
              Pointer.new(p_state_size).write_size_t(state.bytesize) unless p_state_size.null?
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
          lambda { |t, feature_space, state_size, p_state, p_tree_space_data|
            begin
              state = p_state.null? ? nil : p_state.read_bytes(state_size)
              tree_space_data = deserialize.call(Tree.from_handle(t), feature_space.null? ? nil : FeatureSpace.from_handle(feature_space), state)
              p_tree_space_data.write_value(tree_space_data)
              FFI.inc_ref(tree_space_data)
              CCSError.to_native(:CCS_RESULT_SUCCESS)
            rescue => e
              CCS.set_error(e)
            end
          }
        else
          nil
        end
      vector[:del] = delwrapper
      vector[:get_child] = get_childwrapper
      vector[:serialize] = serializewrapper
      vector[:deserialize] = deserializewrapper
      vector.wrappers = [delwrapper, get_childwrapper, serializewrapper, deserializewrapper]
      vector
    end

  end

  TreeSpace::Dynamic = DynamicTreeSpace

end
