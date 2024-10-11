require 'minitest/autorun'
require_relative '../lib/cconfigspace'

class CConfigSpaceTestTreeSpace < Minitest::Test
  def generate_tree(depth, rank)
    ar = depth - rank
    ar = 0 if ar < 0
    tree = CCS::Tree.new(arity: ar, value: depth * 100 + rank)
    ar.times { |i|
      child = generate_tree(depth - 1, i)
      tree.set_child(i, child)
    }
    tree
  end

  def test_static_tree_space
    rng = CCS::Rng.new
    tree = generate_tree(4, 0)
    ts = CCS::StaticTreeSpace.new(name: 'space', tree: tree, rng: rng)
    assert_equal( :CCS_OBJECT_TYPE_TREE_SPACE, ts.object_type )
    assert_equal( "space", ts.name )
    assert_equal( :CCS_TREE_SPACE_TYPE_STATIC, ts.type )
    assert_instance_of( CCS::Rng, ts.rng )
    assert_equal( rng.handle, ts.rng.handle )
    assert_equal( tree.handle, ts.tree.handle )
    assert_equal( tree.handle, ts.get_node_at_position([]).handle )
    assert_equal( 201, ts.get_node_at_position([1, 1]).value )
    assert_equal( [400, 301, 201], ts.get_values_at_position([1, 1]) )

    ts.sample
    ts.samples(100)

    buff = ts.serialize
    ts2 = CCS.deserialize(buffer: buff)
    assert_equal( [400, 301, 201], ts2.get_values_at_position([1, 1]) )
  end

  def test_dynamic_tree_space
    del = lambda { |tree_space| nil }
    get_child = lambda { |tree_space, parent, child_index|
      depth = parent.depth
      child_depth = depth + 1
      arity = 4 - child_depth - child_index
      arity = 0 if arity < 0
      CCS::Tree.new(arity: arity, value: (4 - child_depth)*100 + child_index)
    }
    get_vector_data = lambda { |otype, name|
      assert_equal(:CCS_OBJECT_TYPE_TREE_SPACE, otype)
      assert_equal('space', name)
      [CCS::DynamicTreeSpace.get_vector(del: del, get_child: get_child), nil]
    }

    tree = CCS::Tree.new(arity: 4, value: 400)
    ts = CCS::DynamicTreeSpace.new(name: 'space', tree: tree, del: del, get_child: get_child)
    assert_equal( :CCS_OBJECT_TYPE_TREE_SPACE, ts.object_type )
    assert_equal( :CCS_TREE_SPACE_TYPE_DYNAMIC, ts.type )
    assert_equal( tree.handle, ts.tree.handle )
    assert_equal( tree.handle, ts.get_node_at_position([]).handle )
    assert_equal( 201, ts.get_node_at_position([1, 1]).value )
    assert_equal( [400, 301, 201], ts.get_values_at_position([1, 1]) )

    ts.sample
    ts.samples(100)

    buff = ts.serialize
    ts2 = CCS::deserialize(buffer: buff, vector_callback: get_vector_data)
    assert_equal( [400, 301, 201], ts2.get_values_at_position([1, 1]) )
  end

  def test_tree_configuration
    tree = generate_tree(4, 0)
    ts = CCS::StaticTreeSpace.new(name: 'space', tree: tree)
    tc = CCS::TreeConfiguration.new(tree_space: ts, position: [1, 1])
    assert_equal( tc.tree_space.handle, ts.handle )
    assert_equal( 2, tc.position_size )
    assert_equal( [1, 1], tc.position )
    assert_equal( [400, 301, 201], tc.values )
    assert_equal( ts.get_node_at_position([1, 1]).handle, tc.node.handle )
    tc2 = CCS::TreeConfiguration.new(tree_space: ts, position: [1, 0])
    refute_equal( tc.hash, tc2.hash )
    assert( tc < tc2 || tc > tc2 )
  end

end
