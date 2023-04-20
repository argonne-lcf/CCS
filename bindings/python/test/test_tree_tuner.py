import unittest
import sys
from random import choice
sys.path.insert(1, '.')
sys.path.insert(1, '..')
import cconfigspace as ccs

def generate_tree(depth, rank):
  ar = depth - rank
  ar = 0 if ar < 0 else ar
  tree = ccs.Tree(arity = ar, value = depth * 100.0 + rank)
  for i in range(ar):
    child = generate_tree(depth - 1, i)
    tree.set_child(i, child)
  return tree

def reduce(l):
  res = 0
  for x in l:
    res = res + x
  return res

class TestTreeTuner(unittest.TestCase):

  def create_tuning_problem(self):
    tree = generate_tree(5, 0)
    ts = ccs.StaticTreeSpace(name = 'space', tree = tree)
    os = ccs.ObjectiveSpace(name = "ospace")
    v1 = ccs.NumericalParameter(lower = float('-inf'), upper = float('inf'))
    os.add_parameter(v1)
    e1 = ccs.Variable(parameter = v1)
    os.add_objectives( {e1: ccs.ccs_objective_type.MAXIMIZE} )
    return (ts, os)

  def test_create_random(self):
    (ts, os) = self.create_tuning_problem()
    t = ccs.RandomTreeTuner(name = "tuner", tree_space = ts, objective_space = os)
    t2 = ccs.Object.from_handle(t.handle)
    self.assertEqual("tuner", t.name)
    self.assertEqual(ccs.ccs_tree_tuner_type.RANDOM, t.type)
    evals = [ccs.TreeEvaluation(objective_space = os, configuration = c, values = [reduce(c.values)]) for c in t.ask(100)]
    t.tell(evals)
    hist = t.history
    evals = [ccs.TreeEvaluation(objective_space = os, configuration = c, values = [reduce(c.values)]) for c in t.ask(100)]
    t.tell(evals)
    hist = t.history
    self.assertEqual(200, len(hist))
    optims = t.optimums
    self.assertEqual(1, len(optims))
    best = optims[0].objective_values[0]
    self.assertTrue(all(best >= x.objective_values[0] for x in hist))
    self.assertTrue(t.suggest in [x.configuration for x in optims])
    buff = t.serialize()
    t_copy = ccs.deserialize(buffer = buff)
    hist = t_copy.history
    self.assertEqual(200, len(hist))
    optims_2 = t_copy.optimums
    self.assertEqual(len(optims), len(optims_2))
    best2 = optims_2[0].objective_values[0]
    self.assertEqual(best, best2)
    self.assertTrue(all(best2 >= x.objective_values[0] for x in hist))
    self.assertTrue(t_copy.suggest in [x.configuration for x in optims_2])

  def test_user_defined(self):
    class TunerData:
      def __init__(self):
        self.history = []
        self.optimums = []

    def delete(tuner):
      return None

    def ask(tuner, count):
      if count is None:
        return (None, 1)
      else:
        ts = tuner.tree_space
        return (ts.samples(count), count)

    def tell(tuner, evaluations):
      tuner.tuner_data.history += evaluations
      for e in evaluations:
        discard = False
        new_optimums = []
        for o in tuner.tuner_data.optimums:
          if discard:
            new_optimums.append(o)
          else:
            c = e.compare(o)
            if c == ccs.ccs_comparison.EQUIVALENT or c == ccs.ccs_comparison.WORSE:
              discard = True
              new_optimums.append(o)
            elif c == ccs.ccs_comparison.NOT_COMPARABLE:
              new_optimums.append(o)
        if not discard:
          new_optimums.append(e)
        tuner.tuner_data.optimums = new_optimums
      return None

    def get_history(tuner):
      return tuner.tuner_data.history

    def get_optimums(tuner):
      return tuner.tuner_data.optimums

    def suggest(tuner):
      if not tuner.tuner_data.optimums:
        return ask(tuner, 1)
      else:
        return choice(tuner.tuner_data.optimums).configuration

    (ts, os) = self.create_tuning_problem()
    t = ccs.UserDefinedTreeTuner(name = "tuner", tree_space = ts, objective_space = os, delete = delete, ask = ask, tell = tell, get_optimums = get_optimums, get_history = get_history, suggest = suggest, tuner_data = TunerData())
    t2 = ccs.Object.from_handle(t.handle)
    self.assertEqual("tuner", t.name)
    self.assertEqual(ccs.ccs_tree_tuner_type.USER_DEFINED, t.type)
    evals = [ccs.TreeEvaluation(objective_space = os, configuration = c, values = [reduce(c.values)]) for c in t.ask(100)]
    t.tell(evals)
    hist = t.history
    evals = [ccs.TreeEvaluation(objective_space = os, configuration = c, values = [reduce(c.values)]) for c in t.ask(100)]
    t.tell(evals)
    hist = t.history
    self.assertEqual(200, len(hist))
    optims = t.optimums
    self.assertEqual(1, len(optims))
    best = optims[0].objective_values[0]
    self.assertTrue(all(best >= x.objective_values[0] for x in hist))
    self.assertTrue(t.suggest in [x.configuration for x in optims])
    buff = t.serialize()
    t_copy = ccs.UserDefinedTreeTuner.deserialize(buffer = buff, delete = delete, ask = ask, tell = tell, get_optimums = get_optimums, get_history = get_history, suggest = suggest, tuner_data = TunerData())
    hist = t_copy.history
    self.assertEqual(200, len(hist))
    optims_2 = t_copy.optimums
    self.assertEqual(len(optims), len(optims_2))
    best2 = optims_2[0].objective_values[0]
    self.assertEqual(best, best2)
    self.assertTrue(all(best2 >= x.objective_values[0] for x in hist))
    self.assertTrue(t_copy.suggest in [x.configuration for x in optims_2])



