-- stable

create table blocks (
	address INTEGER,
	end_address INTEGER,
	bytes BLOB,
	hash  VARCHAR(32)
);

create table xrefs (
	address INTEGER,
	xref    INTEGER
);

create table comments (
	time    DATE,
	address INTEGER,
	string  VARCHAR(100)
);

create table labels (
	time    DATE,
	address INTEGER,
	string  VARCHAR(100)
);

create table syscalls (
	address INTEGER,
	block   INTEGER, -- unique id here
	string  VARCHAR(20)
);

-- development

create table trace (
	time    DATE,
	address INTEGER
);

create table entrys (
	address INTEGER
);

create table breakpoints (
	time    DATE,
	address INTEGER,
	enabled BOOLEAN
);

create table dwarf (
	time    DATE,
	address INTEGER,
	string  VARCHAR(100)
);
