var assert = require('assert');
var ref = require('ref');
var ffi = require('ffi');

describe('libtrusterd boot_with_filepath test', function() {
  afterEach(gc)

  it('should be work :)', function() {
    var funcPtr = ffi.Function('int', ['string']);
    var mylib = ffi.Library('./libtrusterd', {
      'boot_with_filepath': ['int', ['string', funcPtr]]
    });
    var filepath = "./trusterd.test01.conf.rb";

    // onReult will call by trusterd.
    var onResult = function(resultVal) {
      assert(false) // shouldn't get here
      return 0;
    }

    // start mruby
    assert.equal(script.length, mylib.boot(filepath, onResult));
  });
});
