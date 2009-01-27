/* radare - LGPL - Copyright 2008-2009 pancake<nopcode.org> */

#include "r_search.h"

/* object life cycle */

int r_search_init(struct r_search_t *s, int mode)
{
	memset(s,'\0', sizeof(struct r_search_t));
	if (!r_search_set_mode(s, mode))
		return 0;
	s->mode = mode;
	s->user = NULL;
	s->callback = NULL;
	s->pattern_size = 0;
	s->string_max = 255;
	s->string_min = 3;
	INIT_LIST_HEAD(&(s->kws));
	INIT_LIST_HEAD(&(s->hits));
	INIT_LIST_HEAD(&(s->hits));
	return 1;
}

int r_search_set_string_limits(struct r_search_t *s, u32 min, u32 max)
{
	if (max < min)
		return 0;
	s->string_min = min;
	s->string_max = max;
	return 1;
}

int r_search_set_mode(struct r_search_t *s, int mode)
{
	int ret = 0;
	switch(mode) {
	case R_SEARCH_KEYWORD:
	case R_SEARCH_REGEXP:
	case R_SEARCH_PATTERN:
	case R_SEARCH_STRING:
	case R_SEARCH_AES:
		s->mode = mode;
		ret = 1;
	}
	return ret;
}

struct r_search_t *r_search_new(int mode)
{
	struct r_search_t *s = MALLOC_STRUCT(struct r_search_t);
	if (r_search_init(s, mode) == -1) {
		free(s);
		s = NULL;
	}
	return s;
}

struct r_search_t *r_search_free(struct r_search_t *s)
{
	free(s);
	return NULL;
}

/* control */

#if 0
int r_search_internal(struct r_search_t *s)
{
         // twice
         radare_cmd("f -hit0_*", 0);
         radare_cmd("f -hit0_*", 0);
         radare_cmd("fs search", 0);
// foreach ranges...
         nhit = 0;
         t = binparse_new(0);
         align = config_get_i("search.align");
         t->callback = &radare_tsearch_callback;
         len = strlen(range);
         // foreach token in range
         for(j=i=0;i<len;i++,j++) {
                 str[j] = range[i];
                 str[j+1] = '\0';
                 switch(range[i+1]) {
                 case '-':
                         num = atoi(str);
                         i++; j=-1;
                         f0=1;
                         break;
                 case '\0':
                 case ',':
                         if (str[0]=='\0') break;
                         num2 = atoi(str);
                         if (f0) {
                                 f0=0;
                                 if (num == -1) {
                                         printf("syntax error\n");
                                         break;
                                 }
                                 for(j = num;j<=num2;j++)
                                         binparse_add_search(t, j);
                         } else  binparse_add_search(t, num2);
                         j=-1;
                         str[0]='\0';
                         i++;
                         break;
                 }
         }
}
#endif

int r_search_initialize(struct r_search_t *s)
{
	struct list_head *pos;

	list_for_each_prev(pos, &s->kws) {
		struct r_search_kw_t *kw = list_entry(pos, struct r_search_kw_t, list);
		kw->count = 0;
		kw->idx = 0;
	}
	return 1;
}

int r_search_mybinparse_update(struct r_search_t *s, u64 from, const u8 *buf, int len)
{
	struct list_head *pos;
	int i, count = 0;

	for(i=0;i<len;i++) {
		list_for_each_prev(pos, &s->kws) {
			struct r_search_kw_t *kw = list_entry(pos, struct r_search_kw_t, list);
			u8 ch = kw->bin_keyword[kw->idx];
			u8 ch2 = buf[i];
			if (kw->binmask_length != 0 && kw->idx < kw->binmask_length) {
				ch &= kw->bin_binmask[kw->idx];
				ch2 &= kw->bin_binmask[kw->idx];
			}
			if (ch == ch2) {
				kw->idx++;
				if (kw->idx == kw->keyword_length) {
					if (s->callback)
						s->callback(kw, s->user, (u64)from+i-kw->keyword_length+1);
					else printf("hit%d_%d 0x%08llx ; %s\n",
						count, kw->count, (u64)from+i+1, buf+i-kw->keyword_length+1);
					kw->idx = 0;
					kw->count++;
				}
			} else kw->idx = 0;
			count++;
		}
		count = 0;
	}
	return 0;
}

int r_search_set_pattern_size(struct r_search_t *s, int size)
{
	s->pattern_size = size;
	return 0;
}

int r_search_set_callback(struct r_search_t *s, int (*callback)(struct r_search_kw_t *, void *, u64), void *user)
{
	s->callback = callback;
	s->user = user;
	return 0;
}

/* TODO: initialize update callback in _init */
int r_search_update(struct r_search_t *s, u64 *from, const u8 *buf, u32 len)
{
	int ret = 0;
	switch(s->mode) {
	case R_SEARCH_KEYWORD:
		r_search_mybinparse_update(s, *from, buf, len);
		break;
	case R_SEARCH_REGEXP:
		break;
	case R_SEARCH_AES:
		ret += r_search_aes_update(s, *from, buf, len);
		*from -= R_SEARCH_AES_BOX_SIZE;
		break;
	case R_SEARCH_STRING:
		ret += r_search_strings_update(buf, s->string_min, s->string_max, 0, *from /*enc*/, "");
		break;
	case R_SEARCH_PATTERN:
		//ret += r_search_pattern_update(buf, s->pattern_size
		break;
	}
	return ret;
}

int r_search_update_i(struct r_search_t *s, u64 from, const u8 *buf, u32 len)
{
	return r_search_update(s, &from, buf, len);
}

#if 0
int r_search_start(struct r_search_t *s)
{
	if (s->bp == NULL)
		return -1;
	if (s->bp->interrupted == 0) {
		/* start search */
	} else {
		/* continue search */
	}
}

int r_search_pause(struct r_search_t *s)
{
	if (s->bp)
		s->bp->interrupted = 1;
	return 0;
}

int r_search_stop(struct r_search_t *s)
{
	/* reset curseek */
	s->seek = 0;
}

int r_search_reset(struct r_search_t *s)
{
	INIT_LIST_HEAD(&s->kws);
	INIT_LIST_HEAD(&s->hits);
}
#endif

/* --- keywords --- */

/* string */
int r_search_kw_add(struct r_search_t *s, const char *kw, const char *bm)
{
	struct r_search_kw_t *k = MALLOC_STRUCT(struct r_search_kw_t);
	if (k == NULL)
		return -1;
	strncpy(k->keyword, kw, sizeof(k->keyword));
	strncpy(k->bin_keyword, kw, sizeof(k->keyword));
	k->keyword_length = strlen(kw);
	strncpy(k->binmask, bm, sizeof(k->binmask));
	k->binmask_length = r_hex_str2bin(bm, k->bin_binmask);
	list_add(&(k->list), &(s->kws));
	return 0;
}

/* hexpairstring */
int r_search_kw_add_hex(struct r_search_t *s, const char *kw, const char *bm)
{
	struct r_search_kw_t *k = MALLOC_STRUCT(struct r_search_kw_t);
	if (k == NULL)
		return -1;
	strncpy(k->keyword, kw, sizeof(k->keyword));
	k->keyword_length = r_hex_str2bin(kw, k->bin_keyword);
	strncpy(k->binmask, bm, sizeof(k->binmask));
	k->binmask_length = r_hex_str2bin(bm, k->bin_binmask);
	list_add(&(k->list), &(s->kws));
	return 0;
}

/* raw bin */
int r_search_kw_add_bin(struct r_search_t *s, const u8 *kw, int kw_len, const u8 *bm, int bm_len)
{
	struct r_search_kw_t *k = MALLOC_STRUCT(struct r_search_kw_t);
	int i;
	if (kw == NULL)
		return -1;
	memcpy(k->bin_keyword, kw, kw_len);
	k->keyword_length = kw_len;
	memcpy(k->bin_binmask, bm, bm_len);
	k->keyword_length = kw_len;
	/* TODO: externalize into hex_bin_to_str() */
	k->keyword[0]='\0';
	for(i=0;i<kw_len;i++) {
		char ch[16];
		sprintf(ch, "%02x", kw[i]);
		strcat(k->keyword, ch);
	}
	k->binmask[0]='\0';
	for(i=0;i<kw_len;i++) {
		char ch[16];
		sprintf(ch, "%02x", bm[i]);
		strcat(k->binmask, ch);
	}
	list_add(&(k->list), &(s->kws));
	return 0;
}

/* show keywords */
struct r_search_kw_t *r_search_kw_list(struct r_search_t *s)
{
	struct list_head *pos;

	list_for_each_prev(pos, &s->kws) {
		struct r_search_kw_t *kw = list_entry(pos, struct r_search_kw_t, list);
		printf("%s %s\n", kw->keyword, kw->binmask);
	}
	return NULL;
}
