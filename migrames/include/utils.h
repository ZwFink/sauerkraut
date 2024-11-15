#ifndef UTILS_HH_INCLUDED
#define UTILS_HH_INCLUDED
#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include "py_structs.h"
#include "pyref.h"

namespace utils {
    namespace py {
       int get_code_stacksize(PyCodeObject *code) {
            return code->co_stacksize;
       }

       int get_code_nlocalsplus(PyCodeObject *code) {
           return code->co_nlocalsplus;
       }

       int get_code_nlocals(PyCodeObject *code) {
           return code->co_nlocals;
       }

     migrames::PyBitcodeInstruction *get_code_adaptive(py_weakref<PyCodeObject> code) {
           return (migrames::PyBitcodeInstruction*) code->co_code_adaptive;
       }

       int get_iframe_localsplus_size(migrames::PyInterpreterFrame *iframe) {
           PyCodeObject *code = (PyCodeObject*) iframe->f_executable.bits;
           if(NULL == code) {
               return 0;
           }
           return get_code_nlocalsplus(code) + code->co_stacksize;
       }

        Py_ssize_t get_instr_offset(struct _frame *frame) {
            PyCodeObject *code = PyFrame_GetCode(frame);
            Py_ssize_t first_instr_addr = (Py_ssize_t) code->co_code_adaptive;
            Py_ssize_t current_instr_addr = (Py_ssize_t) frame->f_frame->instr_ptr;
            return (current_instr_addr - first_instr_addr);
        }

        Py_ssize_t get_offset_for_skipping_call() {
            return 5 * sizeof(_CodeUnit);
        }

        void print_object(PyObject *obj) {
            if (obj == NULL) {
                printf("Error: NULL object passed\n");
                return;
            }
        
            PyObject *str = PyObject_Repr(obj);
            if (str == NULL) {
                PyErr_Print();
                return;
            }
        
            const char *c_str = PyUnicode_AsUTF8(str);
            if (c_str == NULL) {
                Py_DECREF(str);
                PyErr_Print();
                return;
            }
        
            printf("Object contents: %s\n", c_str);
            Py_DECREF(str);
        }
        
        void print_object_type_name(PyObject *obj) {
            if (obj == NULL) {
                printf("Error: NULL object\n");
                return;
            }
        
            PyObject *type = PyObject_Type(obj);
            if (type == NULL) {
                printf("Error: Could not get object type\n");
                PyErr_Print();
                return;
            }
        
            PyObject *type_name = PyObject_GetAttrString(type, "__name__");
            if (type_name == NULL) {
                printf("Error: Could not get type name\n");
                PyErr_Print();
                Py_DECREF(type);
                return;
            }
        
            const char *name = PyUnicode_AsUTF8(type_name);
            if (name == NULL) {
                printf("Error: Could not convert type name to string\n");
                PyErr_Print();
            } else {
                printf("Object type: %s\n", name);
            }
        
            Py_DECREF(type_name);
            Py_DECREF(type);
        }

        Py_ssize_t get_code_size(Py_ssize_t n_instructions) {
            return n_instructions * sizeof(_CodeUnit);
        }

    }
}


#endif // UTILS_HH_INCLUDED