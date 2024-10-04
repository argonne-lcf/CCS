require 'minitest/autorun'
require_relative '../lib/cconfigspace'

class CConfigSpaceTestObjectiveSpace < Minitest::Test
  def test_create
    h = CCS::NumericalParameter::Float.new
    cs = CCS::ConfigurationSpace::new(name: "cs", parameters: [h])
    h1 = CCS::NumericalParameter::Float.new
    h2 = CCS::NumericalParameter::Float.new
    h3 = CCS::NumericalParameter::Float.new
    e1 = CCS::Expression::Add.new(left: h1, right: h2)
    e2 = CCS::Expression::Variable.new(parameter: h3)
    os = CCS::ObjectiveSpace::new(name: "space", search_space: cs, parameters: [h1, h2, h3], objectives: [e1, e2], types: [:CCS_OBJECTIVE_TYPE_MINIMIZE, :CCS_OBJECTIVE_TYPE_MAXIMIZE])
    assert_equal( :CCS_OBJECT_TYPE_OBJECTIVE_SPACE, os.object_type )
    assert_equal( "space", os.name )
    assert_equal( os.search_space.handle, cs.handle)
    assert_equal( 3, os.num_parameters )
    assert_equal( h1, os.parameter(0) )
    assert_equal( h2, os.parameter(1) )
    assert_equal( h3, os.parameter(2) )
    assert_equal( [h1, h2, h3], os.parameters )
    assert_equal( h2, os.parameter_by_name(h2.name) )
    assert_equal( 2, os.objectives.size )
    objs = os.objectives
    assert_equal( e1.handle, objs[0][0].handle )
    assert_equal( :CCS_OBJECTIVE_TYPE_MINIMIZE, objs[0][1] )
    assert_equal( e2.handle, objs[1][0].handle )
    assert_equal( :CCS_OBJECTIVE_TYPE_MAXIMIZE, objs[1][1] )
  end

  def test_features
    f = CCS::NumericalParameter::Float.new
    fs = CCS::FeatureSpace::new(parameters: [f])
    p = CCS::NumericalParameter::Float.new
    cs = CCS::ConfigurationSpace::new(name: "cs", parameters: [p], feature_space: fs)
    h = CCS::NumericalParameter::Float.new
    e1 = CCS::Expression::Add.new(left: f, right: p)
    e2 = CCS::Expression::Variable.new(parameter: h)
    os = CCS::ObjectiveSpace::new(name: "space", search_space: cs, parameters: [h], objectives: [e1, e2], types: [:CCS_OBJECTIVE_TYPE_MINIMIZE, :CCS_OBJECTIVE_TYPE_MAXIMIZE])

    features = CCS::Features::new(feature_space: fs, values: [0.5])
    configuration = CCS::Configuration::new(configuration_space: cs, values: [0.25], features: features)
    evaluation = CCS::Evaluation::new(objective_space: os, values: [-0.25], configuration: configuration)
    assert_equal([0.25+0.5, -0.25], evaluation.objective_values)

    buff = os.serialize
    os = CCS::deserialize(buffer: buff)
    cs = os.search_space
    fs = cs.feature_space
    features = CCS::Features::new(feature_space: fs, values: [0.5])
    configuration = CCS::Configuration::new(configuration_space: cs, values: [0.25], features: features)
    evaluation = CCS::Evaluation::new(objective_space: os, values: [-0.25], configuration: configuration)
    assert_equal([0.25+0.5, -0.25], evaluation.objective_values)
  end

end
