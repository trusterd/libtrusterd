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

// onReult will call by trusterd.
var onResult = function(resultVal) {
  console.log('Result is', resultVal);
  return 0;
}
// start http2 trusterd.
var script = "puts 1+2";
assert.equal(3,mylib.boot(script,onResult));
});

});

