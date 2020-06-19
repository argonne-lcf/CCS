[ '../lib', 'lib' ].each { |d| $:.unshift(d) if File::directory?(d) }
require 'minitest/autorun'
require 'cconfigspace'

class CConfigSpaceTestEvaluation < Minitest::Test
  def setup
    CCS.init
  end

  def test_create
    cs = CCS::ConfigurationSpace::new(name: "cspace")
    h1 = CCS::NumericalHyperparameter::new
    h2 = CCS::NumericalHyperparameter::new
    h3 = CCS::NumericalHyperparameter::new
    cs.add_hyperparameters [h1, h2, h3]
    os = CCS::ObjectiveSpace::new(name: "ospace")
    v1 = CCS::NumericalHyperparameter::new
    v2 = CCS::NumericalHyperparameter::new
    os.add_hyperparameters [v1, v2]
    e1 = CCS::Variable::new(hyperparameter: v1)
    e2 = CCS::Variable::new(hyperparameter: v2)
    os.add_objectives( { e1 => :CCS_MAXIMIZE, e2 => :CCS_MINIMIZE } )
    ev1 = CCS::Evaluation::new(objective_space: os, configuration: cs.sample)
    ev1.set_value(0, 0.5)
    ev1.set_value(v2, 0.6)
    assert_equal( [0.5, 0.6], ev1.values )
    assert_equal( [0.5, 0.6], ev1.objective_values )
    ev2 = CCS::Evaluation::new(objective_space: os, configuration: cs.sample, values: [0.5, 0.6])
    assert_equal( [0.5, 0.6], ev2.values )
    assert_equal( [0.5, 0.6], ev2.objective_values )
    assert_equal( 0, ev1 <=> ev2 )
    assert_equal( :CCS_EQUIVALENT, ev1.cmp(ev2) )
    ev3 = CCS::Evaluation::new(objective_space: os, configuration: cs.sample, values: [0.6, 0.5])
    assert_equal( [0.6, 0.5], ev3.objective_values )
    assert_equal(  1, ev1 <=> ev3 )
    assert_equal( :CCS_WORSE, ev1.cmp(ev3) )
    assert_equal( -1, ev3 <=> ev1 )
    assert_equal( :CCS_BETTER, ev3.cmp(ev1) )
    ev4 = CCS::Evaluation::new(objective_space: os, configuration: cs.sample, values: [0.6, 0.7])
    assert_equal( [0.6, 0.7], ev4.objective_values )
    assert_nil( ev1 <=> ev4 )
    assert_equal( :CCS_NOT_COMPARABLE, ev1.cmp(ev4) )
    assert_nil( ev4 <=> ev1 )
    assert_equal( :CCS_NOT_COMPARABLE, ev4.cmp(ev1) )
  end

end
