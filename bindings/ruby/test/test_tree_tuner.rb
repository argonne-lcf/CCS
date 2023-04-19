[ '../lib', 'lib' ].each { |d| $:.unshift(d) if File::directory?(d) }
require 'minitest/autorun'
require 'cconfigspace'

class CConfigSpaceTestTreeTuner < Minitest::Test

  def setup
    CCS.init
  end

  def generate_tree(depth, rank)
    ar = depth - rank
    ar = 0 if ar < 0
    tree = CCS::Tree.new(arity: ar, value: depth * 100.0 + rank)
    ar.times { |i|
      child = generate_tree(depth - 1, i)
      tree.set_child(i, child)
    }
    tree
  end

  def create_tuning_problem
    tree = generate_tree(5, 0)
    ts = CCS::StaticTreeSpace.new(name: 'space', tree: tree)
    os = CCS::ObjectiveSpace.new(name: 'ospace')
    v1 = CCS::NumericalParameter.new(lower: -Float::INFINITY, upper: Float::INFINITY)
    os.add_parameter(v1)
    e1 = CCS::Variable.new(parameter: v1)
    os.add_objectives( {e1 => :CCS_OBJECTIVE_TYPE_MAXIMIZE} )
    [ts, os]
  end

  def test_create_random
    ts, os = create_tuning_problem
    t = CCS::RandomTreeTuner.new(name: "tuner", tree_space: ts, objective_space: os)
    t2 = CCS::Object.from_handle(t)
    assert_equal(t.class, t2.class)
    assert_equal("tuner", t.name)
    assert_equal(:CCS_TREE_TUNER_RANDOM, t.type)
    evals = t.ask(100).map { |c|
      CCS::TreeEvaluation.new(objective_space: os, configuration: c, values: [c.values.reduce(:+)])
    }
    t.tell(evals)
    hist = t.history
    assert_equal(100, hist.size)
    evals = t.ask(100).map { |c|
      CCS::TreeEvaluation.new(objective_space: os, configuration: c, values: [c.values.reduce(:+)])
    }
    t.tell(evals)
    hist = t.history
    assert_equal(200, hist.size)
    optims = t.optimums
    assert_equal(1, optims.size)
    best = optims[0].objective_values[0]
    assert_equal(hist.map { |e| e.objective_values.first }.max, best)
    assert(optims.map(&:configuration).include?(t.suggest))
    buff = t.serialize
    t_copy = CCS.deserialize(buffer: buff)
    hist = t_copy.history
    assert_equal(200, hist.size)
    optims_2 = t_copy.optimums
    assert_equal(optims.size, optims_2.size)
    best2 = optims_2[0].objective_values[0]
    assert_equal(best, best2)
    assert_equal(hist.map { |e| e.objective_values.first }.max, best2)
    assert(optims_2.map(&:configuration).include?(t_copy.suggest))
  end

  class TreeTunerData
    attr_accessor :history, :optimums
    def initialize
      @history = []
      @optimums = []
    end
  end

  def test_user_defined
    del = lambda { |tuner| nil }
    ask = lambda { |tuner, count|
      if count
        ts = tuner.tree_space
        [ts.samples(count), count]
      else
        [nil, 1]
      end
    }
    tell = lambda { |tuner, evaluations|
      tuner.tuner_data.history.concat(evaluations)
      evaluations.each { |e|
        discard = false
        tuner.tuner_data.optimums = tuner.tuner_data.optimums.collect { |o|
          unless discard
            case e.compare(o)
            when :CCS_COMPARISON_EQUIVALENT, :CCS_COMPARISON_WORSE
              discard = true
              o
            when :CCS_COMPARISON_NOT_COMPARABLE
              o
            else
              nil
            end
          else
            o
          end
        }.compact
        tuner.tuner_data.optimums.push(e) unless discard
      }
    }
    get_history = lambda { |tuner|
      tuner.tuner_data.history
    }
    get_optimums = lambda { |tuner|
      tuner.tuner_data.optimums
    }
    suggest = lambda { |tuner|
      if tuner.tuner_data.optimums.empty?
        ask.call(tuner, 1)
      else
        tuner.tuner_data.optimums.sample.configuration
      end
    }
    ts, os = create_tuning_problem
    t = CCS::UserDefinedTreeTuner.new(name: "tuner", tree_space: ts, objective_space: os, del: del, ask: ask, tell: tell, get_optimums: get_optimums, get_history: get_history, suggest: suggest, tuner_data: TreeTunerData.new)
    t2 = CCS::Object::from_handle(t)
    assert_equal( t.class, t2.class)
    assert_equal( "tuner", t.name )
    assert_equal( :CCS_TREE_TUNER_USER_DEFINED, t.type )
    evals = t.ask(100).map { |c|
      CCS::TreeEvaluation.new(objective_space: os, configuration: c, values: [c.values.reduce(:+)])
    }
    t.tell(evals)
    hist = t.history
    assert_equal(100, hist.size)
    evals = t.ask(100).map { |c|
      CCS::TreeEvaluation.new(objective_space: os, configuration: c, values: [c.values.reduce(:+)])
    }
    t.tell(evals)
    hist = t.history
    assert_equal(200, hist.size)
    optims = t.optimums
    assert_equal(1, optims.size)
    best = optims[0].objective_values[0]
    assert_equal(hist.map { |e| e.objective_values.first }.max, best)
    assert(optims.map(&:configuration).include?(t.suggest))
    buff = t.serialize
    t_copy = CCS::UserDefinedTreeTuner.deserialize(buffer: buff, del: del, ask: ask, tell: tell, get_optimums: get_optimums, get_history: get_history, suggest: suggest, tuner_data: TreeTunerData.new)
    hist = t_copy.history
    assert_equal(200, hist.size)
    optims_2 = t_copy.optimums
    assert_equal(optims.size, optims_2.size)
    best2 = optims_2[0].objective_values[0]
    assert_equal(best, best2)
    assert_equal(hist.map { |e| e.objective_values.first }.max, best2)
    assert(optims_2.map(&:configuration).include?(t_copy.suggest))
  end
end
