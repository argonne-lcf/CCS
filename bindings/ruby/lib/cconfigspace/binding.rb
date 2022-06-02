module CCS
  attach_function :ccs_binding_get_context, [:ccs_binding_t, :pointer], :ccs_result_t
  attach_function :ccs_binding_get_value, [:ccs_binding_t, :size_t, :pointer], :ccs_result_t
  attach_function :ccs_binding_set_value, [:ccs_binding_t, :size_t, :ccs_datum_t], :ccs_result_t
  attach_function :ccs_binding_get_values, [:ccs_binding_t, :size_t, :pointer, :pointer], :ccs_result_t
  attach_function :ccs_binding_get_value_by_name, [:ccs_binding_t, :string, :pointer], :ccs_result_t
  attach_function :ccs_binding_set_value_by_name, [:ccs_binding_t, :string, :ccs_datum_t], :ccs_result_t
  attach_function :ccs_binding_get_value_by_hyperparameter, [:ccs_binding_t, :ccs_hyperparameter_t, :pointer], :ccs_result_t
  attach_function :ccs_binding_set_value_by_hyperparameter, [:ccs_binding_t, :ccs_hyperparameter_t, :ccs_datum_t], :ccs_result_t
  attach_function :ccs_binding_hash, [:ccs_binding_t, :pointer], :ccs_result_t
  attach_function :ccs_binding_cmp, [:ccs_binding_t, :ccs_binding_t, :pointer], :ccs_result_t

  class Binding < Object
    include Comparable
    add_property :hash, :ccs_hash_t, :ccs_binding_hash, memoize: false
    add_handle_property :context, :ccs_context_t, :ccs_binding_get_context, memoize: true

    def set_value(hyperparameter, value)
      d = Datum.from_value(value)
      res = case hyperparameter
        when String
          CCS.ccs_binding_set_value_by_name(@handle, hyperparameter, d)
        when Symbol
          name = hyperparameter.inspect
          CCS.ccs_binding_set_value_by_name(@handle, name, d)
        when Hyperparameter
          CCS.ccs_binding_set_value_by_hyperparameter(@handle, hyperparameter.handle, d)
        when Integer
          CCS.ccs_binding_set_value(@handle, hyperparameter, d)
        else
          raise CCSError, :CCS_INVALID_VALUE
        end
      CCS.error_check(res)
      self
    end

    def value(hyperparameter)
      ptr = MemoryPointer::new(:ccs_datum_t)
      res = case hyperparameter
        when String
          CCS.ccs_binding_get_value_by_name(@handle, hyperparameter, ptr)
        when Symbol
          name = hyperparameter.inspect
          CCS.ccs_binding_get_value_by_name(@handle, name, ptr)
        when Hyperparameter
          CCS.ccs_binding_get_by_hyperparameter(@handle, hyperparameter.handle, ptr)
        when Integer
          CCS.ccs_binding_get_value(@handle, hyperparameter, ptr)
        else
          raise CCSError, :CCS_INVALID_VALUE
        end
      CCS.error_check(res)
      Datum::new(ptr).value
    end

    def values
      count = num_values
      return [] if count == 0
      values = MemoryPointer::new(:ccs_datum_t, count)
      res = CCS.ccs_binding_get_values(@handle, count, values, nil)
      CCS.error_check(res)
      count.times.collect { |i| Datum::new(values[i]).value }
    end

    def num_values
      @num_values ||= begin
        ptr = MemoryPointer::new(:size_t)
        res = CCS.ccs_binding_get_values(@handle, 0, nil, ptr)
        CCS.error_check(res)
        ptr.read_size_t
      end
    end

    def <=>(other)
      ptr = MemoryPointer::new(:int)
      res = CCS.ccs_binding_cmp(@handle, other, ptr)
      CCS.error_check(res)
      return ptr.read_int
    end

    def to_h
      context.hyperparameters.collect(&:name).zip(values).to_h
    end

  end

end
