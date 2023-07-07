import unittest
import sys
import os as _os
from random import choice
sys.path.insert(1, '.')
sys.path.insert(1, '..')
import cconfigspace as ccs
from math import sin

class TestTuner(unittest.TestCase):
  def create_tuning_problem(self):
    h1 = ccs.NumericalParameter.Float(lower = -5.0, upper = 5.0)
    h2 = ccs.NumericalParameter.Float(lower = -5.0, upper = 5.0)
    h3 = ccs.NumericalParameter.Float(lower = -5.0, upper = 5.0)
    cs = ccs.ConfigurationSpace(name = "cspace", parameters = [h1, h2, h3])
    v1 = ccs.NumericalParameter.Float(lower = float('-inf'), upper = float('inf'))
    v2 = ccs.NumericalParameter.Float(lower = float('-inf'), upper = float('inf'))
    e1 = ccs.Expression.Variable(parameter = v1)
    e2 = ccs.Expression.Variable(parameter = v2)
    os = ccs.ObjectiveSpace(name = "ospace", parameters = [v1, v2], objectives = [e1, e2])
    return (cs, os)

  def test_create_random(self):
    (cs, os) = self.create_tuning_problem()
    t = ccs.RandomTuner(name = "tuner", configuration_space = cs, objective_space = os)
    t2 = ccs.Object.from_handle(t.handle)
    self.assertEqual("tuner", t.name)
    self.assertEqual(ccs.TunerType.RANDOM, t.type)
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
    self.assertTrue(t.suggest in [x.configuration for x in optims])
    # test serialization
    buff = t.serialize()
    t_copy = ccs.deserialize(buffer = buff)
    hist = t_copy.history
    self.assertEqual(200, len(hist))
    optims_2 = t_copy.optima
    self.assertEqual(len(optims), len(optims_2))
    objs = [x.objective_values for x in optims_2]
    objs.sort(key = lambda x: x[0])
    self.assertTrue(all(objs[i][1] >= objs[i+1][1] for i in range(len(objs)-1)))
    self.assertTrue(t_copy.suggest in [x.configuration for x in optims_2])

  def test_user_defined(self):
    class TunerData:
      def __init__(self):
        self.history = []
        self.optima = []

    def delete(tuner):
      return None

    def ask(tuner, count):
      if count is None:
        return (None, 1)
      else:
        cs = tuner.configuration_space
        return (cs.samples(count), count)

    def tell(tuner, evaluations):
      tuner.tuner_data.history += evaluations
      for e in evaluations:
        discard = False
        new_optima = []
        for o in tuner.tuner_data.optima:
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
        tuner.tuner_data.optima = new_optima
      return None

    def get_history(tuner):
      return tuner.tuner_data.history

    def get_optima(tuner):
      return tuner.tuner_data.optima

    def suggest(tuner):
      if not tuner.tuner_data.optima:
        return ask(tuner, 1)
      else:
        return choice(tuner.tuner_data.optima).configuration

    (cs, os) = self.create_tuning_problem()
    t = ccs.UserDefinedTuner(name = "tuner", configuration_space = cs, objective_space = os, delete = delete, ask = ask, tell = tell, get_optima = get_optima, get_history = get_history, suggest = suggest, tuner_data = TunerData())
    t2 = ccs.Object.from_handle(t.handle)
    self.assertEqual("tuner", t.name)
    self.assertEqual(ccs.TunerType.USER_DEFINED, t.type)
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
    self.assertTrue(t.suggest in [x.configuration for x in optims])

    # test serialization
    buff = t.serialize()
    t_copy = ccs.UserDefinedTuner.deserialize(buffer = buff, delete = delete, ask = ask, tell = tell, get_optima = get_optima, get_history = get_history, suggest = suggest, tuner_data = TunerData())
    hist = t_copy.history
    self.assertEqual(200, len(hist))
    optims_2 = t_copy.optima
    self.assertEqual(len(optims), len(optims_2))
    objs = [x.objective_values for x in optims_2]
    objs.sort(key = lambda x: x[0])
    self.assertTrue(all(objs[i][1] >= objs[i+1][1] for i in range(len(objs)-1)))
    self.assertTrue(t_copy.suggest in [x.configuration for x in optims_2])

    t.serialize(path = 'tuner.ccs')
    t_copy = ccs.UserDefinedTuner.deserialize(path = 'tuner.ccs', delete = delete, ask = ask, tell = tell, get_optima = get_optima, get_history = get_history, suggest = suggest, tuner_data = TunerData())
    hist = t_copy.history
    self.assertEqual(200, len(hist))
    optims_2 = t_copy.optima
    self.assertEqual(len(optims), len(optims_2))
    objs = [x.objective_values for x in optims_2]
    objs.sort(key = lambda x: x[0])
    self.assertTrue(all(objs[i][1] >= objs[i+1][1] for i in range(len(objs)-1)))
    self.assertTrue(t_copy.suggest in [x.configuration for x in optims_2])
    _os.remove('tuner.ccs')

    file = open( 'tuner.ccs', "wb")
    t.serialize(file_descriptor = file.fileno())
    file.close()
    file = open( 'tuner.ccs', "rb")
    t_copy = ccs.UserDefinedTuner.deserialize(file_descriptor = file.fileno(), delete = delete, ask = ask, tell = tell, get_optima = get_optima, get_history = get_history, suggest = suggest, tuner_data = TunerData())
    file.close()
    hist = t_copy.history
    self.assertEqual(200, len(hist))
    optims_2 = t_copy.optima
    self.assertEqual(len(optims), len(optims_2))
    objs = [x.objective_values for x in optims_2]
    objs.sort(key = lambda x: x[0])
    self.assertTrue(all(objs[i][1] >= objs[i+1][1] for i in range(len(objs)-1)))
    self.assertTrue(t_copy.suggest in [x.configuration for x in optims_2])
    _os.remove('tuner.ccs')


if __name__ == '__main__':
    unittest.main()

