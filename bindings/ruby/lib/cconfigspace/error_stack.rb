module CCS

  attach_function :ccs_get_thread_error, [], :ccs_error_stack_t
  attach_function :ccs_set_thread_error, [:ccs_error_stack_t], :ccs_result_t
  attach_function :ccs_clear_thread_error, [], :void
  attach_function :ccs_create_error_stack, [:pointer, :ccs_result_t, :string], :ccs_result_t
  attach_function :ccs_error_stack_push, [:ccs_error_stack_t, :string, :int, :string], :ccs_result_t
  attach_function :ccs_error_stack_get_message, [:ccs_error_stack_t, :pointer], :ccs_result_t
  attach_function :ccs_error_stack_get_code, [:ccs_error_stack_t, :pointer], :ccs_result_t
  attach_function :ccs_error_stack_get_elems, [:ccs_error_stack_t, :pointer, :pointer], :ccs_result_t

  class ErrorStack < Object
    add_property :code, :ccs_result_t, :ccs_error_stack_get_code, memoize: true

    class Elem < FFI::Struct
      layout :file, :pointer,
             :line, :int,
             :func, :pointer

      def file
        self[:file].read_string
      end

      def line
        self[:line]
      end

      def func
        self[:func].read_string
      end
    end

    def initialize(handle = nil, retain: false, auto_release: true,
                   error: nil, message: nil)
      if handle
        super(handle, retain: retain, auto_release: auto_release)
      else
        msg = message ? message.gsub("%","%%") : nil
        ptr = MemoryPointer::new(:ccs_error_stack_t)
        CCS.error_check CCS.ccs_create_error_stack(ptr, error, msg)
        super(ptr.read_ccs_error_stack_t, retain: false)
        CCS.register_string(@handle, msg) if message
      end
    end

    def push(file, line, func)
      CCS.error_check CCS.ccs_error_stack_push(self.handle, file, line, func)
      CCS.register_string(@handle, file) if file
      CCS.register_string(@handle, func) if func
      self
    end

    def message
      @message ||= begin
        ptr = MemoryPointer::new(:pointer)
        CCS.error_check CCS.ccs_error_stack_get_message(@handle, ptr)
        ptr.read_pointer.read_string
      end
    end

    def elems
      ptr1 = MemoryPointer::new(:size_t)
      ptr2 = MemoryPointer::new(:pointer)
      CCS.error_check CCS.ccs_error_stack_get_elems(@handle, ptr1, ptr2)
      count = ptr1.read_size_t
      ptr = ptr2.read_pointer
      count.times.collect { |i| Elem.new(ptr + i * Elem.size) }
    end
 
  end

  def self.get_thread_error
    handle = CCS.ccs_get_thread_error
    unless handle.null?
      ErrorStack.new(handle)
    else
      nil
    end
  end

  def self.set_thread_error(error)
    CCS.error_check CCS.ccs_set_thread_error(error.handle)
    CCS.error_check CCS.ccs_retain_object(error.handle)
    nil
  end

  def self.clear_thread_error
    ccs_clear_thread_error
  end
end
