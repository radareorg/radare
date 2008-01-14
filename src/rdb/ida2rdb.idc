/*
 * ida2rdb.idc
 * ===========
 *
 * Exports an ida database in a format to be handled by radare and rbdiff.
 *
 * author: pancake <@youterm.com>
 *
 * MOARNFO:
 * * http://www.informit.com/articles/article.aspx?p=353553&seqNum=9&rl=1
 *
 * TODO:
 * * Add stack frame related information (stack size, and so) as comments
 *
 */

#include <idc.idc>

static dumpMeNot(fd, ea)
{
	auto func, comment, sz, i, ref;

	// Loop from start to end in the current segment
	//SegStart(ea);
	for (func=ea; func != BADADDR && func < SegEnd(ea); func=NextFunction(func)) 
	{
		// If the current address is function process it
//		if (GetFunctionFlags(func) != -1) {
			fprintf(fd, "label=0x%08lx %s\n", func, GetFunctionName(func));

			comment = GetFunctionCmt(func, 0);
			if (comment != "")
				fprintf(fd, "comment=0x%08x %s\n", func, comment);

			fprintf(fd, "framesize=0x%08x %d\n", func, GetFrameSize(func));
			
			sz = FindFuncEnd(func);
			fprintf(fd, "bytes=0x%08lx ", func);			
			for(i=func;i<sz;i++)
				fprintf(fd, "%02x ", Byte(i));
			fprintf(fd, "\n");

			// Find all code references to func
			for (ref=RfirstB(func); ref != BADADDR; ref=RnextB(func, ref)) {
				//fprintf(fd, "; xref from %08lX (%s)\n", ref, GetFunctionName(ref));
				fprintf(fd, "xref=0x%08lx 0x%08lx\n", func, ref);
			}
//		}
	}

	for (func=ea; func != BADADDR && func < SegEnd(ea); func=func+1)
	{
		comment = CommentEx(func, 0);
		if (comment != "")
			fprintf(fd, "comment=0x%08x %s\n", func, comment);
		comment = GetConstCmt(func, 0);
		if (comment != "")
			fprintf(fd, "comment=0x%08x %s\n", func, comment);
		comment = GetEnumCmt(func, 0);
		if (comment != "")
			fprintf(fd, "comment=0x%08x %s\n", func, comment);
	}

}

static main() {
	auto fd;
	auto file;
	auto i, func, ref,sz;
	auto ord,ea;
	auto comment;
	auto entry;

	file = GetInputFile()+".txt";
	fd = fopen(file, "w");
	if (!fd) {
		Message("Cannot open '"+file+"'\n");
		Exit(1);
	}

	entry="";
	// Walk entrypoints 
	for ( i=0; ; i++ ) {
		ord = GetEntryOrdinal(i);
		if ( ord == 0 ) break;
		ea = GetEntryPoint(ord);
		fprintf(fd, "entry=0x%08lx %s\n", ea, Name(ea));
		entry = ea;
	}

	// XXX last entrypoint taken as ok??
	dumpMeNot(fd, entry);

	// eof 
	fclose(fd);

	Message(file+"file generated.\n");
}

