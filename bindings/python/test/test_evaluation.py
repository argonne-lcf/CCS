import unittest
import sys
sys.path.insert(1, '.')
sys.path.insert(1, '..')
import cconfigspace as ccs

class TestEvaluation(unittest.TestCase):

  def test_create(self):
    h1 = ccs.NumericalParameter.Float()
    h2 = ccs.NumericalParameter.Float()
    h3 = ccs.NumericalParameter.Float()
    cs = ccs.ConfigurationSpace(name = "cspace", parameters = [h1, h2, h3])
    v1 = ccs.NumericalParameter.Float()
    v2 = ccs.NumericalParameter.Float()
    e1 = ccs.Expression.Variable(parameter = v1)
    e2 = ccs.Expression.Variable(parameter = v2)
    os = ccs.ObjectiveSpace(name = "ospace", search_space = cs, parameters = [v1, v2], objectives = { e1: ccs.ObjectiveType.MAXIMIZE, e2: ccs.ObjectiveType.MINIMIZE })
    ev1 = ccs.Evaluation(objective_space = os, configuration = cs.sample(), values = [0.5, 0.6])
    self.assertEqual( (0.5, 0.6), ev1.values )
    self.assertEqual( (0.5, 0.6), ev1.objective_values )
    self.assertTrue( ev1.check )
    self.assertTrue( os.check(ev1) )
    ev2 = ccs.Evaluation(objective_space = os, configuration = cs.sample(), values = [0.5, 0.6])
    self.assertEqual( (0.5, 0.6), ev2.values )
    self.assertEqual( (0.5, 0.6), ev2.objective_values )
    self.assertEqual( ccs.Comparison.EQUIVALENT, ev1.compare(ev2) )
    ev3 = ccs.Evaluation(objective_space = os, configuration = cs.sample(), values = [0.6, 0.5])
    self.assertEqual( (0.6, 0.5), ev3.objective_values )
    self.assertEqual( ccs.Comparison.WORSE, ev1.compare(ev3) )
    self.assertEqual( ccs.Comparison.BETTER, ev3.compare(ev1) )
    ev4 = ccs.Evaluation(objective_space = os, configuration = cs.sample(), values = [0.6, 0.7])
    self.assertEqual( (0.6, 0.7), ev4.objective_values )
    self.assertEqual( ccs.Comparison.NOT_COMPARABLE, ev1.compare(ev4) )
    self.assertEqual( ccs.Comparison.NOT_COMPARABLE, ev4.compare(ev1) )

  def test_serialize(self):
    h1 = ccs.NumericalParameter.Float()
    h2 = ccs.NumericalParameter.Float()
    h3 = ccs.NumericalParameter.Float()
    cs = ccs.ConfigurationSpace(name = "cspace", parameters = [h1, h2, h3])
    v1 = ccs.NumericalParameter.Float()
    v2 = ccs.NumericalParameter.Float()
    e1 = ccs.Expression.Variable(parameter = v1)
    e2 = ccs.Expression.Variable(parameter = v2)
    os = ccs.ObjectiveSpace(name = "ospace", search_space = cs, parameters = [v1, v2], objectives = { e1: ccs.ObjectiveType.MAXIMIZE, e2: ccs.ObjectiveType.MINIMIZE })
    evref = ccs.Evaluation(objective_space = os, configuration = cs.sample(), values = [0.5, 0.6])
    buff = evref.serialize()
    handle_map = ccs.Map()
    handle_map[cs] = cs
    handle_map[os] = os
    ev = ccs.deserialize(buffer = buff, handle_map = handle_map)
    self.assertEqual( cs.handle.value, ev.configuration.configuration_space.handle.value)
    self.assertEqual( os.handle.value, ev.objective_space.handle.value)
    self.assertEqual( (0.5, 0.6), ev.values )
    self.assertEqual( (0.5, 0.6), ev.objective_values )

if __name__ == '__main__':
    unittest.main()
