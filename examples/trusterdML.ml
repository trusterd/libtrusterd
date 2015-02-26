open Ctypes
open PosixTypes
open Foreign

let cb_t = string @-> returning int

Dl.dlopen ~filename:"libtrusterd.dylib" ~flags:[Dl.RTLD_NOW]

let boot = foreign "boot" (string @-> funptr cb_t @-> returning int)

let f x =
begin
 print_string x;
 -1;
end

boot "puts 1+2;MyCall.my_exec('Hello,this is mruby!')" f
