import unittest
import sys
sys.path.insert(1, '.')
sys.path.insert(1, '..')
import cconfigspace as ccs

class TestObjectiveSpace(unittest.TestCase):

  def test_create(self):
    os = ccs.ObjectiveSpace(name = "space")
    self.assertEqual( "space", os.name )
    self.assertEqual( 0, os.num_parameters )
    self.assertEqual( [], os.objectives )
    h1 = ccs.NumericalParameter()
    h2 = ccs.NumericalParameter()
    h3 = ccs.NumericalParameter()
    os.add_parameter(h1)
    os.add_parameters([h2, h3])
    self.assertEqual( 3, os.num_parameters )
    self.assertEqual( h1, os.parameter(0) )
    self.assertEqual( h2, os.parameter(1) )
    self.assertEqual( h3, os.parameter(2) )
    self.assertEqual( [h1, h2, h3], os.parameters )
    self.assertEqual( h2, os.parameter_by_name(h2.name) )
    e1 = ccs.Expression(t = ccs.ccs_expression_type.ADD, nodes = [h1, h2])
    e2 = ccs.Variable(parameter = h3)
    os.add_objective(e1)
    self.assertEqual( 1, len(os.objectives) )
    os.add_objectives([e2], types = [ccs.ccs_objective_type.MAXIMIZE])
    self.assertEqual( 2, len(os.objectives) )
    objs = os.objectives
    self.assertEqual( e1.handle.value, objs[0][0].handle.value )
    self.assertEqual( ccs.ccs_objective_type.MINIMIZE, objs[0][1] )
    self.assertEqual( e2.handle.value, objs[1][0].handle.value )
    self.assertEqual( ccs.ccs_objective_type.MAXIMIZE, objs[1][1] )
 

if __name__ == '__main__':
    unittest.main()
