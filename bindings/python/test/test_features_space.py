import unittest
import re
import sys
sys.path.insert(1, '.')
sys.path.insert(1, '..')
import cconfigspace as ccs

class TestFeaturesSpace(unittest.TestCase):

  def test_create(self):
    h1 = ccs.NumericalParameter.Float(lower = 0.0, upper = 1.0)
    h2 = ccs.NumericalParameter.Float(lower = 0.0, upper = 1.0)
    h3 = ccs.NumericalParameter.Float(lower = 0.0, upper = 1.0)
    cs = ccs.FeaturesSpace(name = "space", parameters = [h1, h2, h3])
    self.assertEqual( ccs.ObjectType.FEATURES_SPACE, cs.object_type )
    self.assertEqual( "space", cs.name )
    self.assertEqual( 3, cs.num_parameters )
    self.assertEqual( h1, cs.parameter(0) )
    self.assertEqual( h2, cs.parameter(1) )
    self.assertEqual( h3, cs.parameter(2) )
    self.assertEqual( (h1, h2, h3), cs.parameters )
    self.assertEqual( h2, cs.parameter_by_name(h2.name) )

if __name__ == '__main__':
    unittest.main()
