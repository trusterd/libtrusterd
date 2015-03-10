def runTrusterd(path)
#  p path
  mypid = Process.fork {
    f = File.open(path,"r")
    conf = f.read
    f.close
    pid = Process.pid
    pr = Proc.new { |signo|
      puts "SIGTERM!"
      childrenPid = pid.to_i * (-1)
      puts "children pid : " + childrenPid.to_s
      begin
        Process.kill('SIGTERM',childrenPid)
        Process.waitpid(childrenPid)
      rescue => e
        p e
        p "but we continue"
      end
      exit(0)
      break 0
    }
    Signal.trap(:TERM, pr)
    eval conf
    break 0
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

  puts "watchFileLinux start["+Process.pid.to_s+"]"

  begin
    path=File.realpath(path)
  rescue
    return -1
  end

  pid = runTrusterd(path)
  puts "Now start trusterd pid : " + pid.to_s
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
