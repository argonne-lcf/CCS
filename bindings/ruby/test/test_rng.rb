[ '../lib', 'lib' ].each { |d| $:.unshift(d) if File::directory?(d) }
require 'minitest/autorun'
require 'cconfigspace'

class CConfigSpaceTestRng < Minitest::Test
  def setup
    CCS.init
  end

  def test_create
    rng = CCS::Rng::new
    assert_equal( :CCS_RNG, rng.object_type )
    assert_equal( 1, rng.refcount )
    rng = nil
    GC.start
  end

  def test_get
    rng = CCS::Rng::new
    v = rng.get
    assert( v.kind_of?(Integer) )
    assert( v >= rng.min )
    assert( v <= rng.max )
  end

  def test_uniform
    rng = CCS::Rng::new
    v = rng.uniform
    assert( v.kind_of?(Float) )
    assert( v >= 0.0 )
    assert( v <  1.0 )
  end

  def test_set_seed
    rng = CCS::Rng::new
    rng.seed = 10
    v1 = rng.get
    rng.seed = 10
    v2 = rng.get
    assert_equal(v1, v2)
  end

end
