var fs = require('fs');
var ref = require('ref');
var ffi = require('ffi');

var funcPtr = ffi.Function('int', ['string']);

var mylib = ffi.Library('./libtrusterd.dylib', {
  'boot': ['int', ['string', funcPtr]]
});

var onResult = function(resultVal) {
  console.log('Result is', resultVal);
  return 0;
}
var script = fs.readFileSync("./trusterd.conf.rb",{encoding:"UTF-8"});
mylib.boot(script,onResult);
