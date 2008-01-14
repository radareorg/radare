int hash_par(unsigned char *buffer, int len);
unsigned short hash_xorpair(unsigned short *b, int len);
unsigned char hash_xor(unsigned char *b, int len);
unsigned char hash_mod255(unsigned char *b, int len);
unsigned long hash_wrt54gv5v6(unsigned char *pStart, int len);
unsigned short hash_bootp(unsigned char *data, int len);
unsigned char hash_hamdist(char *buf, int len);
