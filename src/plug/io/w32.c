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

#include <windows.h>
#include "../main.h"
#include "../plugin.h"

static HANDLE hFile = NULL;
static int w32_fd = -1;
#define W32_FD 0x29a29a

static void DisplayError(char *pszAPI)
{
#if 0
// only for debugging
	LPVOID lpvMessageBuffer;
	CHAR szPrintBuffer[512];
	DWORD nCharsWritten;

	if (GetLastError() == 0)
		return;

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpvMessageBuffer, 0, NULL);

	wsprintf(szPrintBuffer,
		"ERROR: API    = %s.\n   error code = %d.\n   message    = %s.\n",
		pszAPI, GetLastError(), (char *)lpvMessageBuffer);

	WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE),szPrintBuffer,
		lstrlen(szPrintBuffer),&nCharsWritten,NULL);

	LocalFree(lpvMessageBuffer);
	//ExitProcess(GetLastError());
#endif
}


int w32_handle_fd(int fd)
{
	return fd == w32_fd;
}

int w32_handle_open(const char *file)
{
	if (!memcmp(file, "w32://", 6))
		return 1;
	return 0;
}

ssize_t w32_write(int fd, const void *buf, size_t count)
{
	int written;

	if (fd != w32_fd) {
		return -1;
	}

	WriteFile(hFile, buf, (DWORD)(count), &written, NULL);
	DisplayError("WriteFile");

        return written;
}

ssize_t w32_read(int fd, void *buf, size_t count)
{
	DWORD ret, foo;

	if (fd != w32_fd) {
		return -1;
	}

	foo = ReadFile(hFile, buf, count, &ret, NULL);
	DisplayError("ReadFile");

	if (!foo)
		return -1;
        return (ssize_t) ret;
}

int w32_open(const char *pathname, int flags, mode_t mode)
{
	hFile = CreateFile(pathname+6,
		GENERIC_READ|((flags==O_RDWR)?GENERIC_WRITE:0),
		FILE_SHARE_READ|((flags==O_RDWR)?FILE_SHARE_WRITE:0),
		NULL, OPEN_ALWAYS, 0, NULL);
	DisplayError("CreateFile");
	w32_fd = W32_FD;

	if (hFile == INVALID_HANDLE_VALUE) {
		// XXX must backup old hFile if the open failes
		w32_fd = -1;
	}

	return w32_fd;
}

int w32_close(int fd)
{
	if (fd == w32_fd) {
		CloseHandle(hFile);
		DisplayError("CloseHandle");
		w32_fd = -1;
	}

	return close(fd);
}

u64 w32_lseek(int fildes, u64 offset, int whence)
{
	int mode = 0;
	u64 off;

	if (fildes == w32_fd) {
		switch(whence) {
		case SEEK_SET:
			mode = FILE_BEGIN;
			break;
		case SEEK_CUR:
			mode = FILE_CURRENT;
			break;
		case SEEK_END:
			mode = FILE_END;
			break;
		}
		// this is 32 bits only!!!, the 0 is for the higher bits
		// the return is also 32 bits
#if 0
http://209.85.135.104/search?q=cache:AkdBtlI5sT8J:msdn2.microsoft.com/en-us/library/aa363858.aspx+CreateFile+msdn&hl=en&ct=clnk&cd=1

DWORD WINAPI SetFilePointer(
  __in          HANDLE hFile,
  __in          LONG  lDistanceToMove,
  __in_out_opt  PLONG lpDistanceToMoveHigh,
  __in          DWORD dwMoveMethod
);

#endif
		off = SetFilePointer(hFile, (LONG) offset, 0, mode);
		DisplayError("SetFilePointer");
		return off;
	}

	return lseek(fildes, offset, whence);
}

plugin_t w32_plugin = {
	.name = "w32",
	.desc = "w32 IO api wrapper ( w32:// )",
	.init = NULL,
	.debug = NULL,
	.system = NULL,
	.handle_fd = w32_handle_fd,
	.handle_open = w32_handle_open,
	.open = w32_open,
	.read = w32_read,
	.write = w32_write,
	.lseek = w32_lseek,
	.close = w32_close
};
