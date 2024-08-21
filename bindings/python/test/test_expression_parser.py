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
    self.assertIsInstance(res, ccs.Expression.Multiply)
    self.assertEqual( exp, res.__str__() )

  def test_associativity(self):
    exp = "5 - 2 - 1"
    res = ccs.parse(exp)
    self.assertIsInstance( res, ccs.Expression.Substract )
    self.assertEqual( 2, res.eval() )
    exp = "5 - +(+2 - 1)"
    res = ccs.parse(exp)
    self.assertIsInstance( res, ccs.Expression.Substract )
    self.assertEqual( 4, res.eval() )

  def test_in(self):
    exp = "5 # [3.0, 5]"
    res = ccs.parse(exp)
    self.assertIsInstance( res, ccs.Expression.In )
    self.assertTrue( res.eval() )
    exp = "5 # [3.0, 4]"
    res = ccs.parse(exp)
    self.assertIsInstance( res, ccs.Expression.In )
    self.assertFalse( res.eval() )

  def test_boolean(self):
    exp = "true"
    res = ccs.parse(exp)
    self.assertIsInstance( res, ccs.Expression.Literal )
    self.assertEqual( True, res.eval() )
    self.assertEqual( "true", res.__str__() )
    exp = "false"
    res = ccs.parse(exp)
    self.assertIsInstance( res, ccs.Expression.Literal )
    self.assertEqual( False, res.eval() )
    self.assertEqual( "false", res.__str__() )

  def test_none(self):
    exp = "none"
    res = ccs.parse(exp)
    self.assertIsInstance( res, ccs.Expression.Literal )
    self.assertIsNone( res.eval() )
    self.assertEqual( "none", res.__str__() )

  def test_function(self):
    def func(a, b):
      return a * b
    l = locals()

    exp = "func(3, 4)"
    res = ccs.parse(exp, binding = l)
    self.assertIsInstance( res, ccs.Expression.UserDefined )
    self.assertEqual( 12, res.eval() )
    self.assertEqual( "func(3, 4)", res.__str__() )

    def get_vector_data(otype, name):
      self.assertEqual( ccs.ObjectType.EXPRESSION, otype )
      return ccs.Expression.get_function_vector_data(name, binding = l)

    buff = res.serialize()
    res_copy = ccs.deserialize(buffer = buff, vector_callback = get_vector_data)
    self.assertEqual( "func(3, 4)", res_copy.__str__() )
    self.assertEqual( 12, res_copy.eval() )

if __name__ == '__main__':
    unittest.main()
