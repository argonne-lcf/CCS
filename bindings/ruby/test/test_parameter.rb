require 'minitest/autorun'
require_relative '../lib/cconfigspace'

class CConfigSpaceTestParameter < Minitest::Test
  def test_from_handle_discrete
    values = [0, 1.5, 2, 7.2]
    h = CCS::DiscreteParameter::new(values: values)
    h2 = CCS::Object::from_handle(h)
    assert_equal( h.class, h2.class )
  end

  def discrete_check(values, h)
    assert_equal( :CCS_OBJECT_TYPE_PARAMETER, h.object_type )
    assert_equal( :CCS_PARAMETER_TYPE_DISCRETE, h.type )
    assert_match( /param/, h.name )
    assert_nil( h.user_data )
    assert_equal( 0.2, h.default_value )
    values.each { |v| assert( h.check_value(v) ) }
    refute( h.check_value("foo") )
    v = h.sample
    assert( values.include? v )
    vals = h.samples(100)
    vals.each { |v|
      assert( values.include? v )
    }
  end

  def test_discrete
    values = [0.2, 1.5, 2, 7.2]
    h = CCS::DiscreteParameter::new(values: values)
    discrete_check(values, h)
  end

  def test_serialize_discrete
    values = [0.2, 1.5, 2, 7.2]
    href = CCS::DiscreteParameter::new(values: values)
    buff = href.serialize
    h = CCS::deserialize(buffer: buff)
    discrete_check(values, h)
  end

  def test_ordinal_compare
    values = ["foo", 2, 3.0]
    h = CCS::OrdinalParameter::new(values: values)
    assert_equal( 0, h.compare("foo", "foo") )
    assert_equal( -1, h.compare("foo", 2) )
    assert_equal( -1, h.compare("foo", 3.0) )
    assert_equal( 1, h.compare(2, "foo") )
    assert_equal( 0, h.compare(2, 2) )
    assert_equal( -1, h.compare(2, 3.0) )
    assert_equal( 1, h.compare(3.0, "foo") )
    assert_equal( 1, h.compare(3.0, 2) )
    assert_equal( 0, h.compare(3.0, 3.0) )
    assert_raises(CCS::CCSError, :CCS_RESULT_ERROR_INVALID_VALUE) { h.compare(4.0, "foo") }
  end

  def test_from_handle_ordinal
    values = ["foo", 2, 3.0]
    h = CCS::OrdinalParameter::new(values: values)
    h2 = CCS::Object::from_handle(h)
    assert_equal( h.class, h2.class )
  end

  def ordinal_check(values, h)
    assert_equal( :CCS_OBJECT_TYPE_PARAMETER, h.object_type )
    assert_equal( :CCS_PARAMETER_TYPE_ORDINAL, h.type )
    assert_match( /param/, h.name )
    assert_nil( h.user_data )
    assert_equal( "foo", h.default_value )
    assert_equal( :CCS_DISTRIBUTION_TYPE_UNIFORM, h.default_distribution.type )
    assert_equal( values, h.values )
    assert( h.check_value("foo") )
    assert( h.check_value(2) )
    assert( h.check_value(3.0) )
    refute( h.check_value(1.5) )
    v = h.sample
    assert( values.include? v )
    vals = h.samples(100)
    vals.each { |v|
      assert( values.include? v )
    }
  end

  def test_ordinal
    values = ["foo", 2, 3.0]
    h = CCS::OrdinalParameter::new(values: values)
    ordinal_check(values, h)
  end

  def test_serialize_ordinal
    values = ["foo", 2, 3.0]
    href = CCS::OrdinalParameter::new(values: values)
    buff = href.serialize
    h = CCS::deserialize(buffer: buff)
    ordinal_check(values, h)
  end

  def test_from_handle_categorical
    values = ["foo", 2, 3.0]
    h = CCS::CategoricalParameter::new(values: values)
    h2 = CCS::Object::from_handle(h)
    assert_equal( h.class, h2.class )
  end

  def categorical_check(values, h)
    assert_equal( :CCS_OBJECT_TYPE_PARAMETER, h.object_type )
    assert_equal( :CCS_PARAMETER_TYPE_CATEGORICAL, h.type )
    assert_match( /param/, h.name )
    assert_equal( {'foo': ['bar', 'baz']}, h.user_data )
    assert_equal( "foo", h.default_value )
    assert_equal( :CCS_DISTRIBUTION_TYPE_UNIFORM, h.default_distribution.type )
    assert_equal( values, h.values )
    assert( h.check_value("foo") )
    assert( h.check_value(2) )
    assert( h.check_value(3.0) )
    refute( h.check_value(1.5) )
    v = h.sample
    assert( values.include? v )
    vals = h.samples(100)
    vals.each { |v|
      assert( values.include? v )
    }
  end

  def test_categorical
    values = ["foo", 2, 3.0]
    h = CCS::CategoricalParameter::new(values: values)
    h.user_data = {'foo': ['bar', 'baz']}
    GC.start
    categorical_check(values, h)
  end

  def test_serialize_categorical
    values = ["foo", 2, 3.0]
    href = CCS::CategoricalParameter::new(values: values)
    href.user_data = {'foo': ['bar', 'baz']}
    buff = href.serialize
    h = CCS::deserialize(buffer: buff)
    categorical_check(values, h)
  end

  def test_from_handle_numerical
    h = CCS::NumericalParameter::Float::new
    h2 = CCS::Object::from_handle(h)
    assert_equal( h.class, h2.class )
  end

  def numerical_check(h)
    assert_equal( :CCS_OBJECT_TYPE_PARAMETER, h.object_type )
    assert_equal( :CCS_PARAMETER_TYPE_NUMERICAL, h.type )
    assert_match( /param/, h.name )
    assert_nil( h.user_data )
    assert_equal( 0.0, h.default_value )
    assert_equal( :CCS_DISTRIBUTION_TYPE_UNIFORM, h.default_distribution.type )
    assert( h.check_value(0.5) )
    refute( h.check_value(1.5) )
    v = h.sample
    assert( v.kind_of?(Float) )
    assert( v >= 0.0 && v < 1.0 )
    vals = h.samples(100)
    vals.each { |v|
      assert( v.kind_of?(Float) )
      assert( v >= 0.0 && v < 1.0 )
    }
  end

  def test_create_numerical
    h = CCS::NumericalParameter::Float::new
    numerical_check(h)
  end

  def test_serialize_numerical
    href = CCS::NumericalParameter::Float::new
    buff = href.serialize
    h = CCS::deserialize(buffer: buff)
    numerical_check(h)
  end

  def test_create_numerical_float
    h = CCS::NumericalParameter::Float.new(lower: 0.0, upper: 1.0)
    assert_equal( :CCS_OBJECT_TYPE_PARAMETER, h.object_type )
    assert_equal( :CCS_PARAMETER_TYPE_NUMERICAL, h.type )
    assert_match( /param/, h.name )
    assert_nil( h.user_data )
    assert_equal( 0.0, h.default_value )
    assert_equal( :CCS_DISTRIBUTION_TYPE_UNIFORM, h.default_distribution.type )
    assert_equal( :CCS_NUMERIC_TYPE_FLOAT, h.data_type )
    assert_equal( 0.0, h.lower )
    assert_equal( 1.0, h.upper )
    assert_equal( 0.0, h.quantization )
    assert( h.check_value(0.5) )
    refute( h.check_value(1.5) )
    v = h.sample
    assert( v.kind_of?(Float) )
    assert( v >= 0.0 && v < 1.0 )
    vals = h.samples(100)
    vals.each { |v|
      assert( v.kind_of?(Float) )
      assert( v >= 0.0 && v < 1.0 )
    }
  end

  def test_create_numerical_int
    h = CCS::NumericalParameter::Int.new(lower: 0, upper: 100)
    assert_equal( :CCS_PARAMETER_TYPE_NUMERICAL, h.type )
    assert_match( /param/, h.name )
    assert_nil( h.user_data )
    assert_equal( 0, h.default_value )
    assert_equal( :CCS_DISTRIBUTION_TYPE_UNIFORM, h.default_distribution.type )
    assert_equal( :CCS_NUMERIC_TYPE_INT, h.data_type )
    assert_equal( 0, h.lower )
    assert_equal( 100, h.upper )
    assert_equal( 0, h.quantization )
    assert( h.check_value(50) )
    refute( h.check_value(150) )
    v = h.sample
    assert( v.kind_of?(Integer) )
    assert( v >= 0 && v < 100 )
    vals = h.samples(100)
    vals.each { |v|
      assert( v.kind_of?(Integer) )
      assert( v >= 0 && v < 100 )
    }
  end

  def string_check(h)
    assert_equal( :CCS_PARAMETER_TYPE_STRING, h.type )
    assert_match( /param/, h.name )
    assert_nil( h.user_data )
    assert_raises CCS::CCSError do
      h.sample
    end
  end

  def test_create_string
    h = CCS::StringParameter::new
    string_check(h)
  end

  def test_serialize_string
    href = CCS::StringParameter::new
    buff = href.serialize
    h = CCS::deserialize(buffer: buff)
    string_check(h)
  end

end
