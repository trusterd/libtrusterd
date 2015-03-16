import os
import platform
import sys
from ctypes import *

if platform.system() == "Darwin":
  libext=".dylib"
else:
  libext=".so"

trusterd_module = CDLL('../libtrusterd'+libext)
trusterd_boot_func = trusterd_module.boot_from_file_path
trusterd_boot_func.restype = c_int
trusterd_boot_cgi_func = trusterd_module.boot_from_file_path_cgi
trusterd_boot_cgi_func.restype = c_int

def py_cb_func(script):
  print("py_cb_func",script)
  return 0

def py_cb_cgi_func(script):
  #print("py_cb_cgi_func",script)
  msg = "Hello, Trusterd.this is Python."
  buf = create_string_buffer(msg.encode('UTF-8'))
  c = cast(buf, POINTER(c_char))
  return addressof(c.contents)
  #return buf

CBFUNC=CFUNCTYPE(c_int,c_char_p)
cbfunc=CBFUNC(py_cb_func)
#CBCGIFUNC=CFUNCTYPE(c_int,c_char_p)
CBCGIFUNC=CFUNCTYPE(c_char_p,c_char_p)
cbcgifunc=CBCGIFUNC(py_cb_cgi_func)

path = "./test.conf.rb"
#ret = trusterd_boot_func(create_string_buffer(path.encode('UTF-8')),cbfunc)
ret = trusterd_boot_cgi_func(create_string_buffer(path.encode('UTF-8')),cbcgifunc)
print ('trusterd_boot_func returned:', ret)
