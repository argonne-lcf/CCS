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
    self.assertEqual( ccs.ccs_data_type.NONE, d.type )
    self.assertIsNone( d.value )
    d.value = ccs.ccs_inactive
    self.assertEqual( ccs.ccs_data_type.INACTIVE, d.type )
    self.assertEqual( ccs.ccs_inactive, d.value )
    d.value = False
    self.assertEqual( ccs.ccs_data_type.BOOL, d.type )
    self.assertFalse( d.value )
    d.value = True
    self.assertEqual( ccs.ccs_data_type.BOOL, d.type )
    self.assertTrue( d.value )
    d.value = 1.5
    self.assertEqual( ccs.ccs_data_type.FLOAT, d.type )
    self.assertEqual( 1.5, d.value )
    d.value = 2
    self.assertEqual( ccs.ccs_data_type.INT, d.type )
    self.assertEqual( 2, d.value )
    d.value = "foo"
    self.assertEqual( ccs.ccs_data_type.STRING, d.type )
    self.assertEqual( "foo", d.value )
    d.value = ccs.Rng()
    self.assertEqual( ccs.ccs_data_type.OBJECT, d.type )
    self.assertIsInstance( d.value, ccs.Rng )
    self.assertEqual( ccs.ccs_object_type.RNG, d.value.object_type )

  def test_numeric(self):
    n = ccs.ccs_numeric()
    n.set_value(2)
    self.assertEqual( 2, n.get_value(ccs.ccs_numeric_type.INT))
    n.set_value(1.5)
    self.assertEqual( 1.5, n.get_value(ccs.ccs_numeric_type.FLOAT))

if __name__ == '__main__':
    unittest.main()
