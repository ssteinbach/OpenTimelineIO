#include <Python.h>
#include "opentime.h"
#include "structmember.h"

typedef struct {
    PyObject_HEAD
    PyObject *first; /* first name */
    PyObject *last;  /* last name */
    int number;

    // actual fields
    double value;
    double rate;
} RationalTime;

static void
RationalTime_dealloc(RationalTime* self)
{
    Py_XDECREF(self->first);
    Py_XDECREF(self->last);
    Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject *
RationalTime_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    RationalTime *self;

    self = (RationalTime *)type->tp_alloc(type, 0);
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

static int
RationalTime_init(RationalTime *self, PyObject *args, PyObject *kwds)
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


static PyMemberDef RationalTime_members[] = {
    {"first", T_OBJECT_EX, offsetof(RationalTime, first), 0,
     "first name"},
    {"last", T_OBJECT_EX, offsetof(RationalTime, last), 0,
     "last name"},
    {"number", T_INT, offsetof(RationalTime, number), 0,
     "opentime number"},

    // actual members
    {"value", T_DOUBLE, offsetof(RationalTime, value), 0,
     "point in time"},
    {"rate", T_DOUBLE, offsetof(RationalTime, rate), 0,
     "rate of samples / time"},

    {NULL}  /* Sentinel */
};

static PyObject *
RationalTime_name(RationalTime* self)
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
RationalTime_rescaled_to(RationalTime* self, PyObject* new_rate)
{
    /* static PyObject *format = NULL; */
    /* PyObject *args, *result; */
    /*  */
    /* if (format == NULL) { */
    /*     format = PyString_FromString("%s %s"); */
    /*     if (format == NULL) */
    /*         return NULL; */
    /* } */
    /*  */
    /* if (self->first == NULL) { */
    /*     PyErr_SetString(PyExc_AttributeError, "first"); */
    /*     return NULL; */
    /* } */
    /*  */
    /* if (self->last == NULL) { */
    /*     PyErr_SetString(PyExc_AttributeError, "last"); */
    /*     return NULL; */
    /* } */
    /*  */
    /* args = Py_BuildValue("OO", self->first, self->last); */
    /* if (args == NULL) */
    /*     return NULL; */
    /*  */
    /* result = PyString_Format(format, args); */
    /* Py_DECREF(args); */

    return Py_False;
}


static PyMethodDef RationalTime_methods[] = {
    {"name", (PyCFunction)RationalTime_name, METH_NOARGS,
     "Return the name, combining the first and last name"
    },
    {"rescaled_to", (PyCFunction)RationalTime_rescaled_to, METH_NOARGS,
     "Returns the time for this time converted to new_rate"
    },
    {NULL}  /* Sentinel */
};

static PyTypeObject RationalTimeType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "opentime.RationalTime",             /* tp_name */
    sizeof(RationalTime),             /* tp_basicsize */
    0,                         /* tp_itemsize */
    (destructor)RationalTime_dealloc, /* tp_dealloc */
    0,                         /* tp_print */
    0,                         /* tp_getattr */
    0,                         /* tp_setattr */
    0,                         /* tp_compare */
    0,                         /* tp_repr */
    0,                         /* tp_as_number */
    0,                         /* tp_as_sequence */
    0,                         /* tp_as_mapping */
    0,                         /* tp_hash */
    0,                         /* tp_call */
    0,                         /* tp_str */
    0,                         /* tp_getattro */
    0,                         /* tp_setattro */
    0,                         /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT |
        Py_TPFLAGS_BASETYPE,   /* tp_flags */
    "RationalTime objects",           /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    0,                         /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    RationalTime_methods,             /* tp_methods */
    RationalTime_members,             /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)RationalTime_init,      /* tp_init */
    0,                         /* tp_alloc */
    RationalTime_new,                 /* tp_new */
};

static PyMethodDef module_methods[] = {
    {NULL}  /* Sentinel */
};

#ifndef PyMODINIT_FUNC	/* declarations for DLL import/export */
#define PyMODINIT_FUNC void
#endif
PyMODINIT_FUNC
initopentime(void)
{
    PyObject* m;

    if (PyType_Ready(&RationalTimeType) < 0)
        return;

    m = Py_InitModule3("opentime", module_methods,
                       "Example module that creates an extension type.");

    if (m == NULL)
        return;

    Py_INCREF(&RationalTimeType);
    PyModule_AddObject(m, "RationalTime", (PyObject *)&RationalTimeType);
}
