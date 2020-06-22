module CCS
  class Interval < FFI::Struct
    layout :type, :ccs_numeric_type_t,
           :lower, :ccs_numeric_t,
           :upper, :ccs_numeric_t,
           :lower_included, :ccs_bool_t,
           :upper_included, :ccs_bool_t

    def initialize(*args, type:, lower: nil, upper: nil, lower_included: true, upper_included: false)
      unless [:CCS_NUM_FLOAT, :CCS_NUM_INTEGER].include?(type)
        raise CCSError, :CCS_INVALID_TYPE
      end
      super(*args)
      self[:type] = type
      if lower
        if type == :CCS_NUM_FLOAT
          self[:lower][:f] = lower
        else
          self[:lower][:i] = lower
        end
      end
      if upper
        if type == :CCS_NUM_FLOAT
          self[:upper][:f] = upper
        else
          self[:upper][:i] = upper
        end
      end
      self[:lower_included] = lower_included ? CCS::TRUE : CCS::FALSE
      self[:upper_included] = upper_included ? CCS::TRUE : CCS::FALSE
    end

    def type
      return self[:type]
    end

    def lower
      case self[:type]
      when :CCS_NUM_FLOAT
        self[:lower][:f]
      when :CCS_NUM_INTEGER
        self[:lower][:i]
      else
        raise CCSError, :CCS_INVALID_TYPE
      end
    end

    def lower=(v)
      case self[:type]
      when :CCS_NUM_FLOAT
        self[:lower][:f] = v
      when :CCS_NUM_INTEGER
        self[:lower][:i] = v
      else
        raise CCSError, :CCS_INVALID_TYPE
      end
    end

    def upper
      case self[:type]
      when :CCS_NUM_FLOAT
        self[:upper][:f]
      when :CCS_NUM_INTEGER
        self[:upper][:i]
      else
        raise CCSError, :CCS_INVALID_TYPE
      end
    end

    def upper=(v)
      case self[:type]
      when :CCS_NUM_FLOAT
        self[:upper][:f] = v
      when :CCS_NUM_INTEGER
        self[:upper][:i] = v
      else
        raise CCSError, :CCS_INVALID_TYPE
      end
    end

    def lower_included?
      self[:lower_included] == CCS::FALSE ? false : true
    end

    def upper_included?
      self[:upper_included] == CCS::FALSE ? false : true
    end

    def lower_included=(v)
      self[:lower_included] = v ? CCS::TRUE : CCS::FALSE
    end

    def upper_included=(v)
      self[:upper_included] = v ? CCS::TRUE : CCS::FALSE
    end

    def empty?
      ptr = MemoryPointer::new(:ccs_bool_t)
      res = CCS.ccs_interval_empty(self, ptr)
      CCS.error_check(res)
      ptr.read_ccs_bool_t == CCS::FALSE ? false : true
    end

    def intersect(other)
      intersection = Interval::new(type: :CCS_NUM_FLOAT)
      res = CCS.ccs_interval_intersect(self, other, intersection)
      CCS.error_check(res)
      return intersection
    end

    def ==(other)
      ptr = MemoryPointer::new(:ccs_bool_t)
      res = CCS.ccs_interval_equal(self, other, ptr)
      CCS.error_check(res)
      ptr.read_ccs_bool_t == CCS::FALSE ? false : true
    end

    def include?(v)
      n = Numeric::new
      case self[:type]
      when :CCS_NUM_FLOAT
        n[:f] = v
      when :CCS_NUM_INTEGER
        n[:i] = v
      else
        raise CCSError, :CCS_INVALID_TYPE
      end
      res = CCS.ccs_interval_include(self, n)
      res == CCS::FALSE ? false : true
    end

    def to_s
      s = ""
      s << (lower_included? ? "[" : "(")
      s << " #{lower}, #{upper} "
      s << (upper_included? ? "]" : ")")
    end
  end
  typedef Interval.by_value, :ccs_interval_t

  attach_function :ccs_interval_empty, [Interval.by_ref, :pointer], :ccs_result_t
  attach_function :ccs_interval_intersect, [Interval.by_ref, Interval.by_ref, Interval.by_ref], :ccs_result_t
  attach_function :ccs_interval_equal, [Interval.by_ref, Interval.by_ref, :pointer], :ccs_result_t
  attach_function :ccs_interval_include, [Interval.by_ref, :ccs_numeric_t], :ccs_bool_t

end
