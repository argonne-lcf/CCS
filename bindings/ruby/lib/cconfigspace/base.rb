require 'singleton'
require 'yaml'
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
    alias read_ccs_float_t  read_double
    alias get_ccs_float_t   get_double
    alias read_array_of_ccs_float_t  read_array_of_double
    alias write_array_of_ccs_float_t  write_array_of_double
    alias read_ccs_int_t    read_int64
    alias get_ccs_int_t     get_int64
    alias read_array_of_ccs_int_t  read_array_of_int64
    alias write_array_of_ccs_int_t  write_array_of_int64
    alias read_ccs_bool_t   read_int32
    alias read_ccs_result_t read_int32
    alias read_ccs_hash_t   read_uint32
    if FFI.find_type(:size_t).size == 8
      alias read_size_t read_uint64
      alias write_size_t write_uint64
      alias write_array_of_size_t write_array_of_uint64
    else
      alias read_size_t read_uint32
      alias write_size_t write_uint32
      alias write_array_of_size_t write_array_of_uint32
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
    alias get_ccs_float_t   get_double
    alias read_array_of_ccs_float_t  read_array_of_double
    alias write_array_of_ccs_float_t  write_array_of_double
    alias read_ccs_int_t    read_int64
    alias get_ccs_int_t     get_int64
    alias read_array_of_ccs_int_t  read_array_of_int64
    alias write_array_of_ccs_int_t  write_array_of_int64
    alias read_ccs_bool_t   read_int32
    alias read_ccs_result_t read_int32
    alias read_ccs_hash_t   read_uint32
    if FFI.find_type(:size_t).size == 8
      alias read_size_t read_uint64
      alias write_size_t write_uint64
      alias write_array_of_size_t write_array_of_uint64
    else
      alias read_size_t read_uint32
      alias write_size_t write_uint32
      alias write_array_of_size_t write_array_of_uint32
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

  typedef :pointer, :ccs_object_t
  typedef :ccs_object_t, :ccs_rng_t
  typedef :ccs_object_t, :ccs_distribution_t
  typedef :ccs_object_t, :ccs_hyperparameter_t
  typedef :ccs_object_t, :ccs_expression_t
  typedef :ccs_object_t, :ccs_context_t
  typedef :ccs_object_t, :ccs_configuration_space_t
  typedef :ccs_object_t, :ccs_binding_t
  typedef :ccs_object_t, :ccs_configuration_t
  typedef :ccs_object_t, :ccs_features_space_t
  typedef :ccs_object_t, :ccs_features_t
  typedef :ccs_object_t, :ccs_objective_space_t
  typedef :ccs_object_t, :ccs_evaluation_t
  typedef :ccs_object_t, :ccs_features_evaluation_t
  typedef :ccs_object_t, :ccs_tuner_t
  typedef :ccs_object_t, :ccs_features_tuner_t
  typedef :ccs_object_t, :ccs_map_t
  typedef :ccs_object_t, :ccs_error_stack_t
  class MemoryPointer
    alias read_ccs_object_t read_pointer
    alias read_ccs_rng_t read_ccs_object_t
    alias read_ccs_distribution_t read_ccs_object_t
    alias read_ccs_hyperparameter_t read_ccs_object_t
    alias read_ccs_expression_t read_ccs_object_t
    alias read_ccs_context_t read_ccs_object_t
    alias read_ccs_configuration_space_t read_ccs_object_t
    alias read_ccs_binding_t read_ccs_object_t
    alias read_ccs_configuration_t read_ccs_object_t
    alias read_ccs_features_space_t read_ccs_object_t
    alias read_ccs_features_t read_ccs_object_t
    alias read_ccs_objective_space_t read_ccs_object_t
    alias read_ccs_evaluation_t read_ccs_object_t
    alias read_ccs_features_evaluation_t read_ccs_object_t
    alias read_ccs_tuner_t read_ccs_object_t
    alias read_ccs_features_tuner_t read_ccs_object_t
    alias read_ccs_map_t read_ccs_object_t
    alias read_ccs_error_stack_t read_ccs_object_t
  end

  ObjectType = enum FFI::Type::INT32, :ccs_object_type_t, [
    :CCS_RNG,
    :CCS_DISTRIBUTION,
    :CCS_HYPERPARAMETER,
    :CCS_EXPRESSION,
    :CCS_CONFIGURATION_SPACE,
    :CCS_CONFIGURATION,
    :CCS_OBJECTIVE_SPACE,
    :CCS_EVALUATION,
    :CCS_TUNER,
    :CCS_FEATURES_SPACE,
    :CCS_FEATURES,
    :CCS_FEATURES_EVALUATION,
    :CCS_FEATURES_TUNER,
    :CCS_MAP,
    :CCS_ERROR_STACK ]

  Error = enum FFI::Type::INT32, :ccs_error_t, [
    :CCS_AGAIN,                    1,
    :CCS_SUCCESS,                  0,
    :CCS_INVALID_OBJECT,          -1,
    :CCS_INVALID_VALUE,           -2,
    :CCS_INVALID_TYPE,            -3,
    :CCS_INVALID_SCALE,           -4,
    :CCS_INVALID_DISTRIBUTION,    -5,
    :CCS_INVALID_EXPRESSION,      -6,
    :CCS_INVALID_HYPERPARAMETER,  -7,
    :CCS_INVALID_CONFIGURATION,   -8,
    :CCS_INVALID_NAME,            -9,
    :CCS_INVALID_CONDITION,      -10,
    :CCS_INVALID_TUNER,          -11,
    :CCS_INVALID_GRAPH,          -12,
    :CCS_TYPE_NOT_COMPARABLE,    -13,
    :CCS_INVALID_BOUNDS,         -14,
    :CCS_OUT_OF_BOUNDS,          -15,
    :CCS_SAMPLING_UNSUCCESSFUL,  -16,
    :CCS_OUT_OF_MEMORY,          -17,
    :CCS_UNSUPPORTED_OPERATION,  -18,
    :CCS_INVALID_EVALUATION,     -19,
    :CCS_INVALID_FEATURES,       -20,
    :CCS_INVALID_FEATURES_TUNER, -21,
    :CCS_INVALID_FILE_PATH,      -22,
    :CCS_NOT_ENOUGH_DATA,        -23,
    :CCS_HANDLE_DUPLICATE,       -24,
    :CCS_INVALID_HANDLE,         -25,
    :CCS_SYSTEM_ERROR,           -26,
    :CCS_EXTERNAL_ERROR,         -27 ]

  class MemoryPointer
    def read_ccs_object_type_t
      ObjectType.from_native(read_int32, nil)
    end
    def read_ccs_error_t
      Error.from_native(read_int32, nil)
    end
  end

  DataType = enum FFI::Type::INT32, :ccs_data_type_t, [
    :CCS_NONE,
    :CCS_INTEGER,
    :CCS_FLOAT,
    :CCS_BOOLEAN,
    :CCS_STRING,
    :CCS_INACTIVE,
    :CCS_OBJECT ]

  DatumFlag = enum FFI::Type::INT32, :ccs_datum_flag_t, [
    :CCS_FLAG_DEFAULT, 0,
    :CCS_FLAG_TRANSIENT, (1 << 0),
    :CCS_FLAG_UNPOOLED, (1 << 1),
    :CCS_FLAG_ID, (1 << 2) ]

  DatumFlags = bitmask FFI::Type::UINT32, :ccs_datum_flags_t, [
    :CCS_FLAG_TRANSIENT,
    :CCS_FLAG_UNPOOLED ]

  NumericType = enum FFI::Type::INT32, :ccs_numeric_type_t, [
    :CCS_NUM_INTEGER, DataType.to_native(:CCS_INTEGER, nil),
    :CCS_NUM_FLOAT, DataType.to_native(:CCS_FLOAT, nil) ]

  class MemoryPointer
    def read_ccs_numeric_type_t
      NumericType.from_native(read_int32, nil)
    end
  end

  SerializeFormat = enum FFI::Type::INT32, :ccs_serialize_format_t, [
    :CCS_SERIALIZE_FORMAT_BINARY ]

  SerializeOperation = enum FFI::Type::INT32, :ccs_serialize_operation_t, [
    :CCS_SERIALIZE_OPERATION_SIZE,
    :CCS_SERIALIZE_OPERATION_MEMORY,
    :CCS_SERIALIZE_OPERATION_FILE,
    :CCS_SERIALIZE_OPERATION_FILE_DESCRIPTOR ]

  SerializeOption = enum FFI::Type::INT32, :ccs_serialize_option_t, [
    :CCS_SERIALIZE_OPTION_END, 0,
    :CCS_SERIALIZE_OPTION_NON_BLOCKING,
    :CCS_SERIALIZE_OPTION_CALLBACK ]

  DeserializeOptions = enum FFI::Type::INT32, :ccs_deserialize_option_t, [
    :CCS_DESERIALIZE_OPTION_END, 0,
    :CCS_DESERIALIZE_OPTION_HANDLE_MAP,
    :CCS_DESERIALIZE_OPTION_VECTOR,
    :CCS_DESERIALIZE_OPTION_DATA,
    :CCS_DESERIALIZE_OPTION_NON_BLOCKING,
    :CCS_DESERIALIZE_OPTION_CALLBACK ]

  class Numeric < FFI::Union
    layout :f, :ccs_float_t,
           :i, :ccs_int_t
    def value(type)
      case type
      when :CCS_NUM_FLOAT
        self[:f]
      when :CCS_NUM_INTEGER
        self[:i]
      else
        raise CCSError, :CCS_INVALID_TYPE
      end
    end

    def self.from_value(v)
      case v
      when Float
        n = self::new
        n[:f] = v
        n
      when Integer
        n = self::new
        n[:i] = v
        n
      else
        raise CCSError, :CCS_INVALID_TYPE
      end
    end

    def value=(v)
      case v
      when Float
        self[:f] = v
      when Integer
        self[:i] = v
      else
        raise CCSError, :CCS_INVALID_TYPE
      end
    end
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
    def to_s
      "inactive"
    end

    def inspect
      "inactive"
    end
  end
  Inactive = InactiveClass.instance

  class Datum < FFI::Struct
    layout :value, :ccs_value_t,
           :type, :ccs_data_type_t,
           :flags, :ccs_datum_flags_t
  end
  class Datum
    NONE = self::new
    NONE[:type] = :CCS_NONE
    NONE[:value][:i] = 0
    NONE[:flags] = 0
    TRUE = self::new
    TRUE[:type] = :CCS_BOOLEAN
    TRUE[:value][:i] = CCS::TRUE
    TRUE[:flags] = 0
    FALSE = self::new
    FALSE[:type] = :CCS_BOOLEAN
    FALSE[:value][:i] = CCS::FALSE
    FALSE[:flags] = 0
    INACTIVE = self::new
    INACTIVE[:type] = :CCS_INACTIVE
    INACTIVE[:value][:i] = 0
    INACTIVE[:flags] = 0

    def flags
      self[:flags]
    end

    def flags=(v)
      self[:flags] = v
    end

    def value
      case self[:type]
      when :CCS_NONE
        nil
      when :CCS_INTEGER
        self[:value][:i]
      when :CCS_FLOAT
        self[:value][:f]
      when :CCS_BOOLEAN
        self[:value][:i] == CCS::FALSE ? false : true
      when :CCS_STRING
        self[:value][:s].read_string
      when :CCS_INACTIVE
        Inactive
      when :CCS_OBJECT
        if self[:flags].include?( :CCS_FLAG_ID )
          Object::new(self[:value][:o], retain: false, auto_release: false)
        else
          Object::from_handle(self[:value][:o])
        end
      else
        raise CCSError, :CCS_INVALID_TYPE
      end
    end

    def set_value(v, string_store: nil, object_store: nil)
      @string = nil if defined?(@string) && @string
      @object = nil if defined?(@object) && @object
      case v
      when nil
        self[:type] = :CCS_NONE
        self[:value][:i] = 0
        self[:flags] = 0
      when true
        self[:type] = :CCS_BOOLEAN
        self[:value][:i] = 1
        self[:flags] = 0
      when false
        self[:type] = :CCS_BOOLEAN
        self[:value][:i] = 0
        self[:flags] = 0
      when Inactive
        self[:type] = :CCS_INACTIVE
        self[:value][:i] = 0
        self[:flags] = 0
      when Float
        self[:type] = :CCS_FLOAT
        self[:value][:f] = v
        self[:flags] = 0
      when Integer
        self[:type] = :CCS_INTEGER
        self[:value][:i] = v
        self[:flags] = 0
      when String
        ptr = MemoryPointer::from_string(v)
        if string_store
          string_store.push ptr
        else
          @string = ptr
        end
        self[:type] = :CCS_STRING
        self[:value][:s] = ptr
        self[:flags] = :CCS_FLAG_TRANSIENT
      when Object
        self[:type] = :CCS_OBJECT
        self[:value][:o] = v.handle
        if v.class == Object
          self[:flags] = :CCS_FLAG_ID
        else
          if object_store
            object_store.push v
          else
            @object = v
          end
          self[:flags] = :CCS_FLAG_TRANSIENT
        end
      else
        raise CCSError, :CCS_INVALID_TYPE
      end
      self
    end

    def value=(v)
      set_value(v)
      v
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
        d[:flags] = 0
        d
      when Integer
        d = self::new
        d[:type] = :CCS_INTEGER
        d[:value][:i] = v
        d[:flags] = 0
        d
      when String
        d = self::new
        ptr = MemoryPointer::from_string(v)
        d.instance_variable_set(:@string, ptr)
        d[:type] = :CCS_STRING
        d[:value][:s] = ptr
        d[:flags] = :CCS_FLAG_TRANSIENT
        d
      when Object
        d = self::new
        d[:type] = :CCS_OBJECT
        d[:value][:o] = v.handle
        if v.class == Object
          d[:flags] = :CCS_FLAG_ID
        else
          d[:flags] = :CCS_FLAG_TRANSIENT
          d.instance_variable_set(:@object, v)
        end
        d
      else
        raise CCSError, :CCS_INVALID_TYPE
      end
    end
  end
  typedef Datum.by_value, :ccs_datum_t

  attach_function :ccs_init, [], :ccs_error_t
  attach_function :ccs_fini, [], :ccs_error_t
  attach_function :ccs_get_error_name, [:ccs_error_t, :pointer], :ccs_error_t
  attach_function :ccs_get_version, [], :ccs_version_t
  attach_function :ccs_retain_object, [:ccs_object_t], :ccs_error_t
  attach_function :ccs_release_object, [:ccs_object_t], :ccs_error_t
  attach_function :ccs_object_get_type, [:ccs_object_t, :pointer], :ccs_error_t
  attach_function :ccs_object_get_refcount, [:ccs_object_t, :pointer], :ccs_error_t
  callback :ccs_object_release_callback, [:ccs_object_t, :pointer], :void
  attach_function :ccs_object_set_destroy_callback, [:ccs_object_t, :ccs_object_release_callback, :pointer], :ccs_error_t
  attach_function :ccs_object_set_user_data, [:ccs_object_t, :value], :ccs_error_t
  attach_function :ccs_object_get_user_data, [:ccs_object_t, :pointer], :ccs_error_t
  callback :ccs_object_serialize_callback, [:ccs_object_t, :size_t, :pointer, :pointer, :value], :ccs_error_t
  attach_function :ccs_object_set_serialize_callback, [:ccs_object_t, :ccs_object_serialize_callback, :value], :ccs_error_t
  callback :ccs_object_deserialize_callback, [:ccs_object_t, :size_t, :pointer, :value], :ccs_error_t
  attach_function :ccs_object_serialize, [:ccs_object_t, :ccs_serialize_format_t, :ccs_serialize_operation_t, :varargs], :ccs_error_t
  attach_function :ccs_object_deserialize, [:ccs_object_t, :ccs_serialize_format_t, :ccs_serialize_operation_t, :varargs], :ccs_error_t

  class << self
    alias version ccs_get_version
  end

  class CCSError < StandardError
    attr_reader :error_stack
    attr_reader :code

    def initialize(sym)
      @sym = sym
      @code = CCSError.to_native(@sym)
      @error_stack = CCS.get_thread_error
      @elems = []
      msg = "#{sym}:"
      if @error_stack
        @elems =  @error_stack.elems.collect { |e| "#{e.file}:#{e.line}:in `#{e.func}'" }
        msg << " #{@error_stack.message}" unless @error_stack.message.empty?
      end
      super(msg)
    end

    def set_backtrace(bt)
      super(@elems + bt.reject { |e| e.match(/cconfigspace\/base.*error_check'/) })
    end

    def self.to_native(sym)
      Error.to_native(sym, nil)
    end

    def to_native
      @code
    end
  end

  def self.error_check(result)
    if result != :CCS_SUCCESS && result != :CCS_AGAIN
      raise CCSError, result
    end
  end

  def self.set_error(exc)
    if exc.kind_of?(CCSError)
      stack = exc.error_stack
      stack = ErrorStack.new(error: exc.code) unless stack
    else
      stack = ErrorStack.new(error: :CCS_EXTERNAL_ERROR, message: exc.inspect)
    end
    depth = caller.size - 1
    depth = 1 if depth < 1
    exc.backtrace_locations[0..-depth].each { |s|
      stack.push(s.path, s.lineno, s.label)
    }
    CCS.set_thread_error(stack)
    CCSError.to_native(stack.code)
  end

  def self.init
    error_check ccs_init
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
      src << "  CCS.error_check CCS.#{accessor}(@handle, ptr)\n"
      src << "  ptr.read_#{type}\n"
      src << "  end\n" if memoize
      src << "end\n"
      class_eval src
    end

    def self.add_handle_property(name, type, accessor, memoize: false)
      src = ""
      src << "def #{name}\n"
      src << "  @#{name} ||= begin\n" if memoize
      src << "  ptr = MemoryPointer::new(:#{type})\n"
      src << "  CCS.error_check CCS.#{accessor}(@handle, ptr)\n"
      src << "  Object::from_handle(ptr.read_#{type})\n"
      src << "  end\n" if memoize
      src << "end\n"
      class_eval src
    end

    add_property :object_type, :ccs_object_type_t, :ccs_object_get_type, memoize: true
    add_property :refcount, :uint32, :ccs_object_get_refcount
    attr_reader :handle

    def initialize(handle, retain: false, auto_release: true)
      if !handle
        raise CCSError, :CCS_INVALID_OBJECT
      end
      @handle = handle
      if retain
        CCS.error_check CCS.ccs_retain_object(handle)
      end
      ObjectSpace.define_finalizer(self, Releaser::new(handle)) if auto_release
    end

    def self._from_handle(handle, retain: true, auto_release: true)
      ptr = MemoryPointer::new(:ccs_object_type_t)
      CCS.error_check CCS.ccs_object_get_type(handle, ptr)
      case ptr.read_ccs_object_type_t
      when :CCS_RNG
        CCS::Rng
      when :CCS_DISTRIBUTION
        CCS::Distribution
      when :CCS_HYPERPARAMETER
        CCS::Hyperparameter
      when :CCS_EXPRESSION
        CCS::Expression
      when :CCS_CONFIGURATION_SPACE
        CCS::ConfigurationSpace
      when :CCS_CONFIGURATION
        CCS::Configuration
      when :CCS_FEATURES_SPACE
	CCS::FeaturesSpace
      when :CCS_FEATURES
	CCS::Features
      when :CCS_OBJECTIVE_SPACE
        CCS::ObjectiveSpace
      when :CCS_EVALUATION
        CCS::Evaluation
      when :CCS_FEATURES_EVALUATION
        CCS::FeaturesEvaluation
      when :CCS_TUNER
        CCS::Tuner
      when :CCS_FEATURES_TUNER
        CCS::FeaturesTuner
      when :CCS_MAP
        CCS::Map
      else
        raise CCSError, :CCS_INVALID_OBJECT
      end.from_handle(handle, retain: retain, auto_release: auto_release)
    end

    private_class_method :_from_handle

    def self.from_handle(handle)
      ptr2 = MemoryPointer::new(:int32)
      CCS.error_check CCS.ccs_object_get_refcount(handle, ptr2)
      opts = ptr2.read_int32 == 0 ? {retain: false, auto_release: false} : {}
      _from_handle(handle, **opts)
    end

    def to_ptr
      @handle
    end

    def set_destroy_callback(user_data: nil, &block)
      CCS.set_destroy_callback(@handle, user_data: user_data, &block)
      self
    end

    def set_serialize_callback(user_data: nil, &block)
      CCS.set_serialize_callback(@handle, user_data: user_data, &block)
      self
    end

    def serialize(format: :binary, path: nil, file_descriptor: nil, callback: nil, callback_data: nil)
      raise CCSError, :CCS_INVALID_VALUE if format != :binary
      raise CCSError, :CCS_INVALID_VALUE if path && file_descriptor
      options = []
      if callback
        cb_wrapper = CCS.get_serialize_wrapper(&callback)
        options.concat [:ccs_serialize_option_t, :CCS_SERIALIZE_OPTION_CALLBACK, :ccs_object_serialize_callback, cb_wrapper, :value, callback_data]
      elsif CCS.default_user_data_serializer
        options.concat [:ccs_serialize_option_t, :CCS_SERIALIZE_OPTION_CALLBACK, :ccs_object_serialize_callback, CCS.default_user_data_serializer, :value, nil]
      end
      options.concat [:ccs_serialize_option_t, :CCS_SERIALIZE_OPTION_END]
      format = :CCS_SERIALIZE_FORMAT_BINARY
      if path
        result = nil
        operation = :CCS_SERIALIZE_OPERATION_FILE
        varargs = [:string, path] + options
      elsif file_descriptor
        result = nil
        operation = :CCS_SERIALIZE_OPERATION_FILE_DESCRIPTOR
        varargs = [:int, file_descriptor] + options
      else
        operation = :CCS_SERIALIZE_OPERATION_SIZE
        sz = MemoryPointer::new(:size_t)
        varargs = [:pointer, sz] + options
        CCS.error_check CCS.ccs_object_serialize(@handle, format, operation, *varargs)
        operation = :CCS_SERIALIZE_OPERATION_MEMORY
        result = MemoryPointer::new(sz.read_size_t)
        varargs = [:size_t, sz.read_size_t, :pointer, result] + options
      end
      CCS.error_check CCS.ccs_object_serialize(@handle, format, operation, *varargs)
      return result
    end

    def self.deserialize(format: :binary, handle_map: nil, vector: nil, data: nil, path: nil, buffer: nil, file_descriptor: nil, callback: nil, callback_data: nil)
      raise CCSError, :CCS_INVALID_VALUE if format != :binary
      format = :CCS_SERIALIZE_FORMAT_BINARY
      mode_count = 0
      mode_count += 1 if path
      mode_count += 1 if buffer
      mode_count += 1 if file_descriptor
      raise CCSError, :CCS_INVALID_VALUE unless mode_count == 1
      ptr = MemoryPointer::new(:ccs_object_t)
      options = []
      options.concat [:ccs_deserialize_option_t, :CCS_DESERIALIZE_OPTION_HANDLE_MAP, :ccs_map_t, handle_map.handle] if handle_map
      options.concat [:ccs_deserialize_option_t, :CCS_DESERIALIZE_OPTION_VECTOR, :pointer, vector] if vector
      options.concat [:ccs_deserialize_option_t, :CCS_DESERIALIZE_OPTION_DATA, :value, data] if data
      if callback
        cb_wrapper = CCS.get_deserialize_wrapper(&callback)
        options.concat [:ccs_deserialize_option_t, :CCS_DESERIALIZE_OPTION_CALLBACK, :ccs_object_deserialize_callback, cb_wrapper, :value, callback_data]
      elsif CCS.default_user_data_deserializer
        options.concat [:ccs_deserialize_option_t, :CCS_DESERIALIZE_OPTION_CALLBACK, :ccs_object_deserialize_callback, CCS.default_user_data_deserializer, :value, nil]
      end
      options.concat [:ccs_deserialize_option_t, :CCS_DESERIALIZE_OPTION_END]
      if buffer
        operation = :CCS_SERIALIZE_OPERATION_MEMORY
        varargs = [:size_t, buffer.size, :pointer, buffer] + options
      elsif path
        operation = :CCS_SERIALIZE_OPERATION_FILE
        varargs = [:string, path] + options
      elsif file_descriptor
        operation = :CCS_SERIALIZE_OPERATION_FILE_DESCRIPTOR
        varargs = [:int, file_descriptor] + options
      else
        raise CCSError, :CCS_INVALID_VALUE
      end
      CCS.error_check CCS.ccs_object_deserialize(ptr, format, operation, *varargs)
      return _from_handle(ptr.read_ccs_object_t, retain: false, auto_release: true)
    end

    def user_data
      ptr = MemoryPointer.new(:value)
      CCS.error_check CCS.ccs_object_get_user_data(@handle, ptr)
      ud = ptr.read_value
      ud ? ud : ud.nil? ? false : nil
    end

    def user_data=(ud)
      CCS.error_check CCS.ccs_object_set_user_data(@handle, ud ? ud : ud.nil? ? false : nil)
      CCS.register_user_data(@handle, ud)
      ud
    end

  end

  @@data_store = Hash.new { |h, k| h[k] = { callbacks: [], user_data: nil, serialize_calback: nil, strings: [] } }

  # Delete wrappers are responsible for deregistering the object data_store
  def self.register_vector(handle, vector_data)
    value = handle.address
    raise CCSError, :CCS_INVALID_VALUE if @@data_store.include?(value)
    @@data_store[value][:callbacks].push vector_data
  end

  def self.unregister_vector(handle)
    value = handle.address
    @@data_store.delete(value)
  end

  # If objects don't have a user-defined del operation, then the first time a
  # data needs to be registered a destruction callback is attached.
  def self.register_destroy_callback(handle)
    value = handle.address
    cb = lambda { |_, _|
      @@data_store.delete(value)
    }
    CCS.error_check CCS.ccs_object_set_destroy_callback(handle, cb, nil)
    @@data_store[value][:callbacks].push cb
  end

  def self.register_user_data(handle, user_data)
    value = handle.address
    register_destroy_callback(handle) unless @@data_store.include?(value)
    @@data_store[value][:user_data] = user_data
  end

  def self.register_string(handle, string)
    value = handle.address
    register_destroy_callback(handle) unless @@data_store.include?(value)
    @@data_store[value][:strings].push string
  end

  def self.register_callback(handle, callback_data)
    value = handle.address
    register_destroy_callback(handle) unless @@data_store.include?(value)
    @@data_store[value][:callbacks].push callback_data
  end

  def self.register_serialize_callback(handle, callback_data)
    value = handle.address
    register_destroy_callback(handle) unless @@data_store.include?(value)
    @@data_store[value][:serialize_calback] = callback_data
  end

  def self.set_destroy_callback(handle, user_data: nil, &block)
    raise CCSError, :CCS_INVALID_VALUE if !block
    cb_wrapper = lambda { |object, data|
      block.call(Object.from_handle(object), data)
    }
    CCS.error_check CCS.ccs_object_set_destroy_callback(handle, cb_wrapper, user_data)
    register_callback(handle, [cb_wrapper, user_data])
  end

  def self.get_serialize_wrapper(&block)
    lambda { |object, serialize_data_size, serialize_data, serialize_data_size_ret, cb_data|
      begin
        serialized = block.call(Object.from_handle(object), cb_data, serialize_data_size == 0 ? true : false)
        raise CCSError, :CCS_INVALID_VALUE if !serialize_data.null? && serialize_data_size < serialized.size
        serialize_data.write_bytes(serialized.read_bytes(serialized.size)) unless serialize_data.null?
        Pointer.new(serialize_data_size_ret).write_size_t(serialized.size) unless serialize_data_size_ret.null?
        CCSError.to_native(:CCS_SUCCESS)
      rescue => e
        CCS.set_error(e)
      end
    }
  end

  def self.get_deserialize_wrapper(&block)
    lambda { |obj, serialize_data_size, serialize_data, cb_data|
      begin
        serialized = serialize_data.null? ? nil : serialize_data.slice(0, serialize_data_size)
        block.call(Object.from_handle(obj), serialized, cb_data)
        CCSError.to_native(:CCS_SUCCESS)
      rescue => e
        CCS.set_error(e)
      end
    }
  end

  @yaml_user_data_serializer = get_serialize_wrapper { |obj, _, size|
    FFI::MemoryPointer.from_string(YAML.dump(obj.user_data))
  }

  @yaml_user_data_deserializer = get_deserialize_wrapper { |obj, serialized, _|
    obj.user_data = YAML.load(serialized.read_string)
  }

  class << self
     attr_accessor :default_user_data_serializer, :default_user_data_deserializer
  end
  self.default_user_data_serializer = @yaml_user_data_serializer
  self.default_user_data_deserializer = @yaml_user_data_deserializer

  def self.set_serialize_callback(handle, user_data: nil, &block)
    if block
      cb_wrapper = get_serialize_wrapper(&block)
      cb_data = [cb_wrapper, user_data]
    else
      cb_wrapper = nil
      user_data = nil
      cb_data = nil
    end
    CCS.error_check CCS.ccs_object_set_serialize_callback(handle, cb_wrapper, user_data)
    register_serialize_callback(handle, cb_data)
  end

  def self.deserialize(format: :binary, handle_map: nil, path: nil, buffer: nil, callback: nil, callback_data: nil)
    return CCS::Object.deserialize(format: format, handle_map: handle_map, path: path, buffer: buffer, callback: callback, callback_data: callback_data)
  end

end
