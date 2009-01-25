#include <r_search.h>

char *buffer = "helloworldlibisniceandcoolib2";

int hit(struct r_search_binparse_t *bp, int i, u64 addr)
{
	printf("HIT %d AT %lld (%s)\n", i, addr, buffer+addr);
	return 1;
}

int main(int argc, char **argv)
{
	struct r_search_t *rs;
	rs = r_search_new(R_SEARCH_KEYWORD);
	r_search_kw_add(rs, "lib", "");
	r_search_start(rs);
	rs->bp->callback = &hit;
	printf("Searching for '%s' in '%s'\n", "lib", buffer);
	r_search_update(rs, 0LL, buffer, strlen(buffer));
	r_search_free(rs);
	return 0;
}
