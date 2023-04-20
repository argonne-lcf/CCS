import unittest
import sys
sys.path.insert(1, '.')
sys.path.insert(1, '..')
import cconfigspace as ccs
from math import sin


class TestBase(unittest.TestCase):

  def test_version(self):
    ver = ccs.ccs_get_version()
    self.assertIsInstance(ver, ccs.Version)

  def test_datum_value(self):
    d = ccs.Datum()
    d.value = None
    self.assertEqual( ccs.DataType.NONE, d.type )
    self.assertIsNone( d.value )
    d.value = ccs.inactive
    self.assertEqual( ccs.DataType.INACTIVE, d.type )
    self.assertEqual( ccs.inactive, d.value )
    d.value = False
    self.assertEqual( ccs.DataType.BOOL, d.type )
    self.assertFalse( d.value )
    d.value = True
    self.assertEqual( ccs.DataType.BOOL, d.type )
    self.assertTrue( d.value )
    d.value = 1.5
    self.assertEqual( ccs.DataType.FLOAT, d.type )
    self.assertEqual( 1.5, d.value )
    d.value = 2
    self.assertEqual( ccs.DataType.INT, d.type )
    self.assertEqual( 2, d.value )
    d.value = "foo"
    self.assertEqual( ccs.DataType.STRING, d.type )
    self.assertEqual( "foo", d.value )
    d.value = ccs.Rng()
    self.assertEqual( ccs.DataType.OBJECT, d.type )
    self.assertIsInstance( d.value, ccs.Rng )
    self.assertEqual( ccs.ObjectType.RNG, d.value.object_type )

  def test_numeric(self):
    n = ccs.Numeric()
    n.set_value(2)
    self.assertEqual( 2, n.get_value(ccs.NumericType.INT))
    n.set_value(1.5)
    self.assertEqual( 1.5, n.get_value(ccs.NumericType.FLOAT))

if __name__ == '__main__':
    unittest.main()
