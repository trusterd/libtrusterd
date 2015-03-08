#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <signal.h>

#include <sys/types.h>

#ifdef __APPLE__
#include <unistd.h>
#include <libproc.h>
#include <sys/event.h>
#include <sys/time.h>
#include <fcntl.h>
#endif

#ifdef __linux
#include <sys/wait.h>
#endif

#include "mruby.h"
#include "mruby/proc.h"
#include "mruby/compile.h"
#include "mruby/string.h"

typedef int (*FUNCPTR)(char *script);
FUNCPTR gcb;
char trusterd_conf_path[1024];
FILE *confFile = NULL;

#ifdef __linux
extern const uint8_t watchFileLinux[];
#endif

#ifdef __APPLE__
extern const uint8_t checkFile[];
#endif


void setTrusterdConfPath(const char *filepath)
{
  strncpy(trusterd_conf_path, filepath, 1023);
  trusterd_conf_path[1023] = '\0';
}

char *getTrusterdConfPath()
{
  return trusterd_conf_path;
}

void setCallback(FUNCPTR cb)
{
  gcb = cb;
}

FUNCPTR getCallback()
{
  return gcb;
}
/*
static mrb_value runTrusterd(mrb_state *mrb, mrb_value obj)
{
  FILE *f;
  char *filepath;

  printf("start runTrusterd\n");
  fflush(stdout);
  filepath = getTrusterdConfPath();
  f = fopen(filepath, "r");
  mrb_load_file(mrb, f);
  fclose(f);
  return mrb_nil_value();
}
*/

static mrb_value dofork(mrb_state *mrb, const char *filepath)
{
  mrb_value val;

  /*
  struct RProc *blk;
  mrb_value val, proc;

  printf("start dofork\n");
  setTrusterdConfPath(filepath);
  printf("trusterdconfpath = %s\n",getTrusterdConfPath());
  blk = mrb_proc_new_cfunc(mrb, runTrusterd);
  proc = mrb_obj_value(blk);

  val = mrb_funcall_with_block(mrb, mrb_obj_value(mrb_module_get(mrb, "Process")), mrb_intern(mrb, "fork"), 0, NULL, proc);
  */
  printf("start dofork\n");
  int pid;
  switch (pid = fork()) {
  case 0:
    // start trusterd
    printf("trusterd[%s] is starting...\n",filepath);
    if(confFile != NULL) {
      fclose(confFile);
    }
    confFile = fopen(filepath, "r");
    if(confFile == NULL) {
      printf("oops fopen[%s]!\n",filepath);
    }
    perror("fopen");
    mrb_load_file(mrb, confFile);
    fclose(confFile);
    confFile = NULL;
    _exit(0);
    return  mrb_fixnum_value(0);
  case -1:
    perror("fork fail");
    return  mrb_fixnum_value(-1);
  default:
    printf("trusterd has started.");
    return mrb_fixnum_value(pid);
  }
  printf("end dofork\n");
  return val;
}

mrb_value reload(mrb_state *mrb, mrb_value pid, const char *filepath)
{
  //mrb_value val;
int status;

  // kill pid
  printf("kill pid = %d\n", mrb_fixnum(pid));
  //val = mrb_funcall(mrb, mrb_obj_value(mrb_module_get(mrb, "Process")), "kill", 2, mrb_fixnum_value(9), pid);

  //mrb_funcall(mrb,mrb_obj_value(mrb_module_get(mrb, "Process")),"waitpid",1,val);
  kill(mrb_fixnum(pid),SIGTERM);
  waitpid(mrb_fixnum(pid), &status, 0);
  return dofork(mrb, filepath);
}

#ifdef __linux
int watchTrusterdConfFileInotify(mrb_state *mrb, char *filepath)
{
  mrb_value val;

  mrb_load_irep(mrb, watchFileLinux);

  val = mrb_funcall(mrb, mrb_top_self(mrb), "watchFileLinux", 1,
                    mrb_str_new_cstr(mrb, filepath));
  return 0;
}

#endif

#ifdef __APPLE__
mrb_value get_procpathname(mrb_state* mrb, mrb_value self)
{
  int ret;
  pid_t pid;
  char pathbuf[PROC_PIDPATHINFO_MAXSIZE];

  pid = getpid();
  ret = proc_pidpath(pid, pathbuf, sizeof(pathbuf));

  return mrb_str_new(mrb, pathbuf, strlen(pathbuf));
}

int checkFileCall(mrb_state *mrb, const char *filepath)
{
  mrb_value val;

  val = mrb_funcall(mrb, mrb_top_self(mrb), "checkFile", 1,
                    mrb_str_new_cstr(mrb, filepath));

  return mrb_bool(val);
}

const char *getFullpath(mrb_state *mrb, char *filepath)
{
  mrb_value val;

  // TODO we need handle file error when we not found the file.
  // we need filepath to absolute path.
  val = mrb_funcall(mrb, mrb_obj_value(mrb_class_get(mrb, "File")), "realpath", 1,
                    mrb_str_new_cstr(mrb, filepath));
  return mrb_string_value_ptr(mrb, val);
}

const char *getDirname(mrb_state *mrb, const char *fullpath)
{
  mrb_value val;

  val = mrb_funcall(mrb, mrb_obj_value(mrb_class_get(mrb, "File")), "dirname", 1,
                    mrb_str_new_cstr(mrb, fullpath));
  return mrb_string_value_ptr(mrb, val);
}

int watchTrusterdConfFileKqueue(mrb_state *mrb, char *filepath)
{
  int fd, dfd, kq;
  struct kevent kev[2], kev_r;
  int isFileWatching;
  const char *dirpath;
  const char *fullpath;
  mrb_value pid;

  mrb_load_irep(mrb, checkFile);

  fullpath = getFullpath(mrb, filepath);
  printf("%s\n", fullpath);

  fd = open(fullpath, O_RDONLY);

  dirpath = getDirname(mrb, fullpath);
  dfd = open(dirpath, O_RDONLY);

  // まず起動する
  pid = dofork(mrb, fullpath);


  kq = kqueue();
  EV_SET(&kev[0], fd, EVFILT_VNODE, EV_ADD | EV_ENABLE | EV_CLEAR, NOTE_DELETE | NOTE_WRITE, 0, NULL);
  EV_SET(&kev[1], dfd, EVFILT_VNODE, EV_ADD | EV_ENABLE | EV_CLEAR, NOTE_WRITE, 0, NULL);

  if (kevent(kq, kev, 2, NULL, 0, NULL) == -1) {
    perror("kevent error");
    exit(0);
  }

  isFileWatching = 1;

  while (1) {
    if (isFileWatching) {
      // ディレクトリとファイルを監視
      if (kevent(kq, kev, 2, &kev_r, 1, NULL) == -1) {
        printf("kevent_r error\n");
        exit(0);
      }
    } else {
      // ディレクトリのみ監視
      if (kevent(kq, &kev[1], 1, &kev_r, 1, NULL) == -1) {
        printf("kevent_r error\n");
        exit(0);
      }
    }
    if (isFileWatching == 1) {
      if (kev_r.ident == fd) {
        if (kev_r.fflags & NOTE_WRITE) {
          printf("file was updated!\n");
          close(fd);
fd = open(fullpath, O_RDONLY);
EV_SET(&kev[0], fd, EVFILT_VNODE, EV_ADD | EV_ENABLE | EV_CLEAR, NOTE_DELETE | NOTE_WRITE, 0, NULL);
          pid = reload(mrb, pid, fullpath);
        } else if (kev_r.fflags & NOTE_DELETE) {
          printf("file was deleted!\n");
          close(fd);
          isFileWatching = 0;
        }
      }
    } else {
      if (kev_r.fflags & NOTE_WRITE) {
        printf("%s\n", "update dir");
        if (checkFileCall(mrb, fullpath) == 1) {
          printf("%s\n", "add file!");
          // チェック対象だったら、fdを再取得して、これを監視対象にする。
          fd = open(fullpath, O_RDONLY);
          EV_SET(&kev[0], fd, EVFILT_VNODE, EV_ADD | EV_ENABLE | EV_CLEAR, NOTE_DELETE | NOTE_WRITE, 0, NULL);
          pid = reload(mrb, pid, fullpath);
          isFileWatching = 1;
        }
      }
    }
  }

  return 0;
}
#endif // end __APPLE__


mrb_value exec(mrb_state* mrb, mrb_value self)
{
  char *script;

  printf("class:Call, method:py_exec\n");
  // 第一引数を引数にコールバック関数を実行する。
  mrb_get_args(mrb, "z", &script);

  (*getCallback())(script);
  return self;
}

void mrbAddMyCallBack(mrb_state* mrb, FUNCPTR cb)
{
  setCallback(cb);

  struct RClass *hoge_module;
  hoge_module = mrb_define_module(mrb, "MyCall");
  // クラスを定義する

  // クラスメソッドを定義する
  mrb_define_class_method(mrb, hoge_module, "my_exec", exec, ARGS_REQ(1));
        #ifdef __APPLE__
  mrb_define_class_method(mrb, hoge_module, "procpathname", get_procpathname, ARGS_NONE());
        #endif
}

int watchTrusterdConfFile(mrb_state *mrb, char *filepath)
{
        #ifdef __APPLE__
  return watchTrusterdConfFileKqueue(mrb, filepath);
        #endif
#ifdef __linux
  return watchTrusterdConfFileInotify(mrb, filepath);

#endif
  return -1;
}

int boot_from_file_path(char *filepath, FUNCPTR cb)
{
  int rtn;

  assert(filepath != NULL);

  mrb_state* mrb = mrb_open();

  mrbAddMyCallBack(mrb, cb);

  rtn = watchTrusterdConfFile(mrb, filepath);
  //mrb_load_string(mrb, name);

  mrb_close(mrb);

  return rtn;
}

int boot(char *name, FUNCPTR cb)
{
  assert(name != NULL);
  mrb_state* mrb = mrb_open();

  // のようにmrubyから呼び出し元の言語でコードを渡すことで、
  // 呼び出し元でコードを評価してもらう。
  mrbAddMyCallBack(mrb, cb);

  mrb_load_string(mrb, name);
  mrb_close(mrb);
  return printf("%s", name);
}
