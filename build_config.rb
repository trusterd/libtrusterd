MRuby::Build.new do |conf|
  # load specific toolchain settings

  toolchain :gcc

  enable_debug

  conf.gem :github => 'matsumoto-r/mruby-simplehttp'
  conf.gem :github => 'trusterd/mruby-http2'
  conf.gem :github => 'iij/mruby-io'
  conf.gem :github => 'iij/mruby-dir'
  conf.gem :github => 'iij/mruby-socket'
  conf.gem :github => 'iij/mruby-pack'
  conf.gem :github => 'iij/mruby-process'
  conf.gem :github => 'mattn/mruby-onig-regexp'
  conf.gem :github => 'mattn/mruby-json'
  conf.gem :github => 'kjunichi/mruby-getprocpath'
  conf.gem :github => 'ksss/mruby-signal', :branch => 'master'
  
  if RUBY_PLATFORM =~ /linux/i
    conf.gem :github => 'kjunichi/mruby-inotify'
    conf.gem :github => 'ksss/mruby-file-stat'
  end

  # include the default GEMs
  conf.gembox 'full-core'

  # C compiler settings
  if RUBY_PLATFORM =~ /linux/i
    conf.cc do |cc|
      cc.command = ENV['CC'] || 'qrintf gcc'
      cc.flags << '-fPIC '
    end

    # Linker settings
    conf.linker do |linker|
      linker.flags_after_libraries << '-ljemalloc'
    end
  end

  # when using openssl from brew
  if RUBY_PLATFORM =~ /darwin/i
    sh 'CFLAGS="-I/usr/local/opt/openssl/include";export CFLAGS'
    sh 'LDFLAGS="-L/usr/local/opt/openssl/lib";export LDFLAGS'
    
    conf.cc do |cc|
      cc.flags << '-I /usr/local/include -I/usr/local/opt/openssl/include'
    end

    conf.linker do |linker|
      linker.option_library_path << ' -L/usr/local/lib/ -L/usr/local/opt/openssl/lib/'
    end
  end
end
