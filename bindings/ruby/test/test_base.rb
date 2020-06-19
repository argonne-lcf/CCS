[ '../lib', 'lib' ].each { |d| $:.unshift(d) if File::directory?(d) }
require 'minitest/autorun'
require 'cconfigspace'

class CConfigSpaceTest < Minitest::Test
  def setup
    CCS.init
  end

  def test_version
    ver = CCS.version
    assert(ver.kind_of?(CCS::Version))
  end

  def test_datum_value
    d = CCS::Datum::new
    d[:type] = :CCS_NONE
    assert_nil( d.value )
    d[:type] = :CCS_INACTIVE
    assert_equal( CCS::Inactive, d.value )
    d[:type] = :CCS_BOOLEAN
    d[:value][:i] = CCS::FALSE
    assert_equal( false, d.value )
    d[:type] = :CCS_BOOLEAN
    d[:value][:i] = CCS::TRUE
    assert_equal( true, d.value )
    d[:type] = :CCS_FLOAT
    d[:value][:f] = 15.0
    assert_equal( 15.0, d.value )
    d[:type] = :CCS_INTEGER
    d[:value][:i] = 15
    assert_equal( 15, d.value )
    ptr = CCS::MemoryPointer::from_string("foo")
    d[:type] = :CCS_STRING
    d[:value][:s] = ptr
    assert_equal( "foo", d.value )
    rng = CCS::Rng::new
    d[:type] = :CCS_OBJECT
    d[:value][:o] = rng.handle
    o = d.value
    assert( o.kind_of? CCS::Rng )
    assert_equal( :CCS_RNG, d.value.object_type )
  end

  def test_from_value
    d = CCS::Datum::from_value(nil)
    assert_equal( :CCS_NONE, d[:type] )
    assert_equal( 0, d[:value][:i] )
    d = CCS::Datum::from_value(CCS::Inactive)
    assert_equal( :CCS_INACTIVE, d[:type] )
    assert_equal( 0, d[:value][:i] )
    d = CCS::Datum::from_value(false)
    assert_equal( :CCS_BOOLEAN, d[:type] )
    assert_equal( CCS::FALSE, d[:value][:i] )
    d = CCS::Datum::from_value(true)
    assert_equal( :CCS_BOOLEAN, d[:type] )
    assert_equal( CCS::TRUE, d[:value][:i] )
    d = CCS::Datum::from_value(15)
    assert_equal( :CCS_INTEGER, d[:type] )
    assert_equal( 15, d[:value][:i] )
    d = CCS::Datum::from_value(15.0)
    assert_equal( :CCS_FLOAT, d[:type] )
    assert_equal( 15.0, d[:value][:f] )
    rng = CCS::Rng::new
    d = CCS::Datum::from_value(rng)
    assert_equal( :CCS_OBJECT, d[:type] )
    assert_equal( rng.handle, d[:value][:o] )
  end

  def test_value_affect
    d = CCS::Datum::from_value(nil)
    assert_equal( :CCS_NONE, d[:type] )
    assert_equal( 0, d[:value][:i] )
    d.value = CCS::Inactive
    assert_equal( :CCS_INACTIVE, d[:type] )
    assert_equal( 0, d[:value][:i] )
    d.value = false
    assert_equal( :CCS_BOOLEAN, d[:type] )
    assert_equal( CCS::FALSE, d[:value][:i] )
    d.value = true
    assert_equal( :CCS_BOOLEAN, d[:type] )
    assert_equal( CCS::TRUE, d[:value][:i] )
    d.value = 15
    assert_equal( :CCS_INTEGER, d[:type] )
    assert_equal( 15, d[:value][:i] )
    d.value = 15.0
    assert_equal( :CCS_FLOAT, d[:type] )
    assert_equal( 15.0, d[:value][:f] )
    rng = CCS::Rng::new
    d.value = rng
    assert_equal( :CCS_OBJECT, d[:type] )
    assert_equal( rng.handle, d[:value][:o] )
    d.value = nil
    assert_equal( :CCS_NONE, d[:type] )
    assert_equal( 0, d[:value][:i] )
  end

  def test_numeric
    n = CCS::Numeric::from_value(1)
    assert_equal( 1, n.value(:CCS_NUM_INTEGER) )
    n.value = 2
    assert_equal( 2, n.value(:CCS_NUM_INTEGER) )
    n.value = 1.0
    assert_equal( 1.0, n.value(:CCS_NUM_FLOAT) )
    n = CCS::Numeric::from_value(2.0)
    assert_equal( 2.0, n.value(:CCS_NUM_FLOAT) )
  end
end
