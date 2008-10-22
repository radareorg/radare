#!/usr/bin/perl
sub reg {
	my $reg = shift;
	$reg=~s/d50/eax/g;
	$reg=~s/d51/ebx/g;
	$reg=~s/d52/ecx/g;
	$reg=~s/d53/edi/g; # XXX register allocation
	$reg=~s/d60/eax/g;
	$reg=~s/d61/ebx/g;
	$reg=~s/d62/ecx/g;
	$reg=~s/d63/edi/g;
	$reg=~s/d0/eax/g;
	$reg=~s/d1/ebx/g;
	$reg=~s/d2/ecx/g;
	$reg=~s/d3/edx/g;
	$reg=~s/d4/esi/g;
	$reg=~s/d5/edi/g;
	$reg=~s/s4/eax/g;
	$reg=~s/s5/ebx/g;
	$reg=~s/s6/ecx/g;
	$reg=~s/s7/edx/g;
	$reg=~s/s8/esi/g;
	$reg=~s/s9/edi/g;
	$reg=~s/sp/esp/g;
	$reg=~s/bp/ebp/g;
	$reg=~s/pc/eip/g;
	$reg=~s/ra/ebp/g;
	$reg=~s/sr/eflags/g;
	return $reg;
}

my $addr=0;
while(<STDIN>) {
	$line=$_;
	$line=~s/,/ /g;
	$line=~s/^\ *//g;
	$line=~s/\(8\)/ byte /g;
	$line=~s/\(16\)/ word /g;
	$line=~s/\(32\)/ dword /g;
	$line=~s/\ \ /\ /g;
	$line=~s/\ \ /\ /g;
	chomp($line);
	if ($line=~/^\.offset (.*)$/) {
		$addr=$1;
		next;
	}
	if ($line=~/ret/) {
		print "$addr  ret\n";
	} elsif ($line=~/mov (.*) (.*)/) {
		print "$addr  mov ".reg($1).", ".reg($2)."   ; \n";
	} elsif ($line=~/addi (.*) (.*)/) {
		print "$addr  add ".reg($1).", $2\n";
	} elsif ($line=~/stb (.*) (.*)/) {
		print "$addr  mov $1, ".reg($2)."\n";
	} elsif ($line=~/callr (.*)/) {
		print "$addr  call ".reg($1)."\n";
	} elsif ($line=~/load (.*) (.*)/) {
		print "$addr  mov ".reg($1).", ".reg($2)."   ; \n";
	} elsif ($line=~/li (.*) (.*)/) {
		print "$addr  mov ".reg($1).", $2   ; \n";
	} else {
		print "\x1b[33m$line\x1b[0m\n";
	}

}
