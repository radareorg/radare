all:
	 cat test.x86.s | perl ira-x86.pl | perl ira-clean.pl | perl ira-pseudo.pl

ira:
	 cat test.x86.s | perl ira-x86.pl | perl ira-clean.pl

2:
	 cat test.x86-2.s | perl ira-x86.pl | perl ira-clean.pl

m:
	 cat test.mips.s | perl ira-mips.pl | perl ira-clean.pl

m86:
	 cat test.mips.s | perl ira-mips.pl | perl ira-clean.pl | perl ira-to-x86.pl

m2:
	 cat test.mips.s | perl ira-mips.pl | perl ira-clean.pl | perl ira-pseudo.pl
