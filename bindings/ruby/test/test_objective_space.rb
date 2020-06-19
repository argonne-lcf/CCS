[ '../lib', 'lib' ].each { |d| $:.unshift(d) if File::directory?(d) }
require 'minitest/autorun'
require 'cconfigspace'

class CConfigSpaceTestObjectiveSpace < Minitest::Test
  def setup
    CCS.init
  end

  def test_create
    os = CCS::ObjectiveSpace::new(name: "space")
    assert_equal( :CCS_OBJECTIVE_SPACE, os.object_type )
    assert_equal( "space", os.name )
    assert_equal( 0, os.num_hyperparameters )
    assert_equal( [], os.objectives )
    h1 = CCS::NumericalHyperparameter::new
    h2 = CCS::NumericalHyperparameter::new
    h3 = CCS::NumericalHyperparameter::new
    os.add_hyperparameter(h1)
    os.add_hyperparameters([h2, h3])
    assert_equal( 3, os.num_hyperparameters )
    assert_equal( h1, os.hyperparameter(0) )
    assert_equal( h2, os.hyperparameter(1) )
    assert_equal( h3, os.hyperparameter(2) )
    assert_equal( [h1, h2, h3], os.hyperparameters )
    assert_equal( h2, os.hyperparameter_by_name(h2.name) )
    e1 = CCS::Expression::new(type: :CCS_ADD, nodes: [h1, h2])
    e2 = CCS::Variable::new(hyperparameter: h3)
    os.add_objective(e1)
    assert_equal( 1, os.objectives.size )
    os.add_objectives([e2], types: [:CCS_MAXIMIZE])
    assert_equal( 2, os.objectives.size )
    objs = os.objectives
    assert_equal( e1.handle, objs[0][0].handle )
    assert_equal( :CCS_MINIMIZE, objs[0][1] )
    assert_equal( e2.handle, objs[1][0].handle )
    assert_equal( :CCS_MAXIMIZE, objs[1][1] )
  end

end
