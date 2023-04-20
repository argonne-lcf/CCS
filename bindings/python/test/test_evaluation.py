import unittest
import sys
sys.path.insert(1, '.')
sys.path.insert(1, '..')
import cconfigspace as ccs

class TestEvaluation(unittest.TestCase):

  def test_create(self):
    cs = ccs.ConfigurationSpace(name = "cspace")
    h1 = ccs.NumericalParameter()
    h2 = ccs.NumericalParameter()
    h3 = ccs.NumericalParameter()
    cs.add_parameters([h1, h2, h3])
    os = ccs.ObjectiveSpace(name = "ospace")
    v1 = ccs.NumericalParameter()
    v2 = ccs.NumericalParameter()
    os.add_parameters([v1, v2])
    e1 = ccs.Variable(parameter = v1)
    e2 = ccs.Variable(parameter = v2)
    os.add_objectives( { e1: ccs.ccs_objective_type.MAXIMIZE, e2: ccs.ccs_objective_type.MINIMIZE } )
    ev1 = ccs.Evaluation(objective_space = os, configuration = cs.sample())
    ev1.set_value(0, 0.5)
    ev1.set_value(v2, 0.6)
    self.assertEqual( [0.5, 0.6], ev1.values )
    self.assertEqual( [0.5, 0.6], ev1.objective_values )
    self.assertTrue( ev1.check )
    self.assertTrue( os.check_values(ev1.values) )
    ev2 = ccs.Evaluation(objective_space = os, configuration = cs.sample(), values = [0.5, 0.6])
    self.assertEqual( [0.5, 0.6], ev2.values )
    self.assertEqual( [0.5, 0.6], ev2.objective_values )
    self.assertEqual( ccs.ccs_comparison.EQUIVALENT, ev1.compare(ev2) )
    ev3 = ccs.Evaluation(objective_space = os, configuration = cs.sample(), values = [0.6, 0.5])
    self.assertEqual( [0.6, 0.5], ev3.objective_values )
    self.assertEqual( ccs.ccs_comparison.WORSE, ev1.compare(ev3) )
    self.assertEqual( ccs.ccs_comparison.BETTER, ev3.compare(ev1) )
    ev4 = ccs.Evaluation(objective_space = os, configuration = cs.sample(), values = [0.6, 0.7])
    self.assertEqual( [0.6, 0.7], ev4.objective_values )
    self.assertEqual( ccs.ccs_comparison.NOT_COMPARABLE, ev1.compare(ev4) )
    self.assertEqual( ccs.ccs_comparison.NOT_COMPARABLE, ev4.compare(ev1) )

  def test_serialize(self):
    cs = ccs.ConfigurationSpace(name = "cspace")
    h1 = ccs.NumericalParameter()
    h2 = ccs.NumericalParameter()
    h3 = ccs.NumericalParameter()
    cs.add_parameters([h1, h2, h3])
    os = ccs.ObjectiveSpace(name = "ospace")
    v1 = ccs.NumericalParameter()
    v2 = ccs.NumericalParameter()
    os.add_parameters([v1, v2])
    e1 = ccs.Variable(parameter = v1)
    e2 = ccs.Variable(parameter = v2)
    os.add_objectives( { e1: ccs.ccs_objective_type.MAXIMIZE, e2: ccs.ccs_objective_type.MINIMIZE } )
    evref = ccs.Evaluation(objective_space = os, configuration = cs.sample(), values = [0.5, 0.6])
    buff = evref.serialize()
    handle_map = ccs.Map()
    handle_map[cs] = cs
    handle_map[os] = os
    ev = ccs.deserialize(buffer = buff, handle_map = handle_map)
    self.assertEqual( cs.handle.value, ev.configuration.configuration_space.handle.value)
    self.assertEqual( os.handle.value, ev.objective_space.handle.value)
    self.assertEqual( [0.5, 0.6], ev.values )
    self.assertEqual( [0.5, 0.6], ev.objective_values )

if __name__ == '__main__':
    unittest.main()
