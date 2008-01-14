// SWIG CODE

%module Radare
// %init Radare

%{
        extern char *Rcmd(char *r);
        extern void  Rquit();
%}

// C CODE

extern char *Rcmd(char *r);
extern void Rquit();
