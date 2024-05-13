import unittest
import sys
sys.path.insert(1, '.')
sys.path.insert(1, '..')
import cconfigspace as ccs
from math import sin
from random import choice

class TestTuner(unittest.TestCase):
  def create_tuning_problem(self):
    f1 = ccs.CategoricalParameter(values = [True, False])
    fs = ccs.FeatureSpace(name = "fspace", parameters = [f1])
    h1 = ccs.NumericalParameter.Float(lower = -5.0, upper = 5.0)
    h2 = ccs.NumericalParameter.Float(lower = -5.0, upper = 5.0)
    h3 = ccs.NumericalParameter.Float(lower = -5.0, upper = 5.0)
    cs = ccs.ConfigurationSpace(name = "cspace", feature_space = fs, parameters = [h1, h2, h3])
    v1 = ccs.NumericalParameter.Float(lower = float('-inf'), upper = float('inf'))
    v2 = ccs.NumericalParameter.Float(lower = float('-inf'), upper = float('inf'))
    e1 = ccs.Expression.Variable(parameter = v1)
    e2 = ccs.Expression.Variable(parameter = v2)
    os = ccs.ObjectiveSpace(name = "ospace", search_space = cs, parameters = [v1, v2], objectives = [e1, e2])
    return (fs, os)

  def test_create_random(self):
    (fs, os) = self.create_tuning_problem()
    t = ccs.RandomTuner(name = "tuner", objective_space = os)
    t2 = ccs.Object.from_handle(t.handle)
    self.assertEqual("tuner", t.name)
    self.assertEqual(ccs.TunerType.RANDOM, t.type)
    func = lambda x, y, z: [(x-2)*(x-2), sin(z+y)]
    features_on = ccs.Features(feature_space = fs, values = [True])
    features_off = ccs.Features(feature_space = fs, values = [False])
    evals = [ccs.Evaluation(objective_space = os, configuration = c, values = func(*(c.values))) for c in t.ask(50, features = features_on)]
    t.tell(evals)
    evals = [ccs.Evaluation(objective_space = os, configuration = c, values = func(*(c.values))) for c in t.ask(50, features = features_off)]
    t.tell(evals)
    hist = t.history()
    self.assertEqual(100, len(hist))
    evals = [ccs.Evaluation(objective_space = os, configuration = c, values = func(*(c.values))) for c in t.ask(100, features = features_on)]
    t.tell(evals)
    self.assertEqual(200, t.history_size())
    self.assertEqual(150, t.history_size(features = features_on))
    self.assertEqual(50, t.history_size(features = features_off))
    optims_ref = t.optima()
    optims = t.optima(features = features_on)
    objs = [x.objective_values for x in optims]
    objs.sort(key = lambda x: x[0])
    # assert pareto front
    self.assertTrue(all(objs[i][1] >= objs[i+1][1] for i in range(len(objs)-1)))
    self.assertTrue(t.suggest(features_on) in [x.configuration for x in optims])
    optims = t.optima(features = features_off)
    objs = [x.objective_values for x in optims]
    objs.sort(key = lambda x: x[0])
    # assert pareto front
    self.assertTrue(all(objs[i][1] >= objs[i+1][1] for i in range(len(objs)-1)))
    self.assertTrue(t.suggest(features_off) in [x.configuration for x in optims])
    # test serialization
    buff = t.serialize()
    t_copy = ccs.deserialize(buffer = buff)
    hist = t_copy.history()
    self.assertEqual(200, len(hist))
    optims_2 = t_copy.optima()
    self.assertEqual(len(optims_ref), len(optims_2))


  def test_user_defined(self):
    class TunerData:
      def __init__(self):
        self.history = []
        self.optima = []

    def delete(tuner):
      return None

    def ask(tuner, features, count):
      if count is None:
        return (None, 1)
      else:
        cs = tuner.search_space
        return (cs.samples(count, features = features), count)

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

    def get_history(tuner, features):
      if features is not None:
        return list(filter(lambda e: e.features == features, tuner.tuner_data.history))
      else:
        return tuner.tuner_data.history

    def get_optima(tuner, features):
      if features is not None:
        return list(filter(lambda e: e.features == features, tuner.tuner_data.optima))
      else:
        return tuner.tuner_data.optima

    def suggest(tuner, features):
      optis = list(filter(lambda e: e.features == features, tuner.tuner_data.optima))
      if not optis:
        return ask(tuner, features, 1)
      else:
        return choice(optis).configuration

    (fs, os) = self.create_tuning_problem()
    t = ccs.UserDefinedTuner(name = "tuner", objective_space = os, delete = delete, ask = ask, tell = tell, get_optima = get_optima, get_history = get_history, suggest = suggest, tuner_data = TunerData())
    t2 = ccs.Object.from_handle(t.handle)
    self.assertEqual("tuner", t.name)
    self.assertEqual(ccs.TunerType.USER_DEFINED, t.type)
    self.assertEqual(fs.handle.value, t.feature_space.handle.value)
    self.assertEqual(os.handle.value, t.objective_space.handle.value)
    self.assertEqual(os.search_space.handle.value, t.search_space.handle.value)
    func = lambda x, y, z: [(x-2)*(x-2), sin(z+y)]
    features_on = ccs.Features(feature_space = fs, values = [True])
    features_off = ccs.Features(feature_space = fs, values = [False])
    evals = [ccs.Evaluation(objective_space = os, configuration = c, values = func(*(c.values))) for c in t.ask(50, features = features_on)]
    t.tell(evals)
    evals = [ccs.Evaluation(objective_space = os, configuration = c, values = func(*(c.values))) for c in t.ask(50, features = features_off)]
    t.tell(evals)
    hist = t.history()
    self.assertEqual(100, len(hist))
    evals = [ccs.Evaluation(objective_space = os, configuration = c, values = func(*(c.values))) for c in t.ask(100, features = features_on)]
    t.tell(evals)
    self.assertEqual(200, t.history_size())
    self.assertEqual(150, t.history_size(features = features_on))
    self.assertEqual(50, t.history_size(features = features_off))
    optims_ref = t.optima()
    optims = t.optima(features = features_on)
    objs = [x.objective_values for x in optims]
    objs.sort(key = lambda x: x[0])
    # assert pareto front
    self.assertTrue(all(objs[i][1] >= objs[i+1][1] for i in range(len(objs)-1)))
    self.assertTrue(t.suggest(features_on) in [x.configuration for x in optims])
    optims = t.optima(features = features_off)
    objs = [x.objective_values for x in optims]
    objs.sort(key = lambda x: x[0])
    # assert pareto front
    self.assertTrue(all(objs[i][1] >= objs[i+1][1] for i in range(len(objs)-1)))
    self.assertTrue(t.suggest(features_off) in [x.configuration for x in optims])
    # test serialization
    buff = t.serialize()
    t_copy = ccs.UserDefinedTuner.deserialize(buffer = buff, delete = delete, ask = ask, tell = tell, get_optima = get_optima, get_history = get_history, suggest = suggest, tuner_data = TunerData())
    hist = t_copy.history()
    self.assertEqual(200, len(hist))
    optims_2 = t_copy.optima()
    self.assertEqual(len(optims_ref), len(optims_2))


if __name__ == '__main__':
    unittest.main()

