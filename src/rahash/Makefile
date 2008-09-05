include ../../config.mk
LDFLAGS=-lm
HASH_OBJ=hasher.o md4.o md5c.o crc16.o crc32.o sha1.o hash.o hamdist.o entropy.o sha2.o ../w32utils.o
HASH_BIN=hasher${BINSFX}

all: ${HASH_OBJ} hasher${BINSFX}

hasher${BINSFX}:
	${CC} ${CFLAGS} ${LDFLAGS} -o ${HASH_BIN} ${HASH_OBJ} 

clean:
	-rm -rf ${HASH_BIN} ${HASH_OBJ} a.out a.exe
