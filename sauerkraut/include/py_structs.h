#ifndef PY_STRUCTS_HH_INCLUDED
#define PY_STRUCTS_HH_INCLUDED
#define PY_SSIZE_T_CLEAN
#include <Python.h>

extern "C" {
typedef union _PyStackRef {
    uintptr_t bits;
} _PyStackRef;

// Dummy definition: real definition is in pycore_code.h
typedef struct _CodeUnit {
    uint8_t opcode;
    uint8_t oparg;
} _CodeUnit;

struct _frame {
    PyObject_HEAD
    PyFrameObject *f_back;      /* previous frame, or NULL */
    struct _PyInterpreterFrame *f_frame; /* points to the frame data */
    PyObject *f_trace;          /* Trace function */
    int f_lineno;               /* Current line number. Only valid if non-zero */
    char f_trace_lines;         /* Emit per-line trace events? */
    char f_trace_opcodes;       /* Emit per-opcode trace events? */
    PyObject *f_extra_locals;   /* Dict for locals set by users using f_locals, could be NULL */
    PyObject *f_locals_cache;   /* Backwards compatibility for PyEval_GetLocals */
    PyObject *_f_frame_data[1]; /* Frame data if this frame object owns the frame */
};

struct _PyInterpreterFrame *
_PyThreadState_PushFrame(PyThreadState *tstate, size_t size);

typedef struct _PyInterpreterFrame {
    _PyStackRef f_executable; /* Deferred or strong reference (code object or None) */
    struct _PyInterpreterFrame *previous;
    PyObject *f_funcobj; /* Strong reference. Only valid if not on C stack */
    PyObject *f_globals; /* Borrowed reference. Only valid if not on C stack */
    PyObject *f_builtins; /* Borrowed reference. Only valid if not on C stack */
    PyObject *f_locals; /* Strong reference, may be NULL. Only valid if not on C stack */
    PyFrameObject *frame_obj; /* Strong reference, may be NULL. Only valid if not on C stack */
    _CodeUnit *instr_ptr; /* Instruction currently executing (or about to begin) */
    _PyStackRef *stackpointer;
    uint16_t return_offset;  /* Only relevant during a function call */
    char owner;
    /* Locals and stack */
    _PyStackRef localsplus[1];
} _PyInterpreterFrame;

} // extern "C"

namespace sauerkraut {
    using PyInterpreterFrame = struct _PyInterpreterFrame;
    using PyFrame = struct _frame;
    using PyBitcodeInstruction = _CodeUnit;
}

#endif // PY_STRUCTS_HH_INCLUDED