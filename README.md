# Sauerkraut: Serializing Python's Control State
As `pickle` serializes Python's data state, `sauerkraut` serializes Python's control state.

Concretely, this library equips Python with the ability to stop a running Python function, serialize it to the network or disk, and resume later on. This is useful in different contexts, such as dynamic load balancing and checkpoint/restart.
Sauerkraut is designed to be lightning fast for latency-sensitive HPC applications. Internally, `FlatBuffers` are created to store the serialized state.

This library is still experimental, and subject to change.

## Example
The following example 
```python
import sauerkraut as skt
calls = 0

def fun1(c):
    global calls
    calls += 1
    g = 4
    for i in range(3):
      if i == 0:
          print("Copying frame")
          # copy_frame makes a copy of the current control + data states.
          # When we later run the frame (run_frame), execution will continue
          # directly after the call.
          # Therefore,  line will return twice:
          # The first time when the frame is copied,
          # the second time when the frame is resumed.
          frm_copy = skt.copy_frame()
      # Because copy_frame will return twice, we need to differentiate
      # between the different returns to avoid getting stuck in a loop!
      if calls == 1:
          # The copied frame does not see this write to g
          g = 5
          calls += 1
          # This variable is not serialized,
          # as it's not live
          hidden_inner = 55
          return frm_copy
      else:
          # When running the copied frame, we take this branch.
          # This line will run 3 times
          print(f'calls={calls}, c={c}, g={g}')

    calls = 0
    return 3

frm = fun1(13)
serframe = skt.serialize_frame(frm)
with open('serialized_frame.bin', 'wb') as f:
    f.write(serframe)
with open('serialized_frame.bin', 'rb') as f:
    read_frame = f.read()
code = skt.deserialize_frame(read_frame)
skt.run_frame(code)
```

## Building
Sauerkraut leverages intimate knowledge of CPython internals, and as such is vulnerable to changes in the CPython API and VM.
Currently, Sauerkraut is built for the development version of Python 3.14, and will be ported to earlier versions.
Known to work with CPython on commit `7a2d77c903f29d7ea08b870b8e3fa2130f667a59`'

We recommend to use the Docker container, which can be built or used with:
```bash
# This should take 5-10 minutes
docker build -t sauerkraut -f Dockerfile .
docker run --name="sauerkraut_img" -it library/sauerkraut
# Now we are in the container
cd /sauerkraut/examples
python3 copy_then_serialize.py
```