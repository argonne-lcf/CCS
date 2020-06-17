module CCS

  attach_function :ccs_rng_create, [:pointer], :ccs_result_t
  attach_function :ccs_rng_set_seed, [:ccs_rng_t, :ulong], :ccs_result_t
  attach_function :ccs_rng_get, [:ccs_rng_t, :pointer], :ccs_result_t
  attach_function :ccs_rng_min, [:ccs_rng_t, :pointer], :ccs_result_t
  attach_function :ccs_rng_max, [:ccs_rng_t, :pointer], :ccs_result_t

  class Rng < Object
    add_property :min, :ulong, :ccs_rng_min, memoize: true
    add_property :max, :ulong, :ccs_rng_max, memoize: true

    def initialize(handle = nil, retain: false)
      if handle
        super
      else
        ptr = MemoryPointer::new(:ccs_rng_t)
        res = CCS.ccs_rng_create(ptr)
        CCS.error_check(res)
        super(ptr.read_pointer, retain: false)
      end
    end

    def self.from_handle(handle)
      self.new(handle, retain: true)
    end

    def seed=(s)
      res = CCS.ccs_rng_set_seed(@handle, s)
      CCS.error_check(res)
      s
    end

    def get
      ptr = MemoryPointer::new(:ulong)
      res = CCS.ccs_rng_get(@handle, ptr)
      CCS.error_check(res)
      ptr.read_ulong
    end

    def uniform
      ptr = MemoryPointer::new(:ccs_float_t)
      res = CCS.ccs_rng_get(@handle, ptr)
      CCS.error_check(res)
      ptr.read_ccs_float_t
    end
  end
end
