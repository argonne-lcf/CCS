import unittest
import re
import sys
sys.path.insert(1, '.')
sys.path.insert(1, '..')
import cconfigspace as ccs

class TestConfigurationSpace(unittest.TestCase):

  def test_create(self):
    cs = ccs.ConfigurationSpace(name = "space")
    self.assertEqual( ccs.CONFIGURATION_SPACE, cs.object_type )
    self.assertEqual( "space", cs.name )
    self.assertIsInstance( cs.rng, ccs.Rng )
    self.assertEqual( 0, cs.num_hyperparameters )
    self.assertEqual( [], cs.conditions )
    self.assertEqual( [], cs.forbidden_clauses )
    h1 = ccs.NumericalHyperparameter()
    h2 = ccs.NumericalHyperparameter()
    h3 = ccs.NumericalHyperparameter()
    cs.add_hyperparameter(h1)
    cs.add_hyperparameters([h2, h3])
    self.assertEqual( 3, cs.num_hyperparameters )
    self.assertEqual( h1, cs.hyperparameter(0) )
    self.assertEqual( h2, cs.hyperparameter(1) )
    self.assertEqual( h3, cs.hyperparameter(2) )
    self.assertEqual( [h1, h2, h3], cs.hyperparameters )
    self.assertEqual( h2, cs.hyperparameter_by_name(h2.name) )
    cs.check(cs.default_configuration)
    c = cs.sample()
    cs.check(c)
    self.assertEqual( cs.handle.value, c.configuration_space.handle.value )
    cs.check_values(cs.sample().values)
    for c in cs.samples(100):
      cs.check(c)

  def test_set_distribution(self):
    cs = ccs.ConfigurationSpace(name = "space")
    h1 = ccs.NumericalHyperparameter()
    h2 = ccs.NumericalHyperparameter()
    h3 = ccs.NumericalHyperparameter()
    cs.add_hyperparameters([h1, h2, h3])
    distributions = [ ccs.UniformDistribution.float(lower = 0.1, upper = 0.3),
                      ccs.UniformDistribution.float(lower = 0.2, upper = 0.6) ]
    d = ccs.MultivariateDistribution(distributions = distributions)
    cs.set_distribution(d, [h1, h2])
    (dist, indx) = cs.get_hyperparameter_distribution(h1)
    self.assertEqual( d.handle.value, dist.handle.value )
    self.assertEqual( 0, indx )
    (dist, indx) = cs.get_hyperparameter_distribution(h2)
    self.assertEqual( d.handle.value, dist.handle.value )
    self.assertEqual( 1, indx )
    cs.set_distribution(d, [h3, h1])
    (dist, indx) = cs.get_hyperparameter_distribution(h1)
    self.assertEqual( d.handle.value, dist.handle.value )
    self.assertEqual( 1, indx )
    (dist, indx) = cs.get_hyperparameter_distribution(h3)
    self.assertEqual( d.handle.value, dist.handle.value )
    self.assertEqual( 0, indx )


  def test_conditions(self):
    h1 = ccs.NumericalHyperparameter(lower = -1.0, upper = 1.0, default = 0.0)
    h2 = ccs.NumericalHyperparameter(lower = -1.0, upper = 1.0)
    h3 = ccs.NumericalHyperparameter(lower = -1.0, upper = 1.0)
    cs = ccs.ConfigurationSpace(name = "space")
    cs.add_hyperparameters([h1, h2, h3])
    e1 = ccs.Expression(t = ccs.LESS, nodes = [h2, 0.0])
    cs.set_condition(h3, e1)
    e2 = ccs.Expression(t = ccs.LESS, nodes = [h3, 0.0])
    cs.set_condition(h1, e2)
    e3 = ccs.Expression(t = ccs.LESS, nodes = [h1, 0.0])
    cs.add_forbidden_clause(e3)
    conditions = cs.conditions
    conditional_hyperparameters = cs.conditional_hyperparameters
    unconditional_hyperparameters = cs.unconditional_hyperparameters
    self.assertEqual( 3, len(conditions) )
    self.assertEqual( e2.handle.value, conditions[0].handle.value )
    self.assertIsNone( conditions[1] )
    self.assertEqual( e1.handle.value, conditions[2].handle.value )
    self.assertEqual( 2, len(conditional_hyperparameters) )
    self.assertEqual( 1, len(unconditional_hyperparameters) )
    self.assertEqual( h1.handle.value, conditional_hyperparameters[0].handle.value )
    self.assertEqual( h3.handle.value, conditional_hyperparameters[1].handle.value )
    self.assertEqual( h2.handle.value, unconditional_hyperparameters[0].handle.value )
    forbidden_clauses = cs.forbidden_clauses
    self.assertEqual( 1, len(forbidden_clauses) )
    self.assertEqual( e3.handle.value, forbidden_clauses[0].handle.value )

  def extract_active_parameters(self, values):
    res = ['p1']
    for v in values:
      if v != ccs.ccs_inactive:
        res += ["p{}".format(m[1]) for m in re.finditer('#P(\d)', v)]
    return res

  def test_omp(self):
    p1 = ccs.CategoricalHyperparameter(
      name = 'p1',
      values = [
        ' ',
        '#pragma omp #P2',
        '#pragma omp target teams distribute #P2',
        '#pragma omp target teams distribute #P4',
        '#pragma omp #P3'])
    p2 = ccs.CategoricalHyperparameter(
      name = 'p2',
      values = [
        ' ',
        'parallel for #P3',
        'parallel for #P5',
        'parallel for #P6'])
    p3 = ccs.CategoricalHyperparameter(
      name = 'p3',
      values = [' ', 'simd'])
    p4 = ccs.CategoricalHyperparameter(
      name = 'p4',
      values = [
        ' ',
        'dist_schedule(static)',
        'dist_schedule(static, #P8)'])
    p5 = ccs.CategoricalHyperparameter(
      name = 'p5',
      values = [
        ' ',
        'schedule(#P7,#P8)',
        'schedule(#P7)'])
    p6 = ccs.CategoricalHyperparameter(
      name = 'p6',
      values = [
        ' ',
        'numthreads(#P9)'])
    p7 = ccs.CategoricalHyperparameter(
      name = 'p7',
      values = [
        'static',
        'dynamic'])
    p8 = ccs.OrdinalHyperparameter(
      name = 'p8',
      values = ['1', '8', '16'])
    p9 = ccs.OrdinalHyperparameter(
      name = 'p9',
      values = ['1', '8', '16'])

    cs = ccs.ConfigurationSpace(name = "omp")
    cs.add_hyperparameters([p1, p2, p3, p4, p5, p6, p7, p8, p9])

    cond0 = ccs.Expression(t = ccs.EQUAL, nodes = [p1, '#pragma omp #P2'])
    cond1 = ccs.Expression(t = ccs.EQUAL, nodes = [p1, '#pragma omp target teams distribute #P2'])
    cond2 = ccs.Expression(t = ccs.EQUAL, nodes = [p1, '#pragma omp target teams distribute #P4'])
    cond3 = ccs.Expression(t = ccs.EQUAL, nodes = [p1, '#pragma omp #P3'])

    cond4 = ccs.Expression(t = ccs.EQUAL, nodes = [p2, 'parallel for #P3'])
    cond5 = ccs.Expression(t = ccs.EQUAL, nodes = [p2, 'parallel for #P5'])
    cond6 = ccs.Expression(t = ccs.EQUAL, nodes = [p2, 'parallel for #P6'])

    cond7 = ccs.Expression(t = ccs.EQUAL, nodes = [p4, 'dist_schedule(static, #P8)'])

    cond8 = ccs.Expression(t = ccs.EQUAL, nodes = [p5, 'schedule(#P7)'])
    cond9 = ccs.Expression(t = ccs.EQUAL, nodes = [p5, 'schedule(#P7,#P8)'])

    cond10 = ccs.Expression(t = ccs.EQUAL, nodes = [p6, 'numthreads(#P9)'])

    cs.set_condition(p2, ccs.Expression(t = ccs.OR, nodes = [cond0, cond1]))
    cs.set_condition(p4, cond2)
    cs.set_condition(p3, ccs.Expression(t = ccs.OR, nodes = [cond3, cond4]))
    cs.set_condition(p5, cond5)
    cs.set_condition(p6, cond6)
    cs.set_condition(p7, ccs.Expression(t = ccs.OR, nodes = [cond8, cond9]))
    cs.set_condition(p8, ccs.Expression(t = ccs.OR, nodes = [cond7, cond9]))
    cs.set_condition(p9, cond10)

    forbiddena = ccs.Expression(t = ccs.EQUAL, nodes = [p1, '#pragma omp #P2'])
    forbiddenb = ccs.Expression(t = ccs.EQUAL, nodes = [p2, ' '])
    forbidden0 = ccs.Expression(t = ccs.AND, nodes = [forbiddena, forbiddenb])

    forbiddenc = ccs.Expression(t = ccs.EQUAL, nodes = [p1, '#pragma omp #P3'])
    forbiddend = ccs.Expression(t = ccs.EQUAL, nodes = [p3, ' '])
    forbidden1 = ccs.Expression(t = ccs.AND, nodes = [forbiddenc, forbiddend])
    cs.add_forbidden_clauses([forbidden0, forbidden1])

    all_params = [ "p{}".format(i) for i in range(1,10) ]
    for i in range(1000):
      s = cs.sample()
      s.check()
      active_params = self.extract_active_parameters(s.values)
      for par in active_params:
        self.assertNotEqual( ccs.ccs_inactive, s.value(par) )
      for par in list(set(all_params) - set(active_params)):
        self.assertEqual( ccs.ccs_inactive, s.value(par) )
      self.assertFalse( s.value('p1') == '#pragma omp #P2' and  s.value('p2') == ' ' )
      self.assertFalse( s.value('p1') == '#pragma omp #P3' and  s.value('p3') == ' ' )

  def test_omp_parse(self):
    p1 = ccs.CategoricalHyperparameter(
      name = 'p1',
      values = [
        ' ',
        '#pragma omp #P2',
        '#pragma omp target teams distribute #P2',
        '#pragma omp target teams distribute #P4',
        '#pragma omp #P3'])
    p2 = ccs.CategoricalHyperparameter(
      name = 'p2',
      values = [
        ' ',
        'parallel for #P3',
        'parallel for #P5',
        'parallel for #P6'])
    p3 = ccs.CategoricalHyperparameter(
      name = 'p3',
      values = [' ', 'simd'])
    p4 = ccs.CategoricalHyperparameter(
      name = 'p4',
      values = [
        ' ',
        'dist_schedule(static)',
        'dist_schedule(static, #P8)'])
    p5 = ccs.CategoricalHyperparameter(
      name = 'p5',
      values = [
        ' ',
        'schedule(#P7,#P8)',
        'schedule(#P7)'])
    p6 = ccs.CategoricalHyperparameter(
      name = 'p6',
      values = [
        ' ',
        'numthreads(#P9)'])
    p7 = ccs.CategoricalHyperparameter(
      name = 'p7',
      values = [
        'static',
        'dynamic'])
    p8 = ccs.OrdinalHyperparameter(
      name = 'p8',
      values = ['1', '8', '16'])
    p9 = ccs.OrdinalHyperparameter(
      name = 'p9',
      values = ['1', '8', '16'])

    cs = ccs.ConfigurationSpace(name = "omp")
    cs.add_hyperparameters([p1, p2, p3, p4, p5, p6, p7, p8, p9])

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
      s.check()
      active_params = self.extract_active_parameters(s.values)
      for par in active_params:
        self.assertNotEqual( ccs.ccs_inactive, s.value(par) )
      for par in list(set(all_params) - set(active_params)):
        self.assertEqual( ccs.ccs_inactive, s.value(par) )
      self.assertFalse( s.value('p1') == '#pragma omp #P2' and  s.value('p2') == ' ' )
      self.assertFalse( s.value('p1') == '#pragma omp #P3' and  s.value('p3') == ' ' )

    buff = cs.serialize()
    cs_copy = ccs.Object.deserialize(buff)
    for i in range(1000):
      s = cs_copy.sample()
      s.check()
      active_params = self.extract_active_parameters(s.values)
      for par in active_params:
        self.assertNotEqual( ccs.ccs_inactive, s.value(par) )
      for par in list(set(all_params) - set(active_params)):
        self.assertEqual( ccs.ccs_inactive, s.value(par) )
      self.assertFalse( s.value('p1') == '#pragma omp #P2' and  s.value('p2') == ' ' )
      self.assertFalse( s.value('p1') == '#pragma omp #P3' and  s.value('p3') == ' ' )


if __name__ == '__main__':
    unittest.main()
