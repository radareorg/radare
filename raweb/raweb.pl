#!/usr/bin/env perl
if (@ARGV<1) {
	print "Usage: raweb [/path/to/bin]\n";
	exit;
}
my $file = $ARGV[0];
my $name = substr($file, rindex($file,"/")+1);
my $objdump = "arm-unknown-linux-gnu-objdump -m arm";
$objdump = "objdump -m i386";

mkdir $name;
system("cp $file $name/$name.bin");
system ("$objdump --target=binary -D $file > $name/$name.txt");
system("radare -S 10 $file > $name/$name.str");
system("chmod 777 $name");
system("objdump -d /bin/ls|grep '>:' > $name/$name.tmp");
open FD, "<$name/$name.tmp" || die "oops";
open DD, ">$name/$name.db"  || die "oops";
while(<FD>) {
	/(.*) <(.*)>:/;
	my $offset=sprintf("%x", eval("0x$1")-0x8048000);
	my $label=$2;
	print DD "label $offset $label\n";
}
close DD;
close FD;
