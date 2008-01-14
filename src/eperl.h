#ifndef _INCLUDE_PERL_H_
#define _INCLUDE_PERL_H_

#if HAVE_PERL
#include <EXTERN.h>
#include <XSUB.h>
#include <perl.h>
extern PerlInterpreter *my_perl;
extern void xs_init (pTHX);
#endif

void eperl_init();
void eperl_destroy();

#endif
