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
  end

  def test_user_defined
    history = []
    optimums = []
    del = lambda { |data| nil }
    ask = lambda { |data, count|
      if count > 0
        cs = CCS::ConfigurationSpace::from_handle(data[:common_data][:configuration_space])
        [cs.samples(count), count]
      else
        [nil, 1]
      end
    }
    tell = lambda { |data, evaluations|
      history += evaluations
      evaluations.each { |e|
        discard = false
        optimums = optimums.collect { |o|
          unless discard
            case e.cmp(o)
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
        optimums.push(e) unless discard
      }
    }
    get_history = lambda { |data|
      history
    }
    get_optimums = lambda { |data|
      optimums
    }
    cs, os = create_tuning_problem
    t = CCS::UserDefinedTuner::new(name: "tuner", configuration_space: cs, objective_space: os, del: del, ask: ask, tell: tell, get_optimums: get_optimums, get_history: get_history)
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
  end
end

