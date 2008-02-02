// WII TODO WII TODO WII TODO WII TODO WII TODO //

/* piece of code running on the wii */
int wii_handle_client( int fd ){
	uchar buf[1024];
	uchar cmd;
	uchar flg;
	uchar *ptr;
	int i, c;

	if (fd == -1) {
		printf("Cannot listen.\n");
		return -1;
	}

	while ((c = accept(fd, NULL, NULL))) {
		if (c == -1) {
			printf("Cannot accept\n");
			close(c);
			return -1;
		}

		printf("Client connected\n");
		
		while(1) {
			i = read(c, &cmd, 1);
			if (i==0) {
				printf("Broken pipe\n");
				return -1;
			}

			switch((unsigned char)cmd) {
			case RMT_OPEN:
				read(c, &flg, 1); // flags
				printf("open (%d): ", cmd); fflush(stdout);
				read(c, &cmd, 1); // len
				ptr = malloc(cmd);
				read(c, ptr, cmd); //filename
				printf("(flags: %hhd) len: %hhd filename: '%s'\n",
					flg, cmd, ptr); fflush(stdout);
				buf[0] = RMT_OPEN | RMT_REPLY;
				memcpy(config.file, ptr, cmd);
				config.file[cmd]='\0';
				config.fd = -1;
				radare_open(0);
				endian_memcpy(buf+1, (uchar *)&config.fd, 4);
				write(c, buf, 5);
				free(ptr);
				break;
			case RMT_READ:
				read(c, &buf, 4);
				endian_memcpy((uchar*)&i, buf, 4);
				ptr = (uchar *)malloc(i+5);
				radare_read(0);
				ptr[0] = RMT_READ|RMT_REPLY;
				if (i>config.block_size) i = config.block_size;
				endian_memcpy(ptr+1, (uchar *)&i, 4);
				memcpy(ptr+5, config.block, config.block_size);
				write(c, ptr, i+5);
				break;
			case RMT_WRITE:
				printf("TODO: write\n");
				break;
			case RMT_SEEK:
				read(c, buf, 5);
				endian_memcpy((uchar *)&i, buf+1, 4);
				config.seek = io_lseek(config.fd, i, buf[0]);
				i = (int)config.seek;
				buf[0] = RMT_SEEK | RMT_REPLY;
				endian_memcpy(buf+1, (uchar*)&i, 4);
				write(c, buf, 5);
				break;
			case RMT_CLOSE:
				// XXX : proper shutdown
				printf("TODO: close\n");
				//read(fd, &c, 1);
				close(c);
				break;
			case RMT_SYSTEM:
				// read
				read(c, buf, 4);
				endian_memcpy((uchar*)&i, buf, 4);
				ptr = (uchar *) malloc(i+6);
				ptr[5]='!';
				read(c, ptr+6, i);
				ptr[6+i]='\0';
				update_environment();
				pipe_stdout_to_tmp_file((char*)&buf, (char*)ptr+5);
				{
					FILE *fd = fopen((char*)buf, "r");
					free(ptr); i = 0;
					if (fd == NULL) {
						printf("Cannot open tmpfile\n");
						i = -1;
					} else {
						fseek(fd, 0, SEEK_END);
						i = ftell(fd);
						fseek(fd, 0, SEEK_SET);
						ptr = (uchar *) malloc(i+5);
						fread(ptr+5, i, 1, fd);
						ptr[i+5]='\0';
						fclose(fd);
					}
				}
				unlink((char*)buf);

				// send
				ptr[0] = (RMT_SYSTEM | RMT_REPLY);
				endian_memcpy((uchar*)ptr+1, (uchar*)&i, 4);
				if (i<0)i=0;
				write(c, ptr, i+5);
				printf("REPLY SENT (%d) (%s)\n", i, ptr+5);
				free(ptr);
				break;
			default:
				printf("unknown command 0x%02x\n", cmd);
				close(c);
				return -1;
			}
		}
	}
	return 0;
}
