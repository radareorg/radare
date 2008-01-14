#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHARS 128

char *Rcmd(char *r)
{
	printf("Rcmd executed\n");
	return NULL;
}

void Rquit()
{
	printf("Rquite executed\n");
}

