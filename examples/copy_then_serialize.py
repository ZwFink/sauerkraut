import migrames
import sys
calls = 0

def fun2(g):
    print(g)

def fun1(c):
    global calls
    print("Running with calls == ", calls)
    if calls == 0:
        calls += 1
        a = 5
        b = 45
        c = c + 10
        x = bytes([1, 2, 3, 4, 5])
        print('Calling inner')
        print('is my offset change messed up?')
        frm_copy = migrames.copy_frame()
        print('returning from outer')
        if calls == 1:
            calls += 1
            print("Making a copy of the frame the first time")
            hidden_inner = 55
            return frm_copy
        print(x)
        frm_copy = migrames.copy_frame()
        if calls == 2:
            calls += 1
            hidden_inner = 55
            print("Making a copy of the frame the second time")
            return frm_copy
        print(f'inner, a={a}, b={b}, c={c}, x={x}')
        print("What happens")
        print("f I change som estuff here?")

        return x

frm = fun1(13)
serframe = migrames.serialize_frame(frm)
with open('serialized_frame.bin', 'wb') as f:
    f.write(serframe)
with open('serialized_frame.bin', 'rb') as f:
    read_frame = f.read()
code = migrames.deserialize_frame(read_frame)
frm2 = migrames.run_deserialized_frame(code)
fun2(35)
serfrm2 = migrames.serialize_frame(frm2)
code2 = migrames.deserialize_frame(serfrm2)
migrames.run_deserialized_frame(code2)

print('Done')

