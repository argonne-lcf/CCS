import unittest
import sys
import random
import pickle
sys.path.insert(1, '.')
sys.path.insert(1, '..')
import cconfigspace as ccs

class TestExpression(unittest.TestCase):

  def test_create(self):
    e = ccs.Expression.Add(left = 1.0, right = 2.0)
    self.assertEqual( ccs.ObjectType.EXPRESSION, e.object_type )
    self.assertEqual( ccs.ExpressionType.ADD, e.type )
    self.assertEqual( 2, e.num_nodes )
    nodes = e.nodes
    self.assertEqual( 2, len(nodes) )
    for n in nodes:
      self.assertIsInstance( n, ccs.Expression.Literal )
      self.assertEqual( ccs.ObjectType.EXPRESSION, n.object_type )
      self.assertEqual( ccs.ExpressionType.LITERAL, n.type )
    self.assertEqual( 1.0, nodes[0].value )
    self.assertEqual( 2.0, nodes[1].value )
    self.assertEqual( 3.0, e.eval() )
    self.assertEqual( (), e.parameters )

  def test_to_s(self):
    e = ccs.Expression.Add(left = 1.0, right = 2.0)
    self.assertEqual( "1.0 + 2.0", str(e) )
    e2 = ccs.Expression.Multiply(left = 5.0, right = e)
    self.assertEqual( "5.0 * (1.0 + 2.0)", str(e2) )

  def test_literal(self):
    e = ccs.Expression.Literal(value = 15)
    self.assertEqual( "15" , str(e) )
    e = ccs.Expression.Literal(value = None)
    self.assertEqual( "none" , str(e) )

  def test_variable(self):
    h = ccs.NumericalParameter.Float()
    e = ccs.Expression.Variable(parameter = h)
    self.assertEqual( h.name , str(e) )

  def test_list(self):
    e = ccs.ExpressionList(values = ["foo", 1, 2.0])
    self.assertEqual( "[ 'foo', 1, 2.0 ]", str(e) )
    self.assertEqual( "foo", e.eval(0) )
    self.assertEqual( 1, e.eval(1) )
    self.assertEqual( 2.0, e.eval(2) )
    h = ccs.NumericalParameter.Float(name = "test")
    e2 = ccs.Expression.In(left = h, right = e)
    self.assertEqual( "test # [ 'foo', 1, 2.0 ]", str(e2) )

  def test_unary(self):
    e = ccs.Expression.Not(node = True)
    self.assertEqual( "!true", str(e) )
    self.assertFalse( e.eval() )

  def test_binary(self):
    e = ccs.Expression.Or(left = True, right = False)
    self.assertEqual( "true || false", str(e) )
    self.assertTrue( e.eval() )

  def test_user_defined(self):
    def my_rand(expr, limit):
      return expr.expression_data.randrange(limit)

    def my_serialize(expr):
      return pickle.dumps(expr.expression_data)

    def my_deserialize(state):
      return pickle.loads(state)

    def get_vector_data(otype, name):
      self.assertEqual( ccs.ObjectType.EXPRESSION, otype )
      self.assertEqual( "rand", name)
      return (ccs.Expression.UserDefined.get_vector(evaluate = my_rand, serialize = my_serialize, deserialize = my_deserialize), None)

    limit = 10
    e = ccs.Expression.UserDefined(name = "rand", nodes = [limit], expression_data = random.Random(), evaluate = my_rand, serialize = my_serialize, deserialize = my_deserialize)
    self.assertEqual( "rand(10)", str(e) )
    evals = [ e.eval() for i in range(100) ]
    self.assertTrue( all(i >= 0 and i < limit for i in evals) )

    buff = e.serialize()
    e_copy = ccs.deserialize(buffer = buff, vector_callback = get_vector_data)

    self.assertTrue( all(e.eval() == e_copy.eval() for i in range(100)) )

if __name__ == '__main__':
    unittest.main()
