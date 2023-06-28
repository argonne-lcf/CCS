import unittest
import re
import sys
sys.path.insert(1, '.')
sys.path.insert(1, '..')
import cconfigspace as ccs

class TestConfigurationSpace(unittest.TestCase):

  def test_create(self):
    h1 = ccs.NumericalParameter.Float()
    h2 = ccs.NumericalParameter.Float()
    h3 = ccs.NumericalParameter.Float()
    cs = ccs.ConfigurationSpace(name = "space", parameters = [h1, h2, h3])
    self.assertEqual( ccs.ObjectType.CONFIGURATION_SPACE, cs.object_type )
    self.assertEqual( "space", cs.name )
    self.assertIsInstance( cs.rng, ccs.Rng )
    self.assertEqual( 3, cs.num_parameters )
    self.assertEqual( [None, None, None], cs.conditions )
    self.assertEqual( [], cs.forbidden_clauses )
    self.assertEqual( h1, cs.parameter(0) )
    self.assertEqual( h2, cs.parameter(1) )
    self.assertEqual( h3, cs.parameter(2) )
    self.assertEqual( [h1, h2, h3], cs.parameters )
    self.assertEqual( h2, cs.parameter_by_name(h2.name) )
    self.assertTrue( cs.check(cs.default_configuration) )
    c = cs.sample()
    self.assertTrue( cs.check(c) )
    self.assertEqual( cs.handle.value, c.configuration_space.handle.value )
    self.assertTrue( cs.check_values(cs.sample().values) )
    for c in cs.samples(100):
      self.assertTrue( cs.check(c) )

  def test_set_distribution(self):
    h1 = ccs.NumericalParameter.Float()
    h2 = ccs.NumericalParameter.Float()
    h3 = ccs.NumericalParameter.Float()
    cs = ccs.ConfigurationSpace(name = "space", parameters = [h1, h2, h3])
    distributions = [ ccs.UniformDistribution.Float(lower = 0.1, upper = 0.3),
                      ccs.UniformDistribution.Float(lower = 0.2, upper = 0.6) ]
    d = ccs.MultivariateDistribution(distributions = distributions)
    cs.set_distribution(d, [h1, h2])
    (dist, indx) = cs.get_parameter_distribution(h1)
    self.assertEqual( d.handle.value, dist.handle.value )
    self.assertEqual( 0, indx )
    (dist, indx) = cs.get_parameter_distribution(h2)
    self.assertEqual( d.handle.value, dist.handle.value )
    self.assertEqual( 1, indx )
    cs.set_distribution(d, [h3, h1])
    (dist, indx) = cs.get_parameter_distribution(h1)
    self.assertEqual( d.handle.value, dist.handle.value )
    self.assertEqual( 1, indx )
    (dist, indx) = cs.get_parameter_distribution(h3)
    self.assertEqual( d.handle.value, dist.handle.value )
    self.assertEqual( 0, indx )


  def test_conditions(self):
    h1 = ccs.NumericalParameter.Float(lower = -1.0, upper = 1.0, default = 0.0)
    h2 = ccs.NumericalParameter.Float(lower = -1.0, upper = 1.0)
    h3 = ccs.NumericalParameter.Float(lower = -1.0, upper = 1.0)
    cs = ccs.ConfigurationSpace(name = "space", parameters = [h1, h2, h3])
    e1 = ccs.Expression.Less(left = h2, right = 0.0)
    cs.set_condition(h3, e1)
    e2 = ccs.Expression.Less(left = h3, right = 0.0)
    cs.set_condition(h1, e2)
    e3 = ccs.Expression.Less(left = h1, right = 0.0)
    cs.add_forbidden_clause(e3)
    conditions = cs.conditions
    conditional_parameters = cs.conditional_parameters
    unconditional_parameters = cs.unconditional_parameters
    self.assertEqual( 3, len(conditions) )
    self.assertEqual( e2.handle.value, conditions[0].handle.value )
    self.assertIsNone( conditions[1] )
    self.assertEqual( e1.handle.value, conditions[2].handle.value )
    self.assertEqual( 2, len(conditional_parameters) )
    self.assertEqual( 1, len(unconditional_parameters) )
    self.assertEqual( h1.handle.value, conditional_parameters[0].handle.value )
    self.assertEqual( h3.handle.value, conditional_parameters[1].handle.value )
    self.assertEqual( h2.handle.value, unconditional_parameters[0].handle.value )
    forbidden_clauses = cs.forbidden_clauses
    self.assertEqual( 1, len(forbidden_clauses) )
    self.assertEqual( e3.handle.value, forbidden_clauses[0].handle.value )

  def extract_active_parameters(self, values):
    res = ['p1']
    for v in values:
      if v != ccs.inactive:
        res += ["p{}".format(m[1]) for m in re.finditer('#P(\d)', v)]
    return res

  def test_omp(self):
    p1 = ccs.CategoricalParameter(
      name = 'p1',
      values = [
        ' ',
        '#pragma omp #P2',
        '#pragma omp target teams distribute #P2',
        '#pragma omp target teams distribute #P4',
        '#pragma omp #P3'])
    p2 = ccs.CategoricalParameter(
      name = 'p2',
      values = [
        ' ',
        'parallel for #P3',
        'parallel for #P5',
        'parallel for #P6'])
    p3 = ccs.CategoricalParameter(
      name = 'p3',
      values = [' ', 'simd'])
    p4 = ccs.CategoricalParameter(
      name = 'p4',
      values = [
        ' ',
        'dist_schedule(static)',
        'dist_schedule(static, #P8)'])
    p5 = ccs.CategoricalParameter(
      name = 'p5',
      values = [
        ' ',
        'schedule(#P7,#P8)',
        'schedule(#P7)'])
    p6 = ccs.CategoricalParameter(
      name = 'p6',
      values = [
        ' ',
        'numthreads(#P9)'])
    p7 = ccs.CategoricalParameter(
      name = 'p7',
      values = [
        'static',
        'dynamic'])
    p8 = ccs.OrdinalParameter(
      name = 'p8',
      values = ['1', '8', '16'])
    p9 = ccs.OrdinalParameter(
      name = 'p9',
      values = ['1', '8', '16'])

    cs = ccs.ConfigurationSpace(name = "omp", parameters = [p1, p2, p3, p4, p5, p6, p7, p8, p9])

    cond0 = ccs.Expression.Equal(left = p1, right = '#pragma omp #P2')
    cond1 = ccs.Expression.Equal(left = p1, right = '#pragma omp target teams distribute #P2')
    cond2 = ccs.Expression.Equal(left = p1, right = '#pragma omp target teams distribute #P4')
    cond3 = ccs.Expression.Equal(left = p1, right = '#pragma omp #P3')

    cond4 = ccs.Expression.Equal(left = p2, right = 'parallel for #P3')
    cond5 = ccs.Expression.Equal(left = p2, right = 'parallel for #P5')
    cond6 = ccs.Expression.Equal(left = p2, right = 'parallel for #P6')

    cond7 = ccs.Expression.Equal(left = p4, right = 'dist_schedule(static, #P8)')

    cond8 = ccs.Expression.Equal(left = p5, right = 'schedule(#P7)')
    cond9 = ccs.Expression.Equal(left = p5, right = 'schedule(#P7,#P8)')

    cond10 = ccs.Expression.Equal(left = p6, right = 'numthreads(#P9)')

    cs.set_condition(p2, ccs.Expression.Or(left = cond0, right = cond1))
    cs.set_condition(p4, cond2)
    cs.set_condition(p3, ccs.Expression.Or(left = cond3, right = cond4))
    cs.set_condition(p5, cond5)
    cs.set_condition(p6, cond6)
    cs.set_condition(p7, ccs.Expression.Or(left = cond8, right = cond9))
    cs.set_condition(p8, ccs.Expression.Or(left = cond7, right = cond9))
    cs.set_condition(p9, cond10)

    forbiddena = ccs.ExpressionEqual(left = p1, right = '#pragma omp #P2')
    forbiddenb = ccs.ExpressionEqual(left = p2, right = ' ')
    forbidden0 = ccs.Expression.And(left = forbiddena, right = forbiddenb)

    forbiddenc = ccs.Expression.Equal(left = p1, right = '#pragma omp #P3')
    forbiddend = ccs.Expression.Equal(left = p3, right = ' ')
    forbidden1 = ccs.Expression.And(left = forbiddenc, right = forbiddend)
    cs.add_forbidden_clauses([forbidden0, forbidden1])

    all_params = [ "p{}".format(i) for i in range(1,10) ]
    for i in range(1000):
      s = cs.sample()
      self.assertTrue( s.check() )
      active_params = self.extract_active_parameters(s.values)
      for par in active_params:
        self.assertNotEqual( ccs.inactive, s.value(par) )
      for par in list(set(all_params) - set(active_params)):
        self.assertEqual( ccs.inactive, s.value(par) )
      self.assertFalse( s.value('p1') == '#pragma omp #P2' and  s.value('p2') == ' ' )
      self.assertFalse( s.value('p1') == '#pragma omp #P3' and  s.value('p3') == ' ' )

  def test_omp_parse(self):
    p1 = ccs.CategoricalParameter(
      name = 'p1',
      values = [
        ' ',
        '#pragma omp #P2',
        '#pragma omp target teams distribute #P2',
        '#pragma omp target teams distribute #P4',
        '#pragma omp #P3'])
    p2 = ccs.CategoricalParameter(
      name = 'p2',
      values = [
        ' ',
        'parallel for #P3',
        'parallel for #P5',
        'parallel for #P6'])
    p3 = ccs.CategoricalParameter(
      name = 'p3',
      values = [' ', 'simd'])
    p4 = ccs.CategoricalParameter(
      name = 'p4',
      values = [
        ' ',
        'dist_schedule(static)',
        'dist_schedule(static, #P8)'])
    p5 = ccs.CategoricalParameter(
      name = 'p5',
      values = [
        ' ',
        'schedule(#P7,#P8)',
        'schedule(#P7)'])
    p6 = ccs.CategoricalParameter(
      name = 'p6',
      values = [
        ' ',
        'numthreads(#P9)'])
    p7 = ccs.CategoricalParameter(
      name = 'p7',
      values = [
        'static',
        'dynamic'])
    p8 = ccs.OrdinalParameter(
      name = 'p8',
      values = ['1', '8', '16'])
    p9 = ccs.OrdinalParameter(
      name = 'p9',
      values = ['1', '8', '16'])

    cs = ccs.ConfigurationSpace(name = "omp", parameters = [p1, p2, p3, p4, p5, p6, p7, p8, p9])

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

    all_params = [ "p{}".format(i) for i in range(1,10) ]
    for i in range(1000):
      s = cs.sample()
      self.assertTrue( s.check() )
      active_params = self.extract_active_parameters(s.values)
      for par in active_params:
        self.assertNotEqual( ccs.inactive, s.value(par) )
      for par in list(set(all_params) - set(active_params)):
        self.assertEqual( ccs.inactive, s.value(par) )
      self.assertFalse( s.value('p1') == '#pragma omp #P2' and  s.value('p2') == ' ' )
      self.assertFalse( s.value('p1') == '#pragma omp #P3' and  s.value('p3') == ' ' )

    buff = cs.serialize()
    cs_copy = ccs.Object.deserialize(buffer = buff)
    for i in range(1000):
      s = cs_copy.sample()
      self.assertTrue( s.check() )
      active_params = self.extract_active_parameters(s.values)
      for par in active_params:
        self.assertNotEqual( ccs.inactive, s.value(par) )
      for par in list(set(all_params) - set(active_params)):
        self.assertEqual( ccs.inactive, s.value(par) )
      self.assertFalse( s.value('p1') == '#pragma omp #P2' and  s.value('p2') == ' ' )
      self.assertFalse( s.value('p1') == '#pragma omp #P3' and  s.value('p3') == ' ' )


if __name__ == '__main__':
    unittest.main()
