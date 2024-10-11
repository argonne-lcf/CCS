require 'minitest/autorun'
require_relative '../lib/cconfigspace'

class CConfigSpaceTestTreeEvaluation < Minitest::Test
  def generate_tree(depth, rank)
    ar = depth - rank
    ar = 0 if ar < 0
    tree = CCS::Tree.new(arity: ar, value: depth * 100 + rank)
    ar.times { |i|
      child = generate_tree(depth - 1, i)
      tree.set_child(i, child)
    }
    tree
  end

  def test_create
    tree = generate_tree(4, 0)
    ts = CCS::StaticTreeSpace.new(name: 'space', tree: tree)
    v1 = CCS::NumericalParameter::Float.new
    v2 = CCS::NumericalParameter::Float.new
    e1 = CCS::Expression::Variable.new(parameter: v1)
    e2 = CCS::Expression::Variable.new(parameter: v2)
    os = CCS::ObjectiveSpace.new(name: 'ospace', search_space: ts, parameters: [v1, v2], objectives: { e1 => :CCS_OBJECTIVE_TYPE_MAXIMIZE, e2 => :CCS_OBJECTIVE_TYPE_MINIMIZE })
    ev1 = CCS::Evaluation.new(objective_space: os, configuration: ts.sample, values: [0.5, 0.6])
    assert_equal( [0.5, 0.6], ev1.values )
    assert_equal( [0.5, 0.6], ev1.objective_values )
    ev2 = CCS::Evaluation.new(objective_space: os, configuration: ts.sample, values: [0.5, 0.6])
    assert_equal( [0.5, 0.6], ev2.values )
    assert_equal( [0.5, 0.6], ev2.objective_values )
    assert_equal( :CCS_COMPARISON_EQUIVALENT, ev1.compare(ev2) )
    ev3 = CCS::Evaluation.new(objective_space: os, configuration: ts.sample, values: [0.6, 0.5])
    assert_equal( [0.6, 0.5], ev3.objective_values )
    assert_equal( :CCS_COMPARISON_WORSE, ev1.compare(ev3) )
    assert_equal( :CCS_COMPARISON_BETTER, ev3.compare(ev1) )
    ev4 = CCS::Evaluation.new(objective_space: os, configuration: ts.sample, values: [0.6, 0.7])
    assert_equal( [0.6, 0.7], ev4.objective_values )
    assert_equal( :CCS_COMPARISON_NOT_COMPARABLE, ev1.compare(ev4) )
    assert_equal( :CCS_COMPARISON_NOT_COMPARABLE, ev4.compare(ev1) )
  end

  def test_serialize
    tree = generate_tree(4, 0)
    ts = CCS::StaticTreeSpace.new(name: 'space', tree: tree)
    v1 = CCS::NumericalParameter::Float.new
    v2 = CCS::NumericalParameter::Float.new
    e1 = CCS::Expression::Variable.new(parameter: v1)
    e2 = CCS::Expression::Variable.new(parameter: v2)
    os = CCS::ObjectiveSpace.new(name: 'ospace', search_space: ts, parameters: [v1, v2], objectives: { e1 => :CCS_OBJECTIVE_TYPE_MAXIMIZE, e2 => :CCS_OBJECTIVE_TYPE_MINIMIZE })
    evref = CCS::Evaluation.new(objective_space: os, configuration: ts.sample, values: [0.5, 0.6])
    buff = evref.serialize
    handle_map = CCS::Map.new
    handle_map[ts] = ts
    handle_map[os] = os
    ev = CCS.deserialize(buffer: buff, handle_map: handle_map)
    assert_equal( ts.handle, ev.configuration.tree_space.handle)
    assert_equal( os.handle, ev.objective_space.handle)
    assert_equal( [0.5, 0.6], ev.values )
    assert_equal( [0.5, 0.6], ev.objective_values )
  end

end
