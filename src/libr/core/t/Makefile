OBJ=radare2.o
BIN=radare2

## shared
LIBS=../*.o \
  -Wl,-R../../config/ \
  -Wl,-R../../cons/ \
  -Wl,-R../../line/ \
  -Wl,-R../../io/ \
  -Wl,-R../../cmd/ \
  -Wl,-R../../util/ \
  -Wl,-R../../print/ \
  -Wl,-R../../flags/ \
  -Wl,-R../../asm/ \
  -Wl,-R../../lib/ \
  -Wl,-R../../macro/ \
  -Wl,-R../../debug/ \
  -Wl,-R../../hash/ \
  -Wl,-R../../bin/ \
  -Wl,-R../ \
  -L../../config/ \
  -L../../cons/ \
  -L../../line/ \
  -L../../io/ \
  -L../../cmd/ \
  -L../../util/ \
  -L../../print/ \
  -L../../flags/ \
  -L../../asm/ \
  -L../../lib/ \
  -L../../macro/ \
  -L../../debug/ \
  -L../../hash/ \
  -L../../bin/ \
  -L../ \
  -lr_core -lr_config -lr_cons -lr_line -lr_cmd -lr_util -lr_print -lr_flags -lr_asm -lr_lib -lr_macro -lr_debug -lr_bin -lr_io -lr_hash
LIBS+=-ldl
## static
#LIBS=../*.o \
#  ../../config/*.a \
#  ../../cons/*.a \
#  ../../line/*.a \
#  ../../io/*.a \
#  ../../cmd/*.a \
#  ../../util/*.a \
#  ../../print/*.a \
#  ../../flags/*.a \
#  ../../asm/*.a \
#  ../../lib/*.a \
#  ../../macro/*.a \
#  ../../debug/*.a \
#  ../../hash/*.a
#LIBS+=-ldl -static

include ../../tests.mk
