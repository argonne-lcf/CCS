import unittest
import cconfigspace as ccs
import ctypes as ct
from math import sin

class TestTuner(ccs.UserDefinedTuner):
  def __init__(self, os):
    data = [[], []]

    def delete(tuner):
      return None

    def ask(tuner, count):
      if count is None:
        return (None, 1)
      else:
        cs = tuner.configuration_space
        return (cs.samples(count), count)

    def tell(tuner, evaluations):
      history_optima = tuner.tuner_data
      history_optima[0] += evaluations
      for e in evaluations:
        discard = False
        new_optima = []
        for o in history_optima[1]:
          if discard:
            new_optima.append(o)
          else:
            c = e.compare(o)
            if c == ccs.Comparison.EQUIVALENT or c == ccs.Comparison.WORSE:
              discard = True
              new_optima.append(o)
            elif c == ccs.Comparison.NOT_COMPARABLE:
              new_optima.append(o)
        if not discard:
          new_optima.append(e)
        history_optima[1] = new_optima
      return None

    def get_history(tuner):
      return tuner.tuner_data[0]

    def get_optima(tuner):
      return tuner.tuner_data[1]

    super().__init__(name = "tuner", objective_space = os, delete = delete, ask = ask, tell = tell, get_optima = get_optima, get_history = get_history, tuner_data = data)


def create_test_tuner(os_ptr):
  os_handle = ccs.ccs_objective_space(os_ptr)
  os = ccs.ObjectiveSpace.from_handle(os_handle)
  t = TestTuner(os)
  return t

def create_tuning_problem():
  h1 = ccs.NumericalParameter.Float(lower = -5.0, upper = 5.0)
  h2 = ccs.NumericalParameter.Float(lower = -5.0, upper = 5.0)
  h3 = ccs.NumericalParameter.Float(lower = -5.0, upper = 5.0)
  cs = ccs.ConfigurationSpace(name = "cspace", parameters = [h1, h2, h3])
  v1 = ccs.NumericalParameter.Float(lower = float('-inf'), upper = float('inf'))
  v2 = ccs.NumericalParameter.Float(lower = float('-inf'), upper = float('inf'))
  e1 = ccs.Expression.Variable(parameter = v1)
  e2 = ccs.Expression.Variable(parameter = v2)
  os = ccs.ObjectiveSpace(name = "ospace", search_space = cs, parameters = [v1, v2], objectives = [e1, e2])
  return os


class Test(unittest.TestCase):
  def test_user_defined(self):
    os = create_tuning_problem()
    t = TestTuner(os)
    self.assertEqual("tuner", t.name)
    self.assertEqual(ccs.TunerType.USER_DEFINED, t.type)
    self.assertEqual(os.handle.value, t.objective_space.handle.value)
    self.assertEqual(os.search_space.handle.value, t.configuration_space.handle.value)
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

