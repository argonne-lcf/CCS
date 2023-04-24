[ '../lib', 'lib' ].each { |d| $:.unshift(d) if File::directory?(d) }
require 'minitest/autorun'
require 'cconfigspace'

class CConfigSpaceTestFeaturesSpace < Minitest::Test
  def setup
    CCS.init
  end

  def test_create
    cs = CCS::FeaturesSpace::new(name: "space")
    assert_equal( :CCS_OBJECT_TYPE_FEATURES_SPACE, cs.object_type )
    assert_equal( "space", cs.name )
    assert_equal( 0, cs.num_parameters )
    h1 = CCS::NumericalParameter::Float.new
    h2 = CCS::NumericalParameter::Float.new
    h3 = CCS::NumericalParameter::Float.new
    cs.add_parameter(h1)
    cs.add_parameters([h2, h3])
    assert_equal( 3, cs.num_parameters )
    assert_equal( h1, cs.parameter(0) )
    assert_equal( h2, cs.parameter(1) )
    assert_equal( h3, cs.parameter(2) )
    assert_equal( [h1, h2, h3], cs.parameters )
    assert_equal( h2, cs.parameter_by_name(h2.name) )
  end

end
