import unittest
import re
import sys
sys.path.insert(1, '.')
sys.path.insert(1, '..')
import cconfigspace as ccs

class TestTree(unittest.TestCase):

  def test_create(self):
    rng = ccs.Rng()
    root = ccs.Tree(arity = 4, value = "foo")
    self.assertEqual( "foo", root.value )
    self.assertEqual( 4, root.arity)
    self.assertEqual( None, root.get_child(1) )
    for c in root.children:
      self.assertEqual( None, c )
    self.assertEqual( None, root.parent )
    self.assertEqual( None, root.index )
    self.assertEqual( root.handle.value, root.get_node_at_position([]).handle.value )
    self.assertEqual( [], root.position )
    self.assertEqual( 0, root.depth )

    self.assertEqual( 1.0, root.weight )
    root.weight = 2.0
    self.assertEqual( 2.0, root.weight )
    self.assertEqual( 1.0, root.bias )
    root.bias = 2.0
    self.assertEqual( 2.0, root.bias )
    i = root.sample(rng)
    self.assertTrue( i is None or (i >= 0 and i < 4) )
    for i in root.samples(rng, 100):
      self.assertTrue( i is None or (i >= 0 and i < 4) )

    child = ccs.Tree(arity = 3, value = "bar" )
    self.assertEqual( "bar", child.value )
    root.set_child(2, child)
    self.assertEqual( 2, child.index )
    self.assertEqual( ["foo", "bar"], child.values )
    self.assertEqual( [2], child.position )
    self.assertEqual( 1, child.depth )
    self.assertEqual( child.handle.value, root.get_child(2).handle.value )
    self.assertEqual( child.handle.value, root.get_node_at_position([2]).handle.value )
    self.assertEqual( ["foo", "bar"], root.get_values_at_position([2]) )

    buff = root.serialize()
    tree = ccs.Object.deserialize(buffer = buff)
    self.assertEqual( tree.object_type, ccs.ObjectType.TREE )
    self.assertEqual( ["foo", "bar"], tree.get_values_at_position([2]) )
