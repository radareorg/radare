#!/usr/bin/perl
my %hash = {};
my $level = 0;

print "#include <stdio.h>\n";

while(<STDIN>) {
	my $line = $_;
	if ($line=~/^0x/) {
		$line=~s/0x[^ ]*//;
	}
	$line=~s/\x1b[^m]*.//g;
	$line=~s/\.\.\.//g;
	$line=~s/^\s*//g;
	$line=~s/^;/\/\//;
	#print "  "*$level;
	$level++ if (/{/);
	$level-- if (/}/);
	for($i=0;$i<$level;$i++) { print "  "; };
	next if ($line=~/bp =/);
	next if ($line=~/sp .=/);
	if ($line=~/(.*) = (.*)/) {
		my $var = $1;
		my $val = $2;
		if (!defined($hash{$var})) {
			$hash{$var} = $val;
			print "int ";
		}
	}
	print $line;
}
