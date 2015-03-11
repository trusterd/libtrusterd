def kill_children
  pid = Process.pid
  childrenPid = `pgrep -P #{pid}`
  childrenPid = childrenPid.split("\n")
  puts "children pid : " + childrenPid.to_s
  childrenPid.each{|pid|
    begin
      Process.kill('SIGTERM',pid.to_i)
      Process.waitpid(pid.to_i)
    rescue => e
      p e
      p "but we continue"
    end
  }
end 

def set_sigterm
  pid = Process.pid
  pr = Proc.new { |signo|
    puts "SIGTERM!"
    kill_children
    exit(0)
  }
  Signal.trap(:TERM, pr)
end

def runTrusterd(path)
#  p path
  mypid = Process.fork {
    f = File.open(path,"r")
    conf = f.read
    f.close
    set_sigterm
    eval conf
  }
  if(mypid == 0) 
    puts "Oops! pid=0,so we exit it"
    exit(0)
  end
  return mypid
end

def reloadTrusterdConf(path,pid)
  Process.kill('SIGTERM',pid)
  Process.waitpid pid
#  puts pid
  pid = runTrusterd(path)

#  puts pid
#  puts path
  return pid
end

def watchFileLinux(path)
  # write out my process id
  f = File.open('libtrusterd.pid','w')
  f.write Process.pid
  f.close

  begin
    path=File.realpath(path)
  rescue
    return -1
  end

  pid = runTrusterd(path)
#  puts "Now start trusterd pid : " + pid.to_s
  set_sigterm
  dirname = File.dirname(path)
  notifier = Inotify::Notifier.new
  notifier.watch(dirname, :all_events) do |event|
    #p event.inspect
    if(event.name == File.basename(path))
      if(event.events==[:modify] || event.events == [:moved_to, :move])
        pid = reloadTrusterdConf(path, pid)
      end
    end
  end
  # .process blocks until events are available
  notifier.process while true
  puts "I'm here!"
  return 0
end
#watchFileLinux("examples/test.conf.rb")
