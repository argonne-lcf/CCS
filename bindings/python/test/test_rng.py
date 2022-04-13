import unittest
import sys
sys.path.insert(1, '.')
sys.path.insert(1, '..')
import cconfigspace as ccs
from math import sin


class TestRng(unittest.TestCase):

  def test_create(self):
    rng = ccs.Rng()
    self.assertEqual( ccs.RNG, rng.object_type )
    self.assertEqual( 1, rng.refcount )
    rng = None

  def test_get(self):
    rng = ccs.Rng()
    v = rng.get()
    self.assertIsInstance( v, int)
    self.assertTrue( v >= rng.min )
    self.assertTrue( v <= rng.max )

  def test_uniform(self):
    rng = ccs.Rng()
    v = rng.uniform()
    self.assertIsInstance( v, float)
    self.assertTrue( v >= 0.0 )
    self.assertTrue( v <  1.0 )

  def test_set_seed(self):
    rng = ccs.Rng()
    rng.seed = 10
    v1 = rng.get()
    rng.seed = 10
    v2 = rng.get()
    self.assertEqual(v1, v2)

  def test_serialize(self):
    rng = ccs.Rng()
    rng.seed = 10
    buff = rng.serialize()
    rng2 = ccs.Object.deserialize(buff)
    self.assertEqual( ccs.RNG, rng2.object_type )
    v1 = rng.get()
    v2 = rng2.get()
    self.assertEqual(v1, v2)

if __name__ == '__main__':
    unittest.main()
