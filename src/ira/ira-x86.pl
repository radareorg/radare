#!/usr/bin/perl
#
# parse x86 code and converts to ira assembly
#

my %jumps = {};
my %calls = {};

sub is_reg {
	my $reg = shift;
	return 1
	if ($reg eq "eax"
	 || $reg eq "ebx"
	 || $reg eq "ecx"
	 || $reg eq "edx"
	 || $reg eq "esi"
	 || $reg eq "edi"
	 || $reg eq "ebp"
	 || $reg eq "esp"
	 || $reg eq "eip"
	 || $reg eq "eflags");
	return 0;
}

sub is_inmediate {
	my $reg = shift;
	return 1 if ($reg=~/^[0-9]*/);
	return 0;
}

sub is_addr {
	my $reg = shift;
	return 1 if ($reg=~/\[/);
	return 0;
}

sub reg {
	my $reg = shift;
	$reg=~s/eax/d0/g;
	$reg=~s/ebx/d1/g;
	$reg=~s/ecx/d2/g;
	$reg=~s/edx/d3/g;
	$reg=~s/esi/d4/g;
	$reg=~s/edi/d5/g;
	$reg=~s/esp/sp/g;
	$reg=~s/ebp/bp/g;
	$reg=~s/eip/pc/g;
	$reg=~s/eflags/sr/g;
	return $reg;
}

# TODO: store hash table with list of addresses used.

sub process {
	my $line = shift;
	my $addr = 0;
	$line=~s/;.*//g;
	$line=~s/,/ /g;
	$line=~s/\ \ /\ /g;
	$line=~s/\x1b.*m//g;
	$line=~s/sarl/sar/g;
	$line=~s/shll/sll/g;
	$line=~s/addl/add/g;
	$line=~s/movl/mov/g;
	$line=~s/cmpl/cmp/g;
	chomp($line);
	return if ($line eq "");
	if ($line=~/^0/) {
		$str= $line;
		$str=~/^([^\ ]*)/;
		$addr = $1;
		printf ".offset $addr\n";
	}
	if ($line=~/^\.function (.*)/) {
		print "$line\n";
		next;
	}
	if ($line=~/^\.symbol ([^ ]*) ([^ ]*)/) {
		print "$line\n";
		next;
	}
	if ($line=~/^\.string ([^ ]*) ([^ ]*)/) {
		print "$line\n";
		next;
	}
	if ($line=~/^\.import (.*)/) {
		print "$line\n";
		next;
	}
	if ($line=~/push (.*)/) {
		if (is_reg($1)) {
			printf ". push ".reg($1)."\n";
		} elsif (is_addr($1)) {
			printf ". pusha $1\n";
		} elsif (is_inmediate($1)) {
			printf ". pushi $1\n";
		} else {
			print STDERR "Unknown push '$line'\n";
		}
	} elsif ($line=~/jmp (.*)/) {
		if (is_inmediate($1)) {
			printf ". jmp $1\n"; # TODO: jmp [0x0303] is_addr check!
		} elsif (is_addr($1)) {
			printf ". jmp $1\n"; # TODO: jmp [0x0303] is_addr check!
		} else {
			print STDERR "Unknown jmp '$line'\n";
		}
		$jmps{$1}=$jmps{$1}+1;
	} elsif ($line=~/jnz (.*)/) {
		printf ". jne $1\n"; # TODO: jg [0x0303] is_addr check!
		$jmps{$1}=$jmps{$1}+1;
	} elsif ($line=~/jz (.*)/) {
		printf ". je $1\n"; # TODO: jg [0x0303] is_addr check!
		$jmps{$1}=$jmps{$1}+1;
	} elsif ($line=~/jge (.*)/) {
		printf ". jge $1\n"; # TODO: jg [0x0303] is_addr check!
		$jmps{$1}=$jmps{$1}+1;
	} elsif ($line=~/jg (.*)/) {
		printf ". jg $1\n"; # TODO: jg [0x0303] is_addr check!
		$jmps{$1}=$jmps{$1}+1;
	} elsif ($line=~/neg (.*)/) {
		printf ". neg $1\n";
	} elsif ($line=~/inc ([^ ]*)/) {
		if (is_addr($1)) {
			printf ". adda ".reg($1).", 1\n";
		} elsif (is_reg($1)) {
			printf ". addi ".reg($1).", 1\n";
		} else {
			print STDERR "Unknown INC '$line'\n";
		}
	} elsif ($line=~/or (.*) (.*)/) {
		if (is_reg($1) && is_inmediate($2)) {
			printf ". ori ".reg($1).", $2\n";
		} elsif (is_addr($1) && is_inmediate($2)) {
			printf ". ora $1, $2\n";
		} else {
			print STDERR "Unknown OR '$line'\n";
		}
	} elsif ($line=~/and ([^ ]*) (.*)/) {
		if (is_reg($1) && is_inmediate($2)) {
			printf ". andi ".reg($1).", $2\n";
		} else {
			print STDERR "Unknown AND '$line'\n";
		}
	} elsif ($line=~/add ([^ ]*) (.*)/) {
		if (is_reg($1) && is_inmediate($2)) {
			printf ". addi ".reg($1).", $2\n";
		} elsif (is_addr($1) && is_inmediate($2)) {
			printf ". adda ".reg($1).", ".reg($2)."\n";
		} else {
			print STDERR "Unknown ADDI '$line'\n";
		}
	} elsif ($line=~/shl ([^ ]*) (.*)/) {
		if (is_reg($1) && is_inmediate($2)) {
			printf ". sli ".reg($1).", ".reg($2)."\n";
		} else {
			print STDERR "Unknown SHL '$line'\n";
		}
	} elsif ($line=~/sll ([^ ]*) (.*)/) {
		if (is_reg($1) && is_inmediate($2)) {
			printf ". slli ".reg($1).", $2\n";
		} elsif (is_addr($1) && is_inmediate($2)) {
			printf ". slla ".reg($1).", $2\n";
		} else {
			print STDERR "Unknown SHR '$line'\n";
		}
	} elsif ($line=~/sar ([^ ]*) (.*)/) {
		if (is_reg($1) && is_inmediate($2)) {
			printf ". sari ".reg($1).", $2\n";
		} elsif (is_addr($1) && is_inmediate($2)) {
			printf ". sara ".reg($1).", $2\n";
		} else {
			print STDERR "Unknown SHR '$line'\n";
		}
	} elsif ($line=~/shr ([^ ]*) (.*)/) {
		if (is_reg($1) && is_inmediate($2)) {
			printf ". sri ".reg($1).", $2\n";
		} else {
			print STDERR "Unknown SHR '$line'\n";
		}
	} elsif ($line=~/sub ([^ ]*) (.*)/) {
		if (is_reg($1) && is_inmediate($2)) {
			printf ". addi ".reg($1).", ".reg($2)."\n";
		} else {
			print STDERR "Unknown SUB '$line'\n";
		}
	} elsif ($line=~/lea ([^ ]*) (.*)/) {
		if (is_reg($1) && is_addr($2)) {
			printf ". la ".reg($1).", ".reg($2)."\n";
		} else {
			print STDERR "Unknown LEA '$line'\n";
		}
	} elsif ($line=~/mov ([^ ]*) ([^ ]*) ([^ ]*)/) {
		$o=$1;
		$t=$2;
		$b=$3;
		$line=~s/byte/(8)/;
		$line=~s/word/(16)/;
		$line=~s/dword/(32)/;
		$line=~s/qword/(64)/;
		if ($line=~/\(/) {
			$line=~s/\(.*\) //g; # XXX
			$line=~/mov ([^ ]*) ([^ ]*)/;
			$o=$1;
			$t=$2;
		}
		if (is_reg($o) && is_reg($t)) {
			printf ". mov ".reg($o).", ".reg($t)."\n";
		} elsif (is_reg($o) && is_addr($t)) {
			printf ". la ".reg($o).", $t\n";
		} elsif (is_reg($o) && is_inmediate($t)) {
			printf ". li ".reg($o).", $t\n";
		} elsif (is_addr($o) && is_inmediate($t)) {
			printf ". sti ".reg($o).", $t\n";
		} else {
			print STDERR "Unknown MOV '$line'\n";
		}
	} elsif ($line=~/mov ([^ ]*) ([^ ]*)/) {
		$o=$1;
		$t=$2;
		if (is_reg($o) && is_reg($t)) {
			printf ". mov ".reg($o).", ".reg($t)."\n";
		} elsif (is_reg($o) && is_inmediate($t)) {
			printf ". li ".reg($o).", $t\n";
		} elsif (is_addr($o) && is_inmediate($t)) {
			printf ". sti ".reg($o).", $t\n";
		} else {
			print STDERR "Unknown MOV '$line'\n";
		}
	} elsif ($line=~/cmp (.*) (.*)/) {
		if (is_addr($1) && is_inmediate($2)) {
			printf ". cmpai ".reg($1).", $2\n";
		} else {
			print STDERR "Unknown LEA '$line'\n";
		}
	} elsif ($line=~/call (.*)/) {
		$calls{$1}=$calls{$1}+1;
		if (is_inmediate($1)) {
			printf ". call $1\n"; # TODO resolve address with flags (imports?)
		} elsif (is_addr($1)) {
			printf ". call $1\n"; # TODO resolve address with flags (imports?)
		} elsif (is_reg($1)) {
			printf ". callr $1\n"; # TODO resolve address with flags (imports?)
		} else {
			print STDERR "Unknown CALL '$line'\n";
		}
	} elsif ($line=~/pop (.*)/) {
		printf ". pop ".reg($1)."\n";
	} elsif ($line=~/leave/) {
		printf ". pop sp\n"; # XXX
	} elsif ($line=~/ret/) {
		printf ". ret\n";
	} else {
		print STDERR "Unknown opcode '$line'\n";
	}
}

while(<STDIN>) {
	s/dword//g;
	process($_);
}


while(($key, $val) = each %jmps) {
	print ".label $key\n";
}

while(($key, $val) = each %calls) {
	if ($key=~/\[/||$key=~/[0-9]*/) {
		print ".import $key\n";
		last; # XXX
	}
}
