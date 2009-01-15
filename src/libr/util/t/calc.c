#include <r_util.h>

u64 num_callback(void *userptr, const char *str, int *ok)
{
	if (!strcmp(str, "foo")) {
		*ok=1;
		return 31337;
	}
	*ok = 0;
	return 0;
}

#define test_num(x,y) printf("test '%s' returns 0x%llx\n", y, r_num_math(x,y));
int main()
{
	struct r_num_t num;
	num.callback = &num_callback;
	num.userptr = NULL;

	test_num(&num, "33");
	test_num(&num, "0x84");
	test_num(&num, "44o");
	test_num(&num, "foo");

	return 0;
}
