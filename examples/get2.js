var fs = require('fs');
var ref = require('ref');
var ffi = require('ffi');

if(process.platform === "darwin") {
var funcPtr = ffi.Function('int', ['string']);

var mylib = ffi.Library('../libtrusterd', {
  'boot': ['int', ['string', funcPtr]]
});

// onReult will call by trusterd.
var onResult = function(resultVal) {
  console.log('Result is', resultVal);
  return 0;
}

// start http2 trusterd.
var script = "a = MyCall.procpathname\nputs a\nres = HTTP2::Client.http2_get2( 'https://wiki.matsumoto-r.jp/doku.php?id=start', 'kjunichi')\np HTTP2::Response.new(res).response_headers";
mylib.boot(script,onResult);
mylib.boot("res=HTTP2::Client.http2_get('https://wiki.matsumoto-r.jp/doku.php?id=start')\np res.response_headers",onResult);
}
