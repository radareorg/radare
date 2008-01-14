/*
 * Copyright (C) 2007
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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "binparse.h"
#include "utils.h"

//
// Token file format
// =================
//
// token-list:\n
// \t$\x90\x80\x20\x10$tokname
// \t$hola$toklist
// \t$\xFF\xFF\xFF$toktag
// \t$\xFF\xFF\x/F$tokhelp
// \t$[\x20-\x30]$blah
// \t$$ALL
// --->per ordre, no pot tenir conflicte a esquerres
//

void print_tokenizer ( tokenizer* ptokenizer )
{
	int i;
	for (i=0 ; i < ptokenizer->nlists; i++ )
		print_tok_list(ptokenizer->tls[i]);
}

char* fd_readline ( int fd, char* line, int maxsize )
{
	int i;
	memset(line, 0x00, maxsize); 
	for (i=0; i<maxsize; i++) {
		read (fd, line + i, 1);
		if (line[i] =='\n') break;
	}
	line[i+1]=0;
	return line;
}


int indent_count( int fd )
{
	int ret=0;
	char t;
	read ( fd, &t, 1 );
	while ( t=='\t') {
		read ( fd, &t, 1 );
		ret++;
	}

	// Posiciono a la primera diferent.
	lseek ( fd, -1, SEEK_CUR );

	return ret;
}
unsigned char get_num( char * str, int len )
{
	char* strp;
	int value;

	strp = malloc(sizeof(char)*len);
	memcpy( strp, str, len );
	
	if (strp[0] == '\\') {
		strp[0] = '0';
		sscanf (strp,"%x",&value );
	} else	sscanf (strp,"%c",(char *)&value );
	value = value & 0xFF ;

	free(strp);
	return (unsigned char)value ;
}

int get_range(char *str, int len, unsigned char *cbase)
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

void print_tok_list(tokenlist* toklist) 
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

int tok_parse (char* str, int len, token * tlist )
{
	int i;
	int estat;
	unsigned char tokaux;
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
			tlist[tokact].mintok =  str[i];
                        tlist[tokact].range = 0;
			tokact++;
		} else
		if (estat == 2 ) { // parse \xAA
			if (str[i]=='x' ) {
				//estat = 3;
			} else {
				//Caracter escapat
				tlist[tokact].mintok =  str[i];
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
			
			sscanf ( straux ,"%hhx",&tokaux);
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
	tls->lastpos = 0;
	tls->estat = 0;

	strcpy(tls->name, line+p+1); // XXX bof here
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

tokenizer* binparse_new_from_file(char *file)
{
	int i, fd;
	char line[300];
	tokenlist* tlist;
	tokenizer* tll;
	tokenlist* tllaux[200];

	fd = open ( file, O_RDONLY );
	if (fd == -1) {
		D fprintf(stderr, "Cannot open %s\n", file);
		return NULL;
	}
	//Busco l'string token-list:\n
	while ( strcmp( fd_readline(fd, line, 300 ), "token-list:\n") );

	//Compto quants \t hi ha
	for(i=0; (i<200) && (indent_count(fd) == 1); i++) {
		//printf ("token-decl: %s\n",fd_readline ( fd, line, 300 ) );
		fd_readline ( fd, line, 300 );
		tlist=get_tok_list (  line, 300 ) ;
		//print_tok_list ( tlist );
		tllaux[i] = tlist;
	}

	tll = binparse_new(i);
	memcpy(tll->tls, tllaux, i * sizeof (tokenlist*));
	tll->nlists = i;
	
	return tll ;
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
	for ( i = 0; i < ntok ; i ++ )
		tlist[i].mask = maskout[i%masklen];
}

static tokenlist *binparse_token_mask(char *name, char *token, char *mask)
{
	tokenlist *tls;
//	token * tlist;
	void *tlist = 0;
	int ntok = 0;
	int len;
	int masklen;
	char maskout[300];

	tls = malloc(sizeof( tokenlist )) ;
	// TODO mask not yet done
	len = strlen(token);
	tlist = malloc( sizeof (token) * len+1 );
	ntok = tok_parse(token, len, tlist);

	tls->tl = tlist;
	tls->numtok = ntok;
	tls->lastpos = 0;
	tls->estat = 0;
	strcpy ( tls->name , name ); // XXX bof here!
	
	if ( mask == NULL )
		mask = "ff" ;

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
	t->nlists++;
	snprintf(name, 31, "SEARCH[%d]", n);
	t->tls    = (tokenlist **) realloc(t->tls, t->nlists*sizeof(tokenlist*));
	t->tls[n] = binparse_token_mask(name, string, mask);

	return n;
}

void update_tlist(tokenizer* t, unsigned char inchar, off_t where )
{
	unsigned char cmin;
	unsigned char cmax;
	unsigned char cmask;
	int i;

	for (i=0; i<t->nlists; i++ ) {
		cmin = (t->tls[i]->tl[t->tls[i]->estat]).mintok;

		if (  (t->tls[i]->tl[t->tls[i]->estat]).range > 0 ) {
			// RANGE
			cmax = cmin + (t->tls[i]->tl[t->tls[i]->estat]).range;
		
			if ( (inchar >= cmin) && ( inchar <= cmax ) )
				t->tls[i]->actp[t->tls[i]->estat++] = inchar;
			else	t->tls[i]->estat = 0;
		} else {
			// 1 char
			cmask = (t->tls[i]->tl[t->tls[i]->estat]).mask;
			if (  (inchar&cmask) == (cmin&cmask)   )
				t->tls[i]->actp[t->tls[i]->estat++] = inchar;
			else	t->tls[i]->estat = 0;
		}

		if ( t->tls[i]->estat == (t->tls[i]->numtok) ) {
			t->tls[i]->actp[t->tls[i]->estat+1] = 0 ;
			if (!t->callback(t, i, (unsigned long long)(where-(t->tls[i]->numtok-1)))) // t->tls[i] is the hit
				return;
			t->tls[i]->estat = 0 ;
		}
	}
}

void tokenize(int fd, tokenizer* t)
{
	char ch;
	int where = lseek(fd , 0, SEEK_CUR);
	while(1) {
		if ( read(fd, &ch, 1) <= 0 ) break;
		update_tlist(t, ch, where); 
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
