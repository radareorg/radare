/*
 * Copyright (C) 2007, 2008
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

int rabin_id()
{
  char buf[1024];

  fprintf(stderr, "Automagically identifying file...\n");

  snprintf(buf, 1022, ".!!rabin -rIe '%s'", config.file);
  radare_cmd_raw(buf, 0);
  D {
    snprintf(buf, 1022, "!!rabin -Iv '%s'", config.file);
    radare_cmd_raw(buf, 0);
  }

  return 0;
}

int rabin_flag()
{
  char buf[1024];

  D fprintf(stderr, "Automagically flagging file...\n");
      snprintf(buf, 1022, ".!!rabin -rSisz '%s'", config.file);
  radare_cmd_raw(buf, 0);

  /* TODO: move into rabin dietjava */
  if (!strcmp("java", config_get("asm.arch")))
    java_classdump(config.file);

  return 0;
}
