my %vars={};
my $out="";

while(<STDIN>) {
	$line=$_;
	$oline=$_;
	$line=~s/^0x([^ ]*)//g;
	$line=~s/\x1b[^m]*.//g;
	$line=~s/\.\.\. \) {/) {\n(local-vars)/g;
	$line=~s/, bp//g;
	$line=~s/;\ [a-z]*$//g;
	if ($line=~/=/) {
		$cp=$line;
		$cp=~s/^\ *//g;
		$cp=~/([a-z0-9_A-Z]*)/;
		$vars{$1}=1;
	}
	unless ($oline=~/^0x/) {
		next if ($line=~/;/);
	}
	$out.=$line;
}

my $var ="";
while(($key, $val) = each %vars) {
	$var .= "  int $key;\n" if ($val == 1);
}
$out=~s/\(local-vars\)/$var/;

print $out;
