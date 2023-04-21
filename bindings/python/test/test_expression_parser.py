import unittest
import sys
sys.path.insert(1, '.')
sys.path.insert(1, '..')
import cconfigspace as ccs

class TestExpressionParser(unittest.TestCase):

  def test_parse(self):
    exp = "1.0 + 1 == 2 || +1 == 3e0 && \"y\\nes\" == 'no' "
    res = ccs.parse(exp)
    self.assertIsInstance(res, ccs.Expression)
    self.assertEqual( "1.0 + 1 == 2 || +1 == 3.0 && 'y\\nes' == 'no'", res.__str__() )

  def test_parse_priority(self):
    exp = "(1 + 3) * 2"
    res = ccs.parse(exp)
    self.assertIsInstance(res, ccs.Expression)
    self.assertEqual( exp, res.__str__() )

  def test_associativity(self):
    exp = "5 - 2 - 1"
    res = ccs.parse(exp)
    self.assertIsInstance( res, ccs.Expression )
    self.assertEqual( 2, res.eval() )
    exp = "5 - +(+2 - 1)"
    res = ccs.parse(exp)
    self.assertIsInstance( res, ccs.Expression )
    self.assertEqual( 4, res.eval() )

  def test_in(self):
    exp = "5 # [3.0, 5]"
    res = ccs.parse(exp)
    self.assertIsInstance( res, ccs.Expression )
    self.assertTrue( res.eval() )
    exp = "5 # [3.0, 4]"
    res = ccs.parse(exp)
    self.assertIsInstance( res, ccs.Expression )
    self.assertFalse( res.eval() )

  def test_boolean(self):
    exp = "true"
    res = ccs.parse(exp)
    self.assertIsInstance( res, ccs.Literal )
    self.assertEqual( True, res.eval() )
    self.assertEqual( "true", res.__str__() )
    exp = "false"
    res = ccs.parse(exp)
    self.assertIsInstance( res, ccs.Literal )
    self.assertEqual( False, res.eval() )
    self.assertEqual( "false", res.__str__() )

  def test_none(self):
    exp = "none"
    res = ccs.parse(exp)
    self.assertIsInstance( res, ccs.Literal )
    self.assertIsNone( res.eval() )
    self.assertEqual( "none", res.__str__() )


if __name__ == '__main__':
    unittest.main()
