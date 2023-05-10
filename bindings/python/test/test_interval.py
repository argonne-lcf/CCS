import unittest
import sys
sys.path.insert(1, '.')
sys.path.insert(1, '..')
import cconfigspace as ccs
from math import sin


class TestInterval(unittest.TestCase):

  def test_create(self):
    i = ccs.Interval(t = ccs.NumericType.FLOAT, lower = -1.0, upper = 1.0)
    self.assertEqual( ccs.NumericType.FLOAT, i.type )
    self.assertEqual( -1.0, i.lower )
    self.assertEqual( 1.0, i.upper )
    self.assertTrue( i.lower_included )
    self.assertFalse( i.upper_included )

  def test_empty(self):
    i = ccs.Interval(t = ccs.NumericType.FLOAT, lower = -1.0, upper = 1.0)
    self.assertFalse( i.empty )
    i = ccs.Interval(t = ccs.NumericType.FLOAT, lower = 1.0, upper = -1.0)
    self.assertTrue( i.empty )

  def test_equal(self):
    i1 = ccs.Interval(t = ccs.NumericType.FLOAT, lower = -1.0, upper = 1.0)
    i2 = ccs.Interval(t = ccs.NumericType.FLOAT, lower = -1.0, upper = 1.0)
    self.assertEqual(i1, i2)
    i2 = ccs.Interval(t = ccs.NumericType.FLOAT, lower = -1.0, upper = 1.0, upper_included = True)
    self.assertNotEqual(i1, i2)

  def test_intersect(self):
    i1 = ccs.Interval(t = ccs.NumericType.FLOAT, lower = -1.0, upper = 1.0)
    i2 = ccs.Interval(t = ccs.NumericType.FLOAT, lower = 0.0, upper = 2.0)
    i3 = i1.intersect(i2)
    i4 = ccs.Interval(t = ccs.NumericType.FLOAT, lower = 0.0, upper = 1.0)
    self.assertEqual( i4, i3 )
    i1 = ccs.Interval(t = ccs.NumericType.FLOAT, lower = -1.0, upper = 0.0)
    i2 = ccs.Interval(t = ccs.NumericType.FLOAT, lower = 0.0, upper = 2.0)
    i3 = i1.intersect(i2)
    self.assertTrue( i3.empty )

  def test_contains(self):
    i = ccs.Interval(t = ccs.NumericType.FLOAT, lower = -1.0, upper = 1.0)
    self.assertTrue( i.contains(0.0) )
    self.assertFalse( i.contains(2.0) )
    i = ccs.Interval(t = ccs.NumericType.INT, lower = -5, upper = 5)
    self.assertTrue( i.contains(0) )
    self.assertFalse( i.contains(6) )

if __name__ == '__main__':
    unittest.main()
