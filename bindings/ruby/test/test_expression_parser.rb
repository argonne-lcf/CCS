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
end

