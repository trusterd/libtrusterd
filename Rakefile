desc 'libtrusterd.dylib'


MRUBY_ROOT = "./mruby"
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
sh "gcc #{cflags} -shared -fPIC trusterdBoot.c #{ldflags} #{ldflags_before_libs} #{libs} -o libtrusterd.dylib"
end
