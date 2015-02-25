var assert = require('assert');
var ref = require('ref');
var ffi = require('ffi');

describe('libtrusterd boot test', function () {
  afterEach(gc)

  it('should be work :)', function () {
    var funcPtr = ffi.Function('int', ['string']);
    var mylib = ffi.Library('./libtrusterd', {
      'boot': ['int', ['string', funcPtr]]
    });
    var script = "puts 1+2";

    // onReult will call by trusterd.
    var onResult = function(resultVal) {
      assert(false) // shouldn't get here
      return 0;
    }
    
    // start mruby
    assert.equal(script.length,mylib.boot(script,onResult));
    
  });

  it('should be callback from mruby:)', function () {
    var testString = "abc0123";
    var funcPtr = ffi.Function('int', ['string']);
    var mylib = ffi.Library('./libtrusterd', {
      'boot': ['int', ['string', funcPtr]]
    });
    var script = 'MyCall.my_exec("'+testString+'")';

    // onReult will call by trusterd.
    var onResult = function(resultVal) {
      console.log('Result is', resultVal);
      assert.equal(testString,resultVal);
      return 0;
    }

    mylib.boot(script,onResult);
  });
});

