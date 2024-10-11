require 'minitest/autorun'
require_relative '../lib/cconfigspace'

class CConfigSpaceTestTree < Minitest::Test
  def test_create
    rng = CCS::Rng.new
    root = CCS::Tree.new(arity: 4, value: "foo")
    assert_equal( "foo", root.value )
    assert_equal( 4, root.arity)
    assert_nil( root.get_child(1) )
    root.children.each { |c|
      assert_nil( c )
    }
    assert_nil( root.parent )
    assert_nil( root.index )
    assert_equal( root.handle, root.get_node_at_position([]).handle )
    assert_equal( [], root.position )
    assert_equal( 0, root.depth )

    assert_equal( 1.0, root.weight )
    root.weight = 2.0
    assert_equal( 2.0, root.weight )
    assert_equal( 1.0, root.bias )
    root.bias = 2.0
    assert_equal( 2.0, root.bias )
    i = root.sample(rng)
    assert( i.nil? || (i >= 0 and i < 4) )
    root.samples(rng, 100).each { |i|
      assert( i.nil? || (i >= 0 and i < 4) )
    }

    child = CCS::Tree.new(arity: 3, value: "bar" )
    assert_equal( "bar", child.value )
    root.set_child(2, child)
    assert_equal( 2, child.index )
    assert_equal( ["foo", "bar"], child.values )
    assert_equal( [2], child.position )
    assert_equal( 1, child.depth )
    assert_equal( child.handle, root.get_child(2).handle )
    assert_equal( child.handle, root.get_node_at_position([2]).handle )
    assert_equal( ["foo", "bar"], root.get_values_at_position([2]) )

    buff = root.serialize
    tree = CCS.deserialize(buffer: buff)
    assert_equal( tree.object_type, :CCS_OBJECT_TYPE_TREE )
    assert_equal( ["foo", "bar"], tree.get_values_at_position([2]) )
  end
end
