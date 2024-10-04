require_relative '../lib/cconfigspace'
require 'xmlrpc/server'
require 'base64'

class TunerData
  attr_accessor :history, :optima
  def initialize
    @history = []
    @optima = []
  end
end

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
  [CCS::UserDefinedTuner.get_vector(del: del, ask: ask, tell: tell, get_optima: get_optima, get_history: get_history, suggest: suggest), TunerData.new]
}

s = XMLRPC::Server.new(8080)
count = 0
tuners = {}
# Could be on disk
tuners_store = {}
mutex = Mutex.new

tstruct_byid = lambda { |id|
  mutex.synchronize {
    tuners[id]
  }
}

TunerStruct = Struct.new(:tuner, :handle_map)

s.add_handler('connected') do
  true
end

s.add_handler('tuner.create') do |name, os_string|
  handle_map = CCS::Map.new
  os = CCS.deserialize(buffer: Base64.decode64(os_string), handle_map: handle_map, map_handles: true)
  t = CCS::UserDefinedTuner::new(name: name, objective_space: os, del: del, ask: ask, tell: tell, get_optima: get_optima, get_history: get_history, suggest: suggest, tuner_data: TunerData.new)

  map = CCS::Map.new
  handle_map.pairs.select { |_ , v|
     v.is_a?(CCS::Context) || v.is_a?(CCS::TreeSpace)
    }.each { |k, v|
     map[CCS::Object::new(v.handle, retain: false, auto_release: false)] = k
    }
  tstruct = TunerStruct.new(t, handle_map)
  id = nil
  mutex.synchronize {
    id = count
    count += 1
    tuners[id] = tstruct
  }
  [id, Base64.encode64(map.serialize)]
end

s.add_handler('tuner.ask') do |id, count|
  tstruct_byid[id].tuner.ask(count).collect { |c|
    Base64.encode64(c.serialize)
  }
end

s.add_handler('tuner.tell') do |id, evals|
  tstruct = tstruct_byid[id]
  evals.collect! { |e|
    CCS.deserialize(buffer: Base64.decode64(e), handle_map: tstruct.handle_map)
  }
  tstruct.tuner.tell evals
  true
end

s.add_handler('tuner.history') do |id|
  tstruct_byid[id].tuner.history.collect { |e|
    Base64.encode64(e.serialize)
  }
end

s.add_handler('tuner.history_size') do |id|
  tstruct_byid[id].tuner.history_size
end

s.add_handler('tuner.optima') do |id|
  tstruct_byid[id].tuner.optima.collect { |e|
    Base64.encode64(e.serialize)
  }
end

s.add_handler('tuner.num_optima') do |id|
  tstruct_byid[id].tuner.num_optima
end

s.add_handler('tuner.suggest') do |id|
  e = tstruct_byid[id].tuner.suggest
  Base64.encode64(e.serialize)
end

s.add_handler('tuner.save') do |id|
  tstruct = tstruct_byid[id]
  buff = tstruct.tuner.serialize
  mutex.synchronize {
    tuners_store[tstruct.tuner.name] = buff
  }
  true
end

s.add_handler('tuner.load') do |name|
  buff = nil
  mutex.synchronize {
    buff = tuners_store[name]
  }
  t = CCS.deserialize(buffer: buff, vector_callback: get_vector_data)
  tstruct = TunerStruct.new(t, nil)
  id = nil
  mutex.synchronize {
    id = count
    count += 1
    tuners[id] = tstruct
  }
  [id, Base64.encode64(t.objective_space.serialize)]
end

s.add_handler('tuner.set_handle_map') do |id, map_str|
  tstruct_byid[id].handle_map = CCS.deserialize(buffer: Base64.decode64(map_str))
  true
end

s.serve

