require 'minitest/autorun'
require_relative '../lib/cconfigspace'

class CConfigSpaceTestFeatureSpace < Minitest::Test
  def setup
    CCS.init
  end

  def test_create
    h1 = CCS::NumericalParameter::Float.new
    h2 = CCS::NumericalParameter::Float.new
    h3 = CCS::NumericalParameter::Float.new
    cs = CCS::FeatureSpace::new(name: "space", parameters: [h1, h2, h3])
    assert_equal( :CCS_OBJECT_TYPE_FEATURE_SPACE, cs.object_type )
    assert_equal( "space", cs.name )
    assert_equal( 3, cs.num_parameters )
    assert_equal( h1, cs.parameter(0) )
    assert_equal( h2, cs.parameter(1) )
    assert_equal( h3, cs.parameter(2) )
    assert_equal( [h1, h2, h3], cs.parameters )
    assert_equal( h2, cs.parameter_by_name(h2.name) )
  end

end
