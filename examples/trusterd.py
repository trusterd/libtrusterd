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

def py_cb_func(script):
  print("py_cb_func",script)
  return 0

CBFUNC=CFUNCTYPE(c_int,c_char_p)
cbfunc=CBFUNC(py_cb_func)
path = "./test.conf.rb"
ret = trusterd_boot_func(create_string_buffer(path.encode('UTF-8')),cbfunc)
print ('trusterd_boot_func returned:', ret)
