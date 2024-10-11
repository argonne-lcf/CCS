require 'minitest/autorun'
require_relative '../lib/cconfigspace'

class CConfigSpaceTestInterval < Minitest::Test
  def test_new
    i = CCS::Interval::new(type: :CCS_NUMERIC_TYPE_FLOAT, lower: -1.0, upper: 1.0)
    assert_equal( :CCS_NUMERIC_TYPE_FLOAT, i.type)
    assert_equal(-1.0, i.lower)
    assert_equal( 1.0, i.upper)
    assert( i.lower_included? )
    refute( i.upper_included? )
  end

  def test_empty
    i = CCS::Interval::new(type: :CCS_NUMERIC_TYPE_FLOAT, lower: -1.0, upper: 1.0)
    refute( i.empty? )
    i = CCS::Interval::new(type: :CCS_NUMERIC_TYPE_FLOAT, lower: 1.0, upper: -1.0)
    assert( i.empty? )
  end

  def test_equal
    i1 = CCS::Interval::new(type: :CCS_NUMERIC_TYPE_FLOAT, lower: -1.0, upper: 1.0)
    i2 = CCS::Interval::new(type: :CCS_NUMERIC_TYPE_FLOAT, lower: -1.0, upper: 1.0)
    assert_equal(i1, i2)
    i2 = CCS::Interval::new(type: :CCS_NUMERIC_TYPE_FLOAT, lower: -1.0, upper: 1.0, upper_included: true)
    refute_equal(i1, i2)
  end

  def test_intersect
    i1 = CCS::Interval::new(type: :CCS_NUMERIC_TYPE_FLOAT, lower: -1.0, upper: 1.0)
    i2 = CCS::Interval::new(type: :CCS_NUMERIC_TYPE_FLOAT, lower: 0.0, upper: 2.0)
    i3 = i1.intersect(i2)
    i4 = CCS::Interval::new(type: :CCS_NUMERIC_TYPE_FLOAT, lower: 0.0, upper: 1.0)
    assert_equal(i4, i3)
    i1 = CCS::Interval::new(type: :CCS_NUMERIC_TYPE_FLOAT, lower: -1.0, upper: 0.0)
    i2 = CCS::Interval::new(type: :CCS_NUMERIC_TYPE_FLOAT, lower: 0.0, upper: 2.0)
    i3 = i1.intersect(i2)
    assert(i3.empty?)
  end

  def test_include
    i = CCS::Interval::new(type: :CCS_NUMERIC_TYPE_FLOAT, lower: -1.0, upper: 1.0)
    assert( i.include?(0.0) )
    refute( i.include?(2.0) )
    i = CCS::Interval::new(type: :CCS_NUMERIC_TYPE_INT, lower: -5, upper: 5)
    assert( i.include?(0) )
    refute( i.include?(6) )
  end

end
