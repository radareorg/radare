#!/bin/sh
#
# This is a modified version of a shellscript tool
# that comes with the EDB debugger.
#
# author:   Evan Teran <eteran@alum.rit.edu>
# homepage: http://www.codef00.com/
# license:  GPLv2
#

[ -z "$1" ] || [ ! -e "$1" ] && \
echo "Usage: make_symbolmap.sh [path/to/elf]" && exit 1

[ -z "`file -L $1| grep ELF`" ] && \
echo "This is not an ELF" && exit 1

date
md5sum $1
nm --defined-only -S -P -n $1 2>/dev/null		| awk '{ if(NF == 3) { printf("%s %08x %s %s\n",$3, 0, $2, $1); } else if(NF == 4) { printf("%s %s %s %s\n",$3, $4, $2, $1); } }'
nm --defined-only  -S -P -D -n $1  2>/dev/null	| awk '{ if(NF == 3) { printf("%s %08x %s %s\n",$3, 0, $2, $1); } else if(NF == 4) { printf("%s %s %s %s\n",$3, $4, $2, $1); } }'
