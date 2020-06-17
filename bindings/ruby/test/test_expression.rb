[ '../lib', 'lib' ].each { |d| $:.unshift(d) if File::directory?(d) }
require 'minitest/autorun'
require 'cconfigspace'

class CConfigSpaceTestExpression < Minitest::Test
  def setup
    CCS.init
  end

  def test_create
    e = CCS::Expression::new(type: :CCS_ADD, nodes: [1.0, 2.0])
    assert_equal( :CCS_EXPRESSION, e.object_type )
    assert_equal( :CCS_ADD, e.type )
    assert_equal( 2, e.num_nodes )
    nodes = e.nodes
    assert_equal( 2, nodes.size )
    nodes.each { |n|
      assert( n.kind_of?(CCS::Literal) )
      assert_equal( :CCS_EXPRESSION, n.object_type )
      assert_equal( :CCS_LITERAL, n.type )
    }
    assert_equal( 1.0, nodes[0].value )
    assert_equal( 2.0, nodes[1].value )
    assert_equal( 3.0, e.eval )
    assert_equal( [], e.hyperparameters )
  end

  def test_to_s
    e = CCS::Expression::new(type: :CCS_ADD, nodes: [1.0, 2.0])
    assert_equal( "1.0 + 2.0", e.to_s )
    e2 = CCS::Expression::new(type: :CCS_MULTIPLY, nodes: [5.0, e])
    assert_equal( "5.0 * (1.0 + 2.0)", e2.to_s )
  end

  def test_literal
    e = CCS::Literal::new(value: 15)
    assert_equal( "15" , e.to_s )
  end

  def test_variable
    h = CCS::NumericalHyperparameter::new
    e = CCS::Variable::new(hyperparameter: h)
    assert_equal( h.name , e.to_s )
  end
end
