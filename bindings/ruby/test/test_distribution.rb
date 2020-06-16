[ '../lib', 'lib' ].each { |d| $:.unshift(d) if File::directory?(d) }
require 'minitest/autorun'
require 'cconfigspace'

class CConfigSpaceTestDistribution < Minitest::Test
  def setup
    CCS.init
  end

  def test_create_uniform
    d = CCS::UniformDistribution::new
    assert( d.object_type == :CCS_DISTRIBUTION )
    assert( d.type == :CCS_UNIFORM )
    assert( d.data_type == :CCS_NUM_FLOAT )
    assert( d.scale_type == :CCS_LINEAR )
    assert( d.dimension == 1 )
    i = d.bounds
    assert_equal( :CCS_NUM_FLOAT, i.type)
    assert_equal( 0.0, i.lower)
    assert_equal( 1.0, i.upper)
    assert( i.lower_included? )
    refute( i.upper_included? )
  end

  def test_sample_uniform
    rng = CCS::Rng::new
    d = CCS::UniformDistribution::new
    i = d.bounds
    v = d.sample(rng)
    assert( i.include?(v) )
    a = d.samples(rng, 100)
    assert_equal(100, a.size)
    a.each { |w|
      assert( i.include?(w) )
    }
  end
end
