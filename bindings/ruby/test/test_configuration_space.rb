[ '../lib', 'lib' ].each { |d| $:.unshift(d) if File::directory?(d) }
require 'minitest/autorun'
require 'cconfigspace'

class CConfigSpaceTestConfigurationSpace < Minitest::Test
  def setup
    CCS.init
  end

  def test_create
    cs = CCS::ConfigurationSpace::new(name: "space")
    assert_equal( :CCS_OBJECT_TYPE_CONFIGURATION_SPACE, cs.object_type )
    assert_equal( "space", cs.name )
    assert( cs.rng.kind_of?(CCS::Rng) )
    assert_equal( 0, cs.num_parameters )
    assert_equal( [], cs.conditions )
    assert_equal( [], cs.forbidden_clauses )
    h1 = CCS::NumericalParameter::new
    h2 = CCS::NumericalParameter::new
    h3 = CCS::NumericalParameter::new
    cs.add_parameter(h1)
    cs.add_parameters([h2, h3])
    assert_equal( 3, cs.num_parameters )
    assert_equal( h1, cs.parameter(0) )
    assert_equal( h2, cs.parameter(1) )
    assert_equal( h3, cs.parameter(2) )
    assert_equal( [h1, h2, h3], cs.parameters )
    assert_equal( h2, cs.parameter_by_name(h2.name) )
    assert( cs.check(cs.default_configuration) )
    c = cs.sample
    assert( cs.check(c) )
    assert_equal( cs.handle, c.configuration_space.handle )
    assert( cs.check_values(cs.sample.values) )
    cs.samples(100).each { |c|
      assert( cs.check(c) )
    }
  end

  def test_set_distribution
    cs = CCS::ConfigurationSpace::new(name: "space")
    h1 = CCS::NumericalParameter::new
    h2 = CCS::NumericalParameter::new
    h3 = CCS::NumericalParameter::new
    cs.add_parameters([h1, h2, h3])
    distributions = [ CCS::UniformDistribution::float(lower: 0.1, upper: 0.3), CCS::UniformDistribution::float(lower: 0.2, upper: 0.6) ]
    d = CCS::MultivariateDistribution::new(distributions: distributions)
    cs.set_distribution(d, [h1, h2])
    dist, indx = cs.get_parameter_distribution(h1)
    assert_equal( d.handle, dist.handle )
    assert_equal( 0, indx )
    dist, indx = cs.get_parameter_distribution(h2)
    assert_equal( d.handle, dist.handle )
    assert_equal( 1, indx )
    cs.set_distribution(d, [h3, h1])
    dist, indx = cs.get_parameter_distribution(h1)
    assert_equal( d.handle, dist.handle )
    assert_equal( 1, indx )
    dist, indx = cs.get_parameter_distribution(h3)
    assert_equal( d.handle, dist.handle )
    assert_equal( 0, indx )
  end

  def test_conditions
    h1 = CCS::NumericalParameter::new(lower: -1.0, upper: 1.0, default: 0.0)
    h2 = CCS::NumericalParameter::new(lower: -1.0, upper: 1.0)
    h3 = CCS::NumericalParameter::new(lower: -1.0, upper: 1.0)
    cs = CCS::ConfigurationSpace::new(name: "space")
    cs.add_parameters([h1, h2, h3])
    e1 = CCS::Expression::new(type: :CCS_LESS, nodes: [h2, 0.0])
    cs.set_condition(h3, e1)
    e2 = CCS::Expression::new(type: :CCS_LESS, nodes: [h3, 0.0])
    cs.set_condition(h1, e2)
    e3 = CCS::Expression::new(type: :CCS_LESS, nodes: [h1, 0.0])
    cs.add_forbidden_clause(e3)
    conditions = cs.conditions
    conditional_parameters = cs.conditional_parameters
    unconditional_parameters = cs.unconditional_parameters
    assert_equal( 3, conditions.length )
    assert_equal( e2.handle, conditions[0].handle )
    assert_nil( conditions[1] )
    assert_equal( e1.handle, conditions[2].handle )
    assert_equal( 2, conditional_parameters.length )
    assert_equal( 1, unconditional_parameters.length )
    assert_equal( h1.handle, conditional_parameters[0].handle )
    assert_equal( h3.handle, conditional_parameters[1].handle )
    assert_equal( h2.handle, unconditional_parameters[0].handle )
    forbidden_clauses = cs.forbidden_clauses
    assert_equal( 1, forbidden_clauses.length )
    assert_equal( e3.handle, forbidden_clauses[0].handle )
  end

  def extract_active_parameters(values)
    ['p1'] + values.select { |v|
      v != CCS::Inactive
    }.collect { |v|
      m = v.scan(/#P(\d)/)
      if m
        m.collect { |vi|
          "p#{vi.first.to_i}"
        }
      else
        nil
      end
    }.compact.flatten
  end

  def test_omp
    p1 = CCS::CategoricalParameter::new(
      name: 'p1',
      values: [
        ' ',
        '#pragma omp #P2',
        '#pragma omp target teams distribute #P2',
        '#pragma omp target teams distribute #P4',
        '#pragma omp #P3'])
    p2 = CCS::CategoricalParameter::new(
      name: 'p2',
      values: [
        ' ',
        'parallel for #P3',
        'parallel for #P5',
        'parallel for #P6'])
    p3 = CCS::CategoricalParameter::new(
      name: 'p3',
      values: [' ', 'simd'])
    p4 = CCS::CategoricalParameter::new(
      name: 'p4',
      values: [
        ' ',
        'dist_schedule(static)',
        'dist_schedule(static, #P8)'])
    p5 = CCS::CategoricalParameter::new(
      name: 'p5',
      values: [
        ' ',
        'schedule(#P7,#P8)',
        'schedule(#P7)'])
    p6 = CCS::CategoricalParameter::new(
      name: 'p6',
      values: [
        ' ',
        'numthreads(#P9)'])
    p7 = CCS::CategoricalParameter::new(
      name: 'p7',
      values: [
        'static',
        'dynamic'])
    p8 = CCS::OrdinalParameter::new(
      name: 'p8',
      values: ['1', '8', '16'])
    p9 = CCS::OrdinalParameter::new(
      name: 'p9',
      values: ['1', '8', '16'])

    cs = CCS::ConfigurationSpace::new(name: "omp")
    cs.add_parameters([p1, p2, p3, p4, p5, p6, p7, p8, p9])

    cond0 = CCS::Expression::new(type: :CCS_EQUAL, nodes: [p1, '#pragma omp #P2'])
    cond1 = CCS::Expression::new(type: :CCS_EQUAL, nodes: [p1, '#pragma omp target teams distribute #P2'])
    cond2 = CCS::Expression::new(type: :CCS_EQUAL, nodes: [p1, '#pragma omp target teams distribute #P4'])
    cond3 = CCS::Expression::new(type: :CCS_EQUAL, nodes: [p1, '#pragma omp #P3'])

    cond4 = CCS::Expression::new(type: :CCS_EQUAL, nodes: [p2, 'parallel for #P3'])
    cond5 = CCS::Expression::new(type: :CCS_EQUAL, nodes: [p2, 'parallel for #P5'])
    cond6 = CCS::Expression::new(type: :CCS_EQUAL, nodes: [p2, 'parallel for #P6'])

    cond7 = CCS::Expression::new(type: :CCS_EQUAL, nodes: [p4, 'dist_schedule(static, #P8)'])

    cond8 = CCS::Expression::new(type: :CCS_EQUAL, nodes: [p5, 'schedule(#P7)'])
    cond9 = CCS::Expression::new(type: :CCS_EQUAL, nodes: [p5, 'schedule(#P7,#P8)'])

    cond10 = CCS::Expression::new(type: :CCS_EQUAL, nodes: [p6, 'numthreads(#P9)'])

    cs.set_condition(p2, CCS::Expression::new(type: :CCS_OR, nodes: [cond0, cond1]))
    cs.set_condition(p4, cond2)
    cs.set_condition(p3, CCS::Expression::new(type: :CCS_OR, nodes: [cond3, cond4]))
    cs.set_condition(p5, cond5)
    cs.set_condition(p6, cond6)
    cs.set_condition(p7, CCS::Expression::new(type: :CCS_OR, nodes: [cond8, cond9]))
    cond_p8 = CCS::Expression::new(type: :CCS_OR, nodes: [cond7, cond9])
    cs.set_condition(p8, cond_p8)
    cs.set_condition(p9, cond10)

    forbiddena = CCS::Expression::new(type: :CCS_EQUAL, nodes: [p1, '#pragma omp #P2'])
    forbiddenb = CCS::Expression::new(type: :CCS_EQUAL, nodes: [p2, ' '])
    forbidden0 = CCS::Expression::new(type: :CCS_AND, nodes: [forbiddena, forbiddenb])

    forbiddenc = CCS::Expression::new(type: :CCS_EQUAL, nodes: [p1, '#pragma omp #P3'])
    forbiddend = CCS::Expression::new(type: :CCS_EQUAL, nodes: [p3, ' '])
    forbidden1 = CCS::Expression::new(type: :CCS_AND, nodes: [forbiddenc, forbiddend])
    cs.add_forbidden_clauses([forbidden0, forbidden1])

    all_params = (1..9).collect { |i| "p#{i}" }

    1000.times {
      s = cs.sample
      assert( s.check )
      active_params = extract_active_parameters(s.values)
      active_params.each { |par|
        refute_equal( CCS::Inactive, s.value(par) )
      }
      (all_params - active_params).each { |par|
        assert_equal( CCS::Inactive, s.value(par) )
      }
      refute( s.value('p1') == '#pragma omp #P2' && s.value('p2') == ' ' )
      refute( s.value('p1') == '#pragma omp #P3' && s.value('p3') == ' ' )
    }
  end

  def test_omp_parse
    p1 = CCS::CategoricalParameter::new(
      name: 'p1',
      values: [
        ' ',
        '#pragma omp #P2',
        '#pragma omp target teams distribute #P2',
        '#pragma omp target teams distribute #P4',
        '#pragma omp #P3'])
    p2 = CCS::CategoricalParameter::new(
      name: 'p2',
      values: [
        ' ',
        'parallel for #P3',
        'parallel for #P5',
        'parallel for #P6'])
    p3 = CCS::CategoricalParameter::new(
      name: 'p3',
      values: [' ', 'simd'])
    p4 = CCS::CategoricalParameter::new(
      name: 'p4',
      values: [
        ' ',
        'dist_schedule(static)',
        'dist_schedule(static, #P8)'])
    p5 = CCS::CategoricalParameter::new(
      name: 'p5',
      values: [
        ' ',
        'schedule(#P7,#P8)',
        'schedule(#P7)'])
    p6 = CCS::CategoricalParameter::new(
      name: 'p6',
      values: [
        ' ',
        'numthreads(#P9)'])
    p7 = CCS::CategoricalParameter::new(
      name: 'p7',
      values: [
        'static',
        'dynamic'])
    p8 = CCS::OrdinalParameter::new(
      name: 'p8',
      values: ['1', '8', '16'])
    p9 = CCS::OrdinalParameter::new(
      name: 'p9',
      values: ['1', '8', '16'])

    cs = CCS::ConfigurationSpace::new(name: "omp")
    cs.add_parameters([p1, p2, p3, p4, p5, p6, p7, p8, p9])

    cs.set_condition(p2, "p1 # ['#pragma omp #P2', '#pragma omp target teams distribute #P2']")
    cs.set_condition(p4, "p1 == '#pragma omp target teams distribute #P4'")
    cs.set_condition(p3, "p1 == '#pragma omp #P3' || p2 == 'parallel for #P3'")
    cs.set_condition(p5, "p2 == 'parallel for #P5'")
    cs.set_condition(p6, "p2 == 'parallel for #P6'")
    cs.set_condition(p7, "p5 # ['schedule(#P7)', 'schedule(#P7,#P8)']")
    cs.set_condition(p8, "p4 == 'dist_schedule(static, #P8)' || p5 == 'schedule(#P7,#P8)'")
    cs.set_condition(p9, "p6 == 'numthreads(#P9)'")

    cs.add_forbidden_clauses(["p1 == '#pragma omp #P2' && p2 == ' '",
                              "p1 == '#pragma omp #P3' && p3 == ' '"])

    all_params = (1..9).collect { |i| "p#{i}" }

    1000.times {
      s = cs.sample
      assert( s.check )
      active_params = extract_active_parameters(s.values)
      active_params.each { |par|
        refute_equal( CCS::Inactive, s.value(par) )
      }
      (all_params - active_params).each { |par|
        assert_equal( CCS::Inactive, s.value(par) )
      }
      refute( s.value('p1') == '#pragma omp #P2' && s.value('p2') == ' ' )
      refute( s.value('p1') == '#pragma omp #P3' && s.value('p3') == ' ' )
    }

    buff = cs.serialize
    cs_copy = CCS::deserialize(buffer: buff)
    1000.times {
      s = cs_copy.sample
      assert( s.check )
      active_params = extract_active_parameters(s.values)
      active_params.each { |par|
        refute_equal( CCS::Inactive, s.value(par) )
      }
      (all_params - active_params).each { |par|
        assert_equal( CCS::Inactive, s.value(par) )
      }
      refute( s.value('p1') == '#pragma omp #P2' && s.value('p2') == ' ' )
      refute( s.value('p1') == '#pragma omp #P3' && s.value('p3') == ' ' )
    }

  end
end
