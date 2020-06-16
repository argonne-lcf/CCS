[ '../lib', 'lib' ].each { |d| $:.unshift(d) if File::directory?(d) }
require 'minitest/autorun'
require 'cconfigspace'

class CConfigSpaceTest < Minitest::Test
  def setup
    CCS.init
  end

  def test_version
    ver = CCS.version
    assert(ver.kind_of?(CCS::Version))
  end
end
