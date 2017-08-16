#include <Python.h>
#include "opentime.h"

typedef struct {
    PyObject_HEAD;
    /* Type-specific fields go here. */
    float time;
} opentime_RationalTimeObject;

static PyTypeObject opentime_RationalTimeType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "opentime.RationalTime",             /* tp_name */
    sizeof(opentime_RationalTimeObject), /* tp_basicsize */
    0,                         /* tp_itemsize */
    0,                         /* tp_dealloc */
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
    Py_TPFLAGS_DEFAULT,        /* tp_flags */
    "RationalTime objects",           /* tp_doc */
};

static PyMethodDef opentime_methods[] = {
    /* {"rescaled_to", (PyCFunction)RationalTime_rescaled_to, METH_NOARGS, "Rescale the rational time" */
    /* }, */
    {NULL}  /* Sentinel */
};

#ifndef PyMODINIT_FUNC	/* declarations for DLL import/export */
#define PyMODINIT_FUNC void
#endif
PyMODINIT_FUNC
initopentime(void) 
{
    PyObject* m;

    opentime_RationalTimeType.tp_new = PyType_GenericNew;
    if (PyType_Ready(&opentime_RationalTimeType) < 0)
        return;

    m = Py_InitModule3("opentime", opentime_methods,
                       "Example module that creates an extension type.");

    Py_INCREF(&opentime_RationalTimeType);
    PyModule_AddObject(m, "RationalTime", (PyObject *)&opentime_RationalTimeType);
}
