#include <r_lib.h>

int cb_1(void *a, void *b)
{
	int *bi = (int *)b;
	int value = (int)*bi;
	int (*fun)() = a;
	fun();
	printf("Plugin value: 0x%x\n", b);
	return 0;
}

int ptr()
{
	printf("Data pointer passed properly\n");
	return 0;
}

int main(int argc, char **argv)
{
	struct r_lib_t *lib;
	int ret;

	lib = r_lib_new("radare_plugin");
	r_lib_add_handler(lib, 1, "hello world", &cb_1, &ptr);
	ret = r_lib_open(lib, "./plugin.so");
	if (ret == -1) printf("Cannot open plugin\n");
	else printf("Plugin opened correctly\n");

	return r_lib_free(lib);
}

