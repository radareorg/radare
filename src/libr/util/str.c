#include "r_types.h"
#include <stdio.h>

#define iswhitespace(x) (x==' '||x=='\t')

static const char *nullstr="";
static const char *nullstr_c="(null)";

char *lstrchr(char *str, char chr)
{
        int len = strlen(str);
        for(;len>=0;len--)
                if (str[len]==chr)
                        return str+len;
        return NULL;
}

int _strnstr(char *from, char *to, int size)
{
        int i;
        for(i=0;i<size;i++)
                if (from==NULL||to==NULL||from[i]!=to[i])
                        break;
        return (size!=i);
}

char *slurp(const char *str)
{
        char *ret;
        long sz;
        FILE *fd = fopen(str, "r");
        if (fd == NULL)
                return NULL;
        fseek(fd, 0,SEEK_END);
        sz = ftell(fd);
        fseek(fd, 0,SEEK_SET);
        ret = (char *)malloc(sz+1);
        fread(ret, sz, 1, fd);
        ret[sz]='\0';
        fclose(fd);
        return ret;
}

int word_count(const char *string)
{
        char *text = (char *)string;
        char *tmp  = (char *)string;
        int word   = 0;

        for(;(*text)&&(iswhitespace(*text));text=text+1);

        for(word = 0; *text; word++) {
                for(;*text && !iswhitespace(*text);text = text +1);
                tmp = text;
                for(;*text &&iswhitespace(*text);text = text +1);
                if (tmp == text)
                        word-=1;
        }

        return word-1;
}

int set0word(char *str)
{
        int i;
        char *p;
        if (str[0]=='\0')
                return 0;
        for(i=1,p=str;p[0];p=p+1)if(*p==' '){i++;*p='\0';} // s/ /\0/g
        return i;
}

const char *get0word(const char *str, int idx)
{
        int i;
        const char *ptr = str;
        if (ptr == NULL)
                return nullstr;
        for (i=0;*ptr && i != idx;i++)
                ptr = ptr + strlen(ptr) + 1;
        return ptr;
}

int iswhitechar(char c)
{
        switch(c) {
        case ' ': 
        case '\t':
        case '\n':
        case '\r':
                return 1;
        }       
        return 0;
}

char *strclean(char *str)
{
        int len;
        char *ptr;

        if (str == NULL)
                return NULL;
                
        while(str[0]&&iswhitechar(str[0]))
                str = str + 1;
                
        len = strlen(str);
        
        if (len>0)
        for(ptr = str+len-1;ptr!=str;ptr = ptr - 1) {
                if (iswhitechar(ptr[0])) 
                        ptr[0]='\0';
                else    break;
        }               
        return str;
}

int strhash(const char *str)
{
        int i = 1;
        int a = 0x31;
        int b = 0x337;
        int h;
        for(h = str[0] ; str[i]; i++) {
                h+=str[i]*i*a;
                 a*=b;
        }
        return h&0x7ffffff;
}

// FROM bash::stringlib
#define RESIZE_MALLOCED_BUFFER(str,cind,room,csize,sincr) \
        if ((cind) + (room) >= csize) { \
                while ((cind) + (room) >= csize) \
                csize += (sincr); \
                str = realloc (str, csize); \
        }

/* Replace occurrences of PAT with REP in STRING.  If GLOBAL is non-zero,
   replace all occurrences, otherwise replace only the first.
   This returns a new string; the caller should free it. */

int strsub_memcmp (char *string, char *pat, int len)
{
        int res = 0;
        while(len--) {
                if (*pat!='?')
                        res += *string - *pat;
                string = string+1;
                pat = pat+1;
        }
        return res;
}

char *strsub (char *string, char *pat, char *rep, int global)
{
        int patlen, templen, tempsize, repl, i;
        char *temp, *r;

        patlen = strlen (pat);
        for (temp = (char *)NULL, i = templen = tempsize = 0, repl = 1; string[i]; )
        {
//              if (repl && !memcmp(string + i, pat, patlen)) {
                if (repl && !strsub_memcmp(string + i, pat, patlen)) {
                        RESIZE_MALLOCED_BUFFER (temp, templen, patlen, tempsize, 4096); //UGLY HACK (patlen * 2));
                        if (temp == NULL)
                                return NULL;
                        for (r = rep; *r; )
                                temp[templen++] = *r++;

                        i += patlen;
                        repl = global != 0;
                } else {
                        RESIZE_MALLOCED_BUFFER (temp, templen, 1, tempsize, 4096); // UGLY HACK 16);
                        temp[templen++] = string[i++];
                }
        }
        if (temp != NULL)
                temp[templen] = 0;
        return (temp);
}


char *str_first_word(const char *string)
{
        char *text  = (char *)string;
        char *start = NULL;
        char *ret   = NULL;
        int len     = 0;

        for(;*text &&iswhitespace(*text);text = text + 1);
        start = text;
        for(;*text &&!iswhitespace(*text);text = text + 1) len++;

        /* strdup */
        ret = (char *)malloc(len+1);
        if (ret == 0) {
                fprintf(stderr, "Cannot allocate %d bytes.\n", len+1);
                exit(1);
        }
        strncpy(ret, start, len);
        ret[len]='\0';

        return ret;
}

/* memccmp("foo.bar", "foo.cow, '.') == 0 */
int strccmp(char *dst, char *orig, int ch)
{
        int i;
        for(i=0;orig[i] && orig[i] != ch; i++)
                if (dst[i] != orig[i])
                        return 1;
        return 0;
}

int strccpy(char *dst, char *orig, int ch)
{
        int i;
        for(i=0;orig[i] && orig[i] != ch; i++)
                dst[i] = orig[i];
        dst[i] = '\0';
        return i;
}

const char *strget(const char *str)
{
        if (str == NULL)
                return nullstr_c;
        return str;
}


char *estrdup(char *ptr, const char *string)
{
        if (ptr)
                free(ptr);
        ptr = strdup(string);
        return ptr;
}

void efree(void **ptr)
{
        free (*ptr);
        *ptr = NULL;
}
