import unittest
import sys
sys.path.insert(1, '.')
sys.path.insert(1, '..')
import cconfigspace as ccs

def generate_tree(depth, rank):
  ar = depth - rank
  ar = 0 if ar < 0 else ar
  tree = ccs.Tree(arity = ar, value = depth * 100 + rank)
  for i in range(ar):
    child = generate_tree(depth - 1, i)
    tree.set_child(i, child)
  return tree

class TestTreeEvaluation(unittest.TestCase):

  def test_create(self):
    tree = generate_tree(4, 0)
    ts = ccs.StaticTreeSpace(name = 'space', tree = tree)
    os = ccs.ObjectiveSpace(name = "ospace")
    v1 = ccs.NumericalParameter.Float()
    v2 = ccs.NumericalParameter.Float()
    os.add_parameters([v1, v2])
    e1 = ccs.Expression.Variable(parameter = v1)
    e2 = ccs.Expression.Variable(parameter = v2)
    os.add_objectives( { e1: ccs.ObjectiveType.MAXIMIZE, e2: ccs.ObjectiveType.MINIMIZE } )
    ev1 = ccs.TreeEvaluation(objective_space = os, configuration = ts.sample())
    ev1.set_value(0, 0.5)
    ev1.set_value(v2, 0.6)
    self.assertEqual( [0.5, 0.6], ev1.values )
    self.assertEqual( [0.5, 0.6], ev1.objective_values )
    self.assertTrue( ev1.check )
    self.assertTrue( os.check_values(ev1.values) )
    ev2 = ccs.TreeEvaluation(objective_space = os, configuration = ts.sample(), values = [0.5, 0.6])
    self.assertEqual( [0.5, 0.6], ev2.values )
    self.assertEqual( [0.5, 0.6], ev2.objective_values )
    self.assertEqual( ccs.Comparison.EQUIVALENT, ev1.compare(ev2) )
    ev3 = ccs.TreeEvaluation(objective_space = os, configuration = ts.sample(), values = [0.6, 0.5])
    self.assertEqual( [0.6, 0.5], ev3.objective_values )
    self.assertEqual( ccs.Comparison.WORSE, ev1.compare(ev3) )
    self.assertEqual( ccs.Comparison.BETTER, ev3.compare(ev1) )
    ev4 = ccs.TreeEvaluation(objective_space = os, configuration = ts.sample(), values = [0.6, 0.7])
    self.assertEqual( [0.6, 0.7], ev4.objective_values )
    self.assertEqual( ccs.Comparison.NOT_COMPARABLE, ev1.compare(ev4) )
    self.assertEqual( ccs.Comparison.NOT_COMPARABLE, ev4.compare(ev1) )

  def test_serialize(self):
    tree = generate_tree(4, 0)
    ts = ccs.StaticTreeSpace(name = 'space', tree = tree)
    os = ccs.ObjectiveSpace(name = "ospace")
    v1 = ccs.NumericalParameter.Float()
    v2 = ccs.NumericalParameter.Float()
    os.add_parameters([v1, v2])
    e1 = ccs.Expression.Variable(parameter = v1)
    e2 = ccs.Expression.Variable(parameter = v2)
    os.add_objectives( { e1: ccs.ObjectiveType.MAXIMIZE, e2: ccs.ObjectiveType.MINIMIZE } )
    evref = ccs.TreeEvaluation(objective_space = os, configuration = ts.sample(), values = [0.5, 0.6])
    buff = evref.serialize()
    handle_map = ccs.Map()
    handle_map[ts] = ts
    handle_map[os] = os
    ev = ccs.deserialize(buffer = buff, handle_map = handle_map)
    self.assertEqual( ts.handle.value, ev.configuration.tree_space.handle.value)
    self.assertEqual( os.handle.value, ev.objective_space.handle.value)
    self.assertEqual( [0.5, 0.6], ev.values )
    self.assertEqual( [0.5, 0.6], ev.objective_values )

if __name__ == '__main__':
    unittest.main()
