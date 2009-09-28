all: Display SetRegister EntryDialog SeekTo

Display:
	${CC} `pkg-config --libs --cflags gtk+-2.0` Display.c -o Display

SetRegister:
	${CC} `pkg-config --libs --cflags gtk+-2.0` SetRegister.c -o SetRegister

EntryDialog:
	${CC} `pkg-config --libs --cflags gtk+-2.0` EntryDialog.c -o EntryDialog

SeekTo:
	${CC} `pkg-config --libs --cflags gtk+-2.0` SeekTo.c -o SeekTo

gtkaml:
	gtkamlc -C --pkg posix --pkg gtk+-2.0 Display.gtkaml
	gtkamlc -C --pkg gtk+-2.0 SetRegister.gtkaml
	gtkamlc -C --pkg gtk+-2.0 EntryDialog.gtkaml
	gtkamlc -C --pkg gtk+-2.0 SeekTo.gtkaml

clean:
	rm -rf Display EntryDialog SetRegister SeekTo
