# migrames
Migratable Python Frames. Highly experimental, volatile.

Known to work with CPython on commit `7a2d77c903f29d7ea08b870b8e3fa2130f667a59`'
Currently creates a copy of a Python function and returns an object that will resume execution at the next instruction.
```bash
gcc mymodule.c -shared -fPIC -L$HOME/cpython/ -lpython3.14d -pthread -lm -lutil -ldl -I/u/zanef2/cpython/install_dir/include/python3.14d -o mymodule.so
export PYTHONPATH=$PWD
$HOME/cpython/python ./examples/copy_then_run.py
```