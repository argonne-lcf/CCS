import unittest
import sys
sys.path.insert(1, '.')
sys.path.insert(1, '..')
import cconfigspace as ccs

class TestEvaluation(unittest.TestCase):

  def test_create(self):
    cs = ccs.ConfigurationSpace(name = "cspace")
    h1 = ccs.NumericalHyperparameter()
    h2 = ccs.NumericalHyperparameter()
    h3 = ccs.NumericalHyperparameter()
    cs.add_hyperparameters([h1, h2, h3])
    os = ccs.ObjectiveSpace(name = "ospace")
    v1 = ccs.NumericalHyperparameter()
    v2 = ccs.NumericalHyperparameter()
    os.add_hyperparameters([v1, v2])
    e1 = ccs.Variable(hyperparameter = v1)
    e2 = ccs.Variable(hyperparameter = v2)
    os.add_objectives( { e1: ccs.MAXIMIZE, e2: ccs.MINIMIZE } )
    ev1 = ccs.Evaluation(objective_space = os, configuration = cs.sample())
    ev1.set_value(0, 0.5)
    ev1.set_value(v2, 0.6)
    self.assertEqual( [0.5, 0.6], ev1.values )
    self.assertEqual( [0.5, 0.6], ev1.objective_values )
    self.assertTrue( ev1.check )
    self.assertTrue( os.check(ev1) )
    self.assertTrue( os.check_values(ev1.values) )
    ev2 = ccs.Evaluation(objective_space = os, configuration = cs.sample(), values = [0.5, 0.6])
    self.assertEqual( [0.5, 0.6], ev2.values )
    self.assertEqual( [0.5, 0.6], ev2.objective_values )
    self.assertEqual( ccs.EQUIVALENT, ev1.compare(ev2) )
    ev3 = ccs.Evaluation(objective_space = os, configuration = cs.sample(), values = [0.6, 0.5])
    self.assertEqual( [0.6, 0.5], ev3.objective_values )
    self.assertEqual( ccs.WORSE, ev1.compare(ev3) )
    self.assertEqual( ccs.BETTER, ev3.compare(ev1) )
    ev4 = ccs.Evaluation(objective_space = os, configuration = cs.sample(), values = [0.6, 0.7])
    self.assertEqual( [0.6, 0.7], ev4.objective_values )
    self.assertEqual( ccs.NOT_COMPARABLE, ev1.compare(ev4) )
    self.assertEqual( ccs.NOT_COMPARABLE, ev4.compare(ev1) )

  def test_serialize(self):
    cs = ccs.ConfigurationSpace(name = "cspace")
    h1 = ccs.NumericalHyperparameter()
    h2 = ccs.NumericalHyperparameter()
    h3 = ccs.NumericalHyperparameter()
    cs.add_hyperparameters([h1, h2, h3])
    os = ccs.ObjectiveSpace(name = "ospace")
    v1 = ccs.NumericalHyperparameter()
    v2 = ccs.NumericalHyperparameter()
    os.add_hyperparameters([v1, v2])
    e1 = ccs.Variable(hyperparameter = v1)
    e2 = ccs.Variable(hyperparameter = v2)
    os.add_objectives( { e1: ccs.MAXIMIZE, e2: ccs.MINIMIZE } )
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
