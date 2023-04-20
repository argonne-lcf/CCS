import unittest
import sys
sys.path.insert(1, '.')
sys.path.insert(1, '..')
import cconfigspace as ccs

class TestParameter(unittest.TestCase):

  def test_from_handle_discrete(self):
    values = [0, 1.5, 2, 7.2]
    h = ccs.DiscreteParameter(values = values)
    h2 = ccs.Object.from_handle(h.handle)
    self.assertEqual( h.__class__, h2.__class__ )
    self.assertEqual( h.handle, h2.handle )

  def test_discrete(self):
    values = [0.2, 1.5, 2, 7.2]
    h = ccs.DiscreteParameter(values = values)
    self.assertEqual( ccs.ObjectType.PARAMETER, h.object_type )
    self.assertEqual( ccs.ParameterType.DISCRETE, h.type )
    self.assertTrue( h.name[:5] == "param" )
    self.assertIsNone( h.user_data )
    self.assertEqual( 0.2, h.default_value )
    self.assertFalse( h.check_value("foo") )
    v = h.sample()
    self.assertTrue( v in values )
    vals = h.samples(100)
    for v in vals:
      self.assertTrue( v in values )

  def test_serialize_discrete(self):
    values = [0.2, 1.5, 2, 7.2]
    href = ccs.DiscreteParameter(values = values)
    buff = href.serialize()
    h = ccs.Object.deserialize(buffer = buff)
    self.assertEqual( ccs.ObjectType.PARAMETER, h.object_type )
    self.assertEqual( ccs.ParameterType.DISCRETE, h.type )
    self.assertTrue( h.name[:5] == "param" )
    self.assertIsNone( h.user_data )
    self.assertEqual( 0.2, h.default_value )
    self.assertFalse( h.check_value("foo") )
    v = h.sample()
    self.assertTrue( v in values )
    vals = h.samples(100)
    for v in vals:
      self.assertTrue( v in values )

  def test_ordinal_compare(self):
    values = ["foo", 2, 3.0]
    h = ccs.OrdinalParameter(values = values)
    self.assertEqual( 0, h.compare("foo", "foo") )
    self.assertEqual( -1, h.compare("foo", 2) )
    self.assertEqual( -1, h.compare("foo", 3.0) )
    self.assertEqual( 1, h.compare(2, "foo") )
    self.assertEqual( 0, h.compare(2, 2) )
    self.assertEqual( -1, h.compare(2, 3.0) )
    self.assertEqual( 1, h.compare(3.0, "foo") )
    self.assertEqual( 1, h.compare(3.0, 2) )
    self.assertEqual( 0, h.compare(3.0, 3.0) )
    self.assertRaises( ccs.Error, h.compare, 4.0, "foo" )

  def test_from_handle_ordinal(self):
    values = ["foo", 2, 3.0]
    h = ccs.OrdinalParameter(values = values)
    h2 = ccs.Object.from_handle(h.handle)
    self.assertEqual( h.__class__, h2.__class__ )
    self.assertEqual( h.handle, h2.handle )

  def test_ordinal(self):
    values = ["foo", 2, 3.0]
    h = ccs.OrdinalParameter(values = values)
    self.assertEqual( ccs.ObjectType.PARAMETER, h.object_type )
    self.assertEqual( ccs.ParameterType.ORDINAL, h.type )
    self.assertTrue( h.name[:5] == "param" )
    self.assertIsNone( h.user_data )
    self.assertEqual( "foo", h.default_value )
    self.assertEqual( ccs.DistributionType.UNIFORM, h.default_distribution.type )
    self.assertEqual( values, h.values )
    for v in values:
      self.assertTrue( h.check_value(v) )
    self.assertFalse( h.check_value("bar") )
    v = h.sample()
    self.assertTrue( v in values )
    vals = h.samples(100)
    for v in vals:
      self.assertTrue( v in values )

  def test_serialize_ordinal(self):
    values = ["foo", 2, 3.0]
    href = ccs.OrdinalParameter(values = values)
    buff = href.serialize()
    h = ccs.Object.deserialize(buffer = buff)
    self.assertEqual( ccs.ObjectType.PARAMETER, h.object_type )
    self.assertEqual( ccs.ParameterType.ORDINAL, h.type )
    self.assertTrue( h.name[:5] == "param" )
    self.assertIsNone( h.user_data )
    self.assertEqual( "foo", h.default_value )
    self.assertEqual( ccs.DistributionType.UNIFORM, h.default_distribution.type )
    self.assertEqual( values, h.values )
    for v in values:
      self.assertTrue( h.check_value(v) )
    self.assertFalse( h.check_value("bar") )
    v = h.sample()
    self.assertTrue( v in values )
    vals = h.samples(100)
    for v in vals:
      self.assertTrue( v in values )

  def test_from_handle_categorical(self):
    values = ["foo", 2, 3.0]
    h = ccs.CategoricalParameter(values = values)
    h2 = ccs.Object.from_handle(h.handle)
    self.assertEqual( h.__class__, h2.__class__ )
    self.assertEqual( h.handle, h2.handle )

  def test_categorical(self):
    values = ["foo", 2, 3.0]
    h = ccs.CategoricalParameter(values = values)
    self.assertEqual( ccs.ObjectType.PARAMETER, h.object_type )
    self.assertEqual( ccs.ParameterType.CATEGORICAL, h.type )
    self.assertTrue( h.name[:5] == "param" )
    self.assertIsNone( h.user_data )
    h.user_data = {'foo': ['bar', 'baz']}
    self.assertEqual({'foo': ['bar', 'baz']}, h.user_data)
    self.assertEqual( "foo", h.default_value )
    self.assertEqual( ccs.DistributionType.UNIFORM, h.default_distribution.type )
    self.assertEqual( values, h.values )
    for v in values:
      self.assertTrue( h.check_value(v) )
    self.assertFalse( h.check_value("bar") )
    v = h.sample()
    self.assertTrue( v in values )
    vals = h.samples(100)
    for v in vals:
      self.assertTrue( v in values )

  def test_serialize_categorical(self):
    values = ["foo", 2, 3.0]
    href = ccs.CategoricalParameter(values = values)
    href.user_data = {'foo': ['bar', 'baz']}
    buff = href.serialize()
    h = ccs.Object.deserialize(buffer = buff)
    self.assertEqual( ccs.ObjectType.PARAMETER, h.object_type )
    self.assertEqual( ccs.ParameterType.CATEGORICAL, h.type )
    self.assertTrue( h.name[:5] == "param" )
    self.assertEqual( {'foo': ['bar', 'baz']}, h.user_data )
    self.assertEqual( "foo", h.default_value )
    self.assertEqual( ccs.DistributionType.UNIFORM, h.default_distribution.type )
    self.assertEqual( values, h.values )
    for v in values:
      self.assertTrue( h.check_value(v) )
    self.assertFalse( h.check_value("bar") )
    v = h.sample()
    self.assertTrue( v in values )
    vals = h.samples(100)
    for v in vals:
      self.assertTrue( v in values )

  def test_from_handle_numerical(self):
    h = ccs.NumericalParameter()
    h2 = ccs.Object.from_handle(h.handle)
    self.assertEqual( h.__class__, h2.__class__ )
    self.assertEqual( h.handle, h2.handle )

  def test_numerical(self):
    h = ccs.NumericalParameter()
    self.assertEqual( ccs.ObjectType.PARAMETER, h.object_type )
    self.assertEqual( ccs.ParameterType.NUMERICAL, h.type )
    self.assertTrue( h.name[:5] == "param" )
    self.assertIsNone( h.user_data )
    self.assertEqual( 0.0, h.default_value )
    self.assertEqual( ccs.DistributionType.UNIFORM, h.default_distribution.type )
    self.assertEqual( ccs.NumericType.FLOAT, h.data_type )
    self.assertEqual( 0.0, h.lower )
    self.assertEqual( 1.0, h.upper )
    self.assertEqual( 0.0, h.quantization )
    self.assertTrue( h.check_value(0.5) )
    self.assertFalse( h.check_value(1.5) )
    v = h.sample()
    self.assertIsInstance( v, float )
    self.assertTrue( v >= 0.0 and v < 1.0 )
    vals = h.samples(100)
    for v in vals:
      self.assertIsInstance( v, float )
      self.assertTrue( v >= 0.0 and v < 1.0 )

  def test_serialize_numerical(self):
    href = ccs.NumericalParameter()
    buff = href.serialize()
    h = ccs.Object.deserialize(buffer = buff)
    self.assertEqual( ccs.ObjectType.PARAMETER, h.object_type )
    self.assertEqual( ccs.ParameterType.NUMERICAL, h.type )
    self.assertTrue( h.name[:5] == "param" )
    self.assertIsNone( h.user_data )
    self.assertEqual( 0.0, h.default_value )
    self.assertEqual( ccs.DistributionType.UNIFORM, h.default_distribution.type )
    self.assertEqual( ccs.NumericType.FLOAT, h.data_type )
    self.assertEqual( 0.0, h.lower )
    self.assertEqual( 1.0, h.upper )
    self.assertEqual( 0.0, h.quantization )
    self.assertTrue( h.check_value(0.5) )
    self.assertFalse( h.check_value(1.5) )
    v = h.sample()
    self.assertIsInstance( v, float )
    self.assertTrue( v >= 0.0 and v < 1.0 )
    vals = h.samples(100)
    for v in vals:
      self.assertIsInstance( v, float )
      self.assertTrue( v >= 0.0 and v < 1.0 )

  def test_numerical_float(self):
    h = ccs.NumericalParameter.float(lower = 0.0, upper = 1.0)
    self.assertEqual( ccs.ObjectType.PARAMETER, h.object_type )
    self.assertEqual( ccs.ParameterType.NUMERICAL, h.type )
    self.assertTrue( h.name[:5] == "param" )
    self.assertIsNone( h.user_data )
    self.assertEqual( 0.0, h.default_value )
    self.assertEqual( ccs.DistributionType.UNIFORM, h.default_distribution.type )
    self.assertEqual( ccs.NumericType.FLOAT, h.data_type )
    self.assertEqual( 0.0, h.lower )
    self.assertEqual( 1.0, h.upper )
    self.assertEqual( 0.0, h.quantization )
    self.assertTrue( h.check_value(0.5) )
    self.assertFalse( h.check_value(1.5) )
    v = h.sample()
    self.assertIsInstance( v, float )
    self.assertTrue( v >= 0.0 and v < 1.0 )
    vals = h.samples(100)
    for v in vals:
      self.assertIsInstance( v, float )
      self.assertTrue( v >= 0.0 and v < 1.0 )

  def test_numerical_int(self):
    h = ccs.NumericalParameter.int(lower = 0, upper = 100)
    self.assertEqual( ccs.ObjectType.PARAMETER, h.object_type )
    self.assertEqual( ccs.ParameterType.NUMERICAL, h.type )
    self.assertTrue( h.name[:5] == "param" )
    self.assertIsNone( h.user_data )
    self.assertEqual( 0, h.default_value )
    self.assertEqual( ccs.DistributionType.UNIFORM, h.default_distribution.type )
    self.assertEqual( ccs.NumericType.INT, h.data_type )
    self.assertEqual( 0, h.lower )
    self.assertEqual( 100, h.upper )
    self.assertEqual( 0, h.quantization )
    self.assertTrue( h.check_value(50) )
    self.assertFalse( h.check_value(150) )
    v = h.sample()
    self.assertIsInstance( v, int )
    self.assertTrue( v >= 0 and v < 100 )
    vals = h.samples(100)
    for v in vals:
      self.assertIsInstance( v, int )
      self.assertTrue( v >= 0 and v < 100 )

  def test_string(self):
    h = ccs.StringParameter()
    self.assertEqual( ccs.ObjectType.PARAMETER, h.object_type )
    self.assertEqual( ccs.ParameterType.STRING, h.type )
    self.assertTrue( h.name[:5] == "param" )
    self.assertIsNone( h.user_data )
    with self.assertRaises( ccs.Error ):
      h.sample()

  def test_serialize_string(self):
    href = ccs.StringParameter()
    buff = href.serialize()
    h = ccs.Object.deserialize(buffer = buff)
    self.assertEqual( ccs.ObjectType.PARAMETER, h.object_type )
    self.assertEqual( ccs.ParameterType.STRING, h.type )
    self.assertTrue( h.name[:5] == "param" )
    self.assertIsNone( h.user_data )
    with self.assertRaises( ccs.Error ):
      h.sample()

if __name__ == '__main__':
    unittest.main()
