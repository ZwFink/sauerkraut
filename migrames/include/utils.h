#ifndef UTILS_HH_INCLUDED
#define UTILS_HH_INCLUDED
#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include "py_structs.h"

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
            return current_instr_addr - first_instr_addr;
        }

        Py_ssize_t get_offset_for_skipping_call() {
            return 5 * sizeof(_CodeUnit);
        }
    }
}


#endif // UTILS_HH_INCLUDED