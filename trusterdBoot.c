#include <stdio.h>
#include <assert.h>

#ifdef __APPLE__
#include <unistd.h>
#include <libproc.h>
#endif

#include "mruby.h"
#include "mruby/proc.h"
#include "mruby/compile.h"

typedef int (*FUNCPTR)(char *script);
FUNCPTR gcb;

void setCallback(FUNCPTR cb)
{
	gcb = cb;
}

FUNCPTR getCallback()
{
	return gcb;
}

#ifdef __APPLE__
mrb_value get_procpathname(mrb_state* mrb, mrb_value self)
{
	int ret;
	pid_t pid;
	char pathbuf[PROC_PIDPATHINFO_MAXSIZE];

	pid = getpid();
	ret = proc_pidpath(pid, pathbuf, sizeof(pathbuf));

	return mrb_str_new(mrb,pathbuf,strlen(pathbuf));
}
#endif

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

int boot(char *name, FUNCPTR cb)
{
	assert(name != NULL);
	mrb_state* mrb = mrb_open();

	// のようにmrubyから呼び出し元の言語でコードを渡すことで、
	// 呼び出し元でコードを評価してもらう。
	mrbAddMyCallBack(mrb, cb);

	mrb_load_string(mrb, name);
	mrb_close(mrb);
	return printf("hello %s\n", name);
}
