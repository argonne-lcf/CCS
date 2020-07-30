import unittest
import sys
sys.path.insert(1, '.')
sys.path.insert(1, '..')
import cconfigspace as ccs
from math import sin

class TestTuner(unittest.TestCase):
  def create_tuning_problem(self):
    cs = ccs.ConfigurationSpace(name = "cspace")
    h1 = ccs.NumericalHyperparameter(lower = -5.0, upper = 5.0)
    h2 = ccs.NumericalHyperparameter(lower = -5.0, upper = 5.0)
    h3 = ccs.NumericalHyperparameter(lower = -5.0, upper = 5.0)
    cs.add_hyperparameters([h1, h2, h3])
    os = ccs.ObjectiveSpace(name = "ospace")
    v1 = ccs.NumericalHyperparameter(lower = float('-inf'), upper = float('inf'))
    v2 = ccs.NumericalHyperparameter(lower = float('-inf'), upper = float('inf'))
    os.add_hyperparameters([v1, v2])
    e1 = ccs.Variable(hyperparameter = v1)
    e2 = ccs.Variable(hyperparameter = v2)
    os.add_objectives( [e1, e2] )
    return (cs, os)

  def test_create_random(self):
    (cs, os) = self.create_tuning_problem()
    t = ccs.RandomTuner(name = "tuner", configuration_space = cs, objective_space = os)
    self.assertEqual("tuner", t.name)
    self.assertEqual(ccs.TUNER_RANDOM, t.type.value)
    func = lambda x, y, z: [(x-2)*(x-2), sin(z+y)]
    evals = [ccs.Evaluation(objective_space = os, configuration = c, values = func(*(c.values))) for c in t.ask(100)]
    t.tell(evals)
    hist = t.history
    self.assertEqual(100, len(hist))
    evals = [ccs.Evaluation(objective_space = os, configuration = c, values = func(*(c.values))) for c in t.ask(100)]
    t.tell(evals)
    hist = t.history
    self.assertEqual(200, len(hist))
    optims = t.optimums
    objs = [x.objective_values for x in optims]
    objs.sort(key = lambda x: x[0])
    self.assertTrue(all(objs[i] <= objs[i+1] for i in range(len(objs)-1)))

if __name__ == '__main__':
    unittest.main()

