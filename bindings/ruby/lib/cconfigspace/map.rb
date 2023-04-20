module CCS
  attach_function :ccs_create_map, [:pointer], :ccs_result_t
  attach_function :ccs_map_set, [:ccs_map_t, :ccs_datum_t, :ccs_datum_t], :ccs_result_t
  attach_function :ccs_map_exist, [:ccs_map_t, :ccs_datum_t, :pointer], :ccs_result_t
  attach_function :ccs_map_get, [:ccs_map_t, :ccs_datum_t, :pointer], :ccs_result_t
  attach_function :ccs_map_del, [:ccs_map_t, :ccs_datum_t], :ccs_result_t
  attach_function :ccs_map_get_keys, [:ccs_map_t, :size_t, :pointer, :pointer], :ccs_result_t
  attach_function :ccs_map_get_values, [:ccs_map_t, :size_t, :pointer, :pointer], :ccs_result_t
  attach_function :ccs_map_get_pairs, [:ccs_map_t, :size_t, :pointer, :pointer, :pointer], :ccs_result_t

  class Map < Object
    def initialize(handle = nil, retain: false, auto_release: true)
      if handle
        super
      else
        ptr = MemoryPointer::new(:ccs_map_t)
        CCS.error_check CCS.ccs_create_map(ptr)
        super(ptr.read_ccs_map_t, retain: false)
      end
    end

    def self.from_handle(handle, retain: true, auto_release: true)
      self.new(handle, retain: retain, auto_release: auto_release)
    end

    def size
      ptr = MemoryPointer::new(:size_t)
      CCS.error_check CCS.ccs_map_get_keys(@handle, 0, nil, ptr)
      ptr.read_size_t
    end

    def [](key)
      ptr = MemoryPointer::new(:ccs_datum_t)
      k = Datum.from_value(key)
      CCS.error_check CCS.ccs_map_get(@handle, k, ptr)
      Datum::new(ptr).value
    end

    def []=(key, value)
      k = Datum.from_value(key)
      v = Datum.from_value(value)
      CCS.error_check CCS.ccs_map_set(@handle, k, v)
      value
    end

    def include?(key)
      k = Datum.from_value(key)
      ptr = MemoryPointer::new(:ccs_bool_t)
      CCS.error_check CCS.ccs_map_exist(@handle, k, ptr)
      ptr.read_ccs_bool_t == CCS::FALSE ? false : true
    end

    def keys
      sz = size
      return [] if count == 0
      keys = MemoryPointer::new(:ccs_datum_t, sz)
      CCS.error_check CCS.ccs_map_get_keys(@handle, sz, keys, nil)
      sz.times.collect { |i| Datum::new(keys[i]).value }
    end

    def values
      sz = size
      return [] if count == 0
      values = MemoryPointer::new(:ccs_datum_t, sz)
      CCS.error_check CCS.ccs_map_get_values(@handle, sz, values, nil)
      sz.times.collect { |i| Datum::new(values[i]).value }
    end

    def pairs
      sz = size
      return [] if count == 0
      keys = MemoryPointer::new(:ccs_datum_t, sz)
      values = MemoryPointer::new(:ccs_datum_t, sz)
      CCS.error_check CCS.ccs_map_get_pairs(@handle, sz, keys, values, nil)
      sz.times.collect { |i| [Datum::new(keys[i]).value, Datum::new(values[i]).value] }
    end

    def each
      if block_given?
        pairs.each { |k, v|
          yield k, v
        }
        self
      else
        to_enum(:each)
      end
    end

    def each_key
      if block_given?
        keys.each { |k|
          yield k
        }
      else
        to_enum(:each_key)
      end
    end

    def each_value
      if block_given?
        values.each { |v|
          yield v
        }
      else
        to_enum(:each_value)
      end
    end
  end
end
