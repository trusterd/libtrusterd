var assert = require('assert');
var ffi = require('ffi');
var exec = require('child_process').exec;
var execSync = require('child_process').execSync;
var spawn = require('child_process').spawn;

describe('libtrusterd boot_from_file_path test', function() {
  //afterEach(gc)

  it('should be able to file error handling', function() {
    var funcPtr = ffi.Function('int', ['string']);
    var mylib = ffi.Library('./libtrusterd', {
      'boot_from_file_path': ['int', ['string', funcPtr]]
    });
    var filepath = "./trusterd.test01.conf.rb";

    // onReult will call by trusterd.
    var onResult = function(resultVal) {
      assert(false) // shouldn't get here
      return 0;
    }
    // start mruby
    assert.equal(-1, mylib.boot_from_file_path(filepath, onResult));
  });
  it('should be work :)', function(done) {
    function killTrusterd() {
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
        killTrusterd();
        hasNotAccess = false;
      }
    });
    trusterd.stderr.on('data', function(data){
      if(hasNotAccess) {
        killTrusterd();
        hasNotAccess = false;
      }
    });
  });

  it('shuld work when we change the conf', function(done) {
    function testProc() {
      var rtn = execSync("mruby/build/host/mrbgems/mruby-http2/nghttp2/src/nghttp http://127.0.0.1:8080/test");
      execSync("perl -pi -e 's/8080/8000/' examples/test.conf.rb;perl -MTime::HiRes -e 'Time::HiRes::sleep(0.3)'");
      rtn = execSync("mruby/build/host/mrbgems/mruby-http2/nghttp2/src/nghttp http://127.0.0.1:8000/test");
      assert.equal("/test\nhello trusterd!",rtn);
      execSync("perl -pi -e 's/8000/8080/' examples/test.conf.rb;perl -MTime::HiRes -e 'Time::HiRes::sleep(0.3)'");
      rtn = execSync("mruby/build/host/mrbgems/mruby-http2/nghttp2/src/nghttp http://127.0.0.1:8080/test");
      assert.equal("/test\nhello trusterd!",rtn);

      rtn = execSync("mruby/build/host/mrbgems/mruby-http2/nghttp2/src/nghttp http://127.0.0.1:8080/exit");
      //console.log(rtn);
      assert(true);
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
    //trusterd.stdin.end();
    //assert.equal(0, mylib.boot_from_file_path(filepath, onResult));
  });

/*
  it('shuld work when we change the conf',function() {
    var funcPtr = ffi.Function('int', ['string']);
    var mylib = ffi.Library('./libtrusterd', {
      'boot_from_file_path': ['int', ['string', funcPtr]]
    });
    var filepath = "./examples/test.conf.rb";

    exec("sleep 1; mruby/build/host/mrbgems/mruby-http2/nghttp2/src/nghttp http://127.0.0.1:8080/test;perl -pi -e 's/8080/8000/' examples/test.conf.rb;sleep 1;mruby/build/host/mrbgems/mruby-http2/nghttp2/src/nghttp http://127.0.0.1:8000/test;perl -pi -e 's/8000/8080/' examples/test.conf.rb;sleep 1; mruby/build/host/mrbgems/mruby-http2/nghttp2/src/nghttp http://127.0.0.1:8080/exit",
      function(err,stdout,stderr){
        console.log(stdout);
        console.log(stderr);
        //done();
      });

    // onReult will call by trusterd.
    var onResult = function(resultVal) {
      //assert(false) // shouldn't get here

      console.log("onResult Called.");
      return 0;
    }
    assert.equal(0, mylib.boot_from_file_path(filepath, onResult));


}); */ 
});
