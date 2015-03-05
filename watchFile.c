#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include <fcntl.h>

#include <stdio.h>
#include <stdlib.h>

#include "mruby.h"
#include "mruby/compile.h"
#include "mruby/string.h"

// gcc `mruby/bin/mruby-config --cflags --ldflags --libs --ldflags-before-libs` watchFile.c
//
// File.dirname(File.realpath("./myfile.rb"))
// mrb_load_string_cxt
// Dir.open(File.dirname(File.realpath("./myfile.rb"))).each{|f| p f}

int checkFile(mrb_state *mrb, mrbc_context *cxt, char *filepath)
{
  mrb_value val;

  val = mrb_funcall(mrb, mrb_top_self(mrb), "checkFile", 1,
  mrb_str_new_cstr(mrb, filepath));
 
  return mrb_bool(val);
}

void watchFile(mrb_state *mrb, mrbc_context *cxt, char *filepath)
{
	int fd, dfd, kq;
	struct kevent kev[2], kev_r;
	int isFileWatching;
  char *dirpath;
  mrb_value val;

  fd = open(filepath,O_RDONLY);

  // dirpath = File.dirname(File.realpath("./myfile.rb"))
  val = mrb_funcall(mrb, mrb_obj_value(mrb_class_get(mrb, "File")), "realpath", 1,
  mrb_str_new_cstr(mrb, filepath));
  val = mrb_funcall(mrb, mrb_obj_value(mrb_class_get(mrb, "File")), "dirname", 1,
    val);
  dirpath = mrb_string_value_ptr(mrb, val);

  dfd = open(dirpath,O_RDONLY);

	kq = kqueue();
	EV_SET(&kev[0], fd, EVFILT_VNODE, EV_ADD | EV_ENABLE | EV_CLEAR, NOTE_DELETE | NOTE_WRITE, 0, NULL);
	EV_SET(&kev[1], dfd, EVFILT_VNODE, EV_ADD | EV_ENABLE | EV_CLEAR, NOTE_WRITE, 0, NULL);

	if (kevent(kq, kev, 2, NULL, 0, NULL) == -1) {
    perror("oops");
		printf("kevent error\n");
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
				} else if (kev_r.fflags & NOTE_DELETE) {
					printf("file was deleted!\n");

					isFileWatching = 0;
				}
			}
		} else {
			if (kev_r.fflags & NOTE_WRITE) {
                            printf("%s\n","update dir");
				if (checkFile(mrb, cxt, filepath) == 1) {
                                        printf("%s\n","add file!");
					// チェック対象だったら、fdを再取得して、これを監視対象にする。
					fd = open(filepath, O_RDONLY);
					EV_SET(&kev[0], fd, EVFILT_VNODE, EV_ADD | EV_ENABLE | EV_CLEAR, NOTE_DELETE | NOTE_WRITE, 0, NULL);
					isFileWatching = 1;
				}
			}
		}
	}
}

int main(int argc, char **argv)
{
  FILE* f;
  mrb_state *mrb;
  mrbc_context *cxt;
  mrb_value val;

	char *filepath = "./foo.txt";

	mrb = mrb_open();
	cxt = mrbc_context_new(mrb);
  f = fopen("checkFile.rb", "r");
  mrb_load_file(mrb, f);
  fclose(f);
  
  // File.realpath("./myfile.rb"))
  val = mrb_funcall(mrb, mrb_obj_value(mrb_class_get(mrb, "File")), "realpath", 1,
  mrb_str_new_cstr(mrb, filepath));

  char *fullpath = mrb_string_value_ptr(mrb, val);
  printf("%s\n", fullpath);

	watchFile(mrb, cxt, fullpath);

	mrb_close(mrb);
	exit(0);
}
