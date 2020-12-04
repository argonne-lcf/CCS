ENV["PYTHONPATH"] = "./" + (ENV["PYTHONPATH"] ? ":"+ ENV["PYTHONPATH"] : "")

require 'rubygems'
require_relative '../bindings/ruby/lib/cconfigspace'
require 'pycall/import'
include PyCall::Import
pyimport :test_python

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

cs, os = create_tuning_problem
pt = test_python.create_test_tuner(cs.handle.to_i, os.handle.to_i)
t = CCS::Tuner.from_handle(FFI::Pointer::new(pt.handle.value))

func = lambda { |(x, y, z)|
  [(x-2)**2, Math.sin(z+y)]
}
evals = t.ask(100).collect { |c|
  CCS::Evaluation::new(objective_space: os, configuration: c, values: func[c.values])
}
t.tell evals
hist = t.history
raise "Invalid size" if 100 != hist.size
evals = t.ask(100).collect { |c|
  CCS::Evaluation::new(objective_space: os, configuration: c, values: func[c.values])
}
t.tell evals
raise "Invalid size" if 200 != t.history_size
optims = t.optimums
objs = optims.collect(&:objective_values).sort
p objs
objs.collect { |(_, v)| v }.each_cons(2) { |v1, v2| raise "Invalid results" if (v1 <=> v2) <= 0 }
puts "Success"
