Gem::Specification.new do |s|
  s.name = 'cconfigspace'
  s.version = "0.0.1"
  s.author = "Brice Videau"
  s.email = "bvideau@anl.gov"
  s.homepage = "https://github.com/argonne-lcf/CCS"
  s.summary = "cconfigspace (CCS) ruby bindings"
  s.description = "cconfigspace is a gem defining bindings for the CCS: C Configuration Space and Tuning Library"
  s.files = Dir['cconfigspace.gemspec', 'LICENSE', 'lib/**/*.rb']
  s.license = 'BSD-3-Clause'
  s.required_ruby_version = '>= 2.3.0'
  s.add_dependency 'ffi', '~> 1.13', '>=1.13.0'
  s.add_dependency 'ffi-value', '~> 0.1', '>=0.1.1'
  s.add_dependency 'whittle', '~> 0.0', '>=0.0.8'
end
