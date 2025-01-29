// #include "greenlet_compat.h"
#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <greenlet.h>

struct _greenlet;
// Forward declare the greenlet check function
PyObject* green_getframe(struct _greenlet* self, void* context);  // Changed to match exact symbol

namespace greenlet {
    bool is_greenlet(PyObject *obj) {
        return PyGreenlet_Check(obj) != 0;
    }
    PyFrameObject* getframe(PyObject* self) {   
        return (PyFrameObject*)green_getframe((struct _greenlet*)self, NULL);
    }

    void init_greenlet() {
        PyGreenlet_Import();
    }
}