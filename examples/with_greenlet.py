import sauerkraut
import greenlet
calls = 0


def fun1(c):
    g = 4
    greenlet.getcurrent().parent.switch()
    print(f'calls={calls}, c={c}, g={g}')
    print("Switching out to the parent!")
    greenlet.getcurrent().parent.switch()
    print("Switched back to the parent")

    return 3

def greenlet_resume(deserframe):
    sauerkraut.run_frame(code)

f1_gr = greenlet.greenlet(fun1)
f1_gr.switch(13)
serframe = sauerkraut.copy_frame_from_greenlet(f1_gr, serialize=True)
with open('serialized_frame.bin', 'wb') as f:
    f.write(serframe)
with open('serialized_frame.bin', 'rb') as f:
    read_frame = f.read()
code = sauerkraut.deserialize_frame(read_frame)
gr = greenlet.greenlet(greenlet_resume)
gr.switch(code)
print("Back on the parent, returning to child")
gr.switch(gr)
print("Back on the parent, returned from child")

print('Done')

