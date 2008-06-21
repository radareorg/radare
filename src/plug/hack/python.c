/*
 * Copyright (C) 2007, 2008
 *       pancake <youterm.com>
 *
 * radare is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * radare is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with radare; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#define RADARE_MODULE
#include <plugin.h>
#include <main.h>
#include <Python.h>
#include <structmember.h>
extern int radare_plugin_type;
extern struct plugin_hack_t radare_plugin;
static char *(*rs)(const char *cmd) = NULL;

static PyObject * PyRadare_Exec(PyObject *self, PyObject *args)
{
	const char *command;
	int sts;

	if (!	PyArg_Parse(args, "s", &command))
		return NULL;
	//if (!PyArg_ParseTuple(args, "s", &command))
	//    return NULL;
	sts = system(command);
	return Py_BuildValue("i", sts);
}

/*
   static void **PySpam_API;

   static int
   import_radare(void)
   {
   PyObject *module = PyImport_ImportModule("radare");

   if (module != NULL) {
   PyObject *c_api_object = PyObject_GetAttrString(module, "_C_API");
   if (c_api_object == NULL)
   return -1;
   if (PyCObject_Check(c_api_object))
   PySpam_API = (void **)PyCObject_AsVoidPtr(c_api_object);
   Py_DECREF(c_api_object);
   }
   return 0;
   }
 */

typedef struct {
	PyObject_HEAD
		PyObject *first; /* first name */
	PyObject *last;  /* last name */
	int number;
} Radare;

static void Radare_dealloc(Radare* self)
{
	Py_XDECREF(self->first);
	Py_XDECREF(self->last);
	self->ob_type->tp_free((PyObject*)self);
}

static PyObject * Radare_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
	Radare *self;

	self = (Radare *)type->tp_alloc(type, 0);
	if (self != NULL) {
		self->first = PyString_FromString("");
		if (self->first == NULL) {
			Py_DECREF(self);
			return NULL;
		}

		self->last = PyString_FromString("");
		if (self->last == NULL) {
			Py_DECREF(self);
			return NULL;
		}

		self->number = 0;
	}

	return (PyObject *)self;
}

static PyObject * Radare_eval(Radare* self, PyObject *args)
{
	PyObject *result;
	char *cmd = NULL,*cmd2= NULL;
	char str[1024];

	if (!	PyArg_ParseTuple(args, "s", &cmd))
			return NULL;
	PyArg_ParseTuple(args, "s", &cmd2);
	//	if (!PyArg_ParseTuple(args, "s:cmd", cmd)) {
	//		return NULL;
	//	}

	if (rs == NULL)
		return NULL;
	//result = PyString_Format(format, args);
	str[0]='\0';
	if (cmd2)
		sprintf(str, "eval %s = %s", cmd, cmd2);
	else
		sprintf(str, "eval %s", cmd);
printf("Setr(%s)\n", str);
	result = PyString_FromString(rs(str));
	Py_DECREF(args);

	return result;
}

static PyObject * Radare_cmd(Radare* self, PyObject *args)
{
	PyObject *result;
	char *cmd = NULL;

	if (!	PyArg_ParseTuple(args, "s", &cmd))
		return NULL;

	if (rs == NULL)
		return NULL;
	result = PyString_FromString(rs(cmd));

	return result;
}

	static int
Radare_init(Radare *self, PyObject *args, PyObject *kwds)
{
	PyObject *first=NULL, *last=NULL, *tmp;

	static char *kwlist[] = {"first", "last", "number", NULL};

	if (! PyArg_ParseTupleAndKeywords(args, kwds, "|OOi", kwlist,
				&first, &last,
				&self->number))
		return -1;

	if (first) {
		tmp = self->first;
		Py_INCREF(first);
		self->first = first;
		Py_XDECREF(tmp);
	}

	if (last) {
		tmp = self->last;
		Py_INCREF(last);
		self->last = last;
		Py_XDECREF(tmp);
	}

	return 0;
}

static PyMemberDef Radare_members[] = {
	{"first", T_OBJECT_EX, offsetof(Radare, first), 0,
		"first name"},
	{"last", T_OBJECT_EX, offsetof(Radare, last), 0,
		"last name"},
	{"number", T_INT, offsetof(Radare, number), 0,
		"noddy number"},
	{NULL}  /* Sentinel */
};

static PyMethodDef Radare_methods[] = {
	{"eval", (PyCFunction)Radare_eval, METH_VARARGS,
		"Return the evaluation of a configuration variable string"
	},
	{"cmd", (PyCFunction)Radare_cmd, METH_VARARGS,
		"Executes a radare command and returns a string"
	},
	{NULL}  /* Sentinel */
};


	static PyTypeObject RadareType = {
		PyObject_HEAD_INIT(NULL)
			0,                         /*ob_size*/
		"radare.Radare",             /*tp_name*/
		sizeof(Radare),             /*tp_basicsize*/
		0,                         /*tp_itemsize*/
		(destructor)Radare_dealloc, /*tp_dealloc*/
		0,                         /*tp_print*/
		0,                         /*tp_getattr*/
		0,                         /*tp_setattr*/
		0,                         /*tp_compare*/
		0,                         /*tp_repr*/
		0,                         /*tp_as_number*/
		0,                         /*tp_as_sequence*/
		0,                         /*tp_as_mapping*/
		0,                         /*tp_hash */
		0,                         /*tp_call*/
		0,                         /*tp_str*/
		0,                         /*tp_getattro*/
		0,                         /*tp_setattro*/
		0,                         /*tp_as_buffer*/
		Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
		"Radare objects",           /* tp_doc */
		0,                         /* tp_traverse */
		0,                         /* tp_clear */
		0,                         /* tp_richcompare */
		0,                         /* tp_weaklistoffset */
		0,                         /* tp_iter */
		0,                         /* tp_iternext */
		Radare_methods,             /* tp_methods */
		Radare_members,             /* tp_members */
		0,                         /* tp_getset */
		0,                         /* tp_base */
		0,                         /* tp_dict */
		0,                         /* tp_descr_get */
		0,                         /* tp_descr_set */
		0,                         /* tp_dictoffset */
		(initproc)Radare_init,      /* tp_init */
		0,                         /* tp_alloc */
		Radare_new,                 /* tp_new */
	};

#if 0
static PyMethodDef module_methods[] = {
	{NULL}  /* Sentinel */
};
#endif

void init_radare_module(void)
{
	PyObject* m;

	if (PyType_Ready(&RadareType) < 0)
		return;

	m = Py_InitModule3("r", Radare_methods, //module_methods,
			"Example module that creates an extension type.");

#if 0
	if (m == NULL)
		return;

	Py_INCREF(&RadareType);
	PyModule_AddObject(m, "Radare", (PyObject *)&RadareType);
#endif
}

int epython_init()
{
	Py_Initialize();
	init_radare_module();
	//Py_InitModule3("radare", Radare_methods, NULL);
	PyRun_SimpleString("import r");
	return 0;
}

int epython_destroy()
{
	// do nothing with the snake
	return 0;
}

void python_hack_cmd(const char *input)
{
	if (rs == NULL)
		rs = radare_plugin.resolve("radare_cmd_str");

	if (rs == NULL) {
		printf("cannot find radare_cmd_str\n");
		return;
	}
	epython_init();
	Py_Initialize();
	init_radare_module();
	PyRun_SimpleString("import r");

	if (input && input[0]) {
		FILE *fd  = fopen(input, "r");
		if (fd == NULL) {
			fprintf(stderr, "Cannot open '%s'\n", input);
			fflush(stdout);
		} else {
			PyRun_SimpleFile(fd, input);
			fclose(fd);
		}
	} else {
		char str[1024];
		while(!feof(stdin)) {
			printf("python> ");
			fflush(stdout);
			str[0]='\0';
			fgets(str,1000,stdin);
			if (str[0]=='.'||feof(stdin)||!memcmp(str,"exit",4)||!memcmp(str,"quit",4)||!strcmp(str,"q"))
				break;
			str[strlen(str)]='\0';
			PyRun_SimpleString(str);
		}
	}
	epython_destroy();
}

int radare_plugin_type = PLUGIN_TYPE_HACK;
struct plugin_hack_t radare_plugin = {
	.name = "python",
	.desc = "python plugin",
	.callback = &python_hack_cmd
};
