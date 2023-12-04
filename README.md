[![ci](https://github.com/radareorg/radare/actions/workflows/ci.yml/badge.svg)](https://github.com/radareorg/radare/actions/workflows/ci.yml)

# WARNING

This version is the original code being refactored into [radare2](https://github.com/radareorg/radare2)

```

                  __--~.
               .-'_ ,' |
            .'     \   |
           / | |>   ) /
           \ |     /." _   _   _   _   _
         .-^_| |\  \  |_| | \ |_| |_/ /_
         \/   -| '. ' | | |_/ | | | \ \_

                            pwn them all
```

## SHORT

  radare is a commandline hexadecimal editor.


## DESCRIPTION

  Radare is a toolkit framework for working with binary files having the
  unix philosphy in mind.

  It was born with simplicity in mind. The core of it remains on the
  command line hexadecimal editor that it aims to provide a helper
  tool for reverse engineering, exploiting, fuzzing, binary and data analysis.

  hasher is a hashing utility that allows to hash pieces of files and
  generate reports of changes. This is useful for hard disk analysis,
  reversing, binary diffs, system programs integrity, etc.

  radare comes with a set of IO plugins that wraps all open/read/write
  /seek/close/system calls. This way several plugins has been implemented:

    $ radare -L
    haret       Read WCE memory ( haret://host:port )
    debug       Debugs or attach to a process ( dbg://file or pid://PID )
    gdb         Debugs/attach with gdb (gdb://file, gdb://PID, gdb://host:port)
    gdbx        GDB shell interface 'gdbx://program.exe args' )
    shm         shared memory ( shm://key )
    mmap        memory mapped device ( mmap://file )
    malloc      memory allocation ( malloc://size )
    remote      TCP IO ( listen://:port or connect://host:port and rap:// )
    winedbg     Wine Debugger interface ( winedbg://program.exe )
    socket      socket stream access ( socket://host:port )
    serial      serial port access ( serial:///path/to/dev:speed )
    gxemul      GxEmul Debugger interface ( gxemul://program.arm )
    ewf         EnCase EWF file support ( ewf:// )
    posix       plain posix file access

  Currently I'm working on a Vala frontend to provide an object oriented
  api for directly interfacing with the core of radare and provide a
  complete graphical frontend.


FEATURES

   rasc

   - shellcode helper tool
     - generates paddings with A's, nops, CCs and enumerations (00, 01, 02, ..)
     - most common use shellcodes hardcoded inside
       - x86 32/64, arm, powerpc
       - linux, bsd, solaris, darwin, w32
     - HOST, PORT, CMD shellcode alteration
     - syscall proxy server
   - output in
     - raw
     - hexpairs
     - C unsigned char array
     - execute shellcode (for local testing)


   rasm

   - commandline assembler/disassembler
     - supports x86, olly, java, powerpc...
     - can support any other arch supported by GAS using 'rsc asm'
     - assembling from text file to raw object
     - you can specify the offset and endian manually
   - can disassemble from an hexpair string


   rabin

    - show information about ELF/PE/MZ/CLASS files
      - entrypoint
      - imports
      - exports
      - symbols
      - libraries
      - sections
      - checksums
    - integration with radare core

   rahash

    - generates and checks block checksums of a file
    - can hash from stdin
    - supports multiple hashing algorithms
      - md4, md5
      - crc16, crc32
      - sha1
      - par, xor, xorpair
      - hamdist
      - entropy


   radiff 

    - integrates bdiff, bindiff, bytediff and rdbdiff into a single program
    - generates a raw binary difference between two files
    - recommended to use together with bdcolor
      $ bindiff a b | bdcolor 3


   rsc

    - contains helper scripts for working with binary files
    - asm/dasm - asm->hexpairs / hexpairs->asm
    - bdcolor  - colorizes and filters the bindiff output
    - bytediff - byte per byte binary diff (faster than bindiff, less accurate)
    - spcc     - structure parser c compiler
    - rfile-foreach - find magic signatures on a raw file
    - adict    - assembly dictionary
    - bin2txt  - generates a template text file with info and disasembly of a binary
    - bin2tab | tab2gml - generate a graphml with the program reference calls
    - syms-xrefs - find crossed references to each symbol of a binary
    ... and much more ...


   radare

   - cli and visual modes
   - yank and paste
   - perl/python scripting support
   - virtual base address for on-disk patching
   - vi-like environment and command repetition (3x)
   - debugger for x86-linux/bsd and arm-linux
   - data bookmarking (flags)
   - scripting (no branches or conditionals yet)
   - own magic database (rfile)
   - little/big endian conversions
   - data search
     - multiple keywords at the same time
     - binary masks
     - input format: ascii/binary/widechar
     - ranged searches ( /r 0-2 )
   - show xrefs on arm, x86 and ppc binaries
   - data type views:
     - integer/long/longlong/float/double
     - binary dos/unix timestamps
     - octal
     - hexadecimal (byte, word, dword, qword)
   - data block views:
     - hexadecimal
     - octal
     - binary
     - ascii string
     - widechar strings
     - url encoding
     - shellcode (gas format)
     - c char* array
     - analysis (pA 30 @ esp)
     - disassembly
       - native x86/arm
       - allows to use objdump or any other external command
       - pseudocode syntax
       - colorization
       - inline comments and labels
   - visual mode commands
     - gotoxy command to create your own composed views
     - screen fit
     - colorization of almost everything
     - on the fly disassembly
     - apply magic on current block
     - hjkl-style scrolling
     - print format cycling
     - cursor mode and yank/paste feature


  debugger

  - support for x86-linux/bsd and arm-linux
  - next planed ports: x86.64-linux, w32 and x86.solaris
  - tracing with multiple verbose levels
  - dump/restore of the process
  - full filedescriptor control
  - backtrace viewer
  - attach/detach
  - code injection
  - breakpoints (software, hardware)
  - watchpoints (software, hardware)
    - raw drx control
  - full gpregs and fpregs control
  - continue until user code
  - step into, step over
  - continue until syscall (strace like)
  - supports ktrace on openbsd and netbsd
  - raw memory management of the target process
  - common hacks can be automatized with keybindings
  - full signalling control of the target process
  - /proc/pid/maps info support
  - ollydbg-like keybindings (F7 step , F9 continue, ...)



                       -- Dedicated to the girl I have loved, hated and loved.

AUTHOR

  pancake <@youterm.com>
