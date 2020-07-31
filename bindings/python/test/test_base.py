import unittest
import sys
sys.path.insert(1, '.')
sys.path.insert(1, '..')
import cconfigspace as ccs
from math import sin


class TestBase(unittest.TestCase):

  def test_version(self):
    ver = ccs.ccs_get_version()
    self.assertIsInstance(ver, ccs.ccs_version)

  def test_datum_value(self):
    d = ccs.ccs_datum()
    d.value = None
    self.assertEqual( ccs.NONE, d.type )
    self.assertIsNone( d.value )
    d.value = ccs.ccs_inactive
    self.assertEqual( ccs.INACTIVE, d.type )
    self.assertEqual( ccs.ccs_inactive, d.value )
    d.value = False
    self.assertEqual( ccs.BOOLEAN, d.type )
    self.assertFalse( d.value )
    d.value = True
    self.assertEqual( ccs.BOOLEAN, d.type )
    self.assertTrue( d.value )
    d.value = 1.5
    self.assertEqual( ccs.FLOAT, d.type )
    self.assertEqual( 1.5, d.value )
    d.value = 2
    self.assertEqual( ccs.INTEGER, d.type )
    self.assertEqual( 2, d.value )
    d.value = "foo"
    self.assertEqual( ccs.STRING, d.type )
    self.assertEqual( "foo", d.value )
    d.value = ccs.Rng()
    self.assertEqual( ccs.OBJECT, d.type )
    self.assertIsInstance( d.value, ccs.Rng )
    self.assertEqual( ccs.RNG, d.value.object_type )

  def test_numeric(self):
    n = ccs.ccs_numeric()
    n.set_value(2)
    self.assertEqual( 2, n.get_value(ccs.NUM_INTEGER))
    n.set_value(1.5)
    self.assertEqual( 1.5, n.get_value(ccs.NUM_FLOAT))

if __name__ == '__main__':
    unittest.main()
