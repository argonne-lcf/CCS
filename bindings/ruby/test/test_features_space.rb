[ '../lib', 'lib' ].each { |d| $:.unshift(d) if File::directory?(d) }
require 'minitest/autorun'
require 'cconfigspace'

class CConfigSpaceTestFeaturesSpace < Minitest::Test
  def setup
    CCS.init
  end

  def test_create
    cs = CCS::FeaturesSpace::new(name: "space")
    assert_equal( :CCS_FEATURES_SPACE, cs.object_type )
    assert_equal( "space", cs.name )
    assert_equal( 0, cs.num_hyperparameters )
    h1 = CCS::NumericalHyperparameter::new
    h2 = CCS::NumericalHyperparameter::new
    h3 = CCS::NumericalHyperparameter::new
    cs.add_hyperparameter(h1)
    cs.add_hyperparameters([h2, h3])
    assert_equal( 3, cs.num_hyperparameters )
    assert_equal( h1, cs.hyperparameter(0) )
    assert_equal( h2, cs.hyperparameter(1) )
    assert_equal( h3, cs.hyperparameter(2) )
    assert_equal( [h1, h2, h3], cs.hyperparameters )
    assert_equal( h2, cs.hyperparameter_by_name(h2.name) )
  end

end
