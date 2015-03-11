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
  sh "#{MRUBY_ROOT}/bin/mrbc -BcheckFile checkFile.rb"
  sh "#{MRUBY_ROOT}/bin/mrbc -BcommonUtil common.rb"
  if RUBY_PLATFORM =~ /darwin/i
    sh "gcc #{cflags} -shared -fPIC trusterdBoot.c checkFile.c common.c #{ldflags} #{ldflags_before_libs} #{libs} -o #{LIB_FULL_NAME}"
  end
  if RUBY_PLATFORM =~ /linux/i
    sh "#{MRUBY_ROOT}/bin/mrbc -BwatchFileLinux watchFileLinux.rb"
    ldflags_before_libs.gsub!("libnghttp2.a","")
    sh "clang -shared #{cflags} -fPIC trusterdBoot.c watchFileLinux.c common.c #{ldflags} -L#{ldflags_before_libs} #{libs} -lnghttp2 -o #{LIB_FULL_NAME}"
  end
end
