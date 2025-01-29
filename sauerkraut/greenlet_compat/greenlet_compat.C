#include "greenlet_compat.h"
#include <greenlet.h>

struct _greenlet;
PyObject* green_getframe(struct _greenlet* self, void* context);

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