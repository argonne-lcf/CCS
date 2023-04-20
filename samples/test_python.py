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
    self.__optima = []

    def delete(tuner):
      return None

    def ask(tuner, count):
      if count is None:
        return (None, 1)
      else:
        cs = tuner.configuration_space
        return (cs.samples(count), count)

    def tell(tuner, evaluations):
      self.__history += evaluations
      for e in evaluations:
        discard = False
        new_optima = []
        for o in self.__optima:
          if discard:
            new_optima.append(o)
          else:
            c = e.compare(o)
            if c == ccs.ccs_comparison.EQUIVALENT or c == ccs.ccs_comparison.WORSE:
              discard = True
              new_optima.append(o)
            elif c == ccs.ccs_comparison.NOT_COMPARABLE:
              new_optima.append(o)
        if not discard:
          new_optima.append(e)
        self.__optima = new_optima
      return None

    def get_history(tuner):
      return self.__history

    def get_optima(tuner):
      return self.__optima

    super().__init__(name = "tuner", configuration_space = cs, objective_space = os, delete = delete, ask = ask, tell = tell, get_optima = get_optima, get_history = get_history)


def create_test_tuner(cs_ptr, os_ptr):
  cs_handle = ccs.ccs_configuration_space(cs_ptr)
  os_handle = ccs.ccs_objective_space(os_ptr)
  cs = ccs.ConfigurationSpace.from_handle(cs_handle)
  os = ccs.ObjectiveSpace.from_handle(os_handle)
  t = TestTuner(cs, os)
  return t

def create_tuning_problem():
  cs = ccs.ConfigurationSpace(name = "cspace")
  h1 = ccs.NumericalParameter(lower = -5.0, upper = 5.0)
  h2 = ccs.NumericalParameter(lower = -5.0, upper = 5.0)
  h3 = ccs.NumericalParameter(lower = -5.0, upper = 5.0)
  cs.add_parameters([h1, h2, h3])
  os = ccs.ObjectiveSpace(name = "ospace")
  v1 = ccs.NumericalParameter(lower = float('-inf'), upper = float('inf'))
  v2 = ccs.NumericalParameter(lower = float('-inf'), upper = float('inf'))
  os.add_parameters([v1, v2])
  e1 = ccs.Variable(parameter = v1)
  e2 = ccs.Variable(parameter = v2)
  os.add_objectives( [e1, e2] )
  return (cs, os)


class Test(unittest.TestCase):
  def test_user_defined(self):
    (cs, os) = create_tuning_problem()
    t = TestTuner(cs, os)
    self.assertEqual("tuner", t.name)
    self.assertEqual(ccs.ccs_tuner_type.USER_DEFINED, t.type)
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
    optims = t.optima
    objs = [x.objective_values for x in optims]
    objs.sort(key = lambda x: x[0])
    # assert pareto front
    self.assertTrue(all(objs[i][1] >= objs[i+1][1] for i in range(len(objs)-1)))


if __name__ == '__main__':
    unittest.main()

