def checkFile(filepath)
  filename = filepath.split("/")[-1]
  dirpath = filepath.gsub("/"+filename,"")
  Dir.open(dirpath).each{|f|
    if filename == f
      return true
    end
  }
  return false
end

#p checkFile("/Users/kjw_junichi/work/mruby/libtrusterd/foo.txt")
#p checkFile("/Users/kjw_junichi/work/mruby/libtrusterd/hoge.txt")
