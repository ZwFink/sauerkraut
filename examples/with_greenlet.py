import sauerkraut
import greenlet
import numpy as np


def fun1(c):
    g = 4
    a = np.array([1, 2, 3])
    greenlet.getcurrent().parent.switch()
    a += 1
    print(f'c={c}, g={g}, a={a}')
    print("Switching out to the parent!")
    greenlet.getcurrent().parent.switch()
    print("Switched back to the parent")

    return 3

f1_gr = greenlet.greenlet(fun1)
f1_gr.switch(13)
serframe = sauerkraut.copy_frame_from_greenlet(f1_gr, serialize=True)
with open('serialized_frame.bin', 'wb') as f:
    f.write(serframe)
with open('serialized_frame.bin', 'rb') as f:
    read_frame = f.read()
code = sauerkraut.deserialize_frame(read_frame)
gr = greenlet.greenlet(sauerkraut.run_frame)
gr.switch(code)
print("Back on the parent, returning to child")
gr.switch(gr)
print("Back on the parent, returned from child")

print('Done')

