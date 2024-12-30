# sauerkraut
Migratable Python Frames. Highly experimental, volatile.

Known to work with CPython on commit `7a2d77c903f29d7ea08b870b8e3fa2130f667a59`'
Currently creates a copy of a Python function and returns an object that will resume execution at the next instruction.
```bash
mkdir build && cd build
cmake -DPython_LIBRARY=/u/zanef2/cpython/libpython3.14d.so -DPython_EXECUTABLE=$HOME/cpython/install_dir/bin/python3.14 -DProtobuf_ROOT=/u/zanef2/sauerkraut/spack/var/spack/environments/sauerkraut/.spack-env/view  ..
make
export PYTHONPATH=$PWD
cd ..
$HOME/cpython/python ./examples/copy_then_run.py
```