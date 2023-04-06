import unittest
import re
import sys
sys.path.insert(1, '.')
sys.path.insert(1, '..')
import cconfigspace as ccs

class TestFeaturesSpace(unittest.TestCase):

  def test_create(self):
    cs = ccs.FeaturesSpace(name = "space")
    self.assertEqual( ccs.FEATURES_SPACE, cs.object_type )
    self.assertEqual( "space", cs.name )
    self.assertEqual( 0, cs.num_parameters )
    h1 = ccs.NumericalParameter()
    h2 = ccs.NumericalParameter()
    h3 = ccs.NumericalParameter()
    cs.add_parameter(h1)
    cs.add_parameters([h2, h3])
    self.assertEqual( 3, cs.num_parameters )
    self.assertEqual( h1, cs.parameter(0) )
    self.assertEqual( h2, cs.parameter(1) )
    self.assertEqual( h3, cs.parameter(2) )
    self.assertEqual( [h1, h2, h3], cs.parameters )
    self.assertEqual( h2, cs.parameter_by_name(h2.name) )

if __name__ == '__main__':
    unittest.main()
