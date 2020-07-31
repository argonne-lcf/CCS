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
    self.assertEqual( ccs.DISTRIBUTION, d.object_type )
    self.assertEqual( ccs.ROULETTE, d.type )
    self.assertEqual( ccs.NUM_INTEGER, d.data_type )
    self.assertEqual( ccs.LINEAR, d.scale )
    self.assertEqual( 1, d.dimension )
    a = d.areas
    self.assertTrue(  sum(a) > 0.999 )
    self.assertTrue(  sum(a) < 1.001 )
    for i in range(len(a)):
      self.assertTrue( a[i] < areas[i] * 1.001 / s )
      self.assertTrue( a[i] > areas[i] * 0.999 / s )
    i = d.bounds
    self.assertEqual( ccs.NUM_INTEGER, i.type)
    self.assertEqual( 0, i.lower )
    self.assertEqual( 4, i.upper )
    self.assertTrue( i.lower_included )
    self.assertFalse( i.upper_included )

  def test_from_handle_normal(self):
    d = ccs.NormalDistribution()
    d2 = ccs.Object.from_handle(d.handle)
    self.assertEqual( d.__class__, d2.__class__)
    self.assertEqual( d.handle.value, d2.handle.value)

  def test_create_normal(self):
    d = ccs.NormalDistribution()
    self.assertEqual( ccs.DISTRIBUTION, d.object_type )
    self.assertEqual( ccs.NORMAL, d.type )
    self.assertEqual( ccs.NUM_FLOAT, d.data_type )
    self.assertEqual( ccs.LINEAR, d.scale )
    self.assertEqual( 1, d.dimension )
    self.assertEqual( 0.0, d.mu )
    self.assertEqual( 1.0, d.sigma )
    i = d.bounds
    self.assertEqual( ccs.NUM_FLOAT, i.type )
    self.assertEqual( float('-inf'), i.lower )
    self.assertEqual( float('inf'), i.upper )
    self.assertFalse( i.lower_included )
    self.assertFalse( i.upper_included )

  def test_create_normal_float(self):
    d = ccs.NormalDistribution.float(mu = 2.0, sigma = 6.0)
    self.assertEqual( ccs.DISTRIBUTION, d.object_type )
    self.assertEqual( ccs.NORMAL, d.type )
    self.assertEqual( ccs.NUM_FLOAT, d.data_type )
    self.assertEqual( ccs.LINEAR, d.scale )
    self.assertEqual( 1, d.dimension )
    self.assertEqual( 2.0, d.mu )
    self.assertEqual( 6.0, d.sigma )

  def test_create_normal_int(self):
    d = ccs.NormalDistribution.int(mu = 2.0, sigma = 6.0)
    self.assertEqual( ccs.DISTRIBUTION, d.object_type )
    self.assertEqual( ccs.NORMAL, d.type )
    self.assertEqual( ccs.NUM_INTEGER, d.data_type )
    self.assertEqual( ccs.LINEAR, d.scale )
    self.assertEqual( 1, d.dimension )
    self.assertEqual( 2.0, d.mu )
    self.assertEqual( 6.0, d.sigma )

  def test_sample_normal(self):
    rng = ccs.Rng()
    d = ccs.NormalDistribution()
    i = d.bounds
    v = d.sample(rng)
    self.assertTrue( i.include(v) )
    a = d.samples(rng, 100)
    self.assertEqual( 100, len(a) )
    for v in a:
      self.assertTrue( i.include(v) )

  def test_from_handle_uniform(self):
    d = ccs.UniformDistribution()
    d2 = ccs.Object.from_handle(d.handle)
    self.assertEqual( d.__class__, d2.__class__)
    self.assertEqual( d.handle.value, d2.handle.value)

  def test_create_uniform(self):
    d = ccs.UniformDistribution()
    self.assertEqual( ccs.DISTRIBUTION, d.object_type )
    self.assertEqual( ccs.UNIFORM, d.type )
    self.assertEqual( ccs.NUM_FLOAT, d.data_type )
    self.assertEqual( ccs.LINEAR, d.scale )
    self.assertEqual( 1, d.dimension )
    i = d.bounds
    self.assertEqual( ccs.NUM_FLOAT, i.type )
    self.assertEqual( 0.0, i.lower)
    self.assertEqual( 1.0, i.upper)
    self.assertEqual( 0.0, d.lower)
    self.assertEqual( 1.0, d.upper)
    self.assertTrue( i.lower_included )
    self.assertFalse( i.upper_included )

  def test_create_uniform_float(self):
    d = ccs.UniformDistribution.float(lower = -1.0, upper = 1.0)
    self.assertEqual( ccs.DISTRIBUTION, d.object_type )
    self.assertEqual( ccs.UNIFORM, d.type )
    self.assertEqual( ccs.NUM_FLOAT, d.data_type )
    self.assertEqual( ccs.LINEAR, d.scale )
    self.assertEqual( 1, d.dimension )
    i = d.bounds
    self.assertEqual( ccs.NUM_FLOAT, i.type )
    self.assertEqual( -1.0, i.lower)
    self.assertEqual(  1.0, i.upper)
    self.assertEqual( -1.0, d.lower)
    self.assertEqual(  1.0, d.upper)
    self.assertTrue( i.lower_included )
    self.assertFalse( i.upper_included )

  def test_create_uniform_int(self):
    d = ccs.UniformDistribution.int(lower = 0, upper = 100)
    self.assertEqual( ccs.DISTRIBUTION, d.object_type )
    self.assertEqual( ccs.UNIFORM, d.type )
    self.assertEqual( ccs.NUM_INTEGER, d.data_type )
    self.assertEqual( ccs.LINEAR, d.scale )
    self.assertEqual( 1, d.dimension )
    i = d.bounds
    self.assertEqual( ccs.NUM_INTEGER, i.type )
    self.assertEqual(  0,  i.lower)
    self.assertEqual( 100, i.upper)
    self.assertEqual(   0, d.lower)
    self.assertEqual( 100, d.upper)
    self.assertTrue( i.lower_included )
    self.assertFalse( i.upper_included )

  def test_oversampling_uniform_float(self):
    d = ccs.UniformDistribution.float(lower = -1.0, upper = 1.0)
    i = ccs.ccs_interval(t = ccs.NUM_FLOAT, lower = -0.2, upper = 0.2)
    self.assertTrue( d.oversampling(i) )
    i = ccs.ccs_interval(t = ccs.NUM_FLOAT, lower = -0.2, upper = 2.0)
    self.assertTrue( d.oversampling(i) )
    i = ccs.ccs_interval(t = ccs.NUM_FLOAT, lower = -2.0, upper = 2.0)
    self.assertFalse( d.oversampling(i) )

  def test_oversampling_uniform_int(self):
    d = ccs.UniformDistribution.int(lower = 0, upper = 100)
    i = ccs.ccs_interval(t = ccs.NUM_INTEGER, lower = 5, upper = 50)
    self.assertTrue( d.oversampling(i) )
    i = ccs.ccs_interval(t = ccs.NUM_INTEGER, lower = 5, upper = 150)
    self.assertTrue( d.oversampling(i) )
    i = ccs.ccs_interval(t = ccs.NUM_INTEGER, lower = -5, upper = 150)
    self.assertFalse( d.oversampling(i) )

  def test_sample_uniform(self):
    rng = ccs.Rng()
    d = ccs.UniformDistribution()
    i = d.bounds
    v = d.sample(rng)
    self.assertTrue( i.include(v) )
    a = d.samples(rng, 100)
    self.assertEqual( 100, len(a) )
    for v in a:
      self.assertTrue( i.include(v) )


if __name__ == '__main__':
    unittest.main()
