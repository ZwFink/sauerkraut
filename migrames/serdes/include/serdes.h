#ifndef SERDES_HH_INCLUDED
#define SERDES_HH_INCLUDED
#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <iostream>
#include "flatbuffers/flatbuffers.h"
#include "py_object_generated.h"
#include "py_var_object_head_generated.h"
#include "py_object_head_generated.h"
#include "py_frame_generated.h"
#include "offsets.h"
#include "pyref.h"
#include "py_structs.h"

namespace serdes {
    template<typename Loads, typename Dumps>
    class PyObjectSerdes {
        Loads loads;
        Dumps dumps;
        public:
            PyObjectSerdes(Loads& loads, Dumps& dumps) :
                loads(loads), dumps(dumps) {
            }

            template<typename Builder>
            offsets::PyObjectOffset serialize(Builder &builder, PyObject *obj) {
                auto dumps_result = dumps(obj);
                if(NULL == dumps_result.borrow()) {
                    PyErr_Print();
                }
                Py_ssize_t size = 0;
                char *pickled_data;
                if(PyBytes_AsStringAndSize(*dumps_result, &pickled_data, &size) == -1) {
                    PyErr_Print();
                }
                auto bytes = builder.CreateVector((const uint8_t *)pickled_data, size);
                auto py_obj = pyframe_buffer::CreatePyObject(builder, bytes);

                return py_obj;
            }

            auto deserialize(const pyframe_buffer::PyObject *obj) -> decltype(loads(nullptr)) {
                auto data = obj->data()->data();
                auto size = obj->data()->size();
                PyObject *bytes = PyBytes_FromStringAndSize((const char*)data, size);
                return loads(bytes);
            }


    };

    template<typename Loads, typename Dumps>
    PyObjectSerdes(Loads&, Dumps&) -> PyObjectSerdes<Loads, Dumps>;

    template<typename PyObjectSerializer>
    class PyObjectHeadSerdes {
        PyObjectSerializer serializer;
        public:
            PyObjectHeadSerdes(PyObjectSerializer& serializer) : serializer(serializer) {}

            struct Head {
                pyobject_strongref obj;
            };

            template<typename Builder>
            offsets::PyObjectHeadOffset serialize(Builder &builder, PyObject *obj) {
                auto py_obj = serializer.serialize(builder, obj);
                auto head = pyframe_buffer::CreatePyObjectHead(builder, py_obj);
                return head;
            }

            Head deserialize(const pyframe_buffer::PyObjectHead *obj) {
                auto ob_base = obj->ob_base();
                auto py_obj = serializer.deserialize(ob_base);
                return {py_obj};
            }
    };

    template<typename PyObjectSerializer>
    class PyVarObjectHeadSerdes {
        PyObjectSerializer serializer;
        public:
            PyVarObjectHeadSerdes(PyObjectSerializer& serializer) : serializer(serializer) {}

            struct VarHead {
                pyobject_strongref obj;
                size_t size;
            };

            template<typename Builder>
            offsets::PyVarObjectHeadOffset serialize(Builder &builder, PyObject *obj, size_t size) {
                auto py_obj = serializer.serialize(builder, obj);
                auto var_head = pyframe_buffer::CreatePyVarObjectHead(builder, py_obj, size);
                return var_head;
            }

            VarHead deserialize(const pyframe_buffer::PyVarObjectHead *obj) {
                auto py_obj = serializer.deserialize(obj->ob_base());
                size_t size = obj->ob_size();
                return {py_obj, size};
            }
    };

    template<typename PyObjectSerializer>
    class PyFrameSerdes {
        PyObjectSerializer po_serializer;
        PyObjectHeadSerdes<PyObjectSerializer> poh_serializer;
        public:
            PyFrameSerdes(PyObjectSerializer& po_serializer) : 
                          po_serializer(po_serializer),
                          poh_serializer(po_serializer) {}

            struct PyFrame {
                pyobject_strongref obj_head;
                pyobject_strongref f_trace;
                int f_lineno;
                char f_trace_lines;
                char f_trace_opcodes;
                pyobject_strongref f_extra_locals;
                pyobject_strongref f_locals_cache;
            };

            template<typename Builder>
            offsets::PyFrameOffset serialize(Builder &builder, struct _frame &obj) {
                pyframe_buffer::PyFrameBuilder frame_builder(builder);
                // Do NOT serialize the ob_base.
                // frame_builder.add_ob_base(poh_serializer.serialize(builder, &obj.ob_base));
                
                if(NULL != obj.f_trace) {
                    auto f_trace_ser = po_serializer.serialize(builder, obj.f_trace);
                    frame_builder.add_f_trace(f_trace_ser);
                }

                frame_builder.add_f_lineno(obj.f_lineno);
                frame_builder.add_f_trace_lines(obj.f_trace_lines);
                frame_builder.add_f_trace_opcodes(obj.f_trace_opcodes);

                if(NULL != obj.f_extra_locals) {
                    auto f_extra_locals_ser = po_serializer.serialize(builder, obj.f_extra_locals);
                    frame_builder.add_f_extra_locals(f_extra_locals_ser);
                }

                if(NULL != obj.f_locals_cache) {
                    auto f_locals_cache_ser = po_serializer.serialize(builder, obj.f_locals_cache);
                    frame_builder.add_f_locals_cache(f_locals_cache_ser);
                }
                
                return frame_builder.Finish();
            }

            PyFrame deserialize(const pyframe_buffer::PyFrame *obj) {
                auto ob_base_deser = poh_serializer.deserialize(obj->ob_base());
                pyobject_strongref f_trace_deser;


                if(NULL != obj->f_trace()) {
                    f_trace_deser = po_serializer.deserialize(obj->f_trace());
                }

                auto f_lineno = obj->f_lineno();
                auto f_trace_lines = obj->f_trace_lines();
                auto f_trace_opcodes = obj->f_trace_opcodes();

                pyobject_strongref f_extra_locals_deser;
                if(NULL != obj->f_extra_locals()) {
                    f_extra_locals_deser = po_serializer.deserialize(obj->f_extra_locals());
                }

                pyobject_strongref f_locals_cache_deser;
                if(NULL != obj->f_locals_cache()) {
                    f_locals_cache_deser = po_serializer.deserialize(obj->f_locals_cache());
                }
            return {ob_base_deser, f_trace_deser, f_lineno, f_trace_lines, f_trace_opcodes, f_extra_locals_deser, f_locals_cache_deser};
            }

    };

    template<typename PyObjectSerializer>
    PyFrameSerdes(PyObjectSerializer&) -> PyFrameSerdes<PyObjectSerializer>;
    
}

#endif // SERDES_HH_INCLUDED