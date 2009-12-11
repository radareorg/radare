/*
 * Copyright (C) 2007, 2008, 2009
 *       esteve <@eslack.org>
 *
 * Contributions:
 *       pancake <@youterm.com>
 *
 * radare is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * radare is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with radare; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "main.h"
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "binparse.h"
#include "utils.h"

#if 0

token file example:
------------------------------
token:  Library token
        string: lib
        mask:   ff 00 ff
token:  Fruit for the loom
        string: rt
        mask:   ff ff
------------------------------
#endif

#if 0
/* not necessary */
static void print_tok_list(tokenlist* toklist) 
{
	int i;

	printf ("TOKLIST %s:\n",toklist->name);
	for (i=0; i<toklist->numtok; i++)
		printf ("TOK : %c , range : %d mask : %x\n",
			toklist->tl[i].mintok,
			toklist->tl[i].range,
			toklist->tl[i].mask);
	NEWLINE;
	printf ("\n");
}

static void print_tokenizer ( tokenizer* ptokenizer )
{
	int i;
	for (i=0 ; i < ptokenizer->nlists; i++ )
		print_tok_list(ptokenizer->tls[i]);
}

static char* fd_readline ( int fd, char* line, int maxsize )
{
	int i,ret ;
	memset(line, 0x00, maxsize); 
	for (i=0; i<maxsize; i++) {
		ret = read (fd, line + i, 1);
		if (ret <1) return NULL;
		if (line[i] =='\n') break;
	}
	line[i+1]=0;
	return line;
}

static int indent_count( int fd )
{
	int ret=0;
	char t;
	read ( fd, &t, 1 );
	while ( t=='\t') {
		read ( fd, &t, 1 );
		ret++;
	}

	// Posiciono a la primera diferent.
	lseek ( fd, (off_t)-1, SEEK_CUR );

	return ret;
}
#endif

int search_nocase = 0;

static unsigned char get_num(const char * str, int len)
{
	u8 * strp;
	int value;

	if (len <1 || str == NULL)
		return 0;
	strp = malloc(len+1);
	if (strp == NULL)
		return 0;
	memset(strp, 0, len);
	memcpy(strp, str, len );
	
	if (strp[0] == '\\') {
		strp[0] = '0';
		sscanf ((char*)strp, "%x", &value);
	} else	value = strp[0]; //sscanf (strp,"%c",(char *)&value );
	value = value & 0xFF ;

	free(strp);
	return (unsigned char)value ;
}

static int get_range(char *str, int len, unsigned char *cbase)
{
	int g;
	unsigned char min;
	unsigned char max;
	
	// busca guio
	for ( g= 0; (g < len ) && ( str[g] != '-' ) ; g++ );
	min = get_num ( str, g );
	max = get_num ( str+g+1, len-g-1 );

	*cbase = min;

	return (max-min);
}

static int tok_parse (char* str, int len, token * tlist )
{
	int i;
	int estat;
	int tokaux;
	int tokact=0;
	char straux[5];
	int rangemin = 0; // XXX BUGGY ???
	int rangemax;
	unsigned char cmin;

	estat = 0;	
	for (i=0 ; i < len ; i ++ ) {
		switch (str[i]) {
		case '\\':
			if (estat!=1&&( (estat < 10) || ( estat > 22 ) )) estat = 1;
			else estat++;
			break;
		case '[':
			if ( (estat!=1) && (estat!=10)) {
				estat = 10;
				rangemin = i+1 ;
			}
			else estat++;
			break;
		default:
			if (estat != 0 ) estat ++;
			break;
		}

		if ( estat == 0 ) {
			if (search_nocase)
				tlist[tokact].mintok = tolower(str[i]);
			else tlist[tokact].mintok = str[i];
                        tlist[tokact].range = 0;
			tokact++;
		} else
		if (estat == 2 ) { // parse \xAA
			if (str[i]=='x' ) {
				//estat = 3;
			} else {
				//Caracter escapat
				if (search_nocase)
					tlist[tokact].mintok = tolower(str[i]);
				else tlist[tokact].mintok = str[i];
                	        tlist[tokact].range = 0;
				tokact++;
				estat = 0;
			}
		} else
		if (estat == 3 ) {
			//primer despres de \x <--
			straux[0]='0';
			straux[1]='x';
			straux[2]=str[i];		
		} else
		if (estat == 4 ) {
			//Ja tinc tot el byte
			straux[3]=str[i];		
			straux[4]=0;		
			
			sscanf (straux ,"%x", &tokaux);
			//tokaux = 0xFF&((straux[0]-'0')*16 + (straux[1]-'0'));
			tlist[tokact].mintok =  tokaux;
			tlist[tokact].range = 0;
			tokact++;
			estat = 0;
		} else
		if ( (estat >= 11) && (estat <= 22 ) ) {
			if ( str[i] == ']' ) {
				int irange;
				rangemax = i-1;

				irange = get_range( str+rangemin, rangemax-rangemin+1 , &cmin);
	
				tlist[tokact].mintok =  cmin;
				tlist[tokact].range = irange;
				tokact++;
				
				estat = 0;
			}
		}
	}

	return tokact;
}

// line , IN rep linia tokens, surt llista token
tokenlist* get_tok_list(char* line, int maxlen) 
{
	int i ,p;
	token * tlist;
	tokenlist *tls;
	int ntok;

	tls = malloc ( sizeof ( tokenlist ) ) ;

	for ( i = 0 ; i < maxlen ; i ++ ) if ( line[i] == '$' ) break;
	for ( p = i+1 ; p < maxlen ; p ++ )
		if ( line[p] == '$' && line[p-1] != '\\' ) break;
	
	//Prova, cada caracter un token
	if ( i == (p-1) ) {
		tlist = malloc ( sizeof (token) ) ;
		tlist[0].mintok = 0;
		tlist[0].range = 0xFF;
		ntok = 1;
	} else {
		ntok  = p - i;	
		tlist = malloc( sizeof (token) * ( ntok ) );
		ntok  = tok_parse( line+1, ntok-1, tlist );
	}

	tls->tl = tlist;
	tls->numtok = ntok;
	/* tls->lastpos = 0; */
	tls->estat = 0;

	strncpy(tls->name, line+p+1, 256);
	tls->name[strlen(tls->name)-1] = '\0'; 

	return tls;
}

tokenizer* binparse_new(int size)
{
	tokenizer* tll;
	tll = (tokenizer*)malloc(sizeof( tokenizer ));
	tll->tls = (tokenlist**)malloc(sizeof (tokenlist*) * size);
	tll->nlists = 0;
	return tll;
}

int binparse_add_search(tokenizer *t, int id)
{
	char *token;
	char *mask;
	char tmp[128];
	
	snprintf(tmp, 127, "SEARCH[%d]", id);
	token = getenv(tmp);
	snprintf(tmp, 127, "MASK[%d]", id);
	mask  = getenv(tmp);

	return binparse_add(t, token, mask);
}

static tokenlist *binparse_token_mask(char *name, char *token, char *mask)
{
	tokenlist *tls;
	void *tlist = 0;
	int ntok = 0;
	int len;
	int masklen;
	char maskout[300];

	tls = malloc( sizeof(tokenlist) );
	// TODO mask not yet done
	len = strlen(token);
	tlist = malloc( (sizeof (token) * len) + 1 );
	ntok = tok_parse(token, len, tlist);

	tls->tl = tlist;
	tls->numtok = ntok;
	/* tls->lastpos = 0; */
	tls->estat = 0;
	strcpy ( tls->name , name ); // XXX bof here!
	
	if ( mask == NULL )
		mask = "ff";

	masklen = binparse_get_mask_list ( mask , maskout );
	binparse_apply_mask ( maskout, masklen , tlist , ntok ) ;

	//print_tok_list ( tls ) ;
	return tls;
}


int binparse_add(tokenizer *t, char *string, char *mask)
{
	int n = t->nlists;
	char name[32];

	if (string == NULL)
		return n;
//eprintf("KEY (%s)\n", string);
	t->nlists++;
	snprintf(name, 31, "SEARCH[%d]", n);
	t->tls    = (tokenlist **) realloc(t->tls, t->nlists*sizeof(tokenlist*));
	t->tls[n] = binparse_token_mask(name, string, mask);

	return n;
}

int binparse_add_name(tokenizer *t, char *name, char *string, char *mask)
{
	int ret = binparse_add(t, string, mask);
	strncpy(t->tls[ret]->name, name, 200);
	return ret;
}

char *str_get_arg(const char *buf)
{
	const char *str = strchr(buf, ':');
	if (str != NULL)
		str = strchr(str+1, '\t');
	if (str == NULL)
		return NULL;
	return strdup(str+1);
}

tokenizer* binparse_new_from_file(char *file)
{
	char buf[2049];
	FILE *fd;
	tokenizer *tok;
	char *str  = NULL;
	char *mask = NULL;
	char *name = NULL;

	tok = binparse_new(0);
	fd = fopen(file, "r");
	if (fd == NULL) {
		eprintf("Cannot open file '%s'\n", file);
		return NULL;
	}
	while(!feof(fd)) {
		/* read line */
		buf[0]='\0';
		fgets(buf, 2048, fd);
		if (buf[0]=='\0') continue;
		buf[strlen(buf)-1]='\0';

		/* find token: */
		if (!memcmp(buf, "token:",6)) {
			if (str != NULL) {
				eprintf("new keyword(%s,%s,%s)\n", name, str, mask);
				binparse_add_name(tok, name, str, mask);
				free(name); name = NULL;
				free(str);  str  = NULL;
				free(mask); mask = NULL;
			}
			free(name);
			name = str_get_arg(buf);
		} else
		if (!memcmp(buf, "\tstring:", 8)) {
			str = str_get_arg(buf);
		} else
		if (!memcmp(buf, "\tmask:", 6)) {
			mask = str_get_arg(buf);
		}
	}

	if (str != NULL) {
		eprintf("new keyword(%s,%s,%s)\n", name, str, mask);
		binparse_add_name(tok, name, str, mask);
	}

	free(name);
	free(str);
	free(mask);
	printf("TOKEN ELEMENTS: %d\n", tok->nlists);

	return tok;
}

/* XXX THIS IS WRONG \\x.. is stupid */
tokenizer* binparse_new_from_simple_file(char *file)
{
	int i, j, len;
	char *ptr, buf[4096];
	FILE *fd;
	tokenizer *tok;

	tok = binparse_new(0);
	fd = fopen(file, "r");
	if (fd == NULL) {
		eprintf("Cannot open file '%s'\n", file);
		return NULL;
	}
	while(!feof(fd)) {
		char *foo;
		/* read line */
		buf[0]='\0';
		fgets(buf, 4095, fd);
		if (buf[0]=='\0') continue;
		buf[strlen(buf)-1]='\0';

		ptr = strchr(buf, ' ');
		if (ptr) {
			ptr[0]='\0';
			ptr=ptr+1;
			len = strlen(ptr);
			foo = malloc((len*2)+1);
			foo[0]='\0';
			for(i=j=0;i<len;i+=2,j+=4) {
				strcpy(foo+j, "\\x");
				foo[j+2] = ptr[i];
				foo[j+3] = ptr[i+1];
			}
			foo[j]='\0';
			binparse_add_name(tok, buf, foo, NULL);
			free(foo);
		}
	}

	printf("TOKEN ELEMENTS: %d\n", tok->nlists);

	return tok;
}

int binparse_get_mask_list ( char* mask , char* maskout )
{
	int i,j,k ;
	char num [3];

	num[2] = 0;

	i = 0; 	j = 0; 	k = 0;
	while ( mask[i] != 0 )
	{
		if ( mask[i] != ' ' ) {
			num[j] = mask[i];
			j++;
		}
		i++;
		if ( j == 2 ) {
			sscanf ( num, "%hhx", (unsigned char*)&maskout[k] );
			k++;
			j = 0;
		}
	}

	return k;
}

void binparse_apply_mask (char * maskout, int masklen , token* tlist , int ntok)
{	
	int i;
	if (masklen>0)
	for ( i = 0; i < ntok ; i ++ )
		tlist[i].mask = maskout[i%masklen];
}

/* -1 = error, 0 = skip, 1 = hit found */
int update_tlist(tokenizer* t, u8 inchar, ut64 where )
{
	unsigned char cmin;
	unsigned char cmax;
	unsigned char cmask;
	int i, j;

	if (t->nlists == 0) {
		eprintf("No tokens defined\n");
		config.interrupted = 1;
		return -1;
	}
	if (search_nocase)
		inchar = tolower(inchar);

	for (i=0; i<t->nlists; i++ ) {
		cmin = (t->tls[i]->tl[t->tls[i]->estat]).mintok;
		j = t->tls[i]->estat;
		if (j>t->tls[i]->numtok)
			t->tls[i]->estat = 0;
//printf("%d\n", j);
		if ( (t->tls[i]->tl[j]).range > 0 ) {
			// RANGE
			cmax = cmin + (t->tls[i]->tl[t->tls[i]->estat]).range;
		
			if ( (inchar >= cmin) && ( inchar <= cmax ) )
				t->tls[i]->actp[t->tls[i]->estat++] = inchar;
			else	t->tls[i]->estat = 0;
		} else {
			// 1 char
			cmask = (t->tls[i]->tl[t->tls[i]->estat]).mask;
			if (  (inchar&cmask) == (cmin&cmask)  )
				t->tls[i]->actp[t->tls[i]->estat++] = inchar;
			else	t->tls[i]->estat = 0;
		}

		if ( t->tls[i]->estat == (t->tls[i]->numtok) ) {
			t->tls[i]->actp[t->tls[i]->estat+1] = 0 ;
			t->tls[i]->actp[0] = 0 ; //rststr
			if (t->callback != NULL)
				if (!t->callback(t, i, (ut64)(where-(t->tls[i]->numtok-1)))) // t->tls[i] is the hit
					return 1;
			t->tls[i]->estat = 0 ;
		}
	}

	return 0;
}

void tokenize(int fd, tokenizer* t)
{
	char ch;
	int ret;
	int where = lseek(fd, (off_t)0, SEEK_CUR);

	while(1) {
		if ( read(fd, &ch, 1) <= 0 ) break;
		ret = update_tlist(t, ch, where); 
		if (ret == -1) break;
		where++;
	}
}

int binparser_free(tokenizer* ptokenizer)
{
	int i;
	for (i=0; i<ptokenizer->nlists; i++) {
		free(ptokenizer->tls[i]->tl);
		free(ptokenizer->tls[i]);
	}
	free(ptokenizer->tls);
	free(ptokenizer);

	return 0;
}
