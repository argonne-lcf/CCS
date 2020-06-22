[ '../lib', 'lib' ].each { |d| $:.unshift(d) if File::directory?(d) }
require 'minitest/autorun'
require 'cconfigspace'

class CConfigSpaceTestHyperparameter < Minitest::Test
  def setup
    CCS.init
  end

  def test_ordinal_compare
    values = ["foo", 2, 3.0]
    h = CCS::OrdinalHyperparameter::new(values: values)
    assert_equal( 0, h.compare("foo", "foo") )
    assert_equal( -1, h.compare("foo", 2) )
    assert_equal( -1, h.compare("foo", 3.0) )
    assert_equal( 1, h.compare(2, "foo") )
    assert_equal( 0, h.compare(2, 2) )
    assert_equal( -1, h.compare(2, 3.0) )
    assert_equal( 1, h.compare(3.0, "foo") )
    assert_equal( 1, h.compare(3.0, 2) )
    assert_equal( 0, h.compare(3.0, 3.0) )
    assert_raises(CCS::CCSError, :CCS_INVALID_VALUE) { h.compare(4.0, "foo") }
  end

  def test_from_handle_ordinal
    values = ["foo", 2, 3.0]
    h = CCS::OrdinalHyperparameter::new(values: values)
    h2 = CCS::Object::from_handle(h)
    assert_equal( h.class, h2.class )
  end

  def test_ordinal
    values = ["foo", 2, 3.0]
    h = CCS::OrdinalHyperparameter::new(values: values)
    assert_equal( :CCS_HYPERPARAMETER, h.object_type )
    assert_equal( :CCS_ORDINAL, h.type )
    assert_match( /param/, h.name )
    assert( h.user_data.null? )
    assert_equal( "foo", h.default_value )
    assert_equal( :CCS_UNIFORM, h.default_distribution.type )
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

  def test_from_handle_categorical
    values = ["foo", 2, 3.0]
    h = CCS::CategoricalHyperparameter::new(values: values)
    h2 = CCS::Object::from_handle(h)
    assert_equal( h.class, h2.class )
  end

  def test_categorical
    values = ["foo", 2, 3.0]
    h = CCS::CategoricalHyperparameter::new(values: values)
    assert_equal( :CCS_HYPERPARAMETER, h.object_type )
    assert_equal( :CCS_CATEGORICAL, h.type )
    assert_match( /param/, h.name )
    assert( h.user_data.null? )
    assert_equal( "foo", h.default_value )
    assert_equal( :CCS_UNIFORM, h.default_distribution.type )
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

  def test_from_handle_numerical
    h = CCS::NumericalHyperparameter::new
    h2 = CCS::Object::from_handle(h)
    assert_equal( h.class, h2.class )
  end

  def test_create_numerical
    h = CCS::NumericalHyperparameter::new
    assert_equal( :CCS_HYPERPARAMETER, h.object_type )
    assert_equal( :CCS_NUMERICAL, h.type )
    assert_match( /param/, h.name )
    assert( h.user_data.null? )
    assert_equal( 0.0, h.default_value )
    assert_equal( :CCS_UNIFORM, h.default_distribution.type )
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

  def test_create_numerical_float
    h = CCS::NumericalHyperparameter::float(lower: 0.0, upper: 1.0)
    assert_equal( :CCS_HYPERPARAMETER, h.object_type )
    assert_equal( :CCS_NUMERICAL, h.type )
    assert_match( /param/, h.name )
    assert( h.user_data.null? )
    assert_equal( 0.0, h.default_value )
    assert_equal( :CCS_UNIFORM, h.default_distribution.type )
    assert_equal( :CCS_NUM_FLOAT, h.data_type )
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
    h = CCS::NumericalHyperparameter::int(lower: 0, upper: 100)
    assert_equal( :CCS_NUMERICAL, h.type )
    assert_match( /param/, h.name )
    assert( h.user_data.null? )
    assert_equal( 0, h.default_value )
    assert_equal( :CCS_UNIFORM, h.default_distribution.type )
    assert_equal( :CCS_NUM_INTEGER, h.data_type )
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

end
