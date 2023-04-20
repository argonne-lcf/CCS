require_relative '../bindings/ruby/lib/cconfigspace'

class TestTuner < CCS::UserDefinedTuner
  def initialize(cs, os)
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
      @history += evaluations
      evaluations.each { |e|
        discard = false
        @optima = @optima.collect { |o|
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
        @optima.push(e) unless discard
      }
    }
    get_history = lambda { |tuner|
      @history
    }
    get_optima = lambda { |tuner|
      @optima
    }
    super(name: "tuner", configuration_space: cs, objective_space: os, del: del, ask: ask, tell: tell, get_optima: get_optima, get_history: get_history)
  end
end

def create_test_tuner(cs_ptr, os_ptr)
  cs_handle = FFI::Pointer::new(cs_ptr)
  os_handle = FFI::Pointer::new(os_ptr)
  TestTuner::new(CCS::ConfigurationSpace.from_handle(cs_handle),
                 CCS::ObjectiveSpace.from_handle(os_handle))
end

def create_tuning_problem
  cs = CCS::ConfigurationSpace::new(name: "cspace")
  h1 = CCS::NumericalParameter::new(lower: -5.0, upper: 5.0)
  h2 = CCS::NumericalParameter::new(lower: -5.0, upper: 5.0)
  h3 = CCS::NumericalParameter::new(lower: -5.0, upper: 5.0)
  cs.add_parameters [h1, h2, h3]
  os = CCS::ObjectiveSpace::new(name: "ospace")
  v1 = CCS::NumericalParameter::new(lower: -Float::INFINITY, upper: Float::INFINITY)
  v2 = CCS::NumericalParameter::new(lower: -Float::INFINITY, upper: Float::INFINITY)
  os.add_parameters [v1, v2]
  e1 = CCS::Variable::new(parameter: v1)
  e2 = CCS::Variable::new(parameter: v2)
  os.add_objectives( [e1, e2] )
  [cs, os]
end


