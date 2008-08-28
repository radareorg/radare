#!/usr/bin/perl -w
# Based on the LINUX DISASSEMBLER 2.0799 (C) SiuL+Hacky Jul 1999 
# and on the REAP project http://reap.cjb.net/ <grugq@iname.com>
# This file Copyright 1999 <squeak@xirr.com> under the GPL
# NO WARRANTY. This is not useful software. Send bug reports
# and patches to squeak. version 0.2 Nov 30 1999

use strict;

if ($#ARGV!=0 or ! -r $ARGV[0]) {
	print main::STDERR "usage: $0 binary | more\n" .
		"\tDisplays useful debugging info\n";
	exit(1);
}

my ($ro_data,$ro_size,$ro_start,$ro_foff,%xref);
my (%symbol_table,$src,$func,$offset,$raw,$inst,$dest,$addy);
my ($start,$cnt,$edge);

sub clean_dis($$$$$$) {
	my @a=@_;

	# addy, offset, raw, inst, arguments, nopping, rising_edge

	# no field except perhaps the last should have spaces
	$a[0]=~s/[\s]//g;
	$a[1]=~s/[\s]//g;
	$a[2]=~s/[\s]//g;
	$a[3]=~s/[\s]//g;

	# cleanup up the address/offset to get rid of useless info
	$a[0]=~s/^0x//;
	$a[0]=~s/^([\dA-Fa-f]+).*/$1/;
	$a[0]=~s/\b0(?=[\da-fA-F])//g;

	$a[1]=~s/^0x//;
	$a[1]=~s/^([\dA-Fa-f]+).*/$1/;
	$a[1]=~s/\b0(?=[\da-fA-F])//g;

	# correct the negative number thing
	$a[4]=~s/0x([fF]{2,}[\da-fA-F]*)(?!\>)/sprintf "-0x%x", 1+~hex($1)/eg;

	$a[6]=0;

	if( $a[5]  && ! ($a[3] =~ /^nop/) && ! ( 
		$a[3] =~ /^mov/ and $a[4] =~ /%esi,%esi/)) {
		$a[6]=1;
		$a[5]=0;
		$a[1]="0";
	}

	if( ($a[3] =~ /^ret/) or ($a[3] =~ /^hlt/) ) {
		$a[5]=1;
	}

	return @a;
}

sub parse_dis($$$) {
	my ($start,$nop);
	$start=$_[1];
	$nop=$_[2];
	$_=$_[0];

	s/00000000(?=\d)//g;
	s/^0x//g;
	s/[\s]+/ /g;
	/()/;

	/^	([\dA-Fa-f]+)[\s]+			# $address
		\<([.\w]+)\+?0?x?([0-9A-Fa-f]*)\>[\s]* 	# <$func+$offset>
		(([\dA-Fa-f][\dA-Fa-f][\s])+)[\s]*	# $raw_insn
		([\w]+)(.*)$/x;				# $inst $arguments
	return clean_dis($1,$3,$4,$6,$7,$nop) 
		if(defined $7);

	/^	([\dA-Fa-f]+)[\s]+			# $address
		(([\dA-Fa-f][\dA-Fa-f][\s])+)[\s]*	# $raw_insn
		([\w]+)(.*)$/x;				# $isnt $arguments

	$start=$start?$start:$1;
	return clean_dis($1,sprintf ("%x",hex($1)-hex($start)),
		$2,$4,$5,$nop) 
		if(defined($5));

	return ("","","","","",$nop);
}

sub lookup_addy($){
	if( $addy >= $ro_start  and $addy < $ro_start + $ro_size ) {
		$addy=substr($ro_data, $addy - $ro_start);
		$addy=substr($addy,0,index($addy,"\x00"));
		return "(R '" . $addy . "')";
	}
	return "(R <".$symbol_table{$addy}.">) " 
		if($addy and $symbol_table{$addy});
	return "";
}





my $command ="objdump -drRphx --show-raw-insn --prefix-addresses" .
	" $ARGV[0] 2>&1|";

open(INPUT, $command) or die "Could not open objdump $! $^E.";


# First read the shdr's to find the rodata section
while (<INPUT>) { last if (/\.rodata/); }

s/^[\s]+//; s/[\s]+/ /g;
(undef,undef,$ro_size,$ro_start,undef,$ro_foff,undef)=split(/ /,$_,7);

$ro_size=hex($ro_size);
$ro_start=hex($ro_start);
$ro_foff=hex($ro_foff);

# Now check for a symbol table
while (<INPUT>){ last if /SYMBOL TABLE/ or /Contents of section / or /Disassembly of section /}

if (/SYMBOL TABLE/){ 
	while (<INPUT>) {
		last if(/^\n/);
		s/[\s0]*([\da-fA-F]+).*[\s]([^\s]+)/ $1 $2/g;
		$symbol_table{$1}=$2 if defined($2);
	}
}

# Now jump down to the code
unless (/Disassembly /) { while (<INPUT>) { last if /Disassembly of section/; } }

# Gather data on program flow

$func="";
$start="0";
my	$nop=1;
$cnt=0;
while (<INPUT>){
	if(/Disassembly of section / or /^$/ or /:$/) {
		$nop=1;
		next;
	}
	
	($src, $offset, $raw, $inst, $dest, $nop, $edge)=
		parse_dis($_,$start,$nop);

	/()()/;
	$dest =~ /(0x)?([0-9a-fA-F]+)/;
	$dest = $2;
	$dest =~s/\b0(?=[\da-fA-F])//g;

	if($edge or defined($symbol_table{$src})) {
		$symbol_table{$src} = "FUNC_$cnt" 
			unless defined($symbol_table{$src});
		$func= $symbol_table{$src};
		$cnt++;
		$start=$src;
	}

	$xref{$dest}.= "(J" . (($inst =~ /^jmp/)?"U":"C") . 
		" $src" . ($func?" <$func+$offset>":"") . ") " 
		if ($inst =~ /j/);

	$xref{$dest}.= "(CL $src" . ($func?" <$func+$offset>":"") . ") "
		if ($inst =~ /call/);
}
close(INPUT);

# grab the .rodata section
open(INPUT, "< ".$ARGV[0]);
sysseek(INPUT,$ro_foff,0);
sysread(INPUT,$ro_data,$ro_size);
close(INPUT);

#$command ="objdump -drRsaphx --show-raw-insn --prefix-addresses" .
#	" $ARGV[0] 2>&1|";
# Now redo the command this time with knowledge of the numbers
open(INPUT, $command) or die "Could not open objdump $! $^E";

# Print everything until we start disassembling
while(<INPUT>) {
	s/0x//g;
	# Remove leading zeroes until we get to hexdumps
	s/\b0(?=[\da-fA-F])/ /g;
	last if (/Contents of section / or /Disassembly of section/);
	print;
}

if(/Contents/) {
	while(<INPUT>) {
		last if (/Disassembly of section/);
		print;
	}
}


# Now the code
s/Disassembly of section /\n/;
print;

$start="0";
$nop=1;
my $extra;
while(<INPUT>) {

	if (/Disassembly of section / or /:$/ or /^$/) {
		s/Disassembly of section /\n/g;
		print ;
		$nop=1;
		next;
	}
	($src, $offset, $raw, $inst, $dest,$nop,$edge)=parse_dis($_,$start,$nop);

	$extra="";
	$dest=~s/0*([\da-fA-F]+).*(<.+>)/($extra="(to $1 $2) "),""/e;

	# Lookup up this function
	if(defined($symbol_table{$src})) {
		print "\n<" . $symbol_table{$src} . ">\n";
		$start=$src;
	}

	# Print the main data
	printf "%8.8s (%3.3s) %-16.16s %-5.5s %-20s", 
		$src, $offset, $raw, $inst, $dest;

	# Print who jumps here
	print $extra if($extra);
	print $xref{$src} if (defined($xref{$src}));

	# Read string literals
	if ( ($dest =~ /\$(0x[\da-fA-F]+)/)) {
		$addy=hex($1);
		print lookup_addy($addy);
	}

	print "\n";
};

close(INPUT);

print "\n";
