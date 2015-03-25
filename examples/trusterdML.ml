open Ctypes
open PosixTypes
open Foreign
open Std

let cb_t = string @-> returning int;;

let cgi_cb_t = string @-> returning string;;

Dl.dlopen ~filename:"../libtrusterd.dylib" ~flags:[Dl.RTLD_NOW]

let boot = foreign "boot" (string @-> funptr cb_t @-> returning int)

let boot_cgi = foreign "boot_from_file_path_cgi" (string @-> funptr cgi_cb_t @-> returning int)

let f x =
begin
 print_string x;
 -1;
end

let response_str = "This is Ocaml.";;

let g x =
begin
 (*print_string x;*)
 x.[1] <- 'z';
 response_str;
end

let read_file filename =
 let chan = open_in filename in
  Std.input_all chan

let rbscript = read_file "test.conf.rb";;

boot rbscript f
(*boot_cgi "./test.conf.rb" g*)
