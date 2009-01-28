NAME=r_asm
OBJ=asm.o

# X86
OBJ+=arch/x86/asm.o
OBJ+=arch/x86/pseudo.o
OBJ+=arch/x86/realloc.o
# udis86
OBJ+=arch/x86/udis86/syn.o
OBJ+=arch/x86/udis86/input.o
OBJ+=arch/x86/udis86/udis86.o
OBJ+=arch/x86/udis86/decode.o
OBJ+=arch/x86/udis86/itab.o
OBJ+=arch/x86/udis86/syn-intel.o
OBJ+=arch/x86/udis86/syn-att.o
# olly
OBJ+=arch/x86/ollyasm/disasm.o
OBJ+=arch/x86/ollyasm/asmserv.o
OBJ+=arch/x86/ollyasm/assembl.o

# ARM
OBJ+=arch/arm/asm.o
# gnu arm-dis
OBJ+=arch/arm/gnu/arm-dis.o

# MIPS
OBJ+=arch/mips/asm.o
# gnu mips-dis
OBJ+=arch/mips/gnu/mips-dis.o
OBJ+=arch/mips/gnu/mips16-opc.o
OBJ+=arch/mips/gnu/mips-opc.o


include ../rules.mk
