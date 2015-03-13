var assert = require('assert');
var ref = require('ref');
var ffi = require('ffi');
var exec = require('child_process').exec;
var execSync = require('child_process').execSync;
var spawn = require('child_process').spawn;

describe('libtrusterd /ldd test', function() {
  //afterEach(gc)
  it('shuld work when we change the conf', function(done) {
    function testProc() {
      var rtn = execSync("mruby/build/host/mrbgems/mruby-http2/nghttp2/src/nghttp http://127.0.0.1:8080/ldd");
      console.log("rtn = ["+rtn+"]");
      rtn = rtn.toString();
      if(process.platform == "linux") {
        assert((rtn.indexOf("libc.so")>0) && (rtn.indexOf("libdl.so")>0));
      }
      exec("mruby/build/host/mrbgems/mruby-http2/nghttp2/src/nghttp http://127.0.0.1:8080/exit",
          function(err,stdout,stderr){
            console.log(stdout);
            console.log(stderr);
      });
    }
    var trusterd = spawn("examples/testOnly.sh");
    trusterd.on('close', function (code) {
      assert.equal(0,code);
      done(); 
    });
    var hasNotAccess = true;
    trusterd.stdout.on('data', function(data){ 
      if(hasNotAccess) {
        testProc();
        hasNotAccess = false;
      }
    });
    trusterd.stderr.on('data', function(data){
      if(hasNotAccess) {
        testProc();
        hasNotAccess = false;
      }
    });
  });

});
