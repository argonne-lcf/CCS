[ '../lib', 'lib' ].each { |d| $:.unshift(d) if File::directory?(d) }
require 'minitest/autorun'
require 'cconfigspace'

class CConfigSpaceTestEvaluation < Minitest::Test
  def setup
    CCS.init
  end

  def test_create
    cs = CCS::ConfigurationSpace::new(name: "cspace")
    h1 = CCS::NumericalParameter::Float.new
    h2 = CCS::NumericalParameter::Float.new
    h3 = CCS::NumericalParameter::Float.new
    cs.add_parameters [h1, h2, h3]
    os = CCS::ObjectiveSpace::new(name: "ospace")
    v1 = CCS::NumericalParameter::Float.new
    v2 = CCS::NumericalParameter::Float.new
    os.add_parameters [v1, v2]
    e1 = CCS::Expression::Variable::new(parameter: v1)
    e2 = CCS::Expression::Variable::new(parameter: v2)
    os.add_objectives( { e1 => :CCS_OBJECTIVE_TYPE_MAXIMIZE, e2 => :CCS_OBJECTIVE_TYPE_MINIMIZE } )
    ev1 = CCS::Evaluation::new(objective_space: os, configuration: cs.sample)
    ev1.set_value(0, 0.5)
    ev1.set_value(v2, 0.6)
    assert_equal( [0.5, 0.6], ev1.values )
    assert_equal( [0.5, 0.6], ev1.objective_values )
    assert( ev1.check )
    assert( os.check_values(ev1.values) )
    ev2 = CCS::Evaluation::new(objective_space: os, configuration: cs.sample, values: [0.5, 0.6])
    assert_equal( [0.5, 0.6], ev2.values )
    assert_equal( [0.5, 0.6], ev2.objective_values )
    assert_equal( 0, ev1 <=> ev2 )
    assert_equal( :CCS_COMPARISON_EQUIVALENT, ev1.compare(ev2) )
    ev3 = CCS::Evaluation::new(objective_space: os, configuration: cs.sample, values: [0.6, 0.5])
    assert_equal( [0.6, 0.5], ev3.objective_values )
    assert_equal(  1, ev1 <=> ev3 )
    assert_equal( :CCS_COMPARISON_WORSE, ev1.compare(ev3) )
    assert_equal( -1, ev3 <=> ev1 )
    assert_equal( :CCS_COMPARISON_BETTER, ev3.compare(ev1) )
    ev4 = CCS::Evaluation::new(objective_space: os, configuration: cs.sample, values: [0.6, 0.7])
    assert_equal( [0.6, 0.7], ev4.objective_values )
    assert_nil( ev1 <=> ev4 )
    assert_equal( :CCS_COMPARISON_NOT_COMPARABLE, ev1.compare(ev4) )
    assert_nil( ev4 <=> ev1 )
    assert_equal( :CCS_COMPARISON_NOT_COMPARABLE, ev4.compare(ev1) )
  end

  def test_serialize
    cs = CCS::ConfigurationSpace::new(name: "cspace")
    h1 = CCS::NumericalParameter::Float.new
    h2 = CCS::NumericalParameter::Float.new
    h3 = CCS::NumericalParameter::Float.new
    cs.add_parameters [h1, h2, h3]
    os = CCS::ObjectiveSpace::new(name: "ospace")
    v1 = CCS::NumericalParameter::Float.new
    v2 = CCS::NumericalParameter::Float.new
    os.add_parameters [v1, v2]
    e1 = CCS::Expression::Variable::new(parameter: v1)
    e2 = CCS::Expression::Variable::new(parameter: v2)
    os.add_objectives( { e1 => :CCS_OBJECTIVE_TYPE_MAXIMIZE, e2 => :CCS_OBJECTIVE_TYPE_MINIMIZE } )
    evref = CCS::Evaluation::new(objective_space: os, configuration: cs.sample, values: [0.5, 0.6])
    buff = evref.serialize
    handle_map = CCS::Map::new()
    handle_map[cs] = cs
    handle_map[os] = os
    ev = CCS::deserialize(buffer: buff, handle_map: handle_map)
    assert_equal( cs.handle, ev.configuration.configuration_space.handle )
    assert_equal( os.handle, ev.objective_space.handle )
    assert_equal( [0.5, 0.6], ev.values )
    assert_equal( [0.5, 0.6], ev.objective_values )
  end

end
