include ../../config.mk
LIBS=-lm
HASH_OBJ=rahash.o md4.o md5c.o crc16.o crc32.o sha1.o hash.o hamdist.o entropy.o sha2.o ../w32utils.o
HASH_BIN=rahash${BINSFX}

all: rahash${BINSFX}

rahash${BINSFX}: ${HASH_OBJ}

rahash${BINSFX}:
	${CC} ${CFLAGS} ${LDFLAGS} ${HASH_OBJ} ${LIBS} -o ${HASH_BIN}

clean:
	-rm -rf ${HASH_BIN} ${HASH_OBJ} a.out a.exe
