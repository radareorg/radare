void ver(int l, int b, char *buff)
{
	char *v = (char *)0;

	if(buff[0] == 0x13 && buff[1] == 0x32) {
		printf("BINGO: %x %x\n", (unsigned char)buff[0], (unsigned char)buff[1]);
		/* invalid address exception */
		*v = 0;
	}

	printf("data %x %x\n", (unsigned char)buff[0], (unsigned char)buff[1]);
}


int main(int argc, char **argv)
{
	char buff[] = {'1', '2'};
	ver(1, 2, buff);
	return 0;
}
