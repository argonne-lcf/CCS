[ '../lib', 'lib' ].each { |d| $:.unshift(d) if File::directory?(d) }
require 'minitest/autorun'
require 'cconfigspace'

class CConfigSpaceTestTuner < Minitest::Test
  def setup
    CCS.init
  end

  def create_tuning_problem
    cs = CCS::ConfigurationSpace::new(name: "cspace")
    h1 = CCS::NumericalHyperparameter::new(lower: -5.0, upper: 5.0)
    h2 = CCS::NumericalHyperparameter::new(lower: -5.0, upper: 5.0)
    h3 = CCS::NumericalHyperparameter::new(lower: -5.0, upper: 5.0)
    cs.add_hyperparameters [h1, h2, h3]
    os = CCS::ObjectiveSpace::new(name: "ospace")
    v1 = CCS::NumericalHyperparameter::new(lower: -Float::INFINITY, upper: Float::INFINITY)
    v2 = CCS::NumericalHyperparameter::new(lower: -Float::INFINITY, upper: Float::INFINITY)
    os.add_hyperparameters [v1, v2]
    e1 = CCS::Variable::new(hyperparameter: v1)
    e2 = CCS::Variable::new(hyperparameter: v2)
    os.add_objectives( [e1, e2] )
    [cs, os]
  end

  def test_create_random
    cs, os = create_tuning_problem
    t = CCS::RandomTuner::new(name: "tuner", configuration_space: cs, objective_space: os)
    t2 = CCS::Object::from_handle(t)
    assert_equal( t.class, t2.class)
    assert_equal( "tuner", t.name )
    assert_equal( :CCS_TUNER_RANDOM, t.type )
    func = lambda { |(x, y, z)|
      [(x-2)**2, Math.sin(z+y)]
    }
    evals = t.ask(100).collect { |c|
      CCS::Evaluation::new(objective_space: os, configuration: c, values: func[c.values])
    }
    t.tell evals
    hist = t.history
    assert_equal(100, hist.size)
    evals = t.ask(100).collect { |c|
      CCS::Evaluation::new(objective_space: os, configuration: c, values: func[c.values])
    }
    t.tell evals
    assert_equal(200, t.history_size)
    objs = t.optimums.collect(&:objective_values).sort
    objs.collect { |(_, v)| v }.each_cons(2) { |v1, v2| assert( (v1 <=> v2) > 0 ) }
    assert( t.optimums.collect(&:configuration).include?(t.suggest) )

    buff = t.serialize
    t_copy = CCS.deserialize(buffer: buff)
    hist = t_copy.history
    assert_equal(200, hist.size)
    assert_equal(t.num_optimums, t_copy.num_optimums)
    objs = t_copy.optimums.collect(&:objective_values).sort
    objs.collect { |(_, v)| v }.each_cons(2) { |v1, v2| assert( (v1 <=> v2) > 0 ) }
    assert( t_copy.optimums.collect(&:configuration).include?(t_copy.suggest) )
  end

  class TunerData
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
        cs = tuner.configuration_space
        [cs.samples(count), count]
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
            when :CCS_EQUIVALENT, :CCS_WORSE
              discard = true
              o
            when :CCS_NOT_COMPARABLE
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
    cs, os = create_tuning_problem
    t = CCS::UserDefinedTuner::new(name: "tuner", configuration_space: cs, objective_space: os, del: del, ask: ask, tell: tell, get_optimums: get_optimums, get_history: get_history, suggest: suggest, tuner_data: TunerData.new)
    t2 = CCS::Object::from_handle(t)
    assert_equal( t.class, t2.class)
    assert_equal( "tuner", t.name )
    assert_equal( :CCS_TUNER_USER_DEFINED, t.type )
    func = lambda { |(x, y, z)|
      [(x-2)**2, Math.sin(z+y)]
    }
    evals = t.ask(100).collect { |c|
      CCS::Evaluation::new(objective_space: os, configuration: c, values: func[c.values])
    }
    t.tell evals
    hist = t.history
    assert_equal(100, hist.size)
    evals = t.ask(100).collect { |c|
      CCS::Evaluation::new(objective_space: os, configuration: c, values: func[c.values])
    }
    t.tell evals
    assert_equal(200, t.history_size)
    optims = t.optimums
    objs = optims.collect(&:objective_values).sort
    objs.collect { |(_, v)| v }.each_cons(2) { |v1, v2| assert( (v1 <=> v2) > 0 ) }
    assert( t.optimums.collect(&:configuration).include?(t.suggest) )

    buff = t.serialize
    t_copy = CCS::UserDefinedTuner::deserialize(buffer: buff, del: del, ask: ask, tell: tell, get_optimums: get_optimums, get_history: get_history, suggest: suggest, tuner_data: TunerData.new)
    hist = t_copy.history
    assert_equal(200, hist.size)
    assert_equal(t.num_optimums, t_copy.num_optimums)
    objs = t_copy.optimums.collect(&:objective_values).sort
    objs.collect { |(_, v)| v }.each_cons(2) { |v1, v2| assert( (v1 <=> v2) > 0 ) }
    assert( t_copy.optimums.collect(&:configuration).include?(t_copy.suggest) )

    t.serialize(path: 'tuner.ccs')
    t_copy = CCS::UserDefinedTuner::deserialize(path: 'tuner.ccs', del: del, ask: ask, tell: tell, get_optimums: get_optimums, get_history: get_history, suggest: suggest, tuner_data: TunerData.new)
    hist = t_copy.history
    assert_equal(200, hist.size)
    assert_equal(t.num_optimums, t_copy.num_optimums)
    objs = t_copy.optimums.collect(&:objective_values).sort
    objs.collect { |(_, v)| v }.each_cons(2) { |v1, v2| assert( (v1 <=> v2) > 0 ) }
    assert( t_copy.optimums.collect(&:configuration).include?(t_copy.suggest) )
    File.delete('tuner.ccs')

    f = File.open('tuner.ccs', "wb")
    t.serialize(file_descriptor: f.fileno)
    f.close
    f = File.open('tuner.ccs', "rb")
    t_copy = CCS::UserDefinedTuner::deserialize(file_descriptor: f.fileno, del: del, ask: ask, tell: tell, get_optimums: get_optimums, get_history: get_history, suggest: suggest, tuner_data: TunerData.new)
    f.close
    hist = t_copy.history
    assert_equal(200, hist.size)
    assert_equal(t.num_optimums, t_copy.num_optimums)
    objs = t_copy.optimums.collect(&:objective_values).sort
    objs.collect { |(_, v)| v }.each_cons(2) { |v1, v2| assert( (v1 <=> v2) > 0 ) }
    assert( t_copy.optimums.collect(&:configuration).include?(t_copy.suggest) )
    File.delete('tuner.ccs')
  end
end

