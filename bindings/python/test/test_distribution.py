import unittest
import sys
sys.path.insert(1, '.')
sys.path.insert(1, '..')
import cconfigspace as ccs

class TestDistribution(unittest.TestCase):

  def test_from_handle_Roulette(self):
    d = ccs.RouletteDistribution(areas = [ 1.0, 2.0, 1.0, 0.5 ])
    d2 = ccs.Object.from_handle(d.handle)
    self.assertEqual( d.__class__, d2.__class__)
    self.assertEqual( d.handle.value, d2.handle.value)

  def test_create_roulette(self):
    areas = [ 1.0, 2.0, 1.0, 0.5 ]
    s = sum(areas)
    d = ccs.RouletteDistribution(areas = areas)
    self.assertEqual( ccs.ObjectType.DISTRIBUTION, d.object_type )
    self.assertEqual( ccs.DistributionType.ROULETTE, d.type )
    self.assertEqual( ccs.NumericType.INT, d.data_type )
    self.assertEqual( 1, d.dimension )
    a = d.areas
    self.assertTrue(  sum(a) > 0.999 )
    self.assertTrue(  sum(a) < 1.001 )
    for i in range(len(a)):
      self.assertTrue( a[i] < areas[i] * 1.001 / s )
      self.assertTrue( a[i] > areas[i] * 0.999 / s )
    i = d.bounds
    self.assertEqual( ccs.NumericType.INT, i.type)
    self.assertEqual( 0, i.lower )
    self.assertEqual( 4, i.upper )
    self.assertTrue( i.lower_included )
    self.assertFalse( i.upper_included )

  def test_serialize_roulette(self):
    areas = [ 1.0, 2.0, 1.0, 0.5 ]
    s = sum(areas)
    dref = ccs.RouletteDistribution(areas = areas)
    buff = dref.serialize()
    d = ccs.Object.deserialize(buffer = buff)
    self.assertEqual( ccs.ObjectType.DISTRIBUTION, d.object_type )
    self.assertEqual( ccs.DistributionType.ROULETTE, d.type )
    self.assertEqual( ccs.NumericType.INT, d.data_type )
    self.assertEqual( 1, d.dimension )
    a = d.areas
    self.assertTrue(  sum(a) > 0.999 )
    self.assertTrue(  sum(a) < 1.001 )
    for i in range(len(a)):
      self.assertTrue( a[i] < areas[i] * 1.001 / s )
      self.assertTrue( a[i] > areas[i] * 0.999 / s )
    i = d.bounds
    self.assertEqual( ccs.NumericType.INT, i.type)
    self.assertEqual( 0, i.lower )
    self.assertEqual( 4, i.upper )
    self.assertTrue( i.lower_included )
    self.assertFalse( i.upper_included )


  def test_from_handle_normal(self):
    d = ccs.NormalDistribution.Float()
    d2 = ccs.Object.from_handle(d.handle)
    self.assertEqual( d.__class__, d2.__class__)
    self.assertEqual( d.handle.value, d2.handle.value)

  def test_create_normal(self):
    d = ccs.NormalDistribution.Float()
    self.assertEqual( ccs.ObjectType.DISTRIBUTION, d.object_type )
    self.assertEqual( ccs.DistributionType.NORMAL, d.type )
    self.assertEqual( ccs.NumericType.FLOAT, d.data_type )
    self.assertEqual( ccs.ScaleType.LINEAR, d.scale )
    self.assertEqual( 1, d.dimension )
    self.assertEqual( 0.0, d.mu )
    self.assertEqual( 1.0, d.sigma )
    i = d.bounds
    self.assertEqual( ccs.NumericType.FLOAT, i.type )
    self.assertEqual( float('-inf'), i.lower )
    self.assertEqual( float('inf'), i.upper )
    self.assertFalse( i.lower_included )
    self.assertFalse( i.upper_included )

  def test_serialize_normal(self):
    dref = ccs.NormalDistribution.Float()
    buff = dref.serialize()
    d = ccs.Object.deserialize(buffer = buff)
    self.assertEqual( ccs.ObjectType.DISTRIBUTION, d.object_type )
    self.assertEqual( ccs.DistributionType.NORMAL, d.type )
    self.assertEqual( ccs.NumericType.FLOAT, d.data_type )
    self.assertEqual( ccs.ScaleType.LINEAR, d.scale )
    self.assertEqual( 1, d.dimension )
    self.assertEqual( 0.0, d.mu )
    self.assertEqual( 1.0, d.sigma )
    i = d.bounds
    self.assertEqual( ccs.NumericType.FLOAT, i.type )
    self.assertEqual( float('-inf'), i.lower )
    self.assertEqual( float('inf'), i.upper )
    self.assertFalse( i.lower_included )
    self.assertFalse( i.upper_included )

  def test_create_normal_float(self):
    d = ccs.NormalDistribution.Float(mu = 2.0, sigma = 6.0)
    self.assertEqual( ccs.ObjectType.DISTRIBUTION, d.object_type )
    self.assertEqual( ccs.DistributionType.NORMAL, d.type )
    self.assertEqual( ccs.NumericType.FLOAT, d.data_type )
    self.assertEqual( ccs.ScaleType.LINEAR, d.scale )
    self.assertEqual( 1, d.dimension )
    self.assertEqual( 2.0, d.mu )
    self.assertEqual( 6.0, d.sigma )

  def test_create_normal_int(self):
    d = ccs.NormalDistribution.Int(mu = 2.0, sigma = 6.0)
    self.assertEqual( ccs.ObjectType.DISTRIBUTION, d.object_type )
    self.assertEqual( ccs.DistributionType.NORMAL, d.type )
    self.assertEqual( ccs.NumericType.INT, d.data_type )
    self.assertEqual( ccs.ScaleType.LINEAR, d.scale )
    self.assertEqual( 1, d.dimension )
    self.assertEqual( 2.0, d.mu )
    self.assertEqual( 6.0, d.sigma )

  def test_sample_normal(self):
    rng = ccs.Rng()
    d = ccs.NormalDistribution.Float()
    i = d.bounds
    v = d.sample(rng)
    self.assertTrue( i.contains(v) )
    a = d.samples(rng, 100)
    self.assertEqual( 100, len(a) )
    for v in a:
      self.assertTrue( i.contains(v) )

  def test_from_handle_uniform(self):
    d = ccs.UniformDistribution.Float(lower = 0.0, upper = 1.0)
    d2 = ccs.Object.from_handle(d.handle)
    self.assertEqual( d.__class__, d2.__class__)
    self.assertEqual( d.handle.value, d2.handle.value)

  def test_create_uniform(self):
    d = ccs.UniformDistribution.Float(lower = 0.0, upper = 1.0)
    self.assertEqual( ccs.ObjectType.DISTRIBUTION, d.object_type )
    self.assertEqual( ccs.DistributionType.UNIFORM, d.type )
    self.assertEqual( ccs.NumericType.FLOAT, d.data_type )
    self.assertEqual( ccs.ScaleType.LINEAR, d.scale )
    self.assertEqual( 1, d.dimension )
    i = d.bounds
    self.assertEqual( ccs.NumericType.FLOAT, i.type )
    self.assertEqual( 0.0, i.lower)
    self.assertEqual( 1.0, i.upper)
    self.assertEqual( 0.0, d.lower)
    self.assertEqual( 1.0, d.upper)
    self.assertTrue( i.lower_included )
    self.assertFalse( i.upper_included )

  def test_serialize_uniform(self):
    dref = ccs.UniformDistribution.Float(lower = 0.0, upper = 1.0)
    buff = dref.serialize()
    d = ccs.Object.deserialize(buffer = buff)
    self.assertEqual( ccs.ObjectType.DISTRIBUTION, d.object_type )
    self.assertEqual( ccs.DistributionType.UNIFORM, d.type )
    self.assertEqual( ccs.NumericType.FLOAT, d.data_type )
    self.assertEqual( ccs.ScaleType.LINEAR, d.scale )
    self.assertEqual( 1, d.dimension )
    i = d.bounds
    self.assertEqual( ccs.NumericType.FLOAT, i.type )
    self.assertEqual( 0.0, i.lower)
    self.assertEqual( 1.0, i.upper)
    self.assertEqual( 0.0, d.lower)
    self.assertEqual( 1.0, d.upper)
    self.assertTrue( i.lower_included )
    self.assertFalse( i.upper_included )

  def test_create_uniform_float(self):
    d = ccs.UniformDistribution.Float(lower = -1.0, upper = 1.0)
    self.assertEqual( ccs.ObjectType.DISTRIBUTION, d.object_type )
    self.assertEqual( ccs.DistributionType.UNIFORM, d.type )
    self.assertEqual( ccs.NumericType.FLOAT, d.data_type )
    self.assertEqual( ccs.ScaleType.LINEAR, d.scale )
    self.assertEqual( 1, d.dimension )
    i = d.bounds
    self.assertEqual( ccs.NumericType.FLOAT, i.type )
    self.assertEqual( -1.0, i.lower)
    self.assertEqual(  1.0, i.upper)
    self.assertEqual( -1.0, d.lower)
    self.assertEqual(  1.0, d.upper)
    self.assertTrue( i.lower_included )
    self.assertFalse( i.upper_included )

  def test_create_uniform_int(self):
    d = ccs.UniformDistribution.Int(lower = 0, upper = 100)
    self.assertEqual( ccs.ObjectType.DISTRIBUTION, d.object_type )
    self.assertEqual( ccs.DistributionType.UNIFORM, d.type )
    self.assertEqual( ccs.NumericType.INT, d.data_type )
    self.assertEqual( ccs.ScaleType.LINEAR, d.scale )
    self.assertEqual( 1, d.dimension )
    i = d.bounds
    self.assertEqual( ccs.NumericType.INT, i.type )
    self.assertEqual(  0,  i.lower)
    self.assertEqual( 100, i.upper)
    self.assertEqual(   0, d.lower)
    self.assertEqual( 100, d.upper)
    self.assertTrue( i.lower_included )
    self.assertFalse( i.upper_included )

  def test_oversampling_uniform_float(self):
    d = ccs.UniformDistribution.Float(lower = -1.0, upper = 1.0)
    i = ccs.Interval(t = ccs.NumericType.FLOAT, lower = -0.2, upper = 0.2)
    self.assertTrue( d.is_oversampling(i) )
    i = ccs.Interval(t = ccs.NumericType.FLOAT, lower = -0.2, upper = 2.0)
    self.assertTrue( d.is_oversampling(i) )
    i = ccs.Interval(t = ccs.NumericType.FLOAT, lower = -2.0, upper = 2.0)
    self.assertFalse( d.is_oversampling(i) )

  def test_oversampling_uniform_int(self):
    d = ccs.UniformDistribution.Int(lower = 0, upper = 100)
    i = ccs.Interval(t = ccs.NumericType.INT, lower = 5, upper = 50)
    self.assertTrue( d.is_oversampling(i) )
    i = ccs.Interval(t = ccs.NumericType.INT, lower = 5, upper = 150)
    self.assertTrue( d.is_oversampling(i) )
    i = ccs.Interval(t = ccs.NumericType.INT, lower = -5, upper = 150)
    self.assertFalse( d.is_oversampling(i) )

  def test_sample_uniform(self):
    rng = ccs.Rng()
    d = ccs.UniformDistribution.Float(lower = 0.0, upper = 1.0)
    i = d.bounds
    v = d.sample(rng)
    self.assertTrue( i.contains(v) )
    a = d.samples(rng, 100)
    self.assertEqual( 100, len(a) )
    for v in a:
      self.assertTrue( i.contains(v) )

  def test_create_mixture(self):
    distributions = [ ccs.UniformDistribution.Float(lower = -5.0, upper = 0.0),
                      ccs.UniformDistribution.Float(lower =  0.0, upper = 2.0) ]
    d = ccs.MixtureDistribution(distributions = distributions)
    self.assertEqual( d.object_type, ccs.ObjectType.DISTRIBUTION )
    self.assertEqual( d.type, ccs.DistributionType.MIXTURE )
    self.assertEqual( d.data_types, [ccs.NumericType.FLOAT] )
    self.assertEqual( d.weights, [0.5, 0.5] )
    self.assertEqual( [x.handle.value for x in d.distributions], [x.handle.value for x in distributions] )
    d2 = ccs.Object.from_handle(d.handle)
    self.assertEqual( d.__class__, d2.__class__ )

  def test_serialize_mixture(self):
    distributions = [ ccs.UniformDistribution.Float(lower = -5.0, upper = 0.0),
                      ccs.UniformDistribution.Float(lower =  0.0, upper = 2.0) ]
    dref = ccs.MixtureDistribution(distributions = distributions)
    buff = dref.serialize()
    d = ccs.Object.deserialize(buffer = buff)
    self.assertEqual( d.object_type, ccs.ObjectType.DISTRIBUTION )
    self.assertEqual( d.type, ccs.DistributionType.MIXTURE )
    self.assertEqual( d.data_types, [ccs.NumericType.FLOAT] )
    self.assertEqual( d.weights, [0.5, 0.5] )
    for i in [0,1]:
      d2ref = distributions[i]
      d2 = d.distributions[i]
      self.assertEqual( d2ref.__class__, d2.__class__ )
      self.assertEqual( d2ref.lower, d2.lower )
      self.assertEqual( d2ref.upper, d2.upper )

  def test_create_multivariate(self):
    distributions = [ ccs.UniformDistribution.Float(lower = -5.0, upper = 0.0),
                      ccs.UniformDistribution.Int(lower =  0, upper = 2) ]
    d = ccs.MultivariateDistribution(distributions = distributions)
    self.assertEqual( d.object_type, ccs.ObjectType.DISTRIBUTION )
    self.assertEqual( d.type, ccs.DistributionType.MULTIVARIATE )
    self.assertEqual( d.data_types, [ccs.NumericType.FLOAT, ccs.NumericType.INT] )
    self.assertEqual( [x.handle.value for x in d.distributions], [x.handle.value for x in distributions] )
    d2 = ccs.Object.from_handle(d.handle)
    self.assertEqual( d.__class__, d2.__class__ )

  def test_serialize_multivariate(self):
    distributions = [ ccs.UniformDistribution.Float(lower = -5.0, upper = 0.0),
                      ccs.UniformDistribution.Int(lower =  0, upper = 2) ]
    dref = ccs.MultivariateDistribution(distributions = distributions)
    buff = dref.serialize()
    d = ccs.Object.deserialize(buffer = buff)
    self.assertEqual( d.object_type, ccs.ObjectType.DISTRIBUTION )
    self.assertEqual( d.type, ccs.DistributionType.MULTIVARIATE )
    self.assertEqual( d.data_types, [ccs.NumericType.FLOAT, ccs.NumericType.INT] )
    for i in [0,1]:
      d2ref = distributions[i]
      d2 = d.distributions[i]
      self.assertEqual( d2ref.__class__, d2.__class__ )
      self.assertEqual( d2ref.lower, d2.lower )
      self.assertEqual( d2ref.upper, d2.upper )

  def test_mixture_multidim(self):
    distributions = [ ccs.UniformDistribution.Float(lower = -5.0, upper = 0.0),
                      ccs.UniformDistribution.Int(lower =  0, upper = 2) ]
    d = ccs.MultivariateDistribution(distributions = distributions)
    d2 = ccs.MixtureDistribution(distributions = [d, d])
    self.assertEqual( d2.object_type, ccs.ObjectType.DISTRIBUTION )
    self.assertEqual( d2.type, ccs.DistributionType.MIXTURE )
    self.assertEqual( d2.data_types, [ccs.NumericType.FLOAT, ccs.NumericType.INT] )
    self.assertEqual( d2.weights, [0.5, 0.5] )


if __name__ == '__main__':
    unittest.main()
