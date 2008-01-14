/*
 * Copyright (C) 2007
 *       pancake <pancake@youterm.com>
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

#include "main.h"
#if HAVE_PYTHON
#include <Python.h>
#include <structmember.h>
#define RADARE_MODULE

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

static void
Radare_dealloc(Radare* self)
{
    Py_XDECREF(self->first);
    Py_XDECREF(self->last);
    self->ob_type->tp_free((PyObject*)self);
}

static PyObject *
Radare_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    Radare *self;

    self = (Radare *)type->tp_alloc(type, 0);
    if (self != NULL) {
        self->first = PyString_FromString("");
        if (self->first == NULL)
          {
            Py_DECREF(self);
            return NULL;
          }

        self->last = PyString_FromString("");
        if (self->last == NULL)
          {
            Py_DECREF(self);
            return NULL;
          }

        self->number = 0;
    }

    return (PyObject *)self;
}


static PyObject *
Radare_name(Radare* self)
{
    static PyObject *format = NULL;
    PyObject *args, *result;

    if (format == NULL) {
        format = PyString_FromString("%s %s");
        if (format == NULL)
            return NULL;
    }

    if (self->first == NULL) {
        PyErr_SetString(PyExc_AttributeError, "first");
        return NULL;
    }

    if (self->last == NULL) {
        PyErr_SetString(PyExc_AttributeError, "last");
        return NULL;
    }

    args = Py_BuildValue("OO", self->first, self->last);
    if (args == NULL)
        return NULL;

    result = PyString_Format(format, args);
    Py_DECREF(args);

    return result;
}

static PyObject *
Radare_cmd(Radare* self, PyObject *args)
{
    static PyObject *format = NULL;
    PyObject *result;
	char *cmd = NULL;
	PyObject *obj;


if (!	PyArg_ParseTuple(args, "s", &cmd))
	return NULL;
//	if (!PyArg_ParseTuple(args, "s:cmd", cmd)) {
//		return NULL;
//	}

    //result = PyString_Format(format, args);
	result = PyString_FromString(pipe_command_to_string(cmd));
    ///Py_DECREF(args);

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
    {"name", (PyCFunction)Radare_name, METH_NOARGS,
     "Return the name, combining the first and last name"
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

static PyMethodDef module_methods[] = {
    {NULL}  /* Sentinel */
};

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
}

int epython_destroy()
{
	// do nothing with the snake
}

int epython_eval(char *line)
{
	// XXX keep it simple!
	if (*line=='.') {
		FILE *fd = fopen(line+1,"r");;
		if (fd != NULL) {
			epython_init();
			PyRun_SimpleFile(fd, line+1);
			fclose(fd);
		}
	} else {
		epython_init();
		PyRun_SimpleString(line);
	}
	return 0;
}
#endif
