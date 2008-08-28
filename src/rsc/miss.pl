#!/usr/bin/perl

my @ls=split(/\n/,`ls`);
my @files =split(/\n/,`make list`);
my @miss=();

my $found;
for my $a (@ls) {
	$found = 0;
	for my $b (@files) {
		if ($b eq $a) {
			$found = 1;
			last;
		}
	}
	push(@miss, $a) unless($found);
}
for my $b (@miss) {
	print "$b\n";
}
