[ '../lib', 'lib' ].each { |d| $:.unshift(d) if File::directory?(d) }
require 'minitest/autorun'
require 'cconfigspace'

class CConfigSpaceTestObjectiveSpace < Minitest::Test
  def setup
    CCS.init
  end

  def test_create
    os = CCS::ObjectiveSpace::new(name: "space")
    assert_equal( :CCS_OBJECT_TYPE_OBJECTIVE_SPACE, os.object_type )
    assert_equal( "space", os.name )
    assert_equal( 0, os.num_parameters )
    assert_equal( [], os.objectives )
    h1 = CCS::NumericalParameter::Float.new
    h2 = CCS::NumericalParameter::Float.new
    h3 = CCS::NumericalParameter::Float.new
    os.add_parameter(h1)
    os.add_parameters([h2, h3])
    assert_equal( 3, os.num_parameters )
    assert_equal( h1, os.parameter(0) )
    assert_equal( h2, os.parameter(1) )
    assert_equal( h3, os.parameter(2) )
    assert_equal( [h1, h2, h3], os.parameters )
    assert_equal( h2, os.parameter_by_name(h2.name) )
    e1 = CCS::Expression::Add.new(left: h1, right: h2)
    e2 = CCS::Expression::Variable.new(parameter: h3)
    os.add_objective(e1)
    assert_equal( 1, os.objectives.size )
    os.add_objectives([e2], types: [:CCS_OBJECTIVE_TYPE_MAXIMIZE])
    assert_equal( 2, os.objectives.size )
    objs = os.objectives
    assert_equal( e1.handle, objs[0][0].handle )
    assert_equal( :CCS_OBJECTIVE_TYPE_MINIMIZE, objs[0][1] )
    assert_equal( e2.handle, objs[1][0].handle )
    assert_equal( :CCS_OBJECTIVE_TYPE_MAXIMIZE, objs[1][1] )
  end

end
