module CCS
  attach_function :ccs_binding_get_context, [:ccs_binding_t, :pointer], :ccs_error_t
  attach_function :ccs_binding_get_value, [:ccs_binding_t, :size_t, :pointer], :ccs_error_t
  attach_function :ccs_binding_set_value, [:ccs_binding_t, :size_t, :ccs_datum_t], :ccs_error_t
  attach_function :ccs_binding_get_values, [:ccs_binding_t, :size_t, :pointer, :pointer], :ccs_error_t
  attach_function :ccs_binding_set_values, [:ccs_binding_t, :size_t, :pointer], :ccs_error_t
  attach_function :ccs_binding_get_value_by_name, [:ccs_binding_t, :string, :pointer], :ccs_error_t
  attach_function :ccs_binding_set_value_by_name, [:ccs_binding_t, :string, :ccs_datum_t], :ccs_error_t
  attach_function :ccs_binding_get_value_by_hyperparameter, [:ccs_binding_t, :ccs_hyperparameter_t, :pointer], :ccs_error_t
  attach_function :ccs_binding_set_value_by_hyperparameter, [:ccs_binding_t, :ccs_hyperparameter_t, :ccs_datum_t], :ccs_error_t
  attach_function :ccs_binding_hash, [:ccs_binding_t, :pointer], :ccs_error_t
  attach_function :ccs_binding_cmp, [:ccs_binding_t, :ccs_binding_t, :pointer], :ccs_error_t

  class Binding < Object
    include Comparable
    add_property :hash, :ccs_hash_t, :ccs_binding_hash, memoize: false
    add_handle_property :context, :ccs_context_t, :ccs_binding_get_context, memoize: true

    def set_value(hyperparameter, value)
      d = Datum.from_value(value)
      case hyperparameter
      when String
        CCS.error_check CCS.ccs_binding_set_value_by_name(@handle, hyperparameter, d)
      when Symbol
        name = hyperparameter.inspect
        CCS.error_check CCS.ccs_binding_set_value_by_name(@handle, name, d)
      when Hyperparameter
        CCS.error_check CCS.ccs_binding_set_value_by_hyperparameter(@handle, hyperparameter.handle, d)
      when Integer
        CCS.error_check CCS.ccs_binding_set_value(@handle, hyperparameter, d)
      else
        raise CCSError, :CCS_INVALID_VALUE
      end
      self
    end

    def value(hyperparameter)
      ptr = MemoryPointer::new(:ccs_datum_t)
      case hyperparameter
      when String
        CCS.error_check CCS.ccs_binding_get_value_by_name(@handle, hyperparameter, ptr)
      when Symbol
        name = hyperparameter.inspect
        CCS.error_check CCS.ccs_binding_get_value_by_name(@handle, name, ptr)
      when Hyperparameter
        CCS.error_check CCS.ccs_binding_get_by_hyperparameter(@handle, hyperparameter.handle, ptr)
      when Integer
        CCS.error_check CCS.ccs_binding_get_value(@handle, hyperparameter, ptr)
      else
        raise CCSError, :CCS_INVALID_VALUE
      end
      Datum::new(ptr).value
    end

    def values
      count = num_values
      return [] if count == 0
      values = MemoryPointer::new(:ccs_datum_t, count)
      CCS.error_check CCS.ccs_binding_get_values(@handle, count, values, nil)
      count.times.collect { |i| Datum::new(values[i]).value }
    end

    def set_values(values)
      count = values.size
      raise CCSError, :CCS_INVALID_VALUE if count == 0
      ss = []
      vals = MemoryPointer::new(:ccs_datum_t, count)
      values.each_with_index{ |v, i| Datum::new(vals[i]).set_value(v, string_store: ss) }
      CCS.error_check CCS.ccs_binding_set_values(@handle, count, vals)
      self
    end

    def num_values
      @num_values ||= begin
        ptr = MemoryPointer::new(:size_t)
        CCS.error_check CCS.ccs_binding_get_values(@handle, 0, nil, ptr)
        ptr.read_size_t
      end
    end

    def <=>(other)
      ptr = MemoryPointer::new(:int)
      CCS.error_check CCS.ccs_binding_cmp(@handle, other, ptr)
      return ptr.read_int
    end

    def to_h
      context.hyperparameters.collect(&:name).zip(values).to_h
    end

  end

end
