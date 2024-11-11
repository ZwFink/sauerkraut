#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <stdbool.h>
#include "flatbuffers/flatbuffers.h"
#include "py_object_generated.h"
#include "utils.h"
#include "serdes.h"
#include "pyref.h" 
#include "py_structs.h"


class migrames_modulestate {
    public:
        pyobject_strongref deepcopy;
        pyobject_strongref deepcopy_module;
        pyobject_strongref pickle_module;
        pyobject_strongref pickle_dumps;
        pyobject_strongref pickle_loads;
        migrames_modulestate() {
            deepcopy_module = PyImport_ImportModule("copy");
            deepcopy = PyObject_GetAttrString(*deepcopy_module, "deepcopy");
            pickle_module = PyImport_ImportModule("pickle");
            pickle_dumps = PyObject_GetAttrString(*pickle_module, "dumps");
            pickle_loads = PyObject_GetAttrString(*pickle_module, "loads");
        }

};

class dumps_functor {
    pyobject_weakref pickle_dumps;
    public:
    dumps_functor(pyobject_weakref pickle_dumps) : pickle_dumps(pickle_dumps) {}

    pyobject_strongref operator()(PyObject *obj) {
        PyObject *result = PyObject_CallOneArg(*pickle_dumps, obj);
        return pyobject_strongref::steal(result);
    }
};

class loads_functor {
    pyobject_weakref pickle_loads;
    public:
    loads_functor(pyobject_weakref pickle_loads) : pickle_loads(pickle_loads) {}

    pyobject_strongref operator()(PyObject *obj) {
        PyObject *result = PyObject_CallOneArg(*pickle_loads, obj);
        return pyobject_strongref::steal(result);
    }
};

static migrames_modulestate *migrames_state;

extern "C" {


static inline _PyStackRef *_PyFrame_Stackbase(_PyInterpreterFrame *f) {
    return f->localsplus + ((PyCodeObject*)f->f_executable.bits)->co_nlocalsplus;
}

bool ThreadState_HasStackSpace(PyThreadState *state, int size) {
    return state->datastack_top != NULL && size < state->datastack_limit - state->datastack_top;
}
_PyInterpreterFrame *ThreadState_PushFrame(PyThreadState *tstate, size_t size) {
    if(!ThreadState_HasStackSpace(tstate, size)) {
        return NULL;
    }
    _PyInterpreterFrame *top = (_PyInterpreterFrame*) tstate->datastack_top;
    tstate->datastack_top += size;
    return top;
}

void copy_stack(_PyInterpreterFrame *src, _PyInterpreterFrame *dest) {
    if (src->stackpointer == NULL) {
        return;
    }
    _PyStackRef *src_stack_base = _PyFrame_Stackbase(src);
    _PyStackRef *dest_stack_base = _PyFrame_Stackbase(dest);
    Py_ssize_t stack_size = src->stackpointer - src_stack_base;
    memcpy(dest_stack_base, src_stack_base, stack_size * sizeof(_PyStackRef));
    dest->stackpointer = dest_stack_base + stack_size;
}



PyAPI_FUNC(PyFrameObject *) PyFrame_New(PyThreadState *, PyCodeObject *,
                                        PyObject *, PyObject *);

typedef struct serialized_obj {
    char *data;
    size_t size;
} serialized_obj;

PyObject *GetFrameLocalsFromFrame(PyObject *frame) {
    PyFrameObject *current_frame = (PyFrameObject *)frame;

    PyObject *locals = PyFrame_GetLocals(current_frame);
    if (locals == NULL) {
        return NULL;
    }


    if (PyFrameLocalsProxy_Check(locals)) {
        PyObject* ret = PyDict_New();
        if (ret == NULL) {
            Py_DECREF(locals);
            return NULL;
        }
        if (PyDict_Update(ret, locals) < 0) {
            Py_DECREF(ret);
            Py_DECREF(locals);
            return NULL;
        }
        Py_DECREF(locals);


        return ret;
    }

    assert(PyMapping_Check(locals));
    return locals;
}


// Allocate something that's not part of Python
_PyInterpreterFrame *AllocateFrameToMigrate(size_t size) {
    return (_PyInterpreterFrame *)malloc(size * sizeof(PyObject*));
}

PyObject *deepcopy_object(PyObject *obj) {
    if (obj == NULL) {
        return NULL;
    }
    PyObject *deepcopy = *migrames_state->deepcopy;
    PyObject *copy_obj = PyObject_CallFunction(deepcopy, "O", obj);
    return copy_obj;
}

struct frame_copy_capsule {
    // Strong reference
    PyFrameObject *frame;
    size_t offset;
};

static char copy_frame_capsule_name[] = "Frame Capsule Object";

void frame_copy_capsule_destroy(PyObject *capsule) {
    struct frame_copy_capsule *copy_capsule = (struct frame_copy_capsule *)PyCapsule_GetPointer(capsule, copy_frame_capsule_name);
    Py_DECREF(copy_capsule->frame);
    free(copy_capsule);
}

PyObject *frame_copy_capsule_create(PyFrameObject *frame, size_t offset) {
    struct frame_copy_capsule *copy_capsule = (struct frame_copy_capsule *)malloc(sizeof(struct frame_copy_capsule));
    copy_capsule->frame = frame;
    copy_capsule->offset = offset;
    return PyCapsule_New(copy_capsule, copy_frame_capsule_name, frame_copy_capsule_destroy);
}

void copy_localsplus(_PyInterpreterFrame *to_copy, _PyInterpreterFrame *new_frame, int nlocals, int deepcopy) {
    if (deepcopy) {
        for (int i = 0; i < nlocals; i++) {
            PyObject *local = (PyObject*) to_copy->localsplus[i].bits;
            PyObject *local_copy = deepcopy_object(local);
            new_frame->localsplus[i].bits = (uintptr_t)local_copy;
        }
    } else {
        memcpy(new_frame->localsplus, to_copy->localsplus, nlocals * sizeof(_PyStackRef));
    }
}

PyFrameObject *create_copied_frame(PyThreadState *tstate, _PyInterpreterFrame *to_copy, PyCodeObject *copy_code_obj, PyObject *LocalCopy, size_t offset, int push_frame, int deepcopy_localsplus, int set_previous, int copy_stack_flag) {
    int nlocals = copy_code_obj->co_nlocalsplus;

    PyFrameObject *new_frame = PyFrame_New(tstate, copy_code_obj, to_copy->f_globals, LocalCopy);

    _PyInterpreterFrame *stack_frame;
    if (push_frame) {
        stack_frame = ThreadState_PushFrame(tstate, copy_code_obj->co_framesize);
    } else {
        stack_frame = AllocateFrameToMigrate(copy_code_obj->co_framesize);
    }

    if(stack_frame == NULL) {
        PySys_WriteStderr("<Migrames>: Could not allocate memory for new frame\n");
        return NULL;
    }

    new_frame->f_frame = stack_frame;

    copy_localsplus(to_copy, new_frame->f_frame, nlocals, deepcopy_localsplus);

    new_frame->f_frame->owner = to_copy->owner;
    new_frame->f_frame->previous = set_previous ? to_copy : NULL;
    new_frame->f_frame->f_funcobj = deepcopy_object(to_copy->f_funcobj);
    PyObject *executable_copy = deepcopy_object((PyObject*) to_copy->f_executable.bits);
    new_frame->f_frame->f_executable.bits = (uintptr_t)executable_copy;
    new_frame->f_frame->f_globals = to_copy->f_globals;
    new_frame->f_frame->f_builtins = to_copy->f_builtins;
    new_frame->f_frame->f_locals = to_copy->f_locals;
    new_frame->f_frame->frame_obj = new_frame;
    new_frame->f_frame->stackpointer = new_frame->f_frame->localsplus + nlocals;
    new_frame->f_frame->instr_ptr = (_CodeUnit*) (copy_code_obj->co_code_adaptive + offset);

    if (copy_stack_flag) {
        copy_stack(to_copy, new_frame->f_frame);
    }

    return new_frame;
}

static PyObject *copy_frame(PyObject *self, PyObject *args) {
    using namespace utils;
    struct _frame *frame = (struct _frame*) PyEval_GetFrame();
    _PyInterpreterFrame *to_copy = frame->f_frame;
    PyThreadState *tstate = PyThreadState_Get();
    PyCodeObject *code = PyFrame_GetCode(frame);
    assert(code != NULL);
    PyCodeObject *copy_code_obj = (PyCodeObject *)deepcopy_object((PyObject*)code);

    Py_ssize_t offset = py::get_instr_offset(frame) + py::get_offset_for_skipping_call();
    PyObject *FrameLocals = GetFrameLocalsFromFrame((PyObject*)frame);
    PyObject *LocalCopy = PyDict_Copy(FrameLocals);

    PyFrameObject *new_frame = create_copied_frame(tstate, to_copy, copy_code_obj, LocalCopy, offset, 0, 1, 0, 1);

    PyObject *capsule = frame_copy_capsule_create(new_frame, offset);
    Py_DECREF(copy_code_obj);
    Py_DECREF(LocalCopy);
    Py_DECREF(FrameLocals);

    return capsule;
}

static PyObject *copy_and_run_frame(PyObject *self, PyObject *args) {
    using namespace utils;
    struct _frame *frame = (struct _frame*) PyEval_GetFrame();
    _PyInterpreterFrame *to_copy = frame->f_frame;
    PyThreadState *tstate = PyThreadState_Get();
    PyCodeObject *code = PyFrame_GetCode(frame);
    assert(code != NULL);
    PyCodeObject *copy_code_obj = (PyCodeObject *)deepcopy_object((PyObject*)code);

    Py_ssize_t offset = py::get_instr_offset(frame) + py::get_offset_for_skipping_call();
    PyObject *FrameLocals = GetFrameLocalsFromFrame((PyObject*)frame);
    PyObject *LocalCopy = PyDict_Copy(FrameLocals);

    PyFrameObject *new_frame = create_copied_frame(tstate, to_copy, copy_code_obj, LocalCopy, offset, 1, 0, 1, 1);

    PyObject *res = PyEval_EvalFrame(new_frame);
    Py_DECREF(copy_code_obj);
    Py_DECREF(LocalCopy);
    Py_DECREF(FrameLocals);

    return res;
}

static PyObject *_copy_run_frame_from_capsule(PyObject *capsule) {
    if (PyErr_Occurred()) {
        PyErr_Print();
        return NULL;
    }

    struct frame_copy_capsule *copy_capsule = (struct frame_copy_capsule *)PyCapsule_GetPointer(capsule, copy_frame_capsule_name);
    if (copy_capsule == NULL) {
        return NULL;
    }

    PyFrameObject *frame = copy_capsule->frame;
    size_t offset = copy_capsule->offset;
    _PyInterpreterFrame *to_copy = frame->f_frame;
    PyThreadState *tstate = PyThreadState_Get();
    PyCodeObject *code = PyFrame_GetCode(frame);
    assert(code != NULL);
    PyCodeObject *copy_code_obj = (PyCodeObject *)deepcopy_object((PyObject*)code);

    PyObject *FrameLocals = GetFrameLocalsFromFrame((PyObject*)frame);
    PyObject *LocalCopy = PyDict_Copy(FrameLocals);

    PyFrameObject *new_frame = create_copied_frame(tstate, to_copy, copy_code_obj, LocalCopy, offset, 1, 0, 1, 0);

    PyObject *res = PyEval_EvalFrame(new_frame);
    Py_DECREF(copy_code_obj);
    Py_DECREF(LocalCopy);
    Py_DECREF(FrameLocals);

    return res;
}

static PyObject *run_frame(PyObject *self, PyObject *args) {
    PyObject *capsule;
    if (!PyArg_ParseTuple(args, "O", &capsule)) {
        return NULL;
    }
    return _copy_run_frame_from_capsule(capsule);
}

static PyObject* _serialize_frame_from_capsule(PyObject *capsule) {
    if (PyErr_Occurred()) {
        PyErr_Print();
        return NULL;
    }

    struct frame_copy_capsule *copy_capsule = (struct frame_copy_capsule *)PyCapsule_GetPointer(capsule, copy_frame_capsule_name);
    if (copy_capsule == NULL) {
        return NULL;
    }

    loads_functor loads(migrames_state->pickle_loads);
    dumps_functor dumps(migrames_state->pickle_dumps);

    flatbuffers::FlatBufferBuilder builder{1024};
    serdes::PyObjectSerdes po_serdes(loads, dumps);

    serdes::PyFrameSerdes frame_serdes{po_serdes};

    auto serialized_frame = frame_serdes.serialize(builder, *(static_cast<migrames::PyFrame*>(copy_capsule->frame)));
    builder.Finish(serialized_frame);
    auto buf = builder.GetBufferPointer();
    auto size = builder.GetSize();
    PyObject *bytes = PyBytes_FromStringAndSize((const char *)buf, size);
    return bytes;
}

static PyObject *_deserialize_frame(PyObject *bytes) {
    if(PyErr_Occurred()) {
        PyErr_Print();
        return NULL;
    }
    loads_functor loads(migrames_state->pickle_loads);
    dumps_functor dumps(migrames_state->pickle_dumps);
    serdes::PyObjectSerdes po_serdes(loads, dumps);
    serdes::PyFrameSerdes frame_serdes{po_serdes};

    uint8_t *data = (uint8_t *)PyBytes_AsString(bytes);

    auto serframe = pyframe_buffer::GetPyFrame(data);
    auto deserframe = frame_serdes.deserialize(serframe);
    utils::py::print_object(deserframe.f_frame.f_funcobj.borrow());

    Py_RETURN_NONE;
}

static PyObject *serialize_frame(PyObject *self, PyObject *args) {
    PyObject *capsule;
    if (!PyArg_ParseTuple(args, "O", &capsule)) {
        return NULL;
    }

    return _serialize_frame_from_capsule(capsule);
}

static PyObject *deserialize_frame(PyObject *self, PyObject *args) {
    PyObject *bytes;
    if (!PyArg_ParseTuple(args, "O", &bytes)) {
        return NULL;
    }

    return _deserialize_frame(bytes);
}

static PyMethodDef MyMethods[] = {
    {"copy_and_run_frame", copy_and_run_frame, METH_VARARGS, "Copy the current frame and run it"},
    {"copy_frame", copy_frame, METH_VARARGS, "Copy the current frame"},
    {"run_frame", run_frame, METH_VARARGS, "Run the frame from the capsule"},
    {"serialize_frame", serialize_frame, METH_VARARGS, "Serialize the frame"},
    {"deserialize_frame", deserialize_frame, METH_VARARGS, "Deserialize the frame"},
    {NULL, NULL, 0, NULL}
};

static void migrames_free(void *m) {
    delete migrames_state;
}

static struct PyModuleDef migrames_mod = {
    PyModuleDef_HEAD_INIT,
    "migrames",
    "A module that defines the 'abcd' function",
    -1,
    MyMethods,
    NULL, // slot definitions
    NULL, // traverse function for GC
    NULL, // clear function for GC
    migrames_free // free function for GC
};

PyMODINIT_FUNC PyInit_migrames(void) {
    migrames_state = new migrames_modulestate();
    return PyModule_Create(&migrames_mod);
}

}