#ifndef _INCLUDE_DATA_H_
#define _INCLUDE_DATA_H_

extern struct reflines_t *reflines;
//extern struct list_head data;
//extern struct list_head comments;
extern struct list_head traces;

struct data_t {
	ut64 from;
	ut64 to;
	int type;
	int times;
	ut64 size;
	char arg[128];
	struct list_head list;
};

struct var_type_t {
	char name[128];
	char fmt[128];
	unsigned int size;
	struct list_head list;
};

struct comment_t {
	ut64 offset;
	const char *comment;
	struct list_head list;
};

struct xrefs_t {
	ut64 addr;  /* offset of the cross reference */
	ut64 from;  /* where the code/data is referenced */
	int type;  /* 0 = code, 1 = data, -1 = unknown */
	struct list_head list;
};

struct reflines_t {
	ut64 from;
	ut64 to;
	int index;
	struct list_head list;
};

int data_set_len(ut64 off, ut64 len);
void data_info();
int data_set(ut64 off, int type);
struct data_t *data_add_arg(ut64 off, int type, const char *arg);
struct data_t *data_add(ut64 off, int type);
ut64 data_seek_to(ut64 offset, int type, int idx);
struct data_t *data_get(ut64 offset);
struct data_t *data_get_range(ut64 offset);
struct data_t *data_get_between(ut64 from, ut64 to);
int data_type_range(ut64 offset);
int data_type(ut64 offset);
int data_end(ut64 offset);
int data_size(ut64 offset);
ut64 data_prev(ut64 off, int type);
ut64 data_prev_size(ut64 off, int type);
int data_list(const char *mask);
int data_xrefs_print(ut64 addr, int type);
int data_xrefs_add(ut64 addr, ut64 from, int type);
int data_xrefs_at(ut64 addr);
void data_xrefs_del(ut64 addr, ut64 from, int data /* data or code */);
void data_comment_del(ut64 offset, const char *str);
void data_comment_add(ut64 offset, const char *str);
void data_comment_list(const char *mask);
void data_xrefs_here(ut64 addr);
void data_xrefs_list(const char *mask);
char *data_comment_get(ut64 offset, int lines);
void data_comment_init(int);
void data_reflines_init(int lines, int linescall);
int data_printd(int delta);
int data_var_type_list(const char *mask);
struct var_type_t *data_var_type_get(const char *datatype);
const char *data_var_type_format(const char *datatype);
void data_del(ut64 addr, int type,int len/* data or code */);
void data_del_range(ut64 from, ut64 to);
ut64 var_functions_show(int idx);
int data_get_fun_for(ut64 addr, ut64 *from, ut64 *to);
int data_var_type_add(const char *typename, int size, const char *fmt);

#endif
