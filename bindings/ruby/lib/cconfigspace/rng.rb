module CCS

  attach_function :ccs_create_rng, [:pointer], :ccs_result_t
  attach_function :ccs_rng_set_seed, [:ccs_rng_t, :ulong], :ccs_result_t
  attach_function :ccs_rng_get, [:ccs_rng_t, :pointer], :ccs_result_t
  attach_function :ccs_rng_min, [:ccs_rng_t, :pointer], :ccs_result_t
  attach_function :ccs_rng_max, [:ccs_rng_t, :pointer], :ccs_result_t
  attach_function :ccs_rng_uniform, [:ccs_rng_t, :pointer], :ccs_result_t

  class Rng < Object
    add_property :min, :ulong, :ccs_rng_min, memoize: true
    add_property :max, :ulong, :ccs_rng_max, memoize: true
    add_property :get, :ulong, :ccs_rng_get
    add_property :uniform, :ccs_float_t, :ccs_rng_uniform

    def initialize(handle = nil, retain: false, auto_release: true)
      if handle
        super
      else
        ptr = MemoryPointer::new(:ccs_rng_t)
        CCS.error_check CCS.ccs_create_rng(ptr)
        super(ptr.read_pointer, retain: false)
      end
    end

    def self.from_handle(handle, retain: true, auto_release: true)
      self.new(handle, retain: retain, auto_release: auto_release)
    end

    def seed=(s)
      CCS.error_check CCS.ccs_rng_set_seed(@handle, s)
      s
    end

  end

  DefaultRng = Rng::new
end
