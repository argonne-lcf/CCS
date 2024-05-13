module CCS

  attach_function :ccs_create_tree_configuration, [:ccs_tree_space_t, :ccs_features_t, :size_t, :pointer, :pointer], :ccs_result_t
  attach_function :ccs_tree_configuration_get_tree_space, [:ccs_tree_configuration_t, :pointer], :ccs_result_t
  attach_function :ccs_tree_configuration_get_features, [:ccs_tree_configuration_t, :pointer], :ccs_result_t
  attach_function :ccs_tree_configuration_get_position, [:ccs_tree_configuration_t, :size_t, :pointer, :pointer], :ccs_result_t
  attach_function :ccs_tree_configuration_get_values, [:ccs_tree_configuration_t, :size_t, :pointer, :pointer], :ccs_result_t
  attach_function :ccs_tree_configuration_get_node, [:ccs_tree_configuration_t, :pointer], :ccs_result_t
  attach_function :ccs_tree_configuration_check, [:ccs_tree_configuration_t, :pointer], :ccs_result_t
  attach_function :ccs_tree_configuration_hash, [:ccs_tree_configuration_t, :pointer], :ccs_result_t
  attach_function :ccs_tree_configuration_cmp, [:ccs_tree_configuration_t, :ccs_tree_configuration_t, :pointer], :ccs_result_t

  class TreeConfiguration < Object
    include Comparable
    add_handle_property :tree_space, :ccs_tree_space_t, :ccs_tree_configuration_get_tree_space, memoize: true
    add_handle_property :node, :ccs_tree_t, :ccs_tree_configuration_get_node, memoize: true
    add_property :hash, :ccs_hash_t, :ccs_tree_configuration_hash, memoize: true
    add_optional_handle_property :features, :ccs_features_t, :ccs_tree_configuration_get_features, memoize: true

    def initialize(handle = nil, retain: false, auto_release: true,
                   tree_space: nil, features: nil, position: nil)
      if (handle)
        super(handle, retain: retain, auto_release: auto_release)
      else
        count = position.size
        ptr1 = MemoryPointer::new(:size_t, count)
        ptr1.write_array_of_size_t(position)
        ptr2 = MemoryPointer::new(:ccs_tree_configuration_t)
        CCS.error_check CCS.ccs_create_tree_configuration(tree_space, features, count, ptr1, ptr2)
        super(ptr2.read_ccs_tree_configuration_t, retain: false)
      end
    end

    def self.from_handle(handle, retain: true, auto_release: true)
      self::new(handle, retain: retain, auto_release: auto_release)
    end

    def position_size
      @position_size ||= begin
        ptr = MemoryPointer::new(:size_t)
        CCS.error_check CCS.ccs_tree_configuration_get_position(@handle, 0, nil, ptr)
        ptr.read_size_t
      end
    end

    def position
      @position ||= begin
        count = position_size
        ptr = MemoryPointer::new(:size_t, count)
        CCS.error_check CCS.ccs_tree_configuration_get_position(@handle, count, ptr, nil)
        ptr.read_array_of_size_t(count)
      end
    end

    def values
      @values ||= begin
        count = position_size + 1
        ptr = MemoryPointer::new(:ccs_datum_t, count)
        CCS.error_check CCS.ccs_tree_configuration_get_values(@handle, count, ptr, nil)
        count.times.collect { |i| Datum::new(ptr[i]).value }
      end
    end

    def check
      ptr = MemoryPointer::new(:ccs_bool_t)
      CCS.error_check CCS.ccs_tree_configuration_check(@handle, ptr)
      ptr.read_ccs_bool_t == CCS::FALSE ? false : true
    end

    def <=>(other)
      ptr = MemoryPointer::new(:int)
      CCS.error_check CCS.ccs_tree_configuration_cmp(@handle, other, ptr)
      return ptr.read_int
    end

  end
 
end
