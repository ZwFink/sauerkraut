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
#include "utils.h"

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
                if(NULL == obj) {
                    return NULL;
                }

                auto data = obj->data()->data();
                auto size = obj->data()->size();

                if(NULL == data) {
                    return NULL;
                }
                PyObject *bytes = PyBytes_FromStringAndSize((const char*)data, size);
                return loads(bytes);
            }


    };

    template<typename Loads, typename Dumps>
    PyObjectSerdes(Loads&, Dumps&) -> PyObjectSerdes<Loads, Dumps>;

    template<typename PyCodeObjectSerializer>
    class PyObjectHeadSerdes {
        PyCodeObjectSerializer serializer;
        public:
            PyObjectHeadSerdes(PyCodeObjectSerializer& serializer) : serializer(serializer) {}

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

    template<typename PyCodeObjectSerializer>
    class PyVarObjectHeadSerdes {
        PyCodeObjectSerializer serializer;
        public:
            PyVarObjectHeadSerdes(PyCodeObjectSerializer& serializer) : serializer(serializer) {}

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

    class DeserializedCodeObject {
      public:
        pyobject_strongref co_consts{NULL};
        pyobject_strongref co_names{NULL};
        pyobject_strongref co_exceptiontable{NULL};

        int co_flags;

        int co_argcount;
        int co_posonlyargcount;
        int co_kwonlyargcount;
        int co_stacksize;
        int co_firstlineno;

        int co_nlocalsplus;
        int co_framesize;
        int co_nlocals;
        int co_ncellvars;
        int co_nfreevars;
        int co_version;

        pyobject_strongref co_localsplusnames;
        pyobject_strongref co_localspluskinds;

        pyobject_strongref co_filename;
        pyobject_strongref co_name;
        pyobject_strongref co_qualname;
        pyobject_strongref co_linetable;

        std::vector<unsigned char> co_code_adaptive;
    };

    template <typename PyCodeObjectSerializer>
    class PyCodeObjectSerdes {
        PyCodeObjectSerializer po_serializer;
        template<typename Builder>
        flatbuffers::Offset<flatbuffers::Vector<uint8_t>> serialize_bitcode(Builder &builder, PyCodeObject *code) {
            auto n_instructions = Py_SIZE(code);
            auto total_size_bytes = n_instructions * sizeof(migrames::PyBitcodeInstruction);
            // const migrames::PyBitcodeInstruction *bitcode = (const migrames::PyBitcodeInstruction *) code->co_code_adaptive;
            PyObject *code_instrs = PyCode_GetCode(code);
            char *bitcode = PyBytes_AsString(code_instrs);

            auto bytes = builder.CreateVector((const uint8_t*) bitcode, total_size_bytes);
            return bytes;
        }

        public:
        PyCodeObjectSerdes(PyCodeObjectSerializer& po_serializer) : 
            po_serializer(po_serializer) {}

        template<typename Builder>
        offsets::PyCodeObjectOffset serialize(Builder &builder, PyCodeObject *obj) {
            auto co_consts_ser = (NULL != obj->co_consts) ? 
                std::optional{po_serializer.serialize(builder, obj->co_consts)} : std::nullopt;

            auto co_names_ser = (NULL != obj->co_names) ?
                std::optional{po_serializer.serialize(builder, obj->co_names)} : std::nullopt;

            auto co_exceptiontable_ser = (NULL != obj->co_exceptiontable) ?
                std::optional{po_serializer.serialize(builder, obj->co_exceptiontable)} : std::nullopt;

            auto co_localsplusnames_ser = (NULL != obj->co_localsplusnames) ?
                std::optional{po_serializer.serialize(builder, obj->co_localsplusnames)} : std::nullopt;

            auto co_localspluskinds_ser = (NULL != obj->co_localspluskinds) ?
                std::optional{po_serializer.serialize(builder, obj->co_localspluskinds)} : std::nullopt;

            auto co_filename_ser = (NULL != obj->co_filename) ?
                std::optional{po_serializer.serialize(builder, obj->co_filename)} : std::nullopt;

            auto co_name_ser = (NULL != obj->co_name) ?
                std::optional{po_serializer.serialize(builder, obj->co_name)} : std::nullopt;

            auto co_qualname_ser = (NULL != obj->co_qualname) ?
                std::optional{po_serializer.serialize(builder, obj->co_qualname)} : std::nullopt;

            auto co_linetable_ser = (NULL != obj->co_linetable) ?
                std::optional{po_serializer.serialize(builder, obj->co_linetable)} : std::nullopt;

            auto co_code_adaptive_ser = serialize_bitcode(builder, obj);

            pyframe_buffer::PyCodeObjectBuilder code_builder(builder);

            if (co_consts_ser) {
                code_builder.add_co_consts(co_consts_ser.value());
            }
            if (co_names_ser) {
                code_builder.add_co_names(co_names_ser.value());
            }
            if (co_exceptiontable_ser) {
                code_builder.add_co_exceptiontable(co_exceptiontable_ser.value());
            }

            code_builder.add_co_flags(obj->co_flags);

            code_builder.add_co_argcount(obj->co_argcount);
            code_builder.add_co_posonlyargcount(obj->co_posonlyargcount);
            code_builder.add_co_kwonlyargcount(obj->co_kwonlyargcount);
            code_builder.add_co_stacksize(obj->co_stacksize);
            code_builder.add_co_firstlineno(obj->co_firstlineno);

            code_builder.add_co_nlocalsplus(obj->co_nlocalsplus);
            code_builder.add_co_framesize(obj->co_framesize);
            code_builder.add_co_nlocals(obj->co_nlocals);
            code_builder.add_co_ncellvars(obj->co_ncellvars);
            code_builder.add_co_nfreevars(obj->co_nfreevars);
            code_builder.add_co_version(obj->co_version);

            if (co_localsplusnames_ser) {
                code_builder.add_co_localsplusnames(co_localsplusnames_ser.value());
            }
            if (co_localspluskinds_ser) {
                code_builder.add_co_localspluskinds(co_localspluskinds_ser.value());
            }

            if (co_filename_ser) {
                code_builder.add_co_filename(co_filename_ser.value());
            }
            if (co_name_ser) {
                code_builder.add_co_name(co_name_ser.value());
            }
            if (co_qualname_ser) {
                code_builder.add_co_qualname(co_qualname_ser.value());
            }
            if (co_linetable_ser) {
                code_builder.add_co_linetable(co_linetable_ser.value());
            }
            code_builder.add_co_code_adaptive(co_code_adaptive_ser);

            return code_builder.Finish();
        }

        DeserializedCodeObject deserialize(const pyframe_buffer::PyCodeObject *obj) {
            DeserializedCodeObject deser;
            deser.co_consts = po_serializer.deserialize(obj->co_consts());
            deser.co_names = po_serializer.deserialize(obj->co_names());
            deser.co_exceptiontable = po_serializer.deserialize(obj->co_exceptiontable());

            deser.co_flags = obj->co_flags();

            deser.co_argcount = obj->co_argcount();
            deser.co_posonlyargcount = obj->co_posonlyargcount();
            deser.co_kwonlyargcount = obj->co_kwonlyargcount();
            deser.co_stacksize = obj->co_stacksize();
            deser.co_firstlineno = obj->co_firstlineno();

            deser.co_nlocalsplus = obj->co_nlocalsplus();
            deser.co_framesize = obj->co_framesize();
            deser.co_nlocals = obj->co_nlocals();
            deser.co_ncellvars = obj->co_ncellvars();
            deser.co_nfreevars = obj->co_nfreevars();
            deser.co_version = obj->co_version();

            deser.co_localsplusnames = po_serializer.deserialize(obj->co_localsplusnames());
            deser.co_localspluskinds = po_serializer.deserialize(obj->co_localspluskinds());

            deser.co_filename = po_serializer.deserialize(obj->co_filename());
            deser.co_name = po_serializer.deserialize(obj->co_name());
            deser.co_qualname = po_serializer.deserialize(obj->co_qualname());
            deser.co_linetable = po_serializer.deserialize(obj->co_linetable());

            auto bitcode = obj->co_code_adaptive();
            deser.co_code_adaptive = std::vector<unsigned char>(bitcode->begin(), bitcode->end());

            return deser;
        }
    };

    template<typename PyCodeObjectSerializer>
    PyVarObjectHeadSerdes(PyCodeObjectSerializer&) -> PyVarObjectHeadSerdes<PyCodeObjectSerializer>;

    class DeserializedPyInterpreterFrame {
      public:
        DeserializedCodeObject f_executable;
        pyobject_strongref f_funcobj;
        pyobject_strongref f_globals;
        pyobject_strongref f_builtins;
        pyobject_strongref f_locals;

        uint64_t instr_offset;
        uint16_t return_offset;

        uint8_t owner;

        std::vector<pyobject_strongref> localsplus;

    };

    template<typename PyObjectSerializer>
    class PyInterpreterFrameSerdes {
        PyObjectSerializer po_serializer;
        PyCodeObjectSerdes<PyObjectSerializer> code_serializer;

        template<typename Builder>
        flatbuffers::Offset<flatbuffers::Vector<offsets::PyObjectOffset>> serialize_fast_locals_plus(Builder &builder, migrames::PyInterpreterFrame &obj) {
            auto n_locals = utils::py::get_code_nlocals((PyCodeObject*)obj.f_executable.bits);
            std::vector<offsets::PyObjectOffset> localsplus;
            localsplus.reserve(n_locals);

            for(int i = 0; i < n_locals; i++) {
                auto local = obj.localsplus[i];
                PyObject *local_pyobj = (PyObject*)local.bits;
                // a local can be NULL if it has not
                // initialized in the program by this point
                if(NULL == local_pyobj) {
                    continue;
                }

                auto local_ser = po_serializer.serialize(builder, local_pyobj);
                localsplus.push_back(local_ser);
            }
            auto localsplus_offset = builder.CreateVector(localsplus);
            return localsplus_offset;
        }

        public:
        PyInterpreterFrameSerdes(PyObjectSerializer& po_serializer) : 
            po_serializer(po_serializer),
            code_serializer(po_serializer) {}

        template<typename Builder>
        offsets::PyInterpreterFrameOffset serialize(Builder &builder, migrames::PyInterpreterFrame &obj) {
            auto f_executable_ser = code_serializer.serialize(builder, (PyCodeObject*)obj.f_executable.bits);
            auto f_func_obj_ser = po_serializer.serialize(builder, obj.f_funcobj);
            // for now, we can't do this because of the modules
            // auto f_globals_ser = po_serializer.serialize(builder, obj.f_globals);
            // auto f_builtins_ser = po_serializer.serialize(builder, obj.f_builtins);

            auto f_locals_ser = (NULL != obj.f_locals) ? 
                std::optional{po_serializer.serialize(builder, obj.f_locals)} : std::nullopt;

            auto fast_locals_ser = serialize_fast_locals_plus(builder, obj);

            pyframe_buffer::PyInterpreterFrameBuilder frame_builder(builder);
            frame_builder.add_f_executable(f_executable_ser);
            if(f_locals_ser) {
                frame_builder.add_f_locals(f_locals_ser.value());
            }

            frame_builder.add_f_funcobj(f_func_obj_ser);
            frame_builder.add_instr_offset(utils::py::get_instr_offset(obj.frame_obj));
            frame_builder.add_return_offset(obj.return_offset);
            frame_builder.add_owner(obj.owner);
            frame_builder.add_locals_plus(fast_locals_ser);

            return frame_builder.Finish();

        }

        DeserializedPyInterpreterFrame deserialize(const pyframe_buffer::PyInterpreterFrame *obj) {
            DeserializedPyInterpreterFrame deser;
            deser.f_executable = code_serializer.deserialize(obj->f_executable());
            deser.f_funcobj = po_serializer.deserialize(obj->f_funcobj());
            deser.f_globals = po_serializer.deserialize(obj->f_globals());
            deser.f_builtins = po_serializer.deserialize(obj->f_builtins());
            deser.f_locals = po_serializer.deserialize(obj->f_locals());

            deser.instr_offset = obj->instr_offset();
            deser.return_offset = obj->return_offset();
            deser.owner = obj->owner();

            auto localsplus = obj->locals_plus();
            deser.localsplus.reserve(localsplus->size());
            for(auto local : *localsplus) {
                deser.localsplus.push_back(po_serializer.deserialize(local));
            }

            return deser;
        }
    };

    template<typename PyObjectSerializer>
    PyInterpreterFrameSerdes(PyObjectSerializer&) -> PyInterpreterFrameSerdes<PyObjectSerializer>;

    class DeserializedPyFrame {
      public:
        DeserializedPyInterpreterFrame f_frame;
        pyobject_strongref f_trace;
        int f_lineno;
        char f_trace_lines;
        char f_trace_opcodes;
        pyobject_strongref f_extra_locals;
        pyobject_strongref f_locals_cache;
    };

    template<typename PyObjectSerializer>
    class PyFrameSerdes {
        PyObjectSerializer po_serializer;
        PyObjectHeadSerdes<PyObjectSerializer> poh_serializer;
        public:
            PyFrameSerdes(PyObjectSerializer& po_serializer) : 
                          po_serializer(po_serializer),
                          poh_serializer(po_serializer) {}

            template<typename Builder>
            offsets::PyFrameOffset serialize(Builder &builder, migrames::PyFrame &obj) {
                PyInterpreterFrameSerdes interpreter_frame_serializer(po_serializer);
                auto interp_frame_offset = interpreter_frame_serializer.serialize(builder, *obj.f_frame);

                pyframe_buffer::PyFrameBuilder frame_builder(builder);
                // Do NOT serialize the ob_base.
                // frame_builder.add_ob_base(poh_serializer.serialize(builder, &obj.ob_base));

                frame_builder.add_f_frame(interp_frame_offset);
                
                // TODO: These need to be changed to serialize BEFORE
                // creating the frame_builder.
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

            DeserializedPyFrame deserialize(const pyframe_buffer::PyFrame *obj) {
                // auto ob_base_deser = poh_serializer.deserialize(obj->ob_base());
                DeserializedPyFrame deser;
                PyInterpreterFrameSerdes interpreter_frame_serializer(po_serializer);

                deser.f_frame = interpreter_frame_serializer.deserialize(obj->f_frame());

                deser.f_trace = po_serializer.deserialize(obj->f_trace());

                deser.f_lineno = obj->f_lineno();
                deser.f_trace_lines = obj->f_trace_lines();
                deser.f_trace_opcodes = obj->f_trace_opcodes();

                deser.f_extra_locals = po_serializer.deserialize(obj->f_extra_locals());

                deser.f_locals_cache = po_serializer.deserialize(obj->f_locals_cache());
            return deser;
            }
    };

    template<typename PyObjectSerializer>
    PyFrameSerdes(PyObjectSerializer&) -> PyFrameSerdes<PyObjectSerializer>;
    
}

#endif // SERDES_HH_INCLUDED