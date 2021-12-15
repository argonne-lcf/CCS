import unittest
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
    ev1.check
    os.check(ev1)
    os.check_values(ev1.values)
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

if __name__ == '__main__':
    unittest.main()
