require 'minitest/autorun'
require_relative '../lib/cconfigspace'

class CConfigSpaceTestConfigurationSpace < Minitest::Test
  def setup
    CCS.init
  end

  def test_create
    h1 = CCS::NumericalParameter::Float.new
    h2 = CCS::NumericalParameter::Float.new
    h3 = CCS::NumericalParameter::Float.new
    cs = CCS::ConfigurationSpace::new(name: "space", parameters: [h1, h2, h3])
    assert_equal( :CCS_OBJECT_TYPE_CONFIGURATION_SPACE, cs.object_type )
    assert_equal( "space", cs.name )
    assert( cs.rng.kind_of?(CCS::Rng) )
    assert_equal( 3, cs.num_parameters )
    assert_equal( [nil, nil, nil], cs.conditions )
    assert_equal( [], cs.forbidden_clauses )
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
    h1 = CCS::NumericalParameter::Float.new
    h2 = CCS::NumericalParameter::Float.new
    h3 = CCS::NumericalParameter::Float.new
    cs = CCS::ConfigurationSpace::new(name: "space", parameters: [h1, h2, h3])
    ds = CCS::DistributionSpace::new(configuration_space: cs)
    distributions = [ CCS::UniformDistribution::Float.new(lower: 0.1, upper: 0.3), CCS::UniformDistribution::Float.new(lower: 0.2, upper: 0.6) ]
    d = CCS::MultivariateDistribution::new(distributions: distributions)
    ds.set_distribution(d, [h1, h2])
    dist, indx = ds.get_parameter_distribution(h1)
    assert_equal( d.handle, dist.handle )
    assert_equal( 0, indx )
    dist, indx = ds.get_parameter_distribution(h2)
    assert_equal( d.handle, dist.handle )
    assert_equal( 1, indx )
    ds.set_distribution(d, [h3, h1])
    dist, indx = ds.get_parameter_distribution(h1)
    assert_equal( d.handle, dist.handle )
    assert_equal( 1, indx )
    dist, indx = ds.get_parameter_distribution(h3)
    assert_equal( d.handle, dist.handle )
    assert_equal( 0, indx )
  end

  def test_conditions
    h1 = CCS::NumericalParameter::Float.new(lower: -1.0, upper: 1.0, default: 0.0)
    h2 = CCS::NumericalParameter::Float.new(lower: -1.0, upper: 1.0)
    h3 = CCS::NumericalParameter::Float.new(lower: -1.0, upper: 1.0)
    e1 = CCS::Expression::Less.new(left: h2, right: 0.0)
    e2 = CCS::Expression::Less.new(left: h3, right: 0.0)
    f1 = CCS::Expression::Less.new(left: h1, right: 0.0)
    cs = CCS::ConfigurationSpace::new(name: "space", parameters: [h1, h2, h3], conditions: {h1 => e2, h3 => e1}, forbidden_clauses: [f1])
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
    assert_equal( f1.handle, forbidden_clauses[0].handle )
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

    forbiddena = CCS::Expression::Equal.new(left: p1, right: '#pragma omp #P2')
    forbiddenb = CCS::Expression::Equal.new(left: p2, right: ' ')
    forbidden0 = CCS::Expression::And.new(left: forbiddena, right: forbiddenb)

    forbiddenc = CCS::Expression::Equal.new(left: p1, right: '#pragma omp #P3')
    forbiddend = CCS::Expression::Equal.new(left: p3, right: ' ')
    forbidden1 = CCS::Expression::And.new(left: forbiddenc, right: forbiddend)

    cond0 = CCS::Expression::Equal.new(left: p1, right: '#pragma omp #P2')
    cond1 = CCS::Expression::Equal.new(left: p1, right: '#pragma omp target teams distribute #P2')
    cond2 = CCS::Expression::Equal.new(left: p1, right: '#pragma omp target teams distribute #P4')
    cond3 = CCS::Expression::Equal.new(left: p1, right: '#pragma omp #P3')

    cond4 = CCS::Expression::Equal.new(left: p2, right: 'parallel for #P3')
    cond5 = CCS::Expression::Equal.new(left: p2, right: 'parallel for #P5')
    cond6 = CCS::Expression::Equal.new(left: p2, right: 'parallel for #P6')

    cond7 = CCS::Expression::Equal.new(left: p4, right: 'dist_schedule(static, #P8)')

    cond8 = CCS::Expression::Equal.new(left: p5, right: 'schedule(#P7)')
    cond9 = CCS::Expression::Equal.new(left: p5, right: 'schedule(#P7,#P8)')

    cond10 = CCS::Expression::Equal.new(left: p6, right: 'numthreads(#P9)')

    cs = CCS::ConfigurationSpace::new(name: "omp", parameters: [p1, p2, p3, p4, p5, p6, p7, p8, p9],
                                      conditions: {
                                        p2 => CCS::Expression::Or.new(left: cond0, right: cond1),
                                        p4 => cond2,
                                        p3 => CCS::Expression::Or.new(left: cond3, right: cond4),
                                        p5 => cond5,
                                        p6 => cond6,
                                        p7 => CCS::Expression::Or.new(left: cond8, right: cond9),
                                        p8 => CCS::Expression::Or.new(left: cond7, right: cond9),
                                        p9 => cond10 },
                                      forbidden_clauses: [forbidden0, forbidden1])

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

    cs = CCS::ConfigurationSpace::new(name: "omp", parameters: [p1, p2, p3, p4, p5, p6, p7, p8, p9],
                                      conditions: {
                                        p2 => "p1 # ['#pragma omp #P2', '#pragma omp target teams distribute #P2']",
                                        p4 => "p1 == '#pragma omp target teams distribute #P4'",
                                        p3 => "p1 == '#pragma omp #P3' || p2 == 'parallel for #P3'",
                                        p5 => "p2 == 'parallel for #P5'",
                                        p6 => "p2 == 'parallel for #P6'",
                                        p7 => "p5 # ['schedule(#P7)', 'schedule(#P7,#P8)']",
                                        p8 => "p4 == 'dist_schedule(static, #P8)' || p5 == 'schedule(#P7,#P8)'",
                                        p9 => "p6 == 'numthreads(#P9)'" },
                                      forbidden_clauses: ["p1 == '#pragma omp #P2' && p2 == ' '",
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
