import unittest
import re
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

class TestTreeSpace(unittest.TestCase):

  def test_static_tree_space(self):
    rng = ccs.Rng()
    tree = generate_tree(4, 0)
    ts = ccs.StaticTreeSpace(name = 'space', tree = tree, rng = rng)
    self.assertEqual( ccs.ObjectType.TREE_SPACE, ts.object_type )
    self.assertEqual( "space", ts.name )
    self.assertIsInstance( ts.rng, ccs.Rng )
    self.assertEqual( ccs.TreeSpaceType.STATIC, ts.type )
    self.assertEqual( rng.handle.value, ts.rng.handle.value )
    self.assertEqual( tree.handle.value, ts.tree.handle.value )
    self.assertEqual( tree.handle.value, ts.get_node_at_position([]).handle.value )
    self.assertEqual( 201, ts.get_node_at_position([1, 1]).value )
    self.assertEqual( [400, 301, 201], ts.get_values_at_position([1, 1]) )
    self.assertTrue( ts.check_position([1, 1]) )
    self.assertFalse( ts.check_position([1, 4]) )

    tc = ts.sample()
    self.assertTrue( ts.check_configuration(tc) )

    for x in ts.samples(100):
      self.assertTrue( ts.check_configuration(x) )

    buff = ts.serialize()
    ts2 = ccs.Object.deserialize(buffer = buff)
    self.assertEqual( [400, 301, 201], ts2.get_values_at_position([1, 1]) )

  def test_dynamic_tree_space(self):

    def delete(tree_space):
      return None

    def get_child(tree_space, parent, child_index):
      depth = parent.depth
      child_depth = depth + 1
      arity = 4 - child_depth - child_index
      arity = 0 if arity < 0 else arity
      return ccs.Tree(arity = arity, value = (4 - child_depth)*100 + child_index)

    tree = ccs.Tree(arity = 4, value = 400)
    ts = ccs.DynamicTreeSpace(name = 'space', tree = tree, delete = delete, get_child = get_child)
    self.assertEqual( ccs.ObjectType.TREE_SPACE, ts.object_type )
    self.assertEqual( "space", ts.name )
    self.assertIsInstance( ts.rng, ccs.Rng )
    self.assertEqual( ccs.TreeSpaceType.DYNAMIC, ts.type )
    self.assertEqual( tree.handle.value, ts.tree.handle.value )
    self.assertEqual( tree.handle.value, ts.tree.handle.value )
    self.assertEqual( tree.handle.value, ts.get_node_at_position([]).handle.value )
    self.assertEqual( 201, ts.get_node_at_position([1, 1]).value )
    self.assertEqual( [400, 301, 201], ts.get_values_at_position([1, 1]) )
    self.assertTrue( ts.check_position([1, 1]) )
    self.assertFalse( ts.check_position([1, 4]) )
    tc = ts.sample()
    self.assertTrue( ts.check_configuration(tc) )

    for i in range(100):
      tc = ts.sample()
      self.assertTrue( ts.check_configuration(tc) )

    buff = ts.serialize()
    ts2 = ccs.DynamicTreeSpace.deserialize(buffer = buff, delete = delete, get_child = get_child)
    self.assertEqual( [400, 301, 201], ts2.get_values_at_position([1, 1]) )

  def test_tree_configuration(self):
    tree = generate_tree(4, 0)
    ts = ccs.StaticTreeSpace(name = 'space', tree = tree)
    tc = ts.sample()
    self.assertTrue( tc.check() )
    for x in ts.samples(100):
      self.assertTrue( x.check() )

    tc = ccs.TreeConfiguration(tree_space = ts, position = [1, 1])
    self.assertEqual( tc.tree_space.handle.value, ts.handle.value )
    self.assertEqual( 2, tc.position_size )
    self.assertEqual( [1, 1], tc.position )
    self.assertEqual( [400, 301, 201], tc.values )
    self.assertEqual( ts.get_node_at_position([1, 1]).handle.value, tc.node.handle.value )
    tc2 = ccs.TreeConfiguration(tree_space = ts, position = [1, 0])
    self.assertTrue( tc.check() )
    self.assertTrue( tc2.check() )
    self.assertNotEqual( tc.hash, tc2.hash )
    self.assertTrue( tc < tc2 or tc > tc2 )
