require 'minitest/autorun'
require_relative '../lib/cconfigspace'

class CConfigSpaceTestExpressionParser < Minitest::Test
  def setup
    CCS.init
  end

  def test_parse
    exp = "1.0 + 1 == 2 || +1 == 3e0 && \"y\\nes\" == 'no' "
    res = CCS.parse(exp)
    assert( res.kind_of? CCS::Expression )
    assert_equal( "1.0 + 1 == 2 || +1 == 3.0 && \"y\\nes\" == \"no\"", res.to_s )
  end

  def test_parse_priority
    exp = "(1 + 3) * 2"
    res = CCS.parse(exp)
    assert( res.kind_of? CCS::Expression::Multiply )
    assert_equal( exp, res.to_s )
  end

  def test_associativity
    exp = "5 - 2 - 1"
    res = CCS.parse(exp)
    assert( res.kind_of? CCS::Expression::Substract )
    assert_equal( 2, res.eval )
    exp = "5 - +(+2 - 1)"
    res = CCS.parse(exp)
    assert( res.kind_of? CCS::Expression::Substract )
    assert_equal( 4, res.eval )
  end

  def test_in
    exp = "5 # [3.0, 5]"
    res = CCS.parse(exp)
    assert( res.kind_of? CCS::Expression::In )
    assert_equal( true, res.eval )
    exp = "5 # [3.0, 4]"
    res = CCS.parse(exp)
    assert( res.kind_of? CCS::Expression::In )
    assert_equal( false, res.eval )
  end

  def test_boolean
    exp = "true"
    res = CCS.parse(exp)
    assert( res.kind_of? CCS::Expression::Literal )
    assert_equal( true, res.eval )
    assert_equal( "true", res.to_s )
    exp = "false"
    res = CCS.parse(exp)
    assert( res.kind_of? CCS::Expression::Literal )
    assert_equal( false, res.eval )
    assert_equal( "false", res.to_s )
  end

  def test_none
    exp = "none"
    res = CCS.parse(exp)
    assert( res.kind_of? CCS::Expression::Literal )
    assert_nil( res.eval )
    assert_equal( "none", res.to_s )
  end

  def test_function
    def func(a, b)
      a * b
    end
    exp = "func(3, 4)"
    res = CCS.parse(exp, binding: binding)
    assert( res.kind_of? CCS::Expression::UserDefined )
    assert_equal( "func(3, 4)", res.to_s )
    assert_equal( 12, res.eval )

    get_vector_data = lambda { |otype, name|
      assert_equal(:CCS_OBJECT_TYPE_EXPRESSION, otype)
      CCS::Expression::get_function_vector_data(name, binding: binding)
    }

    buff = res.serialize
    res_copy = CCS::deserialize(buffer: buff, vector_callback: get_vector_data)
    assert_equal( "func(3, 4)", res_copy.to_s )
    assert_equal( 12, res_copy.eval )
  end

end

