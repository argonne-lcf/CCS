import unittest
import cconfigspace as ccs

class TestFeaturesSpace(unittest.TestCase):

  def test_create(self):
    cs = ccs.FeaturesSpace(name = "space")
    self.assertEqual( ccs.FEATURES_SPACE, cs.object_type )
    self.assertEqual( "space", cs.name )
    self.assertEqual( 0, cs.num_hyperparameters )
    h1 = ccs.NumericalHyperparameter()
    h2 = ccs.NumericalHyperparameter()
    h3 = ccs.NumericalHyperparameter()
    cs.add_hyperparameter(h1)
    cs.add_hyperparameters([h2, h3])
    self.assertEqual( 3, cs.num_hyperparameters )
    self.assertEqual( h1, cs.hyperparameter(0) )
    self.assertEqual( h2, cs.hyperparameter(1) )
    self.assertEqual( h3, cs.hyperparameter(2) )
    self.assertEqual( [h1, h2, h3], cs.hyperparameters )
    self.assertEqual( h2, cs.hyperparameter_by_name(h2.name) )

if __name__ == '__main__':
    unittest.main()
