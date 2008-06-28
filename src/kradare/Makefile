obj-m := kradare.o

all:
	make -C /usr/src/linux SUBDIRS=$$PWD modules 

install:
	rmmod -f kradare.ko
	insmod kradare.ko

clean:
	rm -rf kradare.mod.c .kradare.mod.o.cmd kradare.mod.o .tmp_versions  .kradare.o.cmd kradare.o .kradare.ko.cmd kradare.ko


