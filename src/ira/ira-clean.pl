#!/usr/bin/perl

my $file="";
my %labels={};

while(<STDIN>) {
	my $line=$_;
	$file.=$line;
	if ($line=~/^\.label (.*)/) {
		$labels{$1}=1 
	}
}

my $file2 = "";

my @lines= split('\n', $file);
foreach $i (0 .. $#lines) {
	next if ($lines[$i]=~/^\. nop/);
	next if ($lines[$i]=~/^\.label/);
	if ($lines[$i]=~/^\.offset (.*)/) {
		my $addr = $1;
		while(($key, $val) = each %labels) {
			if ($addr eq $key) {
				$file2.=".label $addr\n";
				last;
			}
		}
	}# else {
		$file2.=$lines[$i]."\n";
	#}
}

print $file2;
