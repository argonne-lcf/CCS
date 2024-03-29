require 'minitest/autorun'
require_relative '../lib/cconfigspace'

class CConfigSpaceTestExpression < Minitest::Test
  def setup
    CCS.init
  end

  def test_create
    e = CCS::Expression::Add.new(left: 1.0, right: 2.0)
    assert_equal( :CCS_OBJECT_TYPE_EXPRESSION, e.object_type )
    assert_equal( :CCS_EXPRESSION_TYPE_ADD, e.type )
    assert_equal( 2, e.num_nodes )
    nodes = e.nodes
    assert_equal( 2, nodes.size )
    nodes.each { |n|
      assert( n.kind_of?(CCS::Expression::Literal) )
      assert_equal( :CCS_OBJECT_TYPE_EXPRESSION, n.object_type )
      assert_equal( :CCS_EXPRESSION_TYPE_LITERAL, n.type )
    }
    assert_equal( 1.0, nodes[0].value )
    assert_equal( 2.0, nodes[1].value )
    assert_equal( 3.0, e.eval )
    assert_equal( [], e.parameters )
  end

  def test_to_s
    e = CCS::Expression::Add.new(left: 1.0, right: 2.0)
    assert_equal( "1.0 + 2.0", e.to_s )
    e2 = CCS::Expression::Multiply.new(left: 5.0, right: e)
    assert_equal( "5.0 * (1.0 + 2.0)", e2.to_s )
  end

  def test_literal
    e = CCS::Expression::Literal::new(value: 15)
    assert_equal( "15" , e.to_s )
  end

  def test_variable
    h = CCS::NumericalParameter::Float.new
    e = CCS::Expression::Variable::new(parameter: h)
    assert_equal( h.name , e.to_s )
  end

  def test_list
    e = CCS::Expression::List::new(values: ["foo", 1, 2.0])
    assert_equal( "[ \"foo\", 1, 2.0 ]", e.to_s )
    assert_equal( "foo", e.eval(0) )
    assert_equal( 1, e.eval(1) )
    assert_equal( 2.0, e.eval(2) )
    h = CCS::NumericalParameter::Float.new(name: "test")
    e2 = CCS::Expression::In.new(left: h, right: e)
    assert_equal( "test # [ \"foo\", 1, 2.0 ]",  e2.to_s )
  end

  def test_unary
    e = CCS::Expression::Not.new(node: true)
    assert_equal( "!true", e.to_s )
    assert_equal( false, e.eval )
  end

  def test_binary
    e = CCS::Expression::Or.new(left: true, right: false)
    assert_equal( "true || false", e.to_s)
    assert_equal( true, e.eval )
  end
end
