ENV["PYTHONPATH"] = "./" + (ENV["PYTHONPATH"] ? ":"+ ENV["PYTHONPATH"] : "")

require 'rubygems'
require_relative '../bindings/ruby/lib/cconfigspace'
require 'pycall/import'
include PyCall::Import
pyimport :test_python

def create_tuning_problem
  h1 = CCS::NumericalParameter::Float.new(lower: -5.0, upper: 5.0)
  h2 = CCS::NumericalParameter::Float.new(lower: -5.0, upper: 5.0)
  h3 = CCS::NumericalParameter::Float.new(lower: -5.0, upper: 5.0)
  cs = CCS::ConfigurationSpace::new(name: "cspace", parameters: [h1, h2, h3])
  v1 = CCS::NumericalParameter::Float.new(lower: -Float::INFINITY, upper: Float::INFINITY)
  v2 = CCS::NumericalParameter::Float.new(lower: -Float::INFINITY, upper: Float::INFINITY)
  e1 = CCS::Expression::Variable::new(parameter: v1)
  e2 = CCS::Expression::Variable::new(parameter: v2)
  os = CCS::ObjectiveSpace::new(name: "ospace", parameters: [v1, v2], objectives: [e1, e2])
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
optims = t.optima
objs = optims.collect(&:objective_values).sort
p objs
objs.collect { |(_, v)| v }.each_cons(2) { |v1, v2| raise "Invalid results" if (v1 <=> v2) <= 0 }
puts "Success"
