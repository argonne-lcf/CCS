[ '../lib', 'lib' ].each { |d| $:.unshift(d) if File::directory?(d) }
require 'minitest/autorun'
require 'cconfigspace'

class CConfigSpaceTestDistribution < Minitest::Test
  def setup
    CCS.init
  end

  def test_from_handle_roulette
    areas = [ 1.0, 2.0, 1.0, 0.5 ]
    d = CCS::RouletteDistribution::new(areas: areas)
    d2 = CCS::Object::from_handle(d)
    assert_equal( d.class, d2.class )
  end

  def roulette_check(areas, d)
    sum = areas.reduce(:+)
    assert_equal( :CCS_OBJECT_TYPE_DISTRIBUTION, d.object_type )
    assert_equal( :CCS_ROULETTE, d.type )
    assert_equal( :CCS_NUM_INTEGER, d.data_type )
    assert_equal( 1, d.dimension )
    assert_equal( 4, d.num_areas )
    assert( d.areas.reduce(:+) > 0.999 )
    assert( d.areas.reduce(:+) < 1.001 )
    d.areas.each_with_index { |a, i|
      assert( a < areas[i] * 1.001 / sum && a > areas[i] * 0.999 / sum )
    }
    i = d.bounds
    assert_equal( :CCS_NUM_INTEGER, i.type)
    assert_equal( 0, i.lower)
    assert_equal( 4, i.upper)
    assert( i.lower_included? )
    refute( i.upper_included? )
  end

  def test_create_roulette
    areas = [ 1.0, 2.0, 1.0, 0.5 ]
    d = CCS::RouletteDistribution::new(areas: areas)
    roulette_check(areas, d)
  end

  def test_serialize_roulette
    areas = [ 1.0, 2.0, 1.0, 0.5 ]
    dref = CCS::RouletteDistribution::new(areas: areas)
    buff = dref.serialize
    d = CCS::deserialize(buffer: buff)
    roulette_check(areas, d)
  end

  def test_from_handle_normal
    d = CCS::NormalDistribution::new
    d2 = CCS::Object::from_handle(d)
    assert_equal( d.class, d2.class )
  end

  def normal_check(d)
    assert( d.object_type == :CCS_OBJECT_TYPE_DISTRIBUTION )
    assert( d.type == :CCS_NORMAL )
    assert( d.data_type == :CCS_NUM_FLOAT )
    assert( d.scale == :CCS_LINEAR )
    assert( d.dimension == 1 )
    assert( d.mu == 0.0 )
    assert( d.sigma == 1.0 )
    i = d.bounds
    assert_equal( :CCS_NUM_FLOAT, i.type )
    assert_equal( -Float::INFINITY, i.lower )
    assert_equal( Float::INFINITY, i.upper )
    refute( i.lower_included? )
    refute( i.upper_included? )
  end

  def test_create_normal
    d = CCS::NormalDistribution::new
    normal_check(d)
  end

  def test_serialize_normal
    dref = CCS::NormalDistribution::new
    buff = dref.serialize
    d = CCS::deserialize(buffer: buff)
    normal_check(d)
  end

  def test_create_normal_int
    d = CCS::NormalDistribution::int(mu: 2.0, sigma: 5.0)
    assert( d.object_type == :CCS_OBJECT_TYPE_DISTRIBUTION )
    assert( d.type == :CCS_NORMAL )
    assert( d.data_type == :CCS_NUM_INTEGER )
    assert( d.scale == :CCS_LINEAR )
    assert( d.dimension == 1 )
    assert( d.mu == 2.0 )
    assert( d.sigma == 5.0 )
  end

  def test_create_normal_float
    d = CCS::NormalDistribution::float(mu: 2.0, sigma: 5.0)
    assert( d.object_type == :CCS_OBJECT_TYPE_DISTRIBUTION )
    assert( d.type == :CCS_NORMAL )
    assert( d.data_type == :CCS_NUM_FLOAT )
    assert( d.scale == :CCS_LINEAR )
    assert( d.dimension == 1 )
    assert( d.mu == 2.0 )
    assert( d.sigma == 5.0 )
  end

  def test_sample_normal
    rng = CCS::Rng::new
    d = CCS::NormalDistribution::new
    i = d.bounds
    v = d.sample(rng)
    assert( i.include?(v) )
    a = d.samples(rng, 100)
    assert_equal(100, a.size)
    a.each { |w|
      assert( i.include?(w) )
    }
  end

  def test_from_handle_uniform
    d = CCS::UniformDistribution::new
    d2 = CCS::Object::from_handle(d)
    assert_equal( d.class, d2.class )
  end

  def uniform_check(d)
    assert( d.object_type == :CCS_OBJECT_TYPE_DISTRIBUTION )
    assert( d.type == :CCS_UNIFORM )
    assert( d.data_type == :CCS_NUM_FLOAT )
    assert( d.scale == :CCS_LINEAR )
    assert( d.dimension == 1 )
    i = d.bounds
    assert_equal( :CCS_NUM_FLOAT, i.type )
    assert_equal( 0.0, i.lower )
    assert_equal( 1.0, i.upper )
    assert( i.lower_included? )
    refute( i.upper_included? )
  end

  def test_create_uniform
    d = CCS::UniformDistribution::new
    uniform_check(d)
  end

  def test_serialize_uniform
    dref = CCS::UniformDistribution::new
    buff = dref.serialize
    d = CCS::deserialize(buffer: buff)
    uniform_check(d)
  end

  def test_create_uniform_float
    d = CCS::UniformDistribution::float(lower: -1.0, upper: 1.0)
    assert( d.object_type == :CCS_OBJECT_TYPE_DISTRIBUTION )
    assert( d.type == :CCS_UNIFORM )
    assert( d.data_type == :CCS_NUM_FLOAT )
    assert( d.scale == :CCS_LINEAR )
    assert( d.dimension == 1 )
    i = d.bounds
    assert_equal( :CCS_NUM_FLOAT, i.type)
    assert_equal( -1.0, i.lower)
    assert_equal(  1.0, i.upper)
    assert_equal( -1.0, d.lower)
    assert_equal(  1.0, d.upper)
    assert( i.lower_included? )
    refute( i.upper_included? )
  end

  def test_create_uniform_int
    d = CCS::UniformDistribution::int(lower: 0, upper: 100)
    assert( d.object_type == :CCS_OBJECT_TYPE_DISTRIBUTION )
    assert( d.type == :CCS_UNIFORM )
    assert( d.data_type == :CCS_NUM_INTEGER )
    assert( d.scale == :CCS_LINEAR )
    assert( d.dimension == 1 )
    i = d.bounds
    assert_equal( :CCS_NUM_INTEGER, i.type)
    assert_equal(   0, i.lower)
    assert_equal( 100, i.upper)
    assert_equal(   0, d.lower)
    assert_equal( 100, d.upper)
    assert( i.lower_included? )
    refute( i.upper_included? )
  end

  def test_oversampling_uniform_float
    d = CCS::UniformDistribution::float(lower: -1.0, upper: 1.0)
    i = CCS::Interval::new(type: :CCS_NUM_FLOAT, lower: -0.2, upper: 0.2)
    assert(d.oversampling?(i))
    i = CCS::Interval::new(type: :CCS_NUM_FLOAT, lower: -0.2, upper: 2.0)
    assert(d.oversampling?(i))
    i = CCS::Interval::new(type: :CCS_NUM_FLOAT, lower: -2, upper: 2.0)
    refute(d.oversampling?(i))
  end

  def test_oversampling_uniform_int
    d = CCS::UniformDistribution::int(lower: 0, upper: 100)
    i = CCS::Interval::new(type: :CCS_NUM_INTEGER, lower: 5, upper: 50)
    assert(d.oversampling?(i))
    i = CCS::Interval::new(type: :CCS_NUM_INTEGER, lower: 5, upper: 150)
    assert(d.oversampling?(i))
    i = CCS::Interval::new(type: :CCS_NUM_INTEGER, lower: -5, upper: 150)
    refute(d.oversampling?(i))
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

  def test_create_mixture
    distributions = [ CCS::UniformDistribution::float(lower: -5.0, upper: 0.0), CCS::UniformDistribution::float(lower: 0.0, upper: 2.0) ]
    d = CCS::MixtureDistribution::new(distributions: distributions)
    assert( d.object_type == :CCS_OBJECT_TYPE_DISTRIBUTION )
    assert( d.type == :CCS_MIXTURE )
    assert( d.data_types == [:CCS_NUM_FLOAT] )
    assert( d.weights == [0.5, 0.5] )
    assert( d.distributions.collect(&:handle) == distributions.collect(&:handle) )
    d2 = CCS::Object::from_handle(d)
    assert_equal( d.class, d2.class )
  end

  def test_serialize_mixture
    distributions = [ CCS::UniformDistribution::float(lower: -5.0, upper: 0.0), CCS::UniformDistribution::float(lower: 0.0, upper: 2.0) ]
    dref = CCS::MixtureDistribution::new(distributions: distributions)
    buff = dref.serialize
    d = CCS::deserialize(buffer: buff)
    assert( d.object_type == :CCS_OBJECT_TYPE_DISTRIBUTION )
    assert( d.type == :CCS_MIXTURE )
    assert( d.data_types == [:CCS_NUM_FLOAT] )
    assert( d.weights == [0.5, 0.5] )
    distributions.each_with_index { |d2ref, i|
      d2 = d.distributions[i]
      assert_equal( d2ref.class, d2.class )
      assert_equal( d2ref.lower, d2.lower )
      assert_equal( d2ref.upper, d2.upper )
    }
  end

  def test_create_multivariate
    distributions = [ CCS::UniformDistribution::float(lower: -5.0, upper: 0.0), CCS::UniformDistribution::int(lower: 0, upper: 2) ]
    d = CCS::MultivariateDistribution::new(distributions: distributions)
    assert( d.object_type == :CCS_OBJECT_TYPE_DISTRIBUTION )
    assert( d.type == :CCS_MULTIVARIATE )
    assert( d.data_types == [:CCS_NUM_FLOAT, :CCS_NUM_INTEGER] )
    assert( d.distributions.collect(&:handle) == distributions.collect(&:handle) )
    d2 = CCS::Object::from_handle(d)
    assert_equal( d.class, d2.class )
  end

  def test_serialize_multivariate
    distributions = [ CCS::UniformDistribution::float(lower: -5.0, upper: 0.0), CCS::UniformDistribution::int(lower: 0, upper: 2) ]
    dref = CCS::MultivariateDistribution::new(distributions: distributions)
    buff = dref.serialize
    d = CCS::deserialize(buffer: buff)
    assert( d.object_type == :CCS_OBJECT_TYPE_DISTRIBUTION )
    assert( d.type == :CCS_MULTIVARIATE )
    assert( d.data_types == [:CCS_NUM_FLOAT, :CCS_NUM_INTEGER] )
    distributions.each_with_index { |d2ref, i|
      d2 = d.distributions[i]
      assert_equal( d2ref.class, d2.class )
      assert_equal( d2ref.lower, d2.lower )
      assert_equal( d2ref.upper, d2.upper )
    }
  end

  def test_mixture_multidim
    distributions = [ CCS::UniformDistribution::float(lower: -5.0, upper: 0.0), CCS::UniformDistribution::int(lower: 0, upper: 2) ]
    d = CCS::MultivariateDistribution::new(distributions: distributions)
    d2 = CCS::MixtureDistribution::new(distributions: [d, d])
    assert( d2.object_type == :CCS_OBJECT_TYPE_DISTRIBUTION )
    assert( d2.type == :CCS_MIXTURE )
    assert( d2.data_types == [:CCS_NUM_FLOAT, :CCS_NUM_INTEGER] )
    assert( d2.weights == [0.5, 0.5] )
  end
end
