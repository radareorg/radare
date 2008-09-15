/*
 * Copyright (C) 2007
 *       th0rpe <nopcode.org>
 *
 * radare is part of the radare project
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

#include "libps2fd.h"
#include "wp.h"
#include <string.h>
#include <stdio.h>

int debug_wp_match()
{
	int i, n;

	n = ps.wps_n;
	i = 0;

	while(n > 0) {
		if(ps.wps[i].cond) {
			if(eval_cond(ps.wps[i].cond))
				return i;
			n--;
		}
		i++;
	}

	return -1;
}

int debug_wp_add(const char *str)
{
	int i;

	if(ps.wps_n == (sizeof(ps.wps) / sizeof(WP)))
		return -1;

	for(i = 0; i < sizeof(ps.wps); i++) {
		if(!ps.wps[i].cond) {
			ps.wps[i].cond = (void*) parse_cond(str);
			if(!ps.wps[i].cond) 
				return -2;

			ps.wps[i].str_cond = strdup(str);
			ps.wps_n++;

			break;
		}
	}

	return i;	
}

int debug_wp_rm(int i)
{ 
	if(ps.wps[i].cond == NULL)
		return -1;

	free_cond(ps.wps[i].cond);
        ps.wps[i].cond = NULL;

        if(ps.wps[i].str_cond)
		free(ps.wps[i].str_cond);

        ps.wps_n--;

	return 0;
}

void debug_wp_list()
{
	int i, n = ps.wps_n;

	for(i=0; n>0 ; i++) {
		if(ps.wps[i].cond) {
			printf("%d: %s\n", i, ps.wps[i].str_cond);
			n--;
		}
	}
}
