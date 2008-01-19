/*
 * Copyright (C) 2008
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

#include "main.h"
#include "flags.h"

int project_save(char *file)
{
	FILE *fd;
	struct list_head *pos;
	rad_flag_t *flag;
	int i;
	
	if (strnull(file))
		return 0;

	// TODO: check if exists
	// append .rdb if not defined ??
	fd = fopen(file, "w");

	if (fd == NULL) {
		eprintf("Cannot open '%s' for writing\n", file);
		return 0;
	}

	fprintf(fd, "#RP# Radare Project\n#\n");
	// TODO: store timestamp
	// TODO: all this stuff must be renamed as eval vars or so
	fprintf(fd, "chdir=%s\n", config_get("child.chdir"));
	fprintf(fd, "chroot=%s\n", config_get("child.chroot"));
	fprintf(fd, "setuid=%s\n", config_get("child.setuid"));
	fprintf(fd, "setgid=%s\n", config_get("child.setgid"));
	fprintf(fd, "# Flags\n");
        for (i=0;(flag = flag_get_i(i)); i++)
		fprintf(fd, "flag=0x"OFF_FMTx" %s\n", flag->offset, flag->name);
#if 0
	fprintf(fd, "# Breakpoints\n");
#endif
	fprintf(fd, "# Comments\n");
	list_for_each_prev(pos, &comments) {
		struct comment_t *cmt = list_entry(pos, struct comment_t, list);
		fprintf(fd, "comment=0x"OFF_FMTx" %s\n", cmt->offset, cmt->comment);
	}

	eprintf("Project '%s' saved.\n", file);
	fclose(fd);
	return 1;
}

int project_open(char *file)
{
	FILE *fd;
	char buf[1025];
	int len;
	char *ptr, *ptr2;

	if (strnull(file))
		return 0;

	fd = fopen(file, "r");
	if (fd == NULL) {
		eprintf("Cannot open project '%s'\n", file);
		return 0;
	}

	/* clear stuff */
	flag_clear("*");
	fgets(buf, 1024, fd);

	if (memcmp(buf, "#RP#", 4)) {
		eprintf("Invalid magic.\n");
		return 0;
	}

	for(;;) {
		fgets(buf, 1024, fd);
		if (feof(fd)) break;
		if (buf[0]=='#') continue;
		len = strlen(buf)-1;
		if (buf[len] == '\n' || buf[len] == '\r')
			buf[len]='\0';

		ptr = strchr(buf, '=');
		if (!ptr) continue;
		ptr[0]='\0'; ptr = ptr + 1;
		if (!strcmp(buf, "chdir")) {
			config_set("child.chdir", ptr);
		} else
		if (!strcmp(buf, "chroot")) {
			config_set("child.chroot", ptr);
		} else
		if (!strcmp(buf, "setuid")) {
			config_set("child.setuid", ptr);
		} else
		if (!strcmp(buf, "setgid")) {
			config_set("child.setgid", ptr);
		} else
		if (!strcmp(buf, "flag")) {
			ptr2=strchr(ptr, ' ');
			if (ptr2) {
				ptr2 = ptr2 + 1;
				flag_set(ptr2, get_offset(ptr), 1);
			}
		} else
		if (!strcmp(buf, "comment")) {
			ptr2=strchr(ptr, ' ');
			if (ptr2) {
				ptr2 = ptr2 + 1;
				metadata_comment_add(get_offset(ptr), ptr2);
			}
		} 
	}
	fclose(fd);
// TODO show statistics: N comments, symbols, labels, flags, etc...
	eprintf("Project '%s' loaded\n", file);

	return 1;
}

int project_info(char *file)
{
	eprintf("project file: %s\n", file);
}
