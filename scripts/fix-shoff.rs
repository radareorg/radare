; radare script to fix elf.shoff
; author: pancake -- nopcode.org @ 2009
;
; Usage example:
;  cp /usr/bin/gedit gedit
;  # TRASH ELF SHT PTR
;  echo wx 99 @ 0x21 | radare -nw gedit
;  # OBJDUMP/GDB/LTRACE/... CANNOT DO ANYTHING
;  objdump -d gedit  # objdump: gedit: File format not recognized
;  # FIX ELF SHT PTR
;  echo ".(fix-sht) && q" | radare -i scripts/fix-shoff.rs -nw gedit
;  # TRY OBJDUMP AGAIN :)

(fix-sht
  s 0
  s/ .text
 loop:
  s/x 00
  ? [1:$$+1]
  ?!.loop:
  s +4-$$%4
  f nsht
  wv nsht @ 0x20
)
