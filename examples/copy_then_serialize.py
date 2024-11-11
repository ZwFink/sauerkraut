import migrames
import sys
from io import BytesIO
calls = 0

def do_run_frame(frm):
    return_val = migrames.run_frame(frm)
    print('return_val = ', return_val)

def fun1(c):
    global calls
    print("Running with calls == ", calls)
    if calls == 0:
        calls += 1
        a = 5
        b = 45
        c = c + 10
        print('Calling inner')
        frm_copy = migrames.copy_frame()
        print('returning from outer')
        x = 45
        if calls == 1:
            calls += 1
            hidden_inner = 55
            return frm_copy
        x += 1
        print(f'inner, a={a}, b={b}, c={c}')
        return x

frm = fun1(13)
serframe = migrames.serialize_frame(frm)
with open('serialized_frame', 'wb') as f:
    f.write(serframe)
with open('serialized_frame', 'rb') as f:
    read_frame = f.read()
migrames.deserialize_frame(read_frame)



