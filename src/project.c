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
#include "code.h"
#include "flags.h"
#include "radare.h"

int project_save(const char *file)
{
	char buf[128];
	FILE *fd;
	flag_t *flag;
	char *rdbdir;
	int i, lfs;
	
	if (strnull(file))
		return 0;

	rdbdir = config_get("dir.project");
	if (rdbdir&&rdbdir[0]) {
		util_mkdir(rdbdir);
		chdir(rdbdir);
	}

	// TODO: check if exists
	// append .rdb if not defined ??
	fd = fopen(file, "w");

	if (fd == NULL) {
		eprintf("Cannot open '%s' for writing\n", file);
		return 0;
	}

	fprintf(fd, ";RP; Radare Project\n");
	// TODO: store timestamp
	// TODO: all this stuff must be renamed as eval vars or so
	fprintf(fd, "; file = %s\n", config.file);
	getHTTPDate(buf);
	fprintf(fd, "; timestamp = %s\n", buf);
	fprintf(fd, "; eval child.chdir=%s\n", config_get("child.chdir"));
	fprintf(fd, "; eval child.chroot=%s\n", config_get("child.chroot"));
	fprintf(fd, "; eval child.setuid=%s\n", config_get("child.setuid"));
	fprintf(fd, "; eval child.setgid=%s\n", config_get("child.setgid"));
	fprintf(fd, "eval file.scrfilter = %s\n", config_get("file.scrfilter"));
	fprintf(fd, "eval asm.arch = %s\n", config_get("asm.arch"));
	fprintf(fd, "eval dbg.bep = %s\n", config_get("dbg.bep"));
	fprintf(fd, "eval cfg.bigendian = %s\n", config_get("cfg.bigendian"));
	fprintf(fd, "; Flags\n");
	lfs = -1;
        for (i=0;(flag = flag_get_i(i)); i++) {
		if (flag->space != -1 && lfs != flag->space) {
			fprintf(fd, "fs %s\n", flag_space_get(flag->space));
			lfs = flag->space;
		}
		fprintf(fd, "f %s @ 0x%llx\n", flag->name, flag->offset);
	}
#if 0
	fprintf(fd, "# Breakpoints\n");
	fprintf(fd, "; Comments\n");
	list_for_each_prev(pos, &comments) {
		struct comment_t *cmt = list_entry(pos, struct comment_t, list);
		fprintf(fd, "CC %s @ 0x%08llx\n", cmt->comment, (u64)cmt->offset);
	}
#endif
	cons_set_fd(fileno(fd));
		data_comment_list();
		data_xrefs_list();
		data_list();
		section_list(config.seek, 1);
	cons_flush();
	cons_set_fd(_print_fd);

	fclose(fd);

	config_set("file.project", strdup(file));
	cons_printf("Project '%s' saved.\n", file);

	return 1;
}

/* TODO move outside here */
void radare_changes_close()
{
	int changes;
	if (config.unksize||config.debug)
		return;
	changes = undo_write_size();
	if (changes > 0) {
		radare_cmd_raw("u", 0);
		if (yesno('y', "Do you want save these changes? (Y/n)")) {
			/* done */
		} else {
			D eprintf("Dropping changes ..\n");
			radare_cmd_raw("ua", 0);
		}
	}
	cons_flush();
}

void project_close()
{
	const char *file = config_get("file.project");
	if (!strnull(file)) {
		if (yesno('y', "Do you want to save the '%s' project? (Y/n) ", file)) {
			project_save(file);
		} else
			config_set("file.project", "");
	}
	cons_flush();
}

FILE *project_open_fd(const char *file, const char *mode)
{
	const char *rdbdir;

	rdbdir = config_get("dir.project");
	if (rdbdir && rdbdir[0]) {
		util_mkdir(rdbdir);
		chdir(rdbdir);
	}

	return fopen(file, mode);
}

static const char tmp_buf[128];
const char *project_get_file(const char *file)
{
	int len;
	char buf[1024];
	FILE *fd = project_open_fd(file,"r");

	if (fd == NULL)
		return nullstr;

	while(!feof(fd)) {
		fgets(buf, 1023, fd);
		if (feof(fd)) break;
		len = strlen(buf)-1;
		if (buf[len] == '\n' || buf[len] == '\r')
			buf[len]='\0';
		if (!memcmp(buf, "; file = ", 9)) {
			strncpy(tmp_buf,buf+9, 1023);
			break;
		}
	}

	fclose(fd);
	return tmp_buf;
}

int project_open(const char *file)
{
	FILE *fd;
	char buf[1025];
	int len;

	if (strnull(file))
		return 0;

	fd = project_open_fd(file, "r");
	if (fd == NULL) {
		eprintf("Cannot open project '%s'\n", file);
		return 0;
	}

	/* clear stuff */
	flag_clear("*");
	fgets(buf, 1024, fd);

	if (memcmp(buf, ";RP;", 4)) {
		eprintf("Invalid magic.\n");
		return 0;
	}

	for(;;) {
		fgets(buf, 1024, fd);
		if (feof(fd)) break;
		len = strlen(buf)-1;
		if (buf[len] == '\n' || buf[len] == '\r')
			buf[len]='\0';

		if (!memcmp(buf, "; file = ", 9)) {
			/* TODO open file now! */
		}

		if (buf[0]!=';')
			radare_cmd(buf, 0);
	}
	fclose(fd);
	eprintf("Project '%s' loaded\n", file);

	config_set("file.project", file);

	return 1;
}

int project_info(const char *file)
{
	char *targetfile;

	if (file == NULL || file[0]=='\0')
		file = config_get("file.project");

	if (strnull(file)) {
		eprintf("No project opened\n");
		return 0;
	}
	targetfile = project_get_file(file);

	cons_printf("e file.project = %s\n", file);
	cons_printf("e dir.project = %s\n", config_get("dir.project"));
	cons_printf("; file = %s\n", targetfile);

	return 1;
}
