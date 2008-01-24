include ../config.mk

all:
	${E} 'Compiling java code'
	${Q}${JAVAC} org/nopcode/pancake/radare/*.java
	${Q}${JAVAC} org/nopcode/pancake/radare/data/*.java
	${Q}${JAVAC} org/nopcode/pancake/radare/demos/*.java
	${Q}${JAVAC} org/nopcode/pancake/radare/swing/*.java

jar:
	if [ ! -e org/nopcode/pancake/radare/Radare.class ]; then\
		echo "Java api not compiled.";\
		exit 1;\
	fi
	jar cmf ./META-INF/MANIFEST.MF ./radare.jar `find org | grep class`
	
install:
	mkdir -p ${DATADIR}/radare
	${INSTALL_DATA} radare.jar ${DATADIR}/radare/

clean:
	${E} 'Cleaning java'
	-${Q}rm -rf `find . | grep class` xxx

run:
	#java org.nopcode.pancake.radare.demos.Disassembler /bin/ls
	java org.nopcode.pancake.radare.demos.Find /bin/ls lib
