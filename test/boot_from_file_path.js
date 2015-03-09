var assert = require('assert');
var ref = require('ref');
var ffi = require('ffi');
var exec = require('child_process').exec;

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
  /*
  it('should be work :)', function() {
    var funcPtr = ffi.Function('int', ['string']);
    var mylib = ffi.Library('./libtrusterd', {
      'boot_from_file_path': ['int', ['string', funcPtr]]
    });
    var filepath = "./examples/test.conf.rb";

    // onReult will call by trusterd.
    var onResult = function(resultVal) {
      assert(false) // shouldn't get here
      return 0;
    }

    // start mruby
    exec("sleep 1; mruby/build/host/mrbgems/mruby-http2/nghttp2/src/nghttp http://127.0.0.1:8080/exit");
    assert.equal(0, mylib.boot_from_file_path(filepath, onResult));
  });
  */
});
