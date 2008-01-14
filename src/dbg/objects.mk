# Stupid trick
ifeq ($(CPU),i686)
CPU=i386
endif
ifeq ($(CPU),i586)
CPU=i386
endif


ifeq ($(OS),Windows_NT) 
psOBJ= dbg/win32/utils.o dbg/win32/debug.o
psOBJ+= dbg/win32/fd.o dbg/win32/signal.o
psOBJ+=dbg/arch/${CPU}-hack.o
psOBJ+=dbg/arch/${CPU}.o dbg/arch/${CPU}-bp.o
psOBJ+=dbg/close.o dbg/debug.o dbg/lseek.o dbg/open.o
psOBJ+=dbg/system.o dbg/signal.o
psOBJ+=dbg/mem.o dbg/wp.o dbg/events.o
psOBJ+=dbg/thread.o dbg/parser.o
else
# Objects here
psOBJ=dbg/arch/${CPU}.o dbg/arch/${CPU}-hack.o dbg/arch/${CPU}-bp.o
psOBJ+=dbg/close.o dbg/debug.o dbg/lseek.o dbg/open.o
psOBJ+=dbg/system.o dbg/signal.o
psOBJ+=dbg/mem.o dbg/wp.o dbg/unix/fd.o dbg/events.o
#psOBJ+=dbg/unix/debug.o dbg/unix/fd.o dbg/unix/signal.o
psOBJ+=dbg/unix/signal.o dbg/unix/debug.o
psOBJ+=dbg/procs.o dbg/thread.o dbg/parser.o
endif
