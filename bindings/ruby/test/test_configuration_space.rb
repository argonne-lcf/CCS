[ '../lib', 'lib' ].each { |d| $:.unshift(d) if File::directory?(d) }
require 'minitest/autorun'
require 'cconfigspace'

class CConfigSpaceTestConfigurationSpace < Minitest::Test
  def setup
    CCS.init
  end

  def test_create
    cs = CCS::ConfigurationSpace::new(name: "space")
    assert_equal( :CCS_CONFIGURATION_SPACE, cs.object_type )
    assert_equal( "space", cs.name )
    assert( cs.rng.kind_of?(CCS::Rng) )
    assert_equal( 0, cs.num_hyperparameters )
    assert_equal( [], cs.conditions )
    assert_equal( [], cs.forbidden_clauses )
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
    cs.check(cs.default_configuration)
    cs.check(cs.sample)
    cs.check_values(cs.sample.values)
    cs.samples(100).each { |c|
      cs.check(cs.sample)
    }
  end

  def test_conditions
    h1 = CCS::NumericalHyperparameter::new(lower: -1.0, upper: 1.0, default: 0.0)
    h2 = CCS::NumericalHyperparameter::new(lower: -1.0, upper: 1.0)
    h3 = CCS::NumericalHyperparameter::new(lower: -1.0, upper: 1.0)
    cs = CCS::ConfigurationSpace::new(name: "space")
    cs.add_hyperparameters([h1, h2, h3])
    e1 = CCS::Expression::new(type: :CCS_LESS, nodes: [h2, 0.0])
    cs.set_condition(h3, e1)
    e2 = CCS::Expression::new(type: :CCS_LESS, nodes: [h3, 0.0])
    cs.set_condition(h1, e2)
    e3 = CCS::Expression::new(type: :CCS_LESS, nodes: [h1, 0.0])
    cs.add_forbidden_clause(e3)
    conditions = cs.conditions
    assert_equal( 3, conditions.length )
    assert_equal( e2.handle, conditions[0].handle )
    assert_nil( conditions[1] )
    assert_equal( e1.handle, conditions[2].handle )
    forbidden_clauses = cs.forbidden_clauses
    assert_equal( 1, forbidden_clauses.length )
    assert_equal( e3.handle, forbidden_clauses[0].handle )
  end
end
