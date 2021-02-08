[ '../lib', 'lib' ].each { |d| $:.unshift(d) if File::directory?(d) }
require 'minitest/autorun'
require 'cconfigspace'

class CConfigSpaceTestFeaturesTuner < Minitest::Test
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

    fs = CCS::FeaturesSpace::new(name: "fspace")
    f1 = CCS::CategoricalHyperparameter::new(values: ["on", "off"])
    fs.add_hyperparameter f1
    [cs, fs, os]
  end

  def test_create_random
    cs, fs, os = create_tuning_problem
    t = CCS::RandomFeaturesTuner::new(name: "tuner", configuration_space: cs, features_space: fs, objective_space: os)
    t2 = CCS::Object::from_handle(t)
    assert_equal( t.class, t2.class)
    assert_equal( "tuner", t.name )
    assert_equal( :CCS_FEATURES_TUNER_RANDOM, t.type )
    func = lambda { |(x, y, z)|
      [(x-2)**2, Math.sin(z+y)]
    }
    features_on = CCS::Features.new(features_space: fs, values: ["on"])
    features_off = CCS::Features.new(features_space: fs, values: ["off"])
    evals = t.ask(features_on, 50).collect { |c|
      CCS::FeaturesEvaluation::new(objective_space: os, configuration: c, features: features_on, values: func[c.values])
    }
    t.tell evals
    evals = t.ask(features_off, 50).collect { |c|
      CCS::FeaturesEvaluation::new(objective_space: os, configuration: c, features: features_off, values: func[c.values])
    }
    t.tell evals
    hist = t.history
    assert_equal(100, hist.size)
    evals = t.ask(features_on, 100).collect { |c|
      CCS::FeaturesEvaluation::new(objective_space: os, configuration: c, features: features_on, values: func[c.values])
    }
    t.tell evals
    assert_equal(200, t.history_size)
    assert_equal(150, t.history_size(features: features_on))
    assert_equal(50, t.history_size(features: features_off))
    [features_on, features_off].each { |features|
      objs = t.optimums(features: features).collect(&:objective_values).sort
      objs.collect { |(_, v)| v }.each_cons(2) { |v1, v2| assert( (v1 <=> v2) > 0 ) }
    }
  end

  def test_user_defined
    history = []
    optimums = []
    del = lambda { |tuner| nil }
    ask = lambda { |tuner, features, count|
      if count
        cs = tuner.configuration_space
        [cs.samples(count), count]
      else
        [nil, 1]
      end
    }
    tell = lambda { |tuner, evaluations|
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
    get_history = lambda { |tuner, features|
      if features
        history.select { |e| e.features == features }
      else
        history
      end
    }
    get_optimums = lambda { |tuner, features|
      if features
        optimums.select { |e| e.features == features }
      else
        optimums
      end
    }
    cs, fs, os = create_tuning_problem
    t = CCS::UserDefinedFeaturesTuner::new(name: "tuner", configuration_space: cs, features_space: fs, objective_space: os, del: del, ask: ask, tell: tell, get_optimums: get_optimums, get_history: get_history)
    t2 = CCS::Object::from_handle(t)
    assert_equal( t.class, t2.class)
    assert_equal( "tuner", t.name )
    assert_equal( :CCS_FEATURES_TUNER_USER_DEFINED, t.type )
    func = lambda { |(x, y, z)|
      [(x-2)**2, Math.sin(z+y)]
    }
    features_on = CCS::Features.new(features_space: fs, values: ["on"])
    features_off = CCS::Features.new(features_space: fs, values: ["off"])
    evals = t.ask(features_on, 50).collect { |c|
      CCS::FeaturesEvaluation::new(objective_space: os, configuration: c, features: features_on, values: func[c.values])
    }
    t.tell evals
    evals = t.ask(features_off, 50).collect { |c|
      CCS::FeaturesEvaluation::new(objective_space: os, configuration: c, features: features_off, values: func[c.values])
    }
    t.tell evals
    hist = t.history
    assert_equal(100, hist.size)
    evals = t.ask(features_on, 100).collect { |c|
      CCS::FeaturesEvaluation::new(objective_space: os, configuration: c, features: features_on, values: func[c.values])
    }
    t.tell evals
    assert_equal(200, t.history_size)
    assert_equal(150, t.history_size(features: features_on))
    assert_equal(50, t.history_size(features: features_off))
    [features_on, features_off].each { |features|
      objs = t.optimums(features: features).collect(&:objective_values).sort
      objs.collect { |(_, v)| v }.each_cons(2) { |v1, v2| assert( (v1 <=> v2) > 0 ) }
    }
  end
end

