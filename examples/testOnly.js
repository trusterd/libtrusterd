var fs = require('fs');
var ref = require('ref');
var ffi = require('ffi');

var funcPtr = ffi.Function('int', ['string']);
var funcCgiPtr = ffi.Function('string', ['string']);
var mylib = ffi.Library('./libtrusterd', {
  'boot_from_file_path': ['int', ['string', funcPtr]],
  'boot_from_file_path_cgi': ['int', ['string', funcCgiPtr]]
});

// onReult will call by trusterd.
var onResult = function(resultVal) {
  console.log('Result is', resultVal);
  return 0;
}

var onRequest = function(resultVal) {
  console.log('Result is', resultVal);
  return "<html>Hello, trusted,this is node.js.</html>";
}
// start http2 trusterd.
mylib.boot_from_file_path("examples/test.conf.rb",onResult);
//
