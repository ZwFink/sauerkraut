#ifndef OFFSETS_HH_INCLUDED
#define OFFSETS_HH_INCLUDED
#include "flatbuffers/flatbuffers.h"
#include "py_object_generated.h"
#include "py_var_object_head_generated.h"
#include "py_object_head_generated.h"
#include "py_code_object_generated.h"
#include "py_frame_generated.h"
#include "py_interpreter_frame_generated.h"

namespace offsets {
    using PyObjectOffset = flatbuffers::Offset<pyframe_buffer::PyObject>;
    using PyObjectHeadOffset = flatbuffers::Offset<pyframe_buffer::PyObjectHead>;
    using PyVarObjectHeadOffset = flatbuffers::Offset<pyframe_buffer::PyVarObjectHead>;
    using PyCodeObjectOffset = flatbuffers::Offset<pyframe_buffer::PyCodeObject>;
    using PyFrameOffset = flatbuffers::Offset<pyframe_buffer::PyFrame>;
    using PyInterpreterFrameOffset = flatbuffers::Offset<pyframe_buffer::PyInterpreterFrame>;
}

#endif // OFFSETS_HH_INCLUDED