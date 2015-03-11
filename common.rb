module Libtrusterd
class Util
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
end
end
