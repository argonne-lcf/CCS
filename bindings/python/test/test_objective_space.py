import unittest
import cconfigspace as ccs

class TestObjectiveSpace(unittest.TestCase):

  def test_create(self):
    os = ccs.ObjectiveSpace(name = "space")
    self.assertEqual( "space", os.name )
    self.assertEqual( 0, os.num_hyperparameters )
    self.assertEqual( [], os.objectives )
    h1 = ccs.NumericalHyperparameter()
    h2 = ccs.NumericalHyperparameter()
    h3 = ccs.NumericalHyperparameter()
    os.add_hyperparameter(h1)
    os.add_hyperparameters([h2, h3])
    self.assertEqual( 3, os.num_hyperparameters )
    self.assertEqual( h1, os.hyperparameter(0) )
    self.assertEqual( h2, os.hyperparameter(1) )
    self.assertEqual( h3, os.hyperparameter(2) )
    self.assertEqual( [h1, h2, h3], os.hyperparameters )
    self.assertEqual( h2, os.hyperparameter_by_name(h2.name) )
    e1 = ccs.Expression(t = ccs.ADD, nodes = [h1, h2])
    e2 = ccs.Variable(hyperparameter = h3)
    os.add_objective(e1)
    self.assertEqual( 1, len(os.objectives) )
    os.add_objectives([e2], types = [ccs.MAXIMIZE])
    self.assertEqual( 2, len(os.objectives) )
    objs = os.objectives
    self.assertEqual( e1.handle.value, objs[0][0].handle.value )
    self.assertEqual( ccs.MINIMIZE, objs[0][1] )
    self.assertEqual( e2.handle.value, objs[1][0].handle.value )
    self.assertEqual( ccs.MAXIMIZE, objs[1][1] )
 

if __name__ == '__main__':
    unittest.main()
