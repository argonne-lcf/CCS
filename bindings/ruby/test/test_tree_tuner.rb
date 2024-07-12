require 'minitest/autorun'
require_relative '../lib/cconfigspace'

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
    v1 = CCS::NumericalParameter::Float.new(lower: -Float::INFINITY, upper: Float::INFINITY)
    e1 = CCS::Expression::Variable.new(parameter: v1)
    CCS::ObjectiveSpace.new(name: 'ospace', search_space: ts, parameters: [v1], objectives: {e1 => :CCS_OBJECTIVE_TYPE_MAXIMIZE})
  end

  def test_create_random
    os = create_tuning_problem
    t = CCS::RandomTuner.new(name: "tuner", objective_space: os)
    t2 = CCS::Object.from_handle(t)
    assert_equal(t.class, t2.class)
    assert_equal("tuner", t.name)
    assert_equal(:CCS_TUNER_TYPE_RANDOM, t.type)
    evals = t.ask(100).map { |c|
      CCS::Evaluation.new(objective_space: os, configuration: c, values: [c.values.reduce(:+)])
    }
    t.tell(evals)
    hist = t.history
    assert_equal(100, hist.size)
    evals = t.ask(100).map { |c|
      CCS::Evaluation.new(objective_space: os, configuration: c, values: [c.values.reduce(:+)])
    }
    t.tell(evals)
    hist = t.history
    assert_equal(200, hist.size)
    optims = t.optima
    assert_equal(1, optims.size)
    best = optims[0].objective_values[0]
    assert_equal(hist.map { |e| e.objective_values.first }.max, best)
    assert(optims.map(&:configuration).include?(t.suggest))
    buff = t.serialize
    t_copy = CCS.deserialize(buffer: buff)
    hist = t_copy.history
    assert_equal(200, hist.size)
    optims_2 = t_copy.optima
    assert_equal(optims.size, optims_2.size)
    best2 = optims_2[0].objective_values[0]
    assert_equal(best, best2)
    assert_equal(hist.map { |e| e.objective_values.first }.max, best2)
    assert(optims_2.map(&:configuration).include?(t_copy.suggest))
  end

  class TreeTunerData
    attr_accessor :history, :optima
    def initialize
      @history = []
      @optima = []
    end
  end

  def test_user_defined
    del = lambda { |tuner| nil }
    ask = lambda { |tuner, _, count|
      if count
        ts = tuner.search_space
        [ts.samples(count), count]
      else
        [nil, 1]
      end
    }
    tell = lambda { |tuner, evaluations|
      tuner.tuner_data.history.concat(evaluations)
      evaluations.each { |e|
        discard = false
        tuner.tuner_data.optima = tuner.tuner_data.optima.collect { |o|
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
        tuner.tuner_data.optima.push(e) unless discard
      }
    }
    get_history = lambda { |tuner, _|
      tuner.tuner_data.history
    }
    get_optima = lambda { |tuner, _|
      tuner.tuner_data.optima
    }
    suggest = lambda { |tuner, _|
      if tuner.tuner_data.optima.empty?
        ask.call(tuner, 1)
      else
        tuner.tuner_data.optima.sample.configuration
      end
    }
    get_vector_data = lambda { |otype, name, cb_data|
      assert_equal(:CCS_OBJECT_TYPE_TUNER, otype)
      assert_equal("tuner", name)
      assert_nil(cb_data)
      [CCS::UserDefinedTuner.get_vector(del: del, ask: ask, tell: tell, get_optima: get_optima, get_history: get_history, suggest: suggest), TreeTunerData.new]
    }

    os = create_tuning_problem
    t = CCS::UserDefinedTuner.new(name: "tuner", objective_space: os, del: del, ask: ask, tell: tell, get_optima: get_optima, get_history: get_history, suggest: suggest, tuner_data: TreeTunerData.new)
    t2 = CCS::Object::from_handle(t)
    assert_equal( t.class, t2.class)
    assert_equal( "tuner", t.name )
    assert_equal( :CCS_TUNER_TYPE_USER_DEFINED, t.type )
    evals = t.ask(100).map { |c|
      CCS::Evaluation.new(objective_space: os, configuration: c, values: [c.values.reduce(:+)])
    }
    t.tell(evals)
    hist = t.history
    assert_equal(100, hist.size)
    evals = t.ask(100).map { |c|
      CCS::Evaluation.new(objective_space: os, configuration: c, values: [c.values.reduce(:+)])
    }
    t.tell(evals)
    hist = t.history
    assert_equal(200, hist.size)
    optims = t.optima
    assert_equal(1, optims.size)
    best = optims[0].objective_values[0]
    assert_equal(hist.map { |e| e.objective_values.first }.max, best)
    assert(optims.map(&:configuration).include?(t.suggest))
    buff = t.serialize
    t_copy = CCS.deserialize(buffer: buff, vector_callback: get_vector_data)
    hist = t_copy.history
    assert_equal(200, hist.size)
    optims_2 = t_copy.optima
    assert_equal(optims.size, optims_2.size)
    best2 = optims_2[0].objective_values[0]
    assert_equal(best, best2)
    assert_equal(hist.map { |e| e.objective_values.first }.max, best2)
    assert(optims_2.map(&:configuration).include?(t_copy.suggest))
  end
end
