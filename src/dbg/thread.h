#ifndef THREAD_H
#define THREAD_H

#if __WINDOWS__
#include <windows.h>
#endif

#define TH_MAIN(tid) (ps.pid == tid)

enum {
	TH_OK = 0,
	TH_ERR 
};

typedef struct {
	int tid;
	int status;
	u64 addr;
#if __WINDOWS__
	HANDLE ht;	
#endif
	struct list_head list;
} TH_INFO;

#define TH_STATUS(th, val) (th->status = val)
#define TH_ADDR(th, val) (th->addr = val)

inline void add_th(TH_INFO *th);
inline void del_th(TH_INFO *th);
inline TH_INFO *init_th(pid_t tid, int status);
TH_INFO	*get_th(int tid);
int th_list();
void free_th();

#endif
