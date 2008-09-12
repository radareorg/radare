include ../../config.mk
LDFLAGS=-lm
HASH_OBJ=rahash.o md4.o md5c.o crc16.o crc32.o sha1.o hash.o hamdist.o entropy.o sha2.o ../w32utils.o
HASH_BIN=rahash${BINSFX}

all: ${HASH_OBJ} rahash${BINSFX}

rahash${BINSFX}:
	${CC} ${CFLAGS} ${LDFLAGS} -o ${HASH_BIN} ${HASH_OBJ} 

clean:
	-rm -rf ${HASH_BIN} ${HASH_OBJ} a.out a.exe
