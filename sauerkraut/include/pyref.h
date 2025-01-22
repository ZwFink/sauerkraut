#ifndef PYREF_HH_INCLUDED
#define PYREF_HH_INCLUDED
#define PY_SSIZE_T_CLEAN
#include <Python.h>



template <typename T>
class py_strongref {
    T *obj = NULL;
    public:
    py_strongref(T *obj) : obj(obj) {
        Py_XINCREF((PyObject*) obj);
    }

    py_strongref() : obj(NULL) {}
    py_strongref(const py_strongref &other) : obj(other.obj) {
        Py_XINCREF((PyObject*) obj);
    }

    ~py_strongref() {
        Py_XDECREF((PyObject*) obj);
    }

    py_strongref(py_strongref &&other) {
        obj = other.obj;
        other.obj = NULL;
    }

    // = constructor
    py_strongref &operator=(const py_strongref &other) {
        if (this != &other) {
            Py_XDECREF((PyObject*) obj);
            obj = other.obj;
            Py_XINCREF((PyObject*) obj);
        }
        return *this;
    }

    PyObject *operator*() {
        return this->borrow();
    }

    PyObject *borrow() const {
        return (PyObject*) obj;
    }

    void operator=(PyObject *new_obj) {
        // always steals
        obj = new_obj;
    }

    operator bool() {
        return obj != NULL;
    }

    static py_strongref steal(T *obj) {
        py_strongref<T> ref;
        ref = obj;
        return ref;
    }
};

template <typename T>
class py_weakref {
    T *obj;
    public:
    py_weakref(T *obj) : obj(obj) {
    }

    py_weakref() : obj(NULL) {}
    py_weakref(const py_weakref &other) : obj(other.obj) {
    }
    py_weakref(const py_strongref<T> &other) : obj(other.borrow()) {
    }

    T *operator*() {
        return this->borrow();
    }

    T *borrow() {
        return (T*) obj;
    }

    void operator=(T *new_obj) {
        // always steals
        obj = new_obj;
    }

    operator bool() {
        return obj != NULL;
    }

    T *operator->() {
        return this->borrow();
    }
};

using pyobject_strongref = py_strongref<PyObject>;
using pyobject_weakref = py_weakref<PyObject>;
#endif