require 'minitest/autorun'
require_relative '../lib/cconfigspace'

class CConfigSpaceTestTuner < Minitest::Test
  def create_tuning_problem
    h1 = CCS::NumericalParameter::Float.new(lower: -5.0, upper: 5.0)
    h2 = CCS::NumericalParameter::Float.new(lower: -5.0, upper: 5.0)
    h3 = CCS::NumericalParameter::Float.new(lower: -5.0, upper: 5.0)
    cs = CCS::ConfigurationSpace::new(name: "cspace", parameters: [h1, h2, h3])
    v1 = CCS::NumericalParameter::Float.new(lower: -Float::INFINITY, upper: Float::INFINITY)
    v2 = CCS::NumericalParameter::Float.new(lower: -Float::INFINITY, upper: Float::INFINITY)
    e1 = CCS::Expression::Variable::new(parameter: v1)
    e2 = CCS::Expression::Variable::new(parameter: v2)
    CCS::ObjectiveSpace::new(name: "ospace", search_space: cs, parameters: [v1, v2], objectives: [e1, e2])
  end

  def test_create_random
    os = create_tuning_problem
    t = CCS::RandomTuner::new(name: "tuner", objective_space: os)
    t2 = CCS::Object::from_handle(t)
    assert_equal( t.class, t2.class)
    assert_equal( "tuner", t.name )
    assert_equal( :CCS_TUNER_TYPE_RANDOM, t.type )
    func = lambda { |(x, y, z)|
      [(x-2)**2, Math.sin(z+y)]
    }
    evals = t.ask(100).collect { |c|
      CCS::Evaluation::new(objective_space: os, configuration: c, values: func[c.values])
    }
    t.tell evals
    hist = t.history
    assert_equal(100, hist.size)
    evals = t.ask(100).collect { |c|
      CCS::Evaluation::new(objective_space: os, configuration: c, values: func[c.values])
    }
    t.tell evals
    assert_equal(200, t.history_size)
    objs = t.optima.collect(&:objective_values).sort
    objs.collect { |(_, v)| v }.each_cons(2) { |v1, v2| assert( (v1 <=> v2) > 0 ) }
    assert( t.optima.collect(&:configuration).include?(t.suggest) )

    buff = t.serialize
    t_copy = CCS.deserialize(buffer: buff)
    hist = t_copy.history
    assert_equal(200, hist.size)
    assert_equal(t.num_optima, t_copy.num_optima)
    objs = t_copy.optima.collect(&:objective_values).sort
    objs.collect { |(_, v)| v }.each_cons(2) { |v1, v2| assert( (v1 <=> v2) > 0 ) }
    assert( t_copy.optima.collect(&:configuration).include?(t_copy.suggest) )
  end

  class TunerData
    attr_accessor :history, :optima
    def initialize
      @history = []
      @optima = []
    end
  end

  def test_user_defined
    del = lambda { |tuner| nil }
    ask = lambda { |tuner, _, count|
      if count
        cs = tuner.search_space
        [cs.samples(count), count]
      else
        [nil, 1]
      end
    }
    tell = lambda { |tuner, evaluations|
      tuner.tuner_data.history.concat(evaluations)
      evaluations.each { |e|
        discard = false
        tuner.tuner_data.optima = tuner.tuner_data.optima.collect { |o|
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
        tuner.tuner_data.optima.push(e) unless discard
      }
    }
    get_history = lambda { |tuner, _|
      tuner.tuner_data.history
    }
    get_optima = lambda { |tuner, _|
      tuner.tuner_data.optima
    }
    suggest = lambda { |tuner, _|
      if tuner.tuner_data.optima.empty?
        ask.call(tuner, 1)
      else
        tuner.tuner_data.optima.sample.configuration
      end
    }
    get_vector_data = lambda { |otype, name|
      assert_equal(:CCS_OBJECT_TYPE_TUNER, otype)
      assert_equal("tuner", name)
      [CCS::UserDefinedTuner.get_vector(del: del, ask: ask, tell: tell, get_optima: get_optima, get_history: get_history, suggest: suggest), TunerData.new]
    }

    os = create_tuning_problem
    t = CCS::UserDefinedTuner::new(name: "tuner", objective_space: os, del: del, ask: ask, tell: tell, get_optima: get_optima, get_history: get_history, suggest: suggest, tuner_data: TunerData.new)
    t2 = CCS::Object::from_handle(t)
    assert_equal( t.class, t2.class)
    assert_equal( "tuner", t.name )
    assert_equal( :CCS_TUNER_TYPE_USER_DEFINED, t.type )
    func = lambda { |(x, y, z)|
      [(x-2)**2, Math.sin(z+y)]
    }
    evals = t.ask(100).collect { |c|
      CCS::Evaluation::new(objective_space: os, configuration: c, values: func[c.values])
    }
    t.tell evals
    hist = t.history
    assert_equal(100, hist.size)
    evals = t.ask(100).collect { |c|
      CCS::Evaluation::new(objective_space: os, configuration: c, values: func[c.values])
    }
    t.tell evals
    assert_equal(200, t.history_size)
    optims = t.optima
    objs = optims.collect(&:objective_values).sort
    objs.collect { |(_, v)| v }.each_cons(2) { |v1, v2| assert( (v1 <=> v2) > 0 ) }
    assert( t.optima.collect(&:configuration).include?(t.suggest) )

    buff = t.serialize
    t_copy = CCS::deserialize(buffer: buff, vector_callback: get_vector_data)
    hist = t_copy.history
    assert_equal(200, hist.size)
    assert_equal(t.num_optima, t_copy.num_optima)
    objs = t_copy.optima.collect(&:objective_values).sort
    objs.collect { |(_, v)| v }.each_cons(2) { |v1, v2| assert( (v1 <=> v2) > 0 ) }
    assert( t_copy.optima.collect(&:configuration).include?(t_copy.suggest) )

    t.serialize(path: 'tuner.ccs')
    t_copy = CCS::deserialize(path: 'tuner.ccs', vector_callback: get_vector_data)
    hist = t_copy.history
    assert_equal(200, hist.size)
    assert_equal(t.num_optima, t_copy.num_optima)
    objs = t_copy.optima.collect(&:objective_values).sort
    objs.collect { |(_, v)| v }.each_cons(2) { |v1, v2| assert( (v1 <=> v2) > 0 ) }
    assert( t_copy.optima.collect(&:configuration).include?(t_copy.suggest) )
    File.delete('tuner.ccs')

    f = File.open('tuner.ccs', "wb")
    t.serialize(file_descriptor: f.fileno)
    f.close
    f = File.open('tuner.ccs', "rb")
    t_copy = CCS::deserialize(file_descriptor: f.fileno, vector_callback: get_vector_data)
    f.close
    hist = t_copy.history
    assert_equal(200, hist.size)
    assert_equal(t.num_optima, t_copy.num_optima)
    objs = t_copy.optima.collect(&:objective_values).sort
    objs.collect { |(_, v)| v }.each_cons(2) { |v1, v2| assert( (v1 <=> v2) > 0 ) }
    assert( t_copy.optima.collect(&:configuration).include?(t_copy.suggest) )
    File.delete('tuner.ccs')
  end

  require 'open3'
  require 'xmlrpc/client'
  require 'base64'
  class TunerProxy
    attr_reader :server
    attr_reader :id
    attr_reader :handle_map
    attr_reader :objective_space
    def initialize(name: "", objective_space: nil)
      @server = XMLRPC::Client.new2('http://localhost:8080/RPC2')
      connected = false
      start = Time.now
      while !connected
        begin
          connected = server.call('connected')
        rescue
          raise if Time.now - start > 10
        end
      end
      if objective_space
        @objective_space = objective_space
        @id, result = server.call('tuner.create', name, Base64.encode64(objective_space.serialize))
        @handle_map = CCS.deserialize(buffer: Base64.decode64(result))
      else
        @id, result = server.call('tuner.load', name)
        @handle_map = CCS::Map.new
        @objective_space = CCS.deserialize(buffer: Base64.decode64(result), handle_map: @handle_map, map_handles: true)
        map = CCS::Map.new
        @handle_map.pairs.select { |_ , v|
          v.is_a?(CCS::Context) || v.is_a?(CCS::TreeSpace)
        }.each { |k, v|
          map[CCS::Object::new(v.handle, retain: false, auto_release: false)] = k
        }
        server.call('tuner.set_handle_map', @id, Base64.encode64(map.serialize))
      end
    end

    def ask(count = 1)
      server.call('tuner.ask', @id, count).collect { |c|
        CCS.deserialize(buffer: Base64.decode64(c), handle_map: @handle_map)
      }
    end

    def tell(evals = [])
      evals_serialized = evals.collect { |e| Base64.encode64(e.serialize) }
      server.call('tuner.tell', @id, evals_serialized)
      self
    end

    def history
      server.call('tuner.history', @id).collect { |e|
        CCS.deserialize(buffer: Base64.decode64(e), handle_map: @handle_map)
      }
    end

    def history_size
      server.call('tuner.history_size', @id)
    end

    def optima
      server.call('tuner.optima', @id).collect { |e|
        CCS.deserialize(buffer: Base64.decode64(e), handle_map: @handle_map)
      }
    end

    def num_optima
      server.call('tuner.num_optima', @id)
    end

    def suggest
      e = server.call('tuner.suggest', @id)
      CCS.deserialize(buffer: Base64.decode64(e), handle_map: @handle_map)
    end

    def save
      server.call('tuner.save', @id)
    end
  end

  def test_server
    begin
      pid = nil
      thr = Thread.new do
        Open3.popen2e(RbConfig.ruby, File.join(File.dirname(__FILE__), 'tuner_server.rb')) { |stdin, stdout_stderr, wait_thr|
          pid = wait_thr.pid
          stdout_stderr.read
        }
      end
      os = create_tuning_problem
      t = TunerProxy.new(name: "my_tuner", objective_space: os)
      func = lambda { |(x, y, z)|
        [(x-2)**2, Math.sin(z+y)]
      }
      evals = t.ask(100).collect { |c|
        CCS::Evaluation::new(objective_space: os, configuration: c, values: func[c.values])
      }
      t.tell evals
      hist = t.history
      assert_equal(100, hist.size)
      evals = t.ask(100).collect { |c|
        CCS::Evaluation::new(objective_space: os, configuration: c, values: func[c.values])
      }
      t.tell evals
      assert_equal(200, t.history_size)
      objs = t.optima.collect(&:objective_values).sort
      objs.collect { |(_, v)| v }.each_cons(2) { |v1, v2| assert( (v1 <=> v2) > 0 ) }
      assert( t.optima.collect(&:configuration).include?(t.suggest) )

      t.save
      t_copy = TunerProxy.new(name: "my_tuner")
      hist = t_copy.history
      assert_equal(200, hist.size)
      assert_equal(t.num_optima, t_copy.num_optima)
      objs = t_copy.optima.collect(&:objective_values).sort
      objs.collect { |(_, v)| v }.each_cons(2) { |v1, v2| assert( (v1 <=> v2) > 0 ) }
      assert( t_copy.optima.collect(&:configuration).include?(t_copy.suggest) )
    ensure
      Process.kill("SIGHUP", pid)
      thr.join
    end
  end
end

