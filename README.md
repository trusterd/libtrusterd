# libtrusterd

[![Build Status](https://travis-ci.org/trusterd/libtrusterd.svg?branch=master)](https://travis-ci.org/trusterd/libtrusterd)

[Japanese](README.ja.md)

trustred is a one of the famous web server which implements http2 by mruby language.

I know a word isomorphic.But Some time I like to use some languages at the same time.
It's so fun.

you can use http2 server with your favorite language which support libffi.


## Getting started

```bash
git clone https://github.com/kjunichi/libtrusterd.git
cd libtrusterd
```

```bash
git clone https://github.com/mruby/mruby.git
cd mruby
cp -f ../build_config.rb .
rake
cd ..
rake
```

``` bash
openssl genrsa -des3 -out key.pem 2048
openssl req -new -key key.pem -out cert.csr
openssl x509 -days 365 -req -signkey key.pem < cert.csr >cert.pem
openssl rsa -in key.pem -out key.pem
cp cert.pem key.pem ssl
```

## Examples

```bash
cd examples
npm install kjunichi/node-ffi
npm install ref
ln -s ../ssl .
node trusterd &
open https://127.0.0.1:8080/test
```
### For more examples

See the [examples/README.md](examples/README.md) file.

# Link

- http://trusterd.github.io/
