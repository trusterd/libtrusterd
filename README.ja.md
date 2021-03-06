# これは何か

http2サーバーの[trusterd](https://github.com/trusterd/trusterd)を共有ライブラリ化することで、mruby以外の言語から
libffiを経由してhttp2サーバーを立ち上げ、http2サーバーでリクエストを受け付けた
際に、呼び出し元をコールバックする事を可能とするライブラリ。

# 使い方

```bash
git clone https://github.com/kjunichi/libtrusterd.git
cd libtrusterd
```

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

## SSL証明の準備

なんとかして、SSLの証明書を用意するw。

以下の記事が多少参考になるかも。。

- http://kjunichi.cocolog-nifty.com/misc/2013/09/llhttps-0db2.html

出来たら、
ssl配下に配置する。名前は以下の様にする。

- key.pem
- cert.pem



### それが出来ない人

諦めが肝心w

trusterd.conf.rbを以下の様に編集することで、TLSを無効化出来る。

```diff
- # :tls => false,
+   :tls => false,
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
### Go編

```go
package main

// #cgo LDFLAGS: -L. -ltrusterd
/*
 #include <stdlib.h>
 #include <stdio.h>


 typedef int (*FUNCPTR)(char *script);
 int myCallback(char*);

 int boot(char*, FUNCPTR);

 static void testGoGo() {
 boot("puts 1+2",(FUNCPTR)myCallback);
 printf("%s","hello, Go!\n");
 fflush(stdout);
 }
*/
import "C"

import . "fmt"

//export myCallback
func myCallback(s *C.char) C.int {
    //C.printf("%s\n",s)
    Println(C.GoString(s))
    return 0
}

func main() {
    C.testGoGo()
}
```

```bash
go build trusterdGo.go
./trusterdGo
```
