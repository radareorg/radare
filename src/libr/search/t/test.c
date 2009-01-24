#include <r_search.h>

char *buffer = "helloworldlibisniceandcoolib2";

int main(int argc, char **argv)
{
	struct r_search_t *rs;
	rs = r_search_new();
	r_search_kw_add(rs, "lib", "");
	r_search_start(rs);
	r_search_update(rs, 0LL, buffer, strlen(buffer));
	r_search_free(rs);
	return 0;
}
