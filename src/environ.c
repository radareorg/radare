/*
 * Copyright (C) 2006, 2007
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
#include "utils.h"
#include "dbg/arch/arch.h"

/* spaghetti implementation */
int var_required(char *str, char *var)
{
	char *a = str, *b;

	while(*a) {
		if (*a == '$') {
			a = a + 1;
			if (*a == '{')
				a = a + 1;
			b = a;
			while(*b) {
				b = b + 1;
				if (*b == '}' ||  *b == ' ')
					break;
			}
			if (!memcmp(var, a, b-a))
				return 1;
		}
		a = a + 1;
	}

	return 0;
}

void prepare_environment(char *line)
{
	int i;
	char *offset = (char *)malloc((config.block_size*3)+1);

	/* to think.. +/
	setenv("SYNTAX", config_get("asm.syntax"));
	*/

	setenv("ARCH",   config_get("asm.arch"), 1);
	setenv("NASM",   "1",      1);
	sprintf(offset,  OFF_FMTd, (off_t)config.seek);
	setenv("OFFSET", offset,   1);
	if (config.cursor_mode)
		sprintf(offset,  OFF_FMTd, (off_t)config.seek+config.cursor);
	setenv("CURSOR", offset,   1);
	sprintf(offset,  "%d", (int)config_get_i("file.baddr")); //(off_t)config.baddr);
	setenv("BADDR",  offset,   1);
	sprintf(offset,  "%d",     config.color);
	setenv("COLOR",  offset,   1);
	sprintf(offset,  "%d", config.verbose);
	setenv("VERBOSE",offset,   1);
	setenv("FILE",   config.file, 1);
	sprintf(offset,  OFF_FMTd, config.size);
	setenv("SIZE",   offset,   1);
	sprintf(offset,  "%s", (config.endian)?"big":"little");
	setenv("ENDIAN", offset, 1);
	sprintf(offset,  "%d", config.block_size);
	setenv("BSIZE",  offset, 1);

	if (var_required(line, "BYTES")) {
		*offset = '\0';
		for(i=0;i<config.block_size;i++) {
			char str[128];
			sprintf(str, "%02x ", config.block[i]);
			strcat(offset, str);
		}
		setenv("BYTES",  offset, 1);
	}

	if (var_required(line, "BLOCK")) {
		char file[TMPFILE_MAX];
		make_tmp_file(file);
		CHECK_TMP_FILE(file);
		radare_dump(file, config.block_size);
		setenv("BLOCK", file, 1);
	}
	update_environment();

	free(offset);
}

// ugly hack for usability
void init_environment()
{
	setenv("VISUAL", "0", 1);
	prepare_environment("");
}

void destroy_environment(char *line)
{
	char *file = getenv("BLOCK");

	if (var_required(line, "BLOCK")) {
		file = getenv("BLOCK");
		if (file) {
			unlink(file);
			unsetenv("BLOCK");
		}
	}
}

void update_environment()
{
	char *ptr;

	arch_set_callbacks();

	config.verbose = config_get("cfg.verbose");

	ptr = getenv("COLUMNS");
	if (ptr != NULL)
		config.width = atoi(ptr);
	else
		config.width = terminal_get_columns();
	if (config.width<10)config.width=10;
	config_set_i("scr.width", config.width);

	config.color = config_get("cfg.color");
}