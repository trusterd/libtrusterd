desc 'libtrusterd.dylib'


MRUBY_ROOT = "./mruby"

LIB_NAME = "libtrusterd"
if RUBY_PLATFORM =~ /darwin/i
  LIB_EXT = "dylib"
end
if RUBY_PLATFORM =~ /linux/i
  LIB_EXT = "so"
end
LIB_FULL_NAME = LIB_NAME + "." + LIB_EXT

task :default => "all"
task :all do
  cflags=`#{MRUBY_ROOT}/bin/mruby-config --cflags`
  cflags.chomp!()
  ldflags=`#{MRUBY_ROOT}/bin/mruby-config --ldflags`
  ldflags.chomp!()
  ldflags_before_libs=`#{MRUBY_ROOT}/bin/mruby-config --ldflags-before-libs`
  ldflags_before_libs.chomp!()
  libs=`#{MRUBY_ROOT}/bin/mruby-config --libs`
  libs.chomp!()
p ldflags_before_libs
sh "gcc #{cflags} -shared -fPIC trusterdBoot.c #{ldflags} #{ldflags_before_libs} #{libs} -o #{LIB_FULL_NAME}"
end
