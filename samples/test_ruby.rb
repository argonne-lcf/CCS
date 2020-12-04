require 'rubygems'
require_relative '../bindings/ruby/lib/cconfigspace'

class TestTuner < CCS::UserDefinedTuner
  def initialize(cs, os)
    @history = []
    @optimums = []
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
        @optimums = @optimums.collect { |o|
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
        @optimums.push(e) unless discard
      }
    }
    get_history = lambda { |tuner|
      @history
    }
    get_optimums = lambda { |tuner|
      @optimums
    }
    super(name: "tuner", configuration_space: cs, objective_space: os, del: del, ask: ask, tell: tell, get_optimums: get_optimums, get_history: get_history)
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


