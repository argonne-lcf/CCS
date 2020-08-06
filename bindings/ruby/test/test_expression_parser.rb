[ '../lib', 'lib' ].each { |d| $:.unshift(d) if File::directory?(d) }
require 'minitest/autorun'
require 'cconfigspace'

class CConfigSpaceTestExpressionParser < Minitest::Test
  def setup
    CCS.init
  end

  def test_parse
    m = CCS::ExpressionParser.new.method(:parse)
    exp = "1.0 + 1 == 2 || +1 == 3e0 && \"y\\nes\" == 'no' "
    res = m[exp]
    assert( res.kind_of? CCS::Expression )
    assert_equal( "1.0 + 1 == 2 || +1 == 3.0 && \"y\\nes\" == \"no\"", res.to_s )
  end

  def test_parse_priority
    m = CCS::ExpressionParser.new.method(:parse)
    exp = "(1 + 3) * 2"
    res = m[exp]
    assert( res.kind_of? CCS::Expression )
    assert_equal( exp, res.to_s )
  end

  def test_associativity
    m = CCS::ExpressionParser.new.method(:parse)
    exp = "5 - 2 - 1"
    res = m[exp]
    assert( res.kind_of? CCS::Expression )
    assert_equal( 2, res.eval )
    exp = "5 - +(+2 - 1)"
    res = m[exp]
    assert( res.kind_of? CCS::Expression )
    assert_equal( 4, res.eval )
  end

  def test_in
    m = CCS::ExpressionParser.new.method(:parse)
    exp = "5 # [3.0, 5]"
    res = m[exp]
    assert( res.kind_of? CCS::Expression )
    assert_equal( true, res.eval )
    exp = "5 # [3.0, 4]"
    res = m[exp]
    assert( res.kind_of? CCS::Expression )
    assert_equal( false, res.eval )
  end

  def test_boolean
    m = CCS::ExpressionParser.new.method(:parse)
    exp = "true"
    res = m[exp]
    assert( res.kind_of? CCS::Literal )
    assert_equal( true, res.eval )
    assert_equal( "true", res.to_s )
    exp = "false"
    res = m[exp]
    assert( res.kind_of? CCS::Literal )
    assert_equal( false, res.eval )
    assert_equal( "false", res.to_s )
  end

  def test_none
    m = CCS::ExpressionParser.new.method(:parse)
    exp = "nil"
    res = m[exp]
    assert( res.kind_of? CCS::Literal )
    assert_nil( res.eval )
    assert_equal( "nil", res.to_s )
  end

end

