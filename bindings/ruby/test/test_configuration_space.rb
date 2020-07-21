[ '../lib', 'lib' ].each { |d| $:.unshift(d) if File::directory?(d) }
require 'minitest/autorun'
require 'cconfigspace'

class CConfigSpaceTestConfigurationSpace < Minitest::Test
  def setup
    CCS.init
  end

  def test_create
    cs = CCS::ConfigurationSpace::new(name: "space")
    assert_equal( :CCS_CONFIGURATION_SPACE, cs.object_type )
    assert_equal( "space", cs.name )
    assert( cs.rng.kind_of?(CCS::Rng) )
    assert_equal( 0, cs.num_hyperparameters )
    assert_equal( [], cs.conditions )
    assert_equal( [], cs.forbidden_clauses )
    h1 = CCS::NumericalHyperparameter::new
    h2 = CCS::NumericalHyperparameter::new
    h3 = CCS::NumericalHyperparameter::new
    cs.add_hyperparameter(h1)
    cs.add_hyperparameters([h2, h3])
    assert_equal( 3, cs.num_hyperparameters )
    assert_equal( h1, cs.hyperparameter(0) )
    assert_equal( h2, cs.hyperparameter(1) )
    assert_equal( h3, cs.hyperparameter(2) )
    assert_equal( [h1, h2, h3], cs.hyperparameters )
    assert_equal( h2, cs.hyperparameter_by_name(h2.name) )
    cs.check(cs.default_configuration)
    c = cs.sample
    cs.check(c)
    assert_equal( cs.handle, c.configuration_space.handle )
    cs.check_values(cs.sample.values)
    cs.samples(100).each { |c|
      cs.check(c)
    }
  end

  def test_conditions
    h1 = CCS::NumericalHyperparameter::new(lower: -1.0, upper: 1.0, default: 0.0)
    h2 = CCS::NumericalHyperparameter::new(lower: -1.0, upper: 1.0)
    h3 = CCS::NumericalHyperparameter::new(lower: -1.0, upper: 1.0)
    cs = CCS::ConfigurationSpace::new(name: "space")
    cs.add_hyperparameters([h1, h2, h3])
    e1 = CCS::Expression::new(type: :CCS_LESS, nodes: [h2, 0.0])
    cs.set_condition(h3, e1)
    e2 = CCS::Expression::new(type: :CCS_LESS, nodes: [h3, 0.0])
    cs.set_condition(h1, e2)
    e3 = CCS::Expression::new(type: :CCS_LESS, nodes: [h1, 0.0])
    cs.add_forbidden_clause(e3)
    conditions = cs.conditions
    assert_equal( 3, conditions.length )
    assert_equal( e2.handle, conditions[0].handle )
    assert_nil( conditions[1] )
    assert_equal( e1.handle, conditions[2].handle )
    forbidden_clauses = cs.forbidden_clauses
    assert_equal( 1, forbidden_clauses.length )
    assert_equal( e3.handle, forbidden_clauses[0].handle )
  end

  def test_omp
    p1 = CCS::CategoricalHyperparameter::new(
      name: 'p1',
      values: [
        ' ',
        '#pragma omp #P2',
        '#pragma omp target teams distribute #P2',
        '#pragma omp target teams distribute #P4',
        '#pragma omp #P3'])
    p2 = CCS::CategoricalHyperparameter::new(
      name: 'p2',
      values: [
        ' ',
        'parallel for #P3',
        'parallel for #P5',
        'parallel for #P6'])
    p3 = CCS::CategoricalHyperparameter::new(
      name: 'p3',
      values: [' ', 'simd'])
    p4 = CCS::CategoricalHyperparameter::new(
      name: 'p4',
      values: [
        ' ',
        'dist_schedule(static)',
        'dist_schedule(static, #P8)'])
    p5 = CCS::CategoricalHyperparameter::new(
      name: 'p5',
      values: [
        ' ',
        'schedule(#P7,#P8)',
        'schedule(#P7)'])
    p6 = CCS::CategoricalHyperparameter::new(
      name: 'p6',
      values: [
        ' ',
        'numthreads(#P9)'])
    p7 = CCS::CategoricalHyperparameter::new(
      name: 'p7',
      values: [
        'static',
        'dynamic'])
    p8 = CCS::OrdinalHyperparameter::new(
      name: 'p8',
      values: ['1', '8', '16'])
    p9 = CCS::OrdinalHyperparameter::new(
      name: 'p9',
      values: ['1', '8', '16'])

    cs = CCS::ConfigurationSpace::new(name: "omp")
    cs.add_hyperparameters([p1, p2, p3, p4, p5, p6, p7, p8, p9])

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
      s.check
      active_params = ['p1'] + s.values.select { |v|
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
