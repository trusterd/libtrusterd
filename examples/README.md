# Examples

```bash
ln -s ../ssl .
```

# Go

## OSX

```bash
go build trusterdGo.go
install_name_tool -change libtrusterd.dylib ../libtrusterd.dylib trusterdGo
./trusterdGo
```

## Other

```bash
go run trusterdGo.go
```

# JavaScript

```
npm install node-ffi/node-ffi#gh-241
npm install ref
node trusterd.js
```
# OCaml

```
opam install extlib ctypes
ocamlfind ocamlopt -o trusterdML -linkpkg -package ctypes,ctypes.foreign,extlib trusterdML.ml 
./trusterdML &
```

# Rust

```
cd trusterd_rs
cargo build
```

## OSX

```
install_name_tool -change libtrusterd.dylib ../../libtrusterd.dylib target/debug/trusterd_rs
cargo run
```

## Linux

```
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:../..
cargo run
```

# Python

```
python trusterd.py
```

both 2.x,3.4 can run.

