import unittest
import sys
import os as _os
import signal
import threading
import subprocess
import datetime
import base64
import xmlrpc.client
import ctypes as ct
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
    os = ccs.ObjectiveSpace(name = "ospace", search_space = cs, parameters = [v1, v2], objectives = [e1, e2])
    return os

  def test_create_random(self):
    os = self.create_tuning_problem()
    t = ccs.RandomTuner(name = "tuner", objective_space = os)
    t2 = ccs.Object.from_handle(t.handle)
    self.assertEqual("tuner", t.name)
    self.assertEqual(ccs.TunerType.RANDOM, t.type)
    func = lambda x, y, z: [(x-2)*(x-2), sin(z+y)]
    evals = [ccs.Evaluation(objective_space = os, configuration = c, values = func(*(c.values))) for c in t.ask(100)]
    t.tell(evals)
    hist = t.history()
    self.assertEqual(100, len(hist))
    evals = [ccs.Evaluation(objective_space = os, configuration = c, values = func(*(c.values))) for c in t.ask(100)]
    t.tell(evals)
    hist = t.history()
    self.assertEqual(200, len(hist))
    optims = t.optima()
    objs = [x.objective_values for x in optims]
    objs.sort(key = lambda x: x[0])
    # assert pareto front
    self.assertTrue(all(objs[i][1] >= objs[i+1][1] for i in range(len(objs)-1)))
    self.assertTrue(t.suggest() in [x.configuration for x in optims])
    # test serialization
    buff = t.serialize()
    t_copy = ccs.deserialize(buffer = buff)
    hist = t_copy.history()
    self.assertEqual(200, len(hist))
    optims_2 = t_copy.optima()
    self.assertEqual(len(optims), len(optims_2))
    objs = [x.objective_values for x in optims_2]
    objs.sort(key = lambda x: x[0])
    self.assertTrue(all(objs[i][1] >= objs[i+1][1] for i in range(len(objs)-1)))
    self.assertTrue(t_copy.suggest() in [x.configuration for x in optims_2])

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

    def get_history(tuner, features):
      return tuner.tuner_data.history

    def get_optima(tuner, features):
      return tuner.tuner_data.optima

    def suggest(tuner, features):
      if not tuner.tuner_data.optima:
        return ask(tuner, 1)
      else:
        return choice(tuner.tuner_data.optima).configuration

    def get_vector_data(otype, name):
      return (ccs.UserDefinedTuner.get_vector(delete = delete, ask = ask, tell = tell, get_optima = get_optima, get_history = get_history, suggest = suggest), TunerData())

    os = self.create_tuning_problem()
    t = ccs.UserDefinedTuner(name = "tuner", objective_space = os, delete = delete, ask = ask, tell = tell, get_optima = get_optima, get_history = get_history, suggest = suggest, tuner_data = TunerData())
    t2 = ccs.Object.from_handle(t.handle)
    self.assertEqual("tuner", t.name)
    self.assertEqual(ccs.TunerType.USER_DEFINED, t.type)
    self.assertEqual(os.handle.value, t.objective_space.handle.value)
    self.assertEqual(os.search_space.handle.value, t.search_space.handle.value)
    func = lambda x, y, z: [(x-2)*(x-2), sin(z+y)]
    evals = [ccs.Evaluation(objective_space = os, configuration = c, values = func(*(c.values))) for c in t.ask(100)]
    t.tell(evals)
    hist = t.history()
    self.assertEqual(100, len(hist))
    evals = [ccs.Evaluation(objective_space = os, configuration = c, values = func(*(c.values))) for c in t.ask(100)]
    t.tell(evals)
    hist = t.history()
    self.assertEqual(200, len(hist))
    optims = t.optima()
    objs = [x.objective_values for x in optims]
    objs.sort(key = lambda x: x[0])
    # assert pareto front
    self.assertTrue(all(objs[i][1] >= objs[i+1][1] for i in range(len(objs)-1)))
    self.assertTrue(t.suggest() in [x.configuration for x in optims])

    # test serialization
    buff = t.serialize()
    t_copy = ccs.deserialize(buffer = buff, vector_callback = get_vector_data)
    hist = t_copy.history()
    self.assertEqual(200, len(hist))
    optims_2 = t_copy.optima()
    self.assertEqual(len(optims), len(optims_2))
    objs = [x.objective_values for x in optims_2]
    objs.sort(key = lambda x: x[0])
    self.assertTrue(all(objs[i][1] >= objs[i+1][1] for i in range(len(objs)-1)))
    self.assertTrue(t_copy.suggest() in [x.configuration for x in optims_2])

    t.serialize(path = 'tuner.ccs')
    t_copy = ccs.deserialize(path = 'tuner.ccs', vector_callback = get_vector_data)
    hist = t_copy.history()
    self.assertEqual(200, len(hist))
    optims_2 = t_copy.optima()
    self.assertEqual(len(optims), len(optims_2))
    objs = [x.objective_values for x in optims_2]
    objs.sort(key = lambda x: x[0])
    self.assertTrue(all(objs[i][1] >= objs[i+1][1] for i in range(len(objs)-1)))
    self.assertTrue(t_copy.suggest() in [x.configuration for x in optims_2])
    _os.remove('tuner.ccs')

    file = open( 'tuner.ccs', "wb")
    t.serialize(file_descriptor = file.fileno())
    file.close()
    file = open( 'tuner.ccs', "rb")
    t_copy = ccs.deserialize(file_descriptor = file.fileno(), vector_callback = get_vector_data)
    file.close()
    hist = t_copy.history()
    self.assertEqual(200, len(hist))
    optims_2 = t_copy.optima()
    self.assertEqual(len(optims), len(optims_2))
    objs = [x.objective_values for x in optims_2]
    objs.sort(key = lambda x: x[0])
    self.assertTrue(all(objs[i][1] >= objs[i+1][1] for i in range(len(objs)-1)))
    self.assertTrue(t_copy.suggest() in [x.configuration for x in optims_2])
    _os.remove('tuner.ccs')

  class ServerThread(threading.Thread):
    def __init__(self):
      self.p = None
      threading.Thread.__init__(self)

    def run(self):
      subprocess.run(['python3', _os.path.join(_os.path.abspath(_os.path.dirname(__file__)), 'tuner_server.py')], stdout = subprocess.DEVNULL, stderr = subprocess.DEVNULL)

  class TunerProxy:
    def __init__(self, name = "", objective_space = None):
      self.server = xmlrpc.client.ServerProxy("http://localhost:8000/")
      connected = False
      start = datetime.datetime.now()
      while not connected:
        try:
          connected = self.server.connected()
        except Exception as err:
          if datetime.datetime.now() - start > datetime.timedelta(seconds = 10):
            raise
      if objective_space is not None:
        self.objective_space = objective_space
        buff = objective_space.serialize()
        self.id, result = self.server.create(name, buff)
        self.handle_map = ccs.deserialize(buffer = result.data)
      else:
        self.id, result = self.server.load(name)
        self.handle_map = ccs.Map()
        self.objective_space = ccs.deserialize(buffer = result.data, handle_map = self.handle_map, map_handles = True)
        rev_map = ccs.Map()
        for (k, v) in self.handle_map.pairs():
          if isinstance(v, ccs.Context) or isinstance(v, ccs.TreeSpace):
            rev_map[ccs.Object(v.handle, retain = False, auto_release = False)] = k
        self.server.set_handle_map(self.id, rev_map.serialize())

    def ask(self, count = 1):
      return [ccs.deserialize(buffer = conf_str.data, handle_map = self.handle_map) for conf_str in self.server.ask(self.id, count)]

    def tell(self, evals = []):
      self.server.tell(self.id, [e.serialize() for e in evals])
      return self

    def history(self):
      return [ccs.deserialize(buffer = eval_str.data, handle_map = self.handle_map) for eval_str in self.server.history(self.id)]

    def history_size(self):
      return self.server.history_size(self.id)

    def optima(self):
      return [ccs.deserialize(buffer = eval_str.data, handle_map = self.handle_map) for eval_str in self.server.optima(self.id)]

    def num_optima(self):
      return self.server.num_optima(self.id)

    def suggest(self):
      return ccs.deserialize(buffer = self.server.suggest(self.id).data, handle_map = self.handle_map)

    def save(self):
      return self.server.save(self.id)

    def kill(self):
      return self.server.kill()

  def test_server(self):
    thr = self.ServerThread()
    thr.start()
    try:

      os = self.create_tuning_problem()
      t = self.TunerProxy(name = 'my_tuner', objective_space = os)
      func = lambda x, y, z: [(x-2)*(x-2), sin(z+y)]
      evals = [ccs.Evaluation(objective_space = os, configuration = c, values = func(*(c.values))) for c in t.ask(100)]
      t.tell(evals)
      hist = t.history()
      self.assertEqual(100, len(hist))
      evals = [ccs.Evaluation(objective_space = os, configuration = c, values = func(*(c.values))) for c in t.ask(100)]
      t.tell(evals)
      self.assertEqual(200, t.history_size())
      optims = t.optima()
      objs = [x.objective_values for x in optims]
      objs.sort(key = lambda x: x[0])
      # assert pareto front
      self.assertTrue(all(objs[i][1] >= objs[i+1][1] for i in range(len(objs)-1)))
      self.assertTrue(t.suggest() in [x.configuration for x in optims])

      t.save()
      t_copy = self.TunerProxy(name = "my_tuner")
      hist = t_copy.history()
      self.assertEqual(200, len(hist))
      optims_2 = t_copy.optima()
      self.assertEqual(len(optims), len(optims_2))
      objs = [x.objective_values for x in optims_2]
      objs.sort(key = lambda x: x[0])
      self.assertTrue(all(objs[i][1] >= objs[i+1][1] for i in range(len(objs)-1)))
      self.assertTrue(t_copy.suggest() in [x.configuration for x in optims_2])

    finally:
      xmlrpc.client.ServerProxy("http://localhost:8000/").kill()
      thr.join()

if __name__ == '__main__':
    unittest.main()

