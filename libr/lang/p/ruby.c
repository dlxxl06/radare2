/* radare - LGPL - Copyright 2009 pancake<nopcode.org> */
/* ruby extension for libr (radare2) */

#include "r_lib.h"
#include "r_lang.h"
#include "ruby.h"

#define LIBDIR "/usr/lib"
#define RUBYAPI  LIBDIR"/ruby1.8/radare.rb"

/* for the basic r_core_cmd calls */
#include "r_core.h"
static struct r_core_t *core = NULL;

static VALUE radare_ruby_cmd(VALUE self, VALUE string)
{
	const char *retstr;
	Check_Type(string, T_STRING);
	retstr = r_core_cmd_str(core, RSTRING(string)->ptr);
	if (retstr == NULL || retstr[0]=='\0')
		return rb_str_new2("");
	return rb_str_new2(retstr);
}

static int run(void *user, const char *code, int len)
{
	int err;
	//"require 'irb'; $r = Radare.new(); IRB.start();", &err);
	rb_eval_string_protect(code, &err);
	if (err != 0) {
		printf("error %d handled\n", err);
	}
	return (err==0)?R_TRUE:R_FALSE;
}

static int slurp_ruby(const char *file)
{
	rb_load_file(file);
	ruby_exec();
	return R_TRUE;
}

static int run_file(void *user, const char *file)
{
	return slurp_ruby(file);
}

static int init(void *user)
{
	VALUE rb_RadareCmd;
	core = user;

	ruby_init();
	ruby_init_loadpath();

	rb_RadareCmd = rb_define_class("Radare", rb_cObject);
	rb_define_method(rb_RadareCmd, "cmd", radare_ruby_cmd, 1);
	rb_eval_string_protect("$r = Radare.new()", NULL);

	if (!slurp_ruby(RUBYAPI)) {
		printf("[ruby] error loading ruby api\n");
		//return R_FALSE;
	}
	return R_TRUE;
}

static int rb_fini()
{
	ruby_finalize();
	return R_TRUE;
}

const char *help =
	"Ruby plugin usage:\n"
	" bytes = $r.cmd(\"p8 10\");\n";

static struct r_lang_handle_t r_lang_plugin_ruby = {
	.name = "ruby",
	.desc = "RUBY language extension",
	.init = &init,
	.help = &help,
	.run = &run,
	.run_file = &run_file,
	.set_argv = NULL,
};

struct r_lib_struct_t radare_plugin = {
	.type = R_LIB_TYPE_LANG,
	.data = &r_lang_plugin_ruby,
};
