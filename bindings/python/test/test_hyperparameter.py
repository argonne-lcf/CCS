import unittest
import sys
sys.path.insert(1, '.')
sys.path.insert(1, '..')
import cconfigspace as ccs

class TestHyperparameter(unittest.TestCase):

  def test_from_handle_discrete(self):
    values = [0, 1.5, 2, 7.2]
    h = ccs.DiscreteHyperparameter(values = values)
    h2 = ccs.Object.from_handle(h.handle)
    self.assertEqual( h.__class__, h2.__class__ )
    self.assertEqual( h.handle, h2.handle )

  def test_discrete(self):
    values = [0.2, 1.5, 2, 7.2]
    h = ccs.DiscreteHyperparameter(values = values)
    self.assertEqual( ccs.HYPERPARAMETER, h.object_type )
    self.assertEqual( ccs.HYPERPARAMETER_TYPE_DISCRETE, h.type )
    self.assertTrue( h.name[:5] == "param" )
    self.assertIsNone( h.user_data.value )
    self.assertEqual( 0.2, h.default_value )
    self.assertFalse( h.check_value("foo") )
    v = h.sample()
    self.assertTrue( v in values )
    vals = h.samples(100)
    for v in vals:
      self.assertTrue( v in values )

  def test_serialize_discrete(self):
    values = [0.2, 1.5, 2, 7.2]
    href = ccs.DiscreteHyperparameter(values = values)
    buff = href.serialize()
    h = ccs.Object.deserialize(buffer = buff)
    self.assertEqual( ccs.HYPERPARAMETER, h.object_type )
    self.assertEqual( ccs.HYPERPARAMETER_TYPE_DISCRETE, h.type )
    self.assertTrue( h.name[:5] == "param" )
    self.assertIsNone( h.user_data.value )
    self.assertEqual( 0.2, h.default_value )
    self.assertFalse( h.check_value("foo") )
    v = h.sample()
    self.assertTrue( v in values )
    vals = h.samples(100)
    for v in vals:
      self.assertTrue( v in values )

  def test_ordinal_compare(self):
    values = ["foo", 2, 3.0]
    h = ccs.OrdinalHyperparameter(values = values)
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
    h = ccs.OrdinalHyperparameter(values = values)
    h2 = ccs.Object.from_handle(h.handle)
    self.assertEqual( h.__class__, h2.__class__ )
    self.assertEqual( h.handle, h2.handle )

  def test_ordinal(self):
    values = ["foo", 2, 3.0]
    h = ccs.OrdinalHyperparameter(values = values)
    self.assertEqual( ccs.HYPERPARAMETER, h.object_type )
    self.assertEqual( ccs.HYPERPARAMETER_TYPE_ORDINAL, h.type )
    self.assertTrue( h.name[:5] == "param" )
    self.assertIsNone( h.user_data.value )
    self.assertEqual( "foo", h.default_value )
    self.assertEqual( ccs.UNIFORM, h.default_distribution.type )
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
    href = ccs.OrdinalHyperparameter(values = values)
    buff = href.serialize()
    h = ccs.Object.deserialize(buffer = buff)
    self.assertEqual( ccs.HYPERPARAMETER, h.object_type )
    self.assertEqual( ccs.HYPERPARAMETER_TYPE_ORDINAL, h.type )
    self.assertTrue( h.name[:5] == "param" )
    self.assertIsNone( h.user_data.value )
    self.assertEqual( "foo", h.default_value )
    self.assertEqual( ccs.UNIFORM, h.default_distribution.type )
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
    h = ccs.CategoricalHyperparameter(values = values)
    h2 = ccs.Object.from_handle(h.handle)
    self.assertEqual( h.__class__, h2.__class__ )
    self.assertEqual( h.handle, h2.handle )

  def test_categorical(self):
    values = ["foo", 2, 3.0]
    h = ccs.CategoricalHyperparameter(values = values)
    self.assertEqual( ccs.HYPERPARAMETER, h.object_type )
    self.assertEqual( ccs.HYPERPARAMETER_TYPE_CATEGORICAL, h.type )
    self.assertTrue( h.name[:5] == "param" )
    self.assertIsNone( h.user_data.value )
    self.assertEqual( "foo", h.default_value )
    self.assertEqual( ccs.UNIFORM, h.default_distribution.type )
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
    href = ccs.CategoricalHyperparameter(values = values)
    buff = href.serialize()
    h = ccs.Object.deserialize(buffer = buff)
    self.assertEqual( ccs.HYPERPARAMETER, h.object_type )
    self.assertEqual( ccs.HYPERPARAMETER_TYPE_CATEGORICAL, h.type )
    self.assertTrue( h.name[:5] == "param" )
    self.assertIsNone( h.user_data.value )
    self.assertEqual( "foo", h.default_value )
    self.assertEqual( ccs.UNIFORM, h.default_distribution.type )
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
    h = ccs.NumericalHyperparameter()
    h2 = ccs.Object.from_handle(h.handle)
    self.assertEqual( h.__class__, h2.__class__ )
    self.assertEqual( h.handle, h2.handle )

  def test_numerical(self):
    h = ccs.NumericalHyperparameter()
    self.assertEqual( ccs.HYPERPARAMETER, h.object_type )
    self.assertEqual( ccs.HYPERPARAMETER_TYPE_NUMERICAL, h.type )
    self.assertTrue( h.name[:5] == "param" )
    self.assertIsNone( h.user_data.value )
    self.assertEqual( 0.0, h.default_value )
    self.assertEqual( ccs.UNIFORM, h.default_distribution.type )
    self.assertEqual( ccs.NUM_FLOAT, h.data_type )
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
    href = ccs.NumericalHyperparameter()
    buff = href.serialize()
    h = ccs.Object.deserialize(buffer = buff)
    self.assertEqual( ccs.HYPERPARAMETER, h.object_type )
    self.assertEqual( ccs.HYPERPARAMETER_TYPE_NUMERICAL, h.type )
    self.assertTrue( h.name[:5] == "param" )
    self.assertIsNone( h.user_data.value )
    self.assertEqual( 0.0, h.default_value )
    self.assertEqual( ccs.UNIFORM, h.default_distribution.type )
    self.assertEqual( ccs.NUM_FLOAT, h.data_type )
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
    h = ccs.NumericalHyperparameter.float(lower = 0.0, upper = 1.0)
    self.assertEqual( ccs.HYPERPARAMETER, h.object_type )
    self.assertEqual( ccs.HYPERPARAMETER_TYPE_NUMERICAL, h.type )
    self.assertTrue( h.name[:5] == "param" )
    self.assertIsNone( h.user_data.value )
    self.assertEqual( 0.0, h.default_value )
    self.assertEqual( ccs.UNIFORM, h.default_distribution.type )
    self.assertEqual( ccs.NUM_FLOAT, h.data_type )
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
    h = ccs.NumericalHyperparameter.int(lower = 0, upper = 100)
    self.assertEqual( ccs.HYPERPARAMETER, h.object_type )
    self.assertEqual( ccs.HYPERPARAMETER_TYPE_NUMERICAL, h.type )
    self.assertTrue( h.name[:5] == "param" )
    self.assertIsNone( h.user_data.value )
    self.assertEqual( 0, h.default_value )
    self.assertEqual( ccs.UNIFORM, h.default_distribution.type )
    self.assertEqual( ccs.NUM_INTEGER, h.data_type )
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
    h = ccs.StringHyperparameter()
    self.assertEqual( ccs.HYPERPARAMETER, h.object_type )
    self.assertEqual( ccs.HYPERPARAMETER_TYPE_STRING, h.type )
    self.assertTrue( h.name[:5] == "param" )
    self.assertIsNone( h.user_data.value )
    with self.assertRaises( ccs.Error ):
      h.sample()

  def test_serialize_string(self):
    href = ccs.StringHyperparameter()
    buff = href.serialize()
    h = ccs.Object.deserialize(buffer = buff)
    self.assertEqual( ccs.HYPERPARAMETER, h.object_type )
    self.assertEqual( ccs.HYPERPARAMETER_TYPE_STRING, h.type )
    self.assertTrue( h.name[:5] == "param" )
    self.assertIsNone( h.user_data.value )
    with self.assertRaises( ccs.Error ):
      h.sample()

if __name__ == '__main__':
    unittest.main()
