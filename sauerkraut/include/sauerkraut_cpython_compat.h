#ifndef SAUERKRAUT_CPYTHON_COMPAT_HH_INCLUDED
#define SAUERKRAUT_CPYTHON_COMPAT_HH_INCLUDED
#define PY_SSIZE_T_CLEAN
#include <Python.h>

#if PY_VERSION_HEX >= 0x30D0000 && PY_VERSION_HEX < 0x30E0000
#    warning "Python 3.13 detected"
#    define SAUERKRAUT_PY313 1
#else
#    define SAUERKRAUT_PY313 0
#endif

#if PY_VERSION_HEX >= 0x30E0000 && PY_VERSION_HEX < 0x30F0000
#    define SAUERKRAUT_PY314 1
#else
#    define SAUERKRAUT_PY314 0
#
#endif

#if !SAUERKRAUT_PY313 && !SAUERKRAUT_PY314
#    error "Unsupported Python version"
#endif
#endif 