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
npm install kjunichi/node-ffi
npm install ref
node trusterd.js
```
