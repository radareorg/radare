/*
 * Copyright (C) 2007
 *       pancake <youterm.com>
 *
 * radare is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * radare is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with radare; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "../main.h"
#include <libewf.h>
#include "../plugin.h"

#define EWF_FD 0x19b19b
static int ewf_fd = -1;
LIBEWF_HANDLE *ewf_h = NULL;

int ewf_handle_fd(int fd)
{
	return fd == ewf_fd;
}

int ewf_handle_open(const char *file)
{
	if ((!memcmp(file, "ewf://", 6))
	||  (!memcmp(file, "els://", 6)))
		return 1;
	return 0;
}

ssize_t ewf_write(int fd, const void *buf, size_t count)
{
	return libewf_write_buffer(ewf_h, buf, count);
}

ssize_t ewf_read(int fd, void *buf, size_t count)
{
	return libewf_read_buffer(ewf_h, buf, count);
}

int ewf_open(const char *pathname, int flags, mode_t mode)
{
	// XXX filename list should be dynamic. 1024 limit is ugly
	const char *filenames[1024];
	char *ptr,*optr;
	char hash[1024];
	int i;

	if (!memcmp(pathname, "els://", 6)) {
		FILE *fd = fopen(pathname+6, "r");
		u64 len;
		char *buf;

		if (fd == NULL)
			return -1;
		fseek(fd, 0, SEEK_END);
		len = ftell(fd);
		fseek(fd, 0, SEEK_SET);
		buf = (char *)malloc(len);
		fread(buf, len, 1, fd);
		
		ptr = strchr(buf, '\n');
		for(i=0,optr = buf;ptr&&(ptr=strchr(ptr, '\n'));optr=ptr) {
			ptr[0] = '\0';
			ptr = ptr + 1;
			filenames[i++] = optr;
		}
		filenames[i] = NULL;

		free(buf);
		fclose(fd);

		for(i=0;filenames[i];i++)
			printf("%02x: %s)\n", i, filenames[i]);
	} else {
		filenames[0] = pathname + 6;
		filenames[1] = NULL;
	}
	
	ewf_h = libewf_open(&filenames, 1, 
		(((int)config_get("cfg.write"))==0)?
		LIBEWF_OPEN_READ_WRITE:LIBEWF_OPEN_READ);


	if (ewf_h == NULL)
		ewf_fd = -1;
	else {
		ewf_fd = EWF_FD;
#if 0
		if( ((libewf_internal_handle_t*)ewf_h)->header_values == NULL ) {
			fprintf( stream, "\tNo information found in file.\n" );
		} else {
			libewf_get_header_value_examiner_name(ewf_h, hash, 128);
			eprintf("ExaminerName:     %s\n", hash);
			libewf_get_header_value_case_number(ewf_h, hash, 128);
			eprintf("CaseNumber:       %s\n", hash);
#endif
			eprintf("FormatVersion:    %d\n", libewf_get_format(ewf_h));
			eprintf("CompressionLevel: %d\n", libewf_get_compression_level(ewf_h));
			eprintf("ErrorGranurality: %d\n", libewf_get_error_granularity(ewf_h));
			eprintf("AmountOfSectors:  %d\n", libewf_get_amount_of_sectors(ewf_h));
			eprintf("BytesPerSector:   %d\n", libewf_get_bytes_per_sector(ewf_h));
			eprintf("VolumeType:       %d\n", libewf_get_volume_type(ewf_h));
			eprintf("MediaSize:        %lld\n", libewf_get_media_size(ewf_h));
			eprintf("MediaType:        %d\n", libewf_get_media_type(ewf_h));
			eprintf("MediaFlags:       %d\n", libewf_get_media_flags(ewf_h));
			libewf_get_stored_md5_hash(ewf_h, hash, 128);
			eprintf("CalculatedHash:   %s\n", hash);
#if 0
		}
#endif
	}

	return ewf_fd;
}

int ewf_close(int fd)
{
	if (fd == ewf_fd) {
		libewf_close(ewf_h);
		ewf_fd = -1;
		return 0;
	}

	return -1;
}

u64 ewf_lseek(int fildes, u64 offset, int whence)
{
	u64 off;

	if (fildes == ewf_fd) {
		switch(whence) {
			case SEEK_SET:
				/* ignore */
				break;
			case SEEK_CUR:
				offset += config.seek;
				break;
			case SEEK_END:
				offset = libewf_get_media_size(ewf_h) - offset;
				break;
		}
		libewf_seek_offset(ewf_h, offset);
		return offset;
	}

	return lseek(fildes, offset, whence);
}

plugin_t ewf_plugin = {
	.name = "ewf",
	.desc = "EnCase EWF file support ( ewf:// )",
	.init = NULL,
	.debug = NULL,
	.system = NULL,
	.handle_fd = ewf_handle_fd,
	.handle_open = ewf_handle_open,
	.open = ewf_open,
	.read = ewf_read,
	.write = ewf_write,
	.lseek = ewf_lseek,
	.close = ewf_close
};
