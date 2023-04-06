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
    assert_equal( [], e.parameters )
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
    h = CCS::NumericalParameter::new
    e = CCS::Variable::new(parameter: h)
    assert_equal( h.name , e.to_s )
  end

  def test_list
    e = CCS::List::new(values: ["foo", 1, 2.0])
    assert_equal( "[ \"foo\", 1, 2.0 ]", e.to_s )
    assert_equal( "foo", e.eval(0) )
    assert_equal( 1, e.eval(1) )
    assert_equal( 2.0, e.eval(2) )
    h = CCS::NumericalParameter::new(name: "test")
    e2 = CCS::Expression::new(type: :CCS_IN, nodes: [h, e])
    assert_equal( "test # [ \"foo\", 1, 2.0 ]",  e2.to_s )
  end

  def test_unary
    e = CCS::Expression::unary(type: :CCS_NOT, node: true)
    assert_equal( "!true", e.to_s )
    assert_equal( false, e.eval )
  end

  def test_binary
    e = CCS::Expression::binary(type: :CCS_OR, left: true, right: false)
    assert_equal( "true || false", e.to_s)
    assert_equal( true, e.eval )
  end
end
