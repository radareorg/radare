#include <stdio.h>
#include <string.h>

int main () {
//	unsigned char *bytes = "\x11\x22\x33\x44\x55\x66\x44\x89\x00";
	unsigned char *bytes = "\x90\x90\x01\x10\x55\x66\x44\x89\x00";
	int len = strlen(bytes);
	int optr, ptr = 0;
	char str[128];
	int ilen, i;
	int next;
	int type;

	while ( ptr != -1 && ptr < len ) {
		optr = ptr;
		ptr = dis_inst1 (bytes, ptr, &type);
		if (ptr == -1)
			break;
		next = dis_inst2 (str, bytes, optr);
		ilen = next - optr;
		printf ("0x%04x  %d  (%c)  ", optr, ilen, type);
		for(i = optr; i < optr+ilen; i++) {
			printf ("%02x", bytes[i]);
		}
		printf ("\t%s", str);
		if (ptr != (optr + ilen)) {
			printf("  ;  branch to 0x%04x\n", ptr);
		} else printf("\n");
		ptr = next;
	}
}
