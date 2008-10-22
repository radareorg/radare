#!/usr/bin/perl
#
# parse mips code and converts to ira assembly
#

my %jumps = {};
my %calls = {};

sub is_reg {
	my $reg = shift;
	return 1
	if ($reg eq "s0"
	 || $reg eq "s1"
	 || $reg eq "s2"
	 || $reg eq "s3"
	 || $reg eq "s4"
	 || $reg eq "s5"
	 || $reg eq "s6"
	 || $reg eq "s7"
	 || $reg eq "s8"
	 || $reg eq "a0"
	 || $reg eq "a1"
	 || $reg eq "a2"
	 || $reg eq "ra");
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
	$reg=~s/s0/d0/g;
	$reg=~s/s1/d1/g;
	$reg=~s/s2/d2/g;
	$reg=~s/s3/d3/g;
	$reg=~s/s4/d4/g;
	$reg=~s/s5/d5/g;
	$reg=~s/s6/d6/g;
	$reg=~s/s7/d7/g;
	$reg=~s/s8/d8/g;
	$reg=~s/s9/d9/g;
	$reg=~s/a0/d50/g;
	$reg=~s/a1/d51/g;
	$reg=~s/a2/d52/g;
	$reg=~s/a3/d53/g;
	$reg=~s/v0/d60/g;
	$reg=~s/v1/d61/g;
	$reg=~s/v2/d62/g;
	$reg=~s/v3/d63/g;
	#$reg=~s/ra/ra/g;
	#$reg=~s/sp/sp/g;
	return $reg;
}

# TODO: store hash table with list of addresses used.

sub process {
	my $line = shift;
	my $addr = 0;
	$line=~s/,/ /g;
	$line=~s/;.*//g;
	chomp($line);
	$line=~s/\ \ /\ /g;
	$line=~s/\ \ /\ /g;
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

	if ($line=~/nop/) {
		printf ". nop\n";
	} elsif ($line=~/sb (.*) (.*)/) {
		($o,$t)=($1,$2);
		$t=~s/\)//g;
		$t=~s/\(/\+/g;
		printf ". stb [$t], $o\n";
	} elsif ($line=~/move (.*) (.*)/) {
		printf ". mov ".reg($1).", ".reg($2)."\n";
	} elsif ($line=~/addiu (.*) (.*) (.*)/) {
		if ($1 == $2) {
			printf ". addi ".reg($1).", ".reg($3)."\n";
		} else {
			printf ". addi ".reg($1).", ".reg($2).", ".reg($3)."\n";
		}
	} elsif ($line=~/li (.*) (.*)/) {
		printf ". li ".reg($1).", $2\n";
	} elsif ($line=~/lui (.*) (.*)/) {
		printf ". li ".reg($1).", $2\n";
	} elsif ($line=~/lb[u] (.*) (.*)/) {
		($o,$t)=($1,$2);
		$t=~s/\)//g;
		$t=~s/\(/\+/g;
		printf ". load (8) ".reg($o).", [$t]\n";
	} elsif ($line=~/lw (.*) (.*)/) {
		($o,$t)=($1,$2);
		$t=~s/\)//g;
		$t=~s/\(/\+/g;
		printf ". load ".reg($o).", [$t]\n";
	} elsif ($line=~/jalr (.*)/) {
		printf ". callr ".reg($1)."\n"; # TODO resolve address with flags (imports?)
		$calls{$1}=$calls{$1}+1;
	} elsif ($line=~/jr ra/) {
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
