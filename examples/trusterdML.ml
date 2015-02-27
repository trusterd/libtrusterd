open Ctypes
open PosixTypes
open Foreign
open Std

let cb_t = string @-> returning int;;

Dl.dlopen ~filename:"../libtrusterd.dylib" ~flags:[Dl.RTLD_NOW]

let boot = foreign "boot" (string @-> funptr cb_t @-> returning int)

let f x =
begin
 print_string x;
 -1;
end


let read_file filename =
 let chan = open_in filename in
  Std.input_all chan

let rbscript = read_file "../trusterd.conf.rb";;

boot rbscript f
