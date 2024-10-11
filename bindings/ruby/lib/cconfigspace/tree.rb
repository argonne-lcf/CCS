module CCS

  attach_function :ccs_create_tree, [:size_t, :ccs_datum_t, :pointer], :ccs_result_t
  attach_function :ccs_tree_get_value, [:ccs_tree_t, :pointer], :ccs_result_t
  attach_function :ccs_tree_get_arity, [:ccs_tree_t, :pointer], :ccs_result_t
  attach_function :ccs_tree_set_child, [:ccs_tree_t, :size_t, :ccs_tree_t], :ccs_result_t
  attach_function :ccs_tree_get_child, [:ccs_tree_t, :size_t, :pointer], :ccs_result_t
  attach_function :ccs_tree_get_children, [:ccs_tree_t, :size_t, :pointer, :pointer], :ccs_result_t
  attach_function :ccs_tree_get_parent, [:ccs_tree_t, :pointer, :pointer], :ccs_result_t
  attach_function :ccs_tree_get_position, [:ccs_tree_t, :size_t, :pointer, :pointer], :ccs_result_t
  attach_function :ccs_tree_get_values, [:ccs_tree_t, :size_t, :pointer, :pointer], :ccs_result_t
  attach_function :ccs_tree_get_values_at_position, [:ccs_tree_t, :size_t, :pointer, :size_t, :pointer], :ccs_result_t
  attach_function :ccs_tree_get_node_at_position, [:ccs_tree_t, :size_t, :pointer, :pointer], :ccs_result_t
  attach_function :ccs_tree_get_weight, [:ccs_tree_t, :pointer], :ccs_result_t
  attach_function :ccs_tree_set_weight, [:ccs_tree_t, :ccs_float_t], :ccs_result_t
  attach_function :ccs_tree_get_bias, [:ccs_tree_t, :pointer], :ccs_result_t
  attach_function :ccs_tree_set_bias, [:ccs_tree_t, :ccs_float_t], :ccs_result_t
  attach_function :ccs_tree_sample, [:ccs_tree_t, :ccs_rng_t, :pointer], :ccs_result_t
  attach_function :ccs_tree_samples, [:ccs_tree_t, :ccs_rng_t, :size_t, :pointer], :ccs_result_t

  class Tree < Object
    add_property :arity, :size_t, :ccs_tree_get_arity, memoize: true
    add_property :weight, :ccs_float_t, :ccs_tree_get_weight, memoize: false
    add_property :bias, :ccs_float_t, :ccs_tree_get_bias, memoize: false
    add_property :value, :ccs_datum_t, :ccs_tree_get_value, memoize: true
    add_handle_array_property :children, :ccs_tree_t, :ccs_tree_get_children, memoize: false
    add_array_property :position_items, :size_t, :ccs_tree_get_position, memoize: false
    add_array_property :values, :ccs_datum_t, :ccs_tree_get_values, memoize: false

    def initialize(handle = nil, retain: false, auto_release: true,
                   value: nil, arity: nil)
      if handle
        super(handle, retain: retain, auto_release: auto_release)
      else
        ptr = MemoryPointer::new(:ccs_tree_t)
        CCS.error_check CCS.ccs_create_tree(arity, Datum::from_value(value), ptr)
        super(ptr.read_pointer, retain: false)
      end
    end

    def self.from_handle(handle, retain: true, auto_release: true)
      self.new(handle, retain: retain, auto_release: auto_release)
    end

    def weight=(weight)
      CCS.error_check CCS.ccs_tree_set_weight(@handle, weight)
      weight
    end

    def bias=(bias)
      CCS.error_check CCS.ccs_tree_set_bias(@handle, bias)
      bias
    end

    def set_child(index, child)
      CCS.error_check CCS.ccs_tree_set_child(@handle, index, child)
      self
    end

    def get_child(index)
      ptr = MemoryPointer::new(:ccs_tree_t)
      CCS.error_check CCS.ccs_tree_get_child(@handle, index, ptr)
      h = ptr.read_ccs_tree_t
      h.null? ? nil : Tree.from_handle(h)
    end

    def parent
      ptr = MemoryPointer::new(:ccs_tree_t)
      CCS.error_check CCS.ccs_tree_get_parent(@handle, ptr, nil)
      h = ptr.read_ccs_tree_t
      h.null? ? nil : Tree.from_handle(h)
    end

    def index
      ptr1 = MemoryPointer::new(:ccs_tree_t)
      ptr2 = MemoryPointer::new(:size_t)
      CCS.error_check CCS.ccs_tree_get_parent(@handle, ptr1, ptr2)
      ptr1.read_ccs_tree_t.null? ? nil : ptr2.read_size_t
    end

    alias depth num_position_items
    alias position position_items

    def get_values_at_position(position)
      count1 = position.size
      ptr1 = MemoryPointer::new(:size_t, count1)
      ptr1.write_array_of_size_t(position)
      count2 = count1 + 1
      ptr2 = MemoryPointer::new(:ccs_datum_t, count2)
      CCS.error_check CCS.ccs_tree_get_values_at_position(@handle, count1, ptr1, count2, ptr2)
      count2.times.collect { |i| Datum::new(ptr2[i]).value }
    end

    def get_node_at_position(position)
      count = position.size
      ptr1 = MemoryPointer::new(:size_t, count)
      ptr1.write_array_of_size_t(position)
      ptr2 = MemoryPointer::new(:ccs_tree_t)
      CCS.error_check CCS.ccs_tree_get_node_at_position(@handle, count, ptr1, ptr2)
      Tree.from_handle(ptr2.read_ccs_tree_t)
    end

    def sample(rng)
      ptr = MemoryPointer::new(:size_t)
      CCS.error_check CCS.ccs_tree_sample(@handle, rng, ptr)
      v = ptr.read_size_t
      v == arity ? nil : v
    end

    def samples(rng, count)
      return [] if count == 0
      ptr = MemoryPointer::new(:size_t, count)
      CCS.error_check CCS.ccs_tree_samples(@handle, rng, count, ptr)
      a = arity
      ptr.read_array_of_size_t(count).collect { |v| v == a ? nil : v }
    end

  end

end
