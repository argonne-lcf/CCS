import unittest
import sys
sys.path.insert(1, '.')
sys.path.insert(1, '..')
import cconfigspace as ccs

class TestObjectiveSpace(unittest.TestCase):

  def test_create(self):
    h = ccs.NumericalParameter.Float()
    cs = ccs.ConfigurationSpace(name = "cs", parameters = [h])
    h1 = ccs.NumericalParameter.Float()
    h2 = ccs.NumericalParameter.Float()
    h3 = ccs.NumericalParameter.Float()
    e1 = ccs.Expression.Add(left = h1, right = h2)
    e2 = ccs.Expression.Variable(parameter = h3)
    os = ccs.ObjectiveSpace(name = "space", search_space = cs, parameters = [h1, h2, h3], objectives = [e1, e2], types = [ccs.ObjectiveType.MINIMIZE, ccs.ObjectiveType.MAXIMIZE])
    self.assertEqual( "space", os.name )
    self.assertEqual( cs.handle.value, os.search_space.handle.value )
    self.assertEqual( 3, os.num_parameters )
    self.assertEqual( h1, os.parameter(0) )
    self.assertEqual( h2, os.parameter(1) )
    self.assertEqual( h3, os.parameter(2) )
    self.assertEqual( (h1, h2, h3), os.parameters )
    self.assertEqual( h2, os.parameter_by_name(h2.name) )
    self.assertEqual( 2, len(os.objectives) )
    objs = os.objectives
    self.assertEqual( e1.handle.value, objs[0][0].handle.value )
    self.assertEqual( ccs.ObjectiveType.MINIMIZE, objs[0][1] )
    self.assertEqual( e2.handle.value, objs[1][0].handle.value )
    self.assertEqual( ccs.ObjectiveType.MAXIMIZE, objs[1][1] )
 

if __name__ == '__main__':
    unittest.main()
