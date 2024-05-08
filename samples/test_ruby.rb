require_relative '../bindings/ruby/lib/cconfigspace'

class TestTuner < CCS::UserDefinedTuner
  def initialize(os)
    @history = []
    @optima = []
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
      history_optima = tuner.tuner_data
      history_optima[0] += evaluations
      evaluations.each { |e|
        discard = false
        history_optima[1] = history_optima[1].collect { |o|
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
        history_optima[1].push(e) unless discard
      }
    }
    get_history = lambda { |tuner|
      tuner.tuner_data[0]
    }
    get_optima = lambda { |tuner|
      tuner.tuner_data[1]
    }
    super(name: "tuner", objective_space: os, del: del, ask: ask, tell: tell, get_optima: get_optima, get_history: get_history, tuner_data: [[],[]])
  end
end

def create_test_tuner(os_ptr)
  os_handle = FFI::Pointer::new(os_ptr)
  TestTuner::new(CCS::ObjectiveSpace.from_handle(os_handle))
end

def create_tuning_problem
  h1 = CCS::NumericalParameter::Float.new(lower: -5.0, upper: 5.0)
  h2 = CCS::NumericalParameter::Float.new(lower: -5.0, upper: 5.0)
  h3 = CCS::NumericalParameter::Float.new(lower: -5.0, upper: 5.0)
  cs = CCS::ConfigurationSpace::new(name: "cspace", parameters: [h1, h2, h3])
  v1 = CCS::NumericalParameter::Float.new(lower: -Float::INFINITY, upper: Float::INFINITY)
  v2 = CCS::NumericalParameter::Float.new(lower: -Float::INFINITY, upper: Float::INFINITY)
  e1 = CCS::Expression::Variable::new(parameter: v1)
  e2 = CCS::Expression::Variable::new(parameter: v2)
  os = CCS::ObjectiveSpace::new(name: "ospace", search_space: cs, parameters: [v1, v2], objectives: [e1, e2])
end


