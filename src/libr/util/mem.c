/*
 * Copyright (C) 2008
 *       pancake <@youterm.com>
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

#include "r_util.h"
#include <stdlib.h>

char *estrdup(char *ptr, const char *string)
{
        if (ptr)
                free(ptr);
        ptr = strdup(string);
        return ptr;
}

void efree(void **ptr)
{
        free (*ptr);
        *ptr = NULL;
}

void memcpy_loop(u8 *dest, u8 *orig, int dsize, int osize)
{
        int i=0,j;
        while(i<dsize)
                for(j=0;j<osize && i<dsize;j++)
                        dest[i++] = orig[j];
}

void endian_memcpy_e(u8 *dest, u8 *orig, int size, int endian)
{
        if (endian) {
                memcpy(dest, orig, size);
        } else {
                unsigned char buffer[8];
                switch(size) {
                case 2:
                        buffer[0] = orig[0];
                        dest[0]   = orig[1];
                        dest[1]   = buffer[0];
                        break;
                case 4:
                        memcpy(buffer, orig, 4);
                        dest[0] = buffer[3];
                        dest[1] = buffer[2];
                        dest[2] = buffer[1];
                        dest[3] = buffer[0];
                        break;
                case 8:
                        memcpy(buffer, orig, 8);
                        dest[0] = buffer[7];
                        dest[1] = buffer[6];
                        dest[2] = buffer[5];
                        dest[3] = buffer[4];
                        dest[4] = buffer[3];
                        dest[5] = buffer[2];
                        dest[6] = buffer[1];
                        dest[7] = buffer[0];
                        break;
                default:
                        printf("Invalid size: %d\n", size);
                }
        }
}
