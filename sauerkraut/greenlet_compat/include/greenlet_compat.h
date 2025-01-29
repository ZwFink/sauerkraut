#ifndef GREENLET_COMPAT_H
#define GREENLET_COMPAT_H
#include "sauerkraut_cpython_compat.h"
#include "pyref.h"

namespace greenlet {
    bool is_greenlet(PyObject *obj);
    // PyObject *get_parent(PyObject *obj);
    PyFrameObject *getframe(PyObject *obj);
    void init_greenlet();
}

#endif