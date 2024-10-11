from xmlrpc.server import SimpleXMLRPCServer
from threading import Lock
from dataclasses import dataclass
import sys
import traceback
import base64
import ctypes as ct
from random import choice
sys.path.insert(1, '.')
sys.path.insert(1, '..')
import cconfigspace as ccs

class TunerData:
  def __init__(self):
    self.history = []
    self.optima = []

def deletef(tuner):
  return None

def askf(tuner, features, count):
  if count is None:
    return (None, 1)
  else:
    cs = tuner.search_space
    return (cs.samples(count), count)

def tellf(tuner, evaluations):
  tuner.tuner_data.history += evaluations
  for e in evaluations:
    discard = False
    new_optima = []
    for o in tuner.tuner_data.optima:
      if discard:
        new_optima.append(o)
      else:
        c = e.compare(o)
        if c == ccs.Comparison.EQUIVALENT or c == ccs.Comparison.WORSE:
          discard = True
          new_optima.append(o)
        elif c == ccs.Comparison.NOT_COMPARABLE:
          new_optima.append(o)
    if not discard:
      new_optima.append(e)
    tuner.tuner_data.optima = new_optima
  return None

def get_historyf(tuner, features):
  return tuner.tuner_data.history

def get_optimaf(tuner, features):
  return tuner.tuner_data.optima

def suggestf(tuner, features):
  if not tuner.tuner_data.optima:
    return ask(tuner, 1)
  else:
    return choice(tuner.tuner_data.optima).configuration

def get_vector_data(otype, name):
  return (ccs.UserDefinedTuner.get_vector(delete = deletef, ask = askf, tell = tellf, get_optima = get_optimaf, get_history = get_historyf, suggest = suggestf), TunerData())

@dataclass
class TunerStruct:
  tuner: list
  handle_map: ccs.Map = None

class MyServer(SimpleXMLRPCServer):
  def serve_forever(self):
    self.quit = 0
    while not self.quit:
      self.handle_request()

s = MyServer(('localhost', 8000))
s.register_introspection_functions()

@s.register_function
def connected():
  return True

count = 0
tuners = dict()
tuners_store = dict()
mutex = Lock()

def get_tstruct(ident):
  with mutex:
    return tuners[ident]

@s.register_function
def create(name, os_string):
  global count
  handle_map = ccs.Map()
  os = ccs.deserialize(buffer = os_string.data, handle_map = handle_map, map_handles = True)
  t = ccs.UserDefinedTuner(name = name, objective_space = os, delete = deletef, ask = askf, tell = tellf, get_optima = get_optimaf, get_history = get_historyf, suggest = suggestf, tuner_data = TunerData())

  rev_map = ccs.Map()
  for (k, v) in handle_map.pairs():
    if isinstance(v, ccs.Context) or isinstance(v, ccs.TreeSpace):
      rev_map[ccs.Object(v.handle, retain = False, auto_release = False)] = k
  buff = rev_map.serialize()
  tstruct = TunerStruct(t, handle_map)
  ident = None
  with mutex:
    ident = count
    count += 1
    tuners[ident] = tstruct
  return (ident, buff)

@s.register_function
def ask(ident, count):
  return [c.serialize() for c in get_tstruct(ident).tuner.ask(count)]

@s.register_function
def tell(ident, evals):
  tstruct = get_tstruct(ident)
  tstruct.tuner.tell([ccs.deserialize(buffer = e.data, handle_map = tstruct.handle_map) for e in evals])
  return True

@s.register_function
def history(ident):
  return [e.serialize() for e in get_tstruct(ident).tuner.history()]

@s.register_function
def history_size(ident):
  return get_tstruct(ident).tuner.history_size()

@s.register_function
def optima(ident):
  return [e.serialize() for e in get_tstruct(ident).tuner.optima()]

@s.register_function
def num_optima(ident):
  return get_tstruct(ident).tuner.num_optima()

@s.register_function
def suggest(ident):
  return get_tstruct(ident).tuner.suggest().serialize()

@s.register_function
def save(ident):
  tstruct = get_tstruct(ident)
  with mutex:
    tuners_store[tstruct.tuner.name] = tstruct.tuner.serialize()
  return True

@s.register_function
def load(name):
  global count
  buff = None
  with mutex:
    buff = tuners_store[name]
  t = ccs.deserialize(buffer = buff, vector_callback = get_vector_data)
  tstruct = TunerStruct(t)
  ident = None
  with mutex:
    ident = count
    count += 1
    tuners[ident] = tstruct
  return (ident, t.objective_space.serialize())

@s.register_function
def set_handle_map(ident, map_str):
  get_tstruct(ident).handle_map = ccs.deserialize(buffer = map_str.data)
  return True

@s.register_function
def kill():
  s.quit = 1
  return True

s.serve_forever()
