var assert = require('assert');
var ref = require('ref');
var ffi = require('ffi');

var SO_EXT = ".so";
if(process.platform === "darwin") {
SO_EXT=".dylib";
}

var funcPtr = ffi.Function('int', ['string']);
var mylib = ffi.Library('../libtrusterd'+SO_EXT, {
'boot': ['int', ['string', funcPtr]]
});
// onReult will call by trusterd.
var onResult = function(resultVal) {
  console.log('Result is', resultVal);
  return 0;
}
// start http2 trusterd.
var script = "puts 1+2";
mylib.boot(script,onResult);

