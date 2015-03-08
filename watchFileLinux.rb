def runTrusterd(path)
  pid = Process.fork() {
    f = File.open(path,"r")
    conf = f.read
    f.close
    eval conf
  }
  return pid
end

def reloadTrusterdConf(path,pid)
  Process.kill('SIGTERM',pid)
  Process.waitpid pid
  puts pid
  pid = runTrusterd(path)

  puts pid
  puts path
  return pid
end

def watchFileLinux(path)

  path=File.realpath(path)
  pid = runTrusterd(path)
  dirname = File.dirname(path)
  notifier = Inotify::Notifier.new

  notifier.watch(dirname, :all_events) do |event|
    #puts event.inspect
    if(event.name == File.basename(path))
      if(event.events==[:modify] || event.events == [:moved_to, :move])
        pid = reloadTrusterdConf(path, pid)
      end
    end
  end
  # .process blocks until events are available
  notifier.process while true
end

#watchFileLinux("examples/test.conf.rb")
