#!/usr/bin/perl

my $jc="\x1b[32m";
my $nc="\x1b[0m";
my $cc="\x1b[33m";
my $lc="\x1b[31m";
my $sc="\x1b[31m";

my @stack=();
my %la={};
my @labels=();
my $cmp0;
my $cmp1;
my %symbols={};

my $out="";

while(<STDIN>) {
	my $line = $_;
	$line=~s/\.//g;
	$line=~s/,//g;
	$line=~s/\ \ /\ /g;
#	next if (/sp/); ## XXX BUGGY
	chomp($line);
	if ($line=~/^offset (.*)/) {
		$addr = $1;
	} elsif ($line=~/^function (.*)/) {
		$out.= "$addr  int $1 ( ... ) {\n";
	} elsif ($line=~/^string ([^ ]*) (.*)$/) {
		$strings{$1}=$2;
	} elsif ($line=~/^symbol ([^ ]*) ([^ ]*)/) {
		$symbols{$1}=$2;
	#	$out.= "label_$1:\n";
	} elsif ($line=~/^label (.*)/) {
		push @labels, $1;
		$out.= "$addr  ".$lc."label_$1:$nc\n";
	} elsif ($line=~/pushi ([^ ]*)/) {
		push @stack, $1;
	} elsif ($line=~/pop ([^ ]*)/) {
		pop @stack;
	} elsif ($line=~/push ([^ ]*)/) {
		push @stack, $1;
	} elsif ($line=~/sti \[([a-z.0-9]*)\] ([^ ]*)/) {
		push @stack, $2; # if ($1=~/sp/);
	} elsif ($line=~/sti ([^ ]*) (.*)$/) {
		$out.="$addr  $1 = $2;\n";
		# XXX ignored
	} elsif ($line=~/call ([^ ]*)/) {
		$out.= "$addr    $1 ( ";
		for $i (0 .. $#stack) {
			$op   = $stack[$#stack-$i];
			if ($op ne "bp") {
				$out .= $op;
				$out .=", " if ($i < $#stack && $stack[$#stack-$i+1] eq "bp");
			}
		}
		$out.= " );\n";
		@stack=();
	} elsif ($line=~/mov ([^ ]*) (.*)/) {
		$out.= "$addr  $1 = $2;\n";
	} elsif ($line=~/sti ([^ ]*) (.*)/) {
		$out.="$addr  $1 = $2;\n";
	} elsif ($line=~/sri ([^ ]*) (.*)/) {
		$out.="$addr  $1 >>= $2;\n"; ## XXX nop is possible
	} elsif ($line=~/sli ([^ ]*) (.*)/) {
		$out.="$addr  $1 <<= $2;\n"; ## XXX nop is possible
	} elsif ($line=~/li (.*) (.*)/) {
		$out.="$addr  $1 = $2;\n";
	} elsif ($line=~/la (.*) (.*)/) {
		$la{$1}=$2;
	} elsif ($line=~/andi ([^ ]*) (.*)/) {
		$out.="$addr  $1 &= $2;\n";
	} elsif ($line=~/adda (.*) (.*)/) {
		($o,$t)=($1,$2);
		$o=~s/\[//g;
		$o=~s/\]//g;
		if ($la{$o} eq "") {
			$out.="$addr  $o += $t;\n";
		} else {
			$out.="$addr  $la{$o} += $t;\n";
		}
	} elsif ($line=~/cmpai (.*) (.*)/) {
		$cmp0=$1;
		$cmp1=$2;
	} elsif ($line=~/ret/) {
		$out.="$addr  return;\n  }\n";
	} elsif ($line=~/jmp (.*)/) {
		$out.="$addr  $jc goto$nc label_$1;\n";
	} elsif ($line=~/jne (.*)/) {
		$out.="$addr  $cc if$nc ($cmp0 != $cmp1)$jc goto$nc label_$1;\n";
	} elsif ($line=~/je (.*)/) {
		$out.="$addr  $cc if$nc ($cmp0 == $cmp1)$jc goto$nc label_$1;\n";
	} elsif ($line=~/jg (.*)/) {
		$out.="$addr  $cc if$nc ($cmp0 > $cmp1)$jc goto$nc label_$1;\n";
	} elsif ($line=~/addi (.*) (.*)/) {
		$out.="$addr  $1 += $2;\n";
	} elsif ($line=~/^subi (.*) (.*)/) {
		$out.="$addr  $1 -= $2;\n";
	} elsif ($line=~/^sri (.*) (.*)/) {
		if ($shr_r ==$1 ) {
			$shr_i
		} else {
			$shr_r=$1;
			$shr_i=$2;
		}
	} elsif ($line=~/^sli (.*),(.*)/) {
		$shr_r=$1;
		$shr_i=$2;
	} else {
		$out.="\x1b[33m                      ; $line\x1b[0m\n";
	}
}

while(($key, $val) = each %symbols) {
	$out=~s/$key/$cc$val$nc/g;
}

while(($key, $val) = each %strings) {
	$out=~s/$key/$sc$val$nc/g;
}

$out=~s/bp-0x/var_/g;
$out=~s/\[var/var/g;
$out=~s/\]//g;
$out=~s/0x(\d*);/$1;/g;
$out=~s/0x(\d*)\)/$1\)/g;

print $out;
