import unittest
import sys
sys.path.insert(1, '../bindings/python')
sys.path.insert(1, '../../bindings/python')
import cconfigspace as ccs
import ctypes as ct
from math import sin

class TestTuner(ccs.UserDefinedTuner):
  def __init__(self, cs, os):
    self.__history = []
    self.__optimums = []

    def delete(data):
      return None

    def ask(data, count):
      if count is None:
        return (None, 1)
      else:
        cs = ccs.ConfigurationSpace.from_handle(ccs.ccs_configuration_space(data.common_data.configuration_space))
        return (cs.samples(count), count)

    def tell(data, evaluations):
      self.__history += evaluations
      for e in evaluations:
        discard = False
        new_optimums = []
        for o in self.__optimums:
          if discard:
            new_optimums.append(o)
          else:
            c = e.cmp(o)
            if c == ccs.EQUIVALENT or c == ccs.WORSE:
              discard = True
              new_optimums.append(o)
            elif c == ccs.NOT_COMPARABLE:
              new_optimums.append(o)
        if not discard:
          new_optimums.append(e)
        self.__optimums = new_optimums
      return None

    def get_history(data):
      return self.__history

    def get_optimums(data):
      return self.__optimums

    super().__init__(name = "tuner", configuration_space = cs, objective_space = os, delete = delete, ask = ask, tell = tell, get_optimums = get_optimums, get_history = get_history)


def create_test_tuner(cs_ptr, os_ptr):
  cs_handle = ccs.ccs_configuration_space(cs_ptr)
  os_handle = ccs.ccs_objective_space(os_ptr)
  cs = ccs.ConfigurationSpace.from_handle(cs_handle)
  os = ccs.ObjectiveSpace.from_handle(os_handle)
  t = TestTuner(cs, os)
  return t

def create_tuning_problem():
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


class Test(unittest.TestCase):
  def test_user_defined(self):
    (cs, os) = create_tuning_problem()
    t = TestTuner(cs, os)
    self.assertEqual("tuner", t.name)
    self.assertEqual(ccs.TUNER_USER_DEFINED, t.type.value)
    self.assertEqual(cs.handle.value, t.configuration_space.handle.value)
    self.assertEqual(os.handle.value, t.objective_space.handle.value)
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
    # assert pareto front
    self.assertTrue(all(objs[i][1] >= objs[i+1][1] for i in range(len(objs)-1)))


if __name__ == '__main__':
    unittest.main()

