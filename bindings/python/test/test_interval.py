import unittest
import cconfigspace as ccs


class TestInterval(unittest.TestCase):

  def test_create(self):
    i = ccs.ccs_interval(t = ccs.NUM_FLOAT, lower = -1.0, upper = 1.0)
    self.assertEqual( ccs.NUM_FLOAT, i.type )
    self.assertEqual( -1.0, i.lower )
    self.assertEqual( 1.0, i.upper )
    self.assertTrue( i.lower_included )
    self.assertFalse( i.upper_included )

  def test_empty(self):
    i = ccs.ccs_interval(t = ccs.NUM_FLOAT, lower = -1.0, upper = 1.0)
    self.assertFalse( i.empty )
    i = ccs.ccs_interval(t = ccs.NUM_FLOAT, lower = 1.0, upper = -1.0)
    self.assertTrue( i.empty )

  def test_equal(self):
    i1 = ccs.ccs_interval(t = ccs.NUM_FLOAT, lower = -1.0, upper = 1.0)
    i2 = ccs.ccs_interval(t = ccs.NUM_FLOAT, lower = -1.0, upper = 1.0)
    self.assertEqual(i1, i2)
    i2 = ccs.ccs_interval(t = ccs.NUM_FLOAT, lower = -1.0, upper = 1.0, upper_included = True)
    self.assertNotEqual(i1, i2)

  def test_intersect(self):
    i1 = ccs.ccs_interval(t = ccs.NUM_FLOAT, lower = -1.0, upper = 1.0)
    i2 = ccs.ccs_interval(t = ccs.NUM_FLOAT, lower = 0.0, upper = 2.0)
    i3 = i1.intersect(i2)
    i4 = ccs.ccs_interval(t = ccs.NUM_FLOAT, lower = 0.0, upper = 1.0)
    self.assertEqual( i4, i3 )
    i1 = ccs.ccs_interval(t = ccs.NUM_FLOAT, lower = -1.0, upper = 0.0)
    i2 = ccs.ccs_interval(t = ccs.NUM_FLOAT, lower = 0.0, upper = 2.0)
    i3 = i1.intersect(i2)
    self.assertTrue( i3.empty )

  def test_include(self):
    i = ccs.ccs_interval(t = ccs.NUM_FLOAT, lower = -1.0, upper = 1.0)
    self.assertTrue( i.include(0.0) )
    self.assertFalse( i.include(2.0) )
    i = ccs.ccs_interval(t = ccs.NUM_INTEGER, lower = -5, upper = 5)
    self.assertTrue( i.include(0) )
    self.assertFalse( i.include(6) )

if __name__ == '__main__':
    unittest.main()
