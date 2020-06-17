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
end
