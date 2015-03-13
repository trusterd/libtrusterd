module Libtrusterd
  class Util


    def Util.ldd
      path = Getprocpath.get
      lddcmd = `which ldd`
      if lddcmd.length == 0
        # sorry for windows
        lddcmd = "otool -L"
      else
        lddcmd="ldd"
      end
      rtn = `#{lddcmd} #{path}`
      return rtn
    end

    def Util.write_pid
      f = File.open('libtrusterd.pid','w')
      f.write Process.pid
      f.close
    end

    def Util.kill_children
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

    def Util.set_sigterm
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

#Libtrusterd::Util.write_pid
#Libtrusterd::Util.set_sigterm
#sleep(1000)
#puts Libtrusterd::Util.ldd
