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
  if RUBY_PLATFORM =~ /darwin/i
    sh "gcc #{cflags} -shared -fPIC trusterdBoot.c #{ldflags} #{ldflags_before_libs} #{libs} -o #{LIB_FULL_NAME}"
  end
  if RUBY_PLATFORM =~ /linux/i
    ldflags_before_libs.gsub!("libnghttp2.a","")
    sh "echo #{ldflags_before_libs}"
    sh "clang -shared #{cflags} -fPIC trusterdBoot.c #{ldflags} -L#{ldflags_before_libs} #{libs} -lnghttp2 -o #{LIB_FULL_NAME}"
  end
end
