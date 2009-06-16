r("e scr.color=false");
r("e asm.profile=simple");
@pd = split(/\n/, r("pd 20"));
for $i (0 .. $#pd) {
	$pd[$i]=~/([^\s]*)(.*)$/;
	chomp(my $offset = $1);
	chomp(my $opcode = $2);
	$opcode=~s/^[\t|\s]*//g;
	$offset=~s/^[\t|\s]*//g;
	print "$offset => $opcode\n";
}
