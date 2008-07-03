#!/usr/bin/perl
#
# mmx2ls2f
#
#   Intel x86-MMX assembly code translator to MIPS Loongson2{E,F}
#
# License: GPL
#
# author: pancake <youterm.com>
#

my %regs = ();

# MMX
$regs{"mm0"} = "f0";
$regs{"mm1"} = "f2";
$regs{"mm2"} = "f4";
$regs{"mm3"} = "f6";
$regs{"mm4"} = "f8";
$regs{"mm5"} = "f10";
$regs{"mm6"} = "f12";
$regs{"mm7"} = "f14";

# GP
$regs{"ebx"} = "f0";
$regs{"ecx"} = "t3"; # counter
$regs{"esi"} = "t4";
$regs{"edi"} = "t5";


while(<STDIN>)
{
	chomp(my $str = $_);

	$str=~s/%%/%/g;
	for $a (keys(%regs)) {
		if ($str=~/$a/) {
			eval ("\$str=~s/$a/$regs{$a}/gei;");
		}
	}

	# instructions
	$str=~s/movd\s+\(%(.+)\)\s*,\s*%([^\s]+)/"ldc1 \$$1, \$$2"/ge;
	#$str=~s/psrad\s+\$(.+)\s*,\s*%([^\s]+)/li \$t1, \$f18, \$t1psraw \$$2, \$$2, \$$1"/ge;
	$str=~s/jnz/bnez/ge;
	$str=~s/ret/jr \$ra\nnop/g;
	$str=~s/movq\s+(\d+)\(%(.+)\)\s*,\s*%([^\s]+)/ldc1 \$$3, $1 (\$$2)/g;
	$str=~s/movq\s+%\[(.+)\]\s*,\s*%([^\s]+)/ldc1 \$$2, 0($1)/g;
	$str=~s/movq\s+%(.+)\s*,\s*%\[([^\s]+)\]/sdc1 \$$1, 0($2)/g;
	$str=~s/movq\s+%(.+)\s*,\s*(\d+)\(%([^\s]+)\)/sdc1 \$$1, $2(\$$3)/g;
	$str=~s/movq\s+%(.+)\s*,\s*%([^\s]+)/mov\.d \$$2, \$$1/g;
	$str=~s/pmullw\s+%\[(.+)\]\s*,\s*%([^\s]+)/ldc1 \$f20, 0($1)\npmullh \$$2, \$$2, \$f20/g;
	$str=~s/pmullw\s+%(.+)\s*,\s*%([^\s]+)/pmullh \$$2, \$$2, \$$1/g; ## IS THIS OK ?
	$str=~s/psraw\s+\$(.+)\s*,\s*%([^\s]+)/li \$t2, $1\ndmtc1 \$t2, \$f18\npsrah \$$2, \$$2, \$f18/g;
	$str=~s/psubw\s+%(.+)\s*,\s*%([^\s]+)/psubh \$$2, \$$2, \$$1/g;
	$str=~s/paddw\s+%(.+)\s*,\s*%([^\s]+)/paddh \$$2, \$$2, \$$1/g;
	$str=~s/movl\s+\$(\d+)\s*,\s*%([^\s]+)/li \$$2, $1/g;
	$str=~s/psllw\s+\$(\d+)\s*,\s*%([^\s]+)/li \$t1, $1\ndmtc1 \$t1, \$f18\npsllh \$$2, \$$2, \$f18/g;
	$str=~s/punpcklwd\s+%(.+)\s*,\s*%([^\s]+)/punpcklhw \$$2, \$$2, \$$1/g;
	$str=~s/punpckhwd\s+%(.+)\s*,\s*%([^\s]+)/punpckhhw \$$2, \$$2, \$$1/g;
	$str=~s/punpckldq\s+%(.+)\s*,\s*%([^\s]+)/punpcklwd \$$2, \$$2, \$$1/g;
	$str=~s/punpckhdq\s+%(.+)\s*,\s*%([^\s]+)/punpckhwd \$$2, \$$2, \$$1/g;
	$str=~s/packssdw\s+%(.+)\s*,\s*%([^\s]+)/packsswh \$$2, \$$2, \$$1/g;
	$str=~s/packuswb\s+%(.+)\s*,\s*%([^\s]+)/packushb \$$2, \$$2, \$$1/g;
	$str=~s/addl\s+\$(\d+), %([^\s]+)/addi \$$2, \$$2, $1/g;
	$str=~s/subl\s+\$(\d+), %([^\s]+)/addi \$$2, \$$2, -$1/g;
	$str=~s/[^\d]0\((.*)\)/$1/g;
	$str=~s/\%f/\$f/g;

# TODO
# punpckldq    %mm7, %mm1
# punpckhdq    %mm7, %mm1

	$str=~s/^\s+//g;
	$str=~s/^\t+//g;

	print "$str\n" if ($str);
}
