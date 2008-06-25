#ifndef BINPARSE_H
#define BINPARSE_H

#include "main.h"

typedef struct _token {
	unsigned char mintok;	//Token, or base
	unsigned char range; 	//0 nomes el mintok, ( maxtoken - mintoken )
	unsigned char mask;
} token;

typedef struct _tokenlist {
	token* tl;
	int numtok;
	char name [300];
	char actp[300]; //aux pel parseig actual
	int estat;
	/* int lastpos; XXX unused */
} tokenlist;

typedef struct _tokenizer {
	tokenlist** tls;
	int nlists;
	int (*callback)(struct _tokenizer *t, int i, u64 where);
} tokenizer;

tokenizer* binparse_new();
tokenizer* binparse_new_from_file(char *file);
int binparse_get_mask_list ( char* mask , char* maskout );
int binparse_add_search(tokenizer *t, int id);
int binparser_free(tokenizer* ptokenizer);
int update_tlist( tokenizer*  ptok, unsigned char inchar, u64 where);
tokenlist* get_tok_list(  char* line, int maxlen ) ;
tokenizer* build_tokenizer( char * file ) ;
void tokenize(int fd, tokenizer* ptok);
int binparse_add(tokenizer *t, char *string, char *mask);
void binparse_apply_mask (char * maskout, int masklen , token* tlist , int ntok) ;

#endif
