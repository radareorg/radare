int main()
{
	int a=0;
	if (a==0x55) {
		printf("666\n");
patata:
		printf("777\n");
	} else {
		printf("B\n");
		goto patata;
	}
	return 0;
}
