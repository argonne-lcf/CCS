require 'singleton'
module CCS
  extend FFI::Library

  if ENV["LIBCCONFIGSPACE_SO"]
    ffi_lib ENV["LIBCCONFIGSPACE_SO"]
  else
    ffi_lib "cconfigspace"
  end

  class Pointer < FFI::Pointer
    def initialize(*args)
      if args.length == 2 then
        super(CCS::find_type(args[0]), args[1])
      else
        super(*args)
      end
    end
  end

  class MemoryPointer < FFI::MemoryPointer
    def initialize(size, count = 1, clear = true)
      if size.is_a?(Symbol)
        size = CCS::find_type(size)
      end
      super(size, count, clear)
    end
  end

  typedef :double, :ccs_float_t
  typedef :int64, :ccs_int_t
  typedef :int32, :ccs_bool_t
  typedef :int32, :ccs_result_t
  typedef :uint32, :ccs_hash_t

  class MemoryPointer
    alias read_ccs_float_t  read_double
    alias read_array_of_ccs_float_t  read_array_of_double
    alias write_array_of_ccs_float_t  write_array_of_double
    alias read_ccs_int_t    read_int64
    alias read_array_of_ccs_int_t  read_array_of_int64
    alias write_array_of_ccs_int_t  write_array_of_int64
    alias read_ccs_bool_t   read_int32
    alias read_ccs_result_t read_int32
    alias read_ccs_hash_t   read_uint32
    if FFI.find_type(:size_t).size == 8
      alias read_size_t read_uint64
    else
      alias read_size_t read_uint32
    end
  end

  class Version < FFI::Struct
    layout :revision, :uint16,
           :patch,    :uint16,
           :minor,    :uint16,
           :major,    :uint16
  end
  typedef Version.by_value, :ccs_version_t

  TRUE = 1
  FALSE = 0

  typedef :pointer, :ccs_rng_t
  typedef :pointer, :ccs_distribution_t
  typedef :pointer, :ccs_hyperparameter_t
  typedef :pointer, :ccs_expression_t
  typedef :pointer, :ccs_context_t
  typedef :pointer, :ccs_configuration_space_t
  typedef :pointer, :ccs_configuration_t
  typedef :pointer, :ccs_objective_space_t
  typedef :pointer, :ccs_evaluation_t
  typedef :pointer, :ccs_tuner_t
  typedef :pointer, :ccs_object_t

  Error = enum FFI::Type::INT32, :ccs_error_t, [
    :CCS_SUCCESS,
    :CCS_INVALID_OBJECT,
    :CCS_INVALID_VALUE,
    :CCS_INVALID_TYPE,
    :CCS_INVALID_SCALE,
    :CCS_INVALID_DISTRIBUTION,
    :CCS_INVALID_HYPERPARAMETER,
    :CCS_INVALID_CONFIGURATION,
    :CCS_INVALID_NAME,
    :CCS_INVALID_CONDITION,
    :CCS_INVALID_GRAPH,
    :CCS_TYPE_NOT_COMPARABLE,
    :CCS_INVALID_BOUNDS,
    :CCS_OUT_OF_BOUNDS,
    :CCS_SAMPLING_UNSUCCESSFUL,
    :CCS_INACTIVE_HYPERPARAMETER,
    :CCS_OUT_OF_MEMORY,
    :CCS_UNSUPPORTED_OPERATION ]

  ObjectType = enum FFI::Type::INT32, :ccs_object_type_t, [
    :CCS_RNG,
    :CCS_DISTRIBUTION,
    :CCS_HYPERPARAMETER,
    :CCS_EXPRESSION,
    :CCS_CONFIGURATION_SPACE,
    :CCS_CONFIGURATION,
    :CCS_OBJECTIVE_SPACE,
    :CCS_EVALUATION,
    :CCS_TUNER ]
  class MemoryPointer
    def read_ccs_object_type_t
      ObjectType.from_native(read_int32, nil)
    end
  end

  DataType = enum FFI::Type::INT64, :ccs_data_type_t, [
    :CCS_NONE,
    :CCS_INTEGER,
    :CCS_FLOAT,
    :CCS_BOOLEAN,
    :CCS_STRING,
    :CCS_INACTIVE,
    :CCS_OBJECT ]

  NumericType = enum FFI::Type::INT64, :ccs_numeric_type_t, [
    :CCS_NUM_INTEGER, DataType.to_native(:CCS_INTEGER, nil),
    :CCS_NUM_FLOAT, DataType.to_native(:CCS_FLOAT, nil) ]
  class MemoryPointer
    def read_ccs_numeric_type_t
      NumericType.from_native(read_int32, nil)
    end
  end

  class Numeric < FFI::Union
    layout :f, :ccs_float_t,
           :i, :ccs_int_t
  end
  typedef Numeric.by_value, :ccs_numeric_t

  class Value < FFI::Union
    layout :f, :ccs_float_t,
           :i, :ccs_int_t,
           :s, :pointer,
           :o, :ccs_object_t
  end
  typedef Value.by_value, :ccs_value_t

  class InactiveClass
    include Singleton
  end
  Inactive = InactiveClass.instance

  class Datum < FFI::Struct
    layout :value, :ccs_value_t,
           :type, :ccs_data_type_t
  end
  class Datum
    NONE = self::new
    NONE[:type] = :CCS_NONE
    NONE[:value][:i] = 0
    TRUE = self::new
    TRUE[:type] = :CCS_BOOLEAN
    TRUE[:value][:i] = CCS::TRUE
    FALSE = self::new
    FALSE[:type] = :CCS_BOOLEAN
    FALSE[:value][:i] = CCS::FALSE
    INACTIVE = self::new
    INACTIVE[:type] = :CCS_INACTIVE
    INACTIVE[:value][:i] = 0
    def value
      case self[:type]
      when :NONE
        nil
      when :CCS_INTEGER
        self[:value][:i]
      when :CCS_FLOAT
        self[:value][:f]
      when :CCS_BOOLEAN
        self[:value][:i] == :CCS_FALSE ? false : true
      when :CCS_STRING
        self[:value][:s].read_string
      when :CCS_INACTIVE
        Inactive
      when :CCS_OBJECT
        Object::from_handle(self[:value][:o])
      end
    end

    def self.from_value(v)
      case v
      when nil
        NONE
      when true
        TRUE
      when false
        FALSE
      when Inactive
        INACTIVE
      when Float
        d = self::new
        d[:type] = :CCS_FLOAT
        d[:value][:f] = v
        d
      when Integer
        d = self::new
        d[:type] = :CCS_INTEGER
        d[:value][:i] = v
        d
      when String
        d = self::new
        ptr = MemoryPointer::from_string(v)
        d.instance_variable_set(:@string, ptr)
        d[:type] = :STRING
        d[:valus][:s] = ptr
        d
      when Object
        d = self::new
        d[:type] = :CCS_OBJECT
        d[:value][:o] = v.handle
        d.instance_variable_set(:@object, v)
        d
      else
        raise StandardError, :CCS_INVALID_VALUE
      end
    end
  end
  typedef Datum.by_value, :ccs_datum_t

  attach_function :ccs_init, [], :ccs_result_t
  attach_function :ccs_get_version, [], :ccs_version_t
  attach_function :ccs_retain_object, [:ccs_object_t], :ccs_result_t
  attach_function :ccs_release_object, [:ccs_object_t], :ccs_result_t
  attach_function :ccs_object_get_type, [:ccs_object_t, :pointer], :ccs_result_t
  attach_function :ccs_object_get_refcount, [:ccs_object_t, :pointer], :ccs_result_t

  class << self
    alias version ccs_get_version
  end

  def self.error_check(result)
    if result < 0
      raise StandardError, Error.from_native(-result, nil)
    end
  end

  def self.init
    res = ccs_init
    error_check(res)
    self
  end

  class Object
    class Releaser
      def initialize(handle)
        @handle = handle
      end

      def call(id)
        CCS.ccs_release_object(@handle)
      end
    end
    def self.add_property(name, type, accessor, memoize: false)
      src = ""
      src << "def #{name}\n"
      src << "  @#{name} ||= begin\n" if memoize
      src << "  ptr = MemoryPointer::new(:#{type})\n"
      src << "  res = CCS.#{accessor}(@handle, ptr)\n"
      src << "  CCS.error_check(res)\n"
      src << "  ptr.read_#{type}\n"
      src << "  end\n" if memoize
      src << "end\n"
      class_eval src
    end

    add_property :object_type, :ccs_object_type_t, :ccs_object_get_type, memoize: true
    add_property :refcount, :uint32, :ccs_object_get_refcount
    attr_reader :handle
    def initialize(handle, retain: false, auto_release: true)
      @handle = handle
      if retain
        res = CCS.ccs_retain_object(handle)
        CCS.error_check(res)
      end
      ObjectSpace.define_finalizer(self, Releaser::new(handle)) if auto_release
    end

    def self.from_handle(handle)
      ptr = MemoryPointer::new(:ccs_object_type_t)
      res = CCS.ccs_object_get_type(handle, ptr)
      CCS.error_check(res)
      case ptr.read_ccs_object_type_t
      when :CCS_RNG
        CCS::Rng::from_handle(handle)
      when :CCS_DISTRIBUTION
        CCS::Distribution::from_handle(handle)
      when :CCS_HYPERPARAMETER
        CCS::Hyperparameter::from_handle(handle)
      when :CCS_EXPRESSION
        CCS::Expression::from_handle(handle)
      when :CCS_CONFIGURATION_SPACE
        CCS::ConfigurationSpace::from_handle(handle)
      when :CCS_CONFIGURATION
        CCS::Configuration::from_handle(handle)
      when :CCS_OBJECTIVE_SPACE
        CCS::ObjectiveSpace::from_handle(handle)
      when :CCS_EVALUATION
        CCS::Evaluation::from_handle(handle)
      when :CCS_TUNER
        CCS::Tuner::from_handle(handle)
      else
        raise StandardError, :CCS_INVALID_OBJECT
      end
    end

    def to_ptr
      @handle
    end

  end

end
