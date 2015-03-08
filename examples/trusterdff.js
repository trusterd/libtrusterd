var fs = require('fs');
var ref = require('ref');
var ffi = require('ffi');

var LIBEXT=".so";
if(process.platform === "darwin") {
  LIBEXT=".dylib";
}
if(process.platform === "win32") {
  LIBEXT=".dll";
}


var funcPtr = ffi.Function('int', ['string']);
var mylib = ffi.Library('../libtrusterd', {
  'boot_from_file_path': ['int', ['string', funcPtr]]
});

// onReult will call by trusterd.
var onResult = function(resultVal) {
  console.log('Result is', resultVal);
  return 0;
}

// start http2 trusterd.
//var script = fs.readFileSync("../trusterd.conf.rb",{encoding:"UTF-8"});
mylib.boot_from_file_path("test.conf.rb",onResult);
//
