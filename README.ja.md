# これは何か

http2サーバーの[trusterd](https://github.com/trusterd/trusterd)を共有ライブラリ化することで、mruby以外の言語から
libffiを経由してhttp2サーバーを立ち上げ、http2サーバーでリクエストを受け付けた
際に、呼び出し元をコールバックする事を可能とするライブラリ。

# 使い方

## OSXでHomebrweを使用している場合

以下の環境変数の設定とlibopensslの有効化が必要。

### 依存ライブラリのインストール

```bash
brew install libev
brew install libxml2
brew install libevent
brew install zlib
brew install spdylay
```

### 環境変数の設定

```bash
export ACLOCAL_PATH=/usr/local/Cellar/libxml2/2.9.2/share/aclocal/
export PKG_CONFIG_PATH=/usr/local/Cellar/openssl/1.0.2/lib/pkgconfig:/usr/local/Cellar/zlib/1.2.8/lib/pkgconfig/
```

### libopensslの有効化

```bash
brew link openssl --force
```

作業終了後に

```
brew unlink openssl
```

## mrubyの準備

```
git clone https://github.com/mruby/mruby.git
cd mruby
cp -f ../build_config.rb .
rake
cd ..
```

## ライブラリ本体のビルド

```
rake
```

## 動かす

### js編

```
npm install kjunichi/node-ffi
npm install ref
```

```js
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
```

### OCaml編

```
opam update
opam install ctypes
```

```ml
#use "topfind";;
#require "ctypes.foreign";;
#require "ctypes.top";;
open Ctypes;;
open PosixTypes;;
open Foreign;;

let cb_t = string @-> returning int;;

Dl.dlopen ~filename:"libtrusterd.dylib" ~flags:[Dl.RTLD_NOW];;

let boot = foreign "boot" (string @-> funptr cb_t @-> returning int);;

let f x =
begin
 print_string x;
 -1;
end;;

boot "puts 1+2;MyCall.my_exec('Hello,this is mruby!')" f;;
```
