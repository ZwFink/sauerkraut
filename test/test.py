import sauerkraut as skt
import greenlet
import numpy as np
calls = 0

def test1_fn(c):
    global calls
    calls += 1
    g = 4
    frm_copy = skt.copy_current_frame()
    if calls == 1:
        g = 5
        calls += 1
        hidden_inner = 55
        return frm_copy
    else:
        print(f'calls={calls}, c={c}, g={g}')

    calls = 0
    return 3


def test_copy_then_serialize():
    global calls
    calls = 0
    frm = test1_fn(55)
    serframe = skt.serialize_frame(frm)
    with open('serialized_frame.bin', 'wb') as f:
        f.write(serframe)
    with open('serialized_frame.bin', 'rb') as f:
        read_frame = f.read()
    code = skt.deserialize_frame(read_frame)
    retval = skt.run_frame(code)
    assert retval == 3
    print("Test 'copy_then_serialize' passed")


def test2_fn(c):
    global calls
    calls += 1
    g = 4
    frame_bytes = skt.copy_current_frame(serialize=True)
    if calls == 1:
        g = 5
        calls += 1
        hidden_inner = 55
        return frame_bytes
    else:
        print(f'calls={calls}, c={c}, g={g}')
        retval = calls + c + g
    return retval


def test_combined_copy_serialize():
    global calls
    calls = 0

    frame_bytes = test2_fn(13)
    with open('serialized_frame.bin', 'wb') as f:
        f.write(frame_bytes)
    with open('serialized_frame.bin', 'rb') as f:
        read_frame = f.read()
    retval = skt.deserialize_frame(read_frame, run=True)

    print('Function returned with:', retval)
    assert retval == 19
    print("Test combined_copy_serialize passed")

def for_loop_fn(c):
    global calls
    calls += 1
    g = 4
    print("About to start the loop")

    sum = 0
    for i in range(3):
        for j in range(6):
            sum += 1
            if i == 0 and j == 0:
                print("Copying frame")
                frm_copy = skt.copy_current_frame(serialize=True)
                # 
            if calls == 1:
                g = 5
                calls += 1
                hidden_inner = 55
                return frm_copy
            else:
                print(f'calls={calls}, c={c}, g={g}')

    calls = 0
    return sum


def test_for_loop():
    global calls
    calls = 0

    serframe = for_loop_fn(42)
    with open('serialized_frame.bin', 'wb') as f:
        f.write(serframe)
    with open('serialized_frame.bin', 'rb') as f:
        read_frame = f.read()
    code = skt.deserialize_frame(read_frame)
    iters_run = skt.run_frame(code)

    assert iters_run == 18
    print("Test 'for_loop' passed")
def greenlet_fn(c):
    a = np.array([1, 2, 3])
    greenlet.getcurrent().parent.switch()
    a += 1
    print(f'c={c}, a={a}')
    greenlet.getcurrent().parent.switch()
    return 3

def test_greenlet():
    gr = greenlet.greenlet(greenlet_fn)
    gr.switch(13)
    serframe = skt.copy_frame_from_greenlet(gr, serialize=True)
    with open('serialized_frame.bin', 'wb') as f:
        f.write(serframe)
    with open('serialized_frame.bin', 'rb') as f:
        read_frame = f.read()
    code = skt.deserialize_frame(read_frame)
    gr = greenlet.greenlet(skt.run_frame)
    gr.switch(code)
    print("Test 'greenlet' passed")

def replace_locals_fn(c):
    a = 1
    b = 2
    greenlet.getcurrent().parent.switch()
    return a + b + c

def test_replace_locals():
    gr = greenlet.greenlet(replace_locals_fn)
    gr.switch(13)
    serframe = skt.copy_frame_from_greenlet(gr, serialize=True)
    code = skt.deserialize_frame(serframe)
    res = skt.run_frame(code, replace_locals={'a': 9})
    print(f"The result is {res}")
    assert res == 24

    serframe = skt.copy_frame_from_greenlet(gr, serialize=True)
    code = skt.deserialize_frame(serframe)
    res = skt.run_frame(code, replace_locals={'b': 35})
    print(f"The result is {res}")
    assert res == 49

    serframe = skt.copy_frame_from_greenlet(gr, serialize=True)
    code = skt.deserialize_frame(serframe)
    res = skt.run_frame(code, replace_locals={'a': 9, 'b': 35, 'c': 100})
    print(f"The result is {res}")
    assert res == 144

    print("Test 'replace_locals' passed")


def exclude_locals_greenletfn(c):
    a = 1
    b = 2
    greenlet.getcurrent().parent.switch()
    return a + b + c


def exclude_locals_current_frame_fn(c, exclude_locals=None):
    global calls
    calls += 1
    g = 4
    frame_bytes = skt.copy_current_frame(serialize=True, exclude_locals=exclude_locals)
    if calls == 1:
        g = 5
        calls += 1
        hidden_inner = 55
        return frame_bytes
    else:
        print(f'calls={calls}, c={c}, g={g}')
        retval = calls + c + g
    return retval


def test_exclude_locals_greenlet():
    gr = greenlet.greenlet(exclude_locals_greenletfn)
    gr.switch(13)
    serframe = skt.copy_frame_from_greenlet(gr, serialize=True, exclude_locals={'a'})
    deserframe = skt.deserialize_frame(serframe)
    try:
        res = skt.run_frame(deserframe)
    except TypeError as e:
        print("When you forget to replace an excluded local, 'None' is used in its place!")

    result = skt.deserialize_frame(serframe, replace_locals={'a': 9}, run=True)
    assert result == 24

    gr2 = greenlet.greenlet(exclude_locals_greenletfn)
    gr2.switch(13)
    serframe = skt.copy_frame_from_greenlet(gr2, serialize=True, exclude_locals=['c', 'b'])
    deserframe = skt.deserialize_frame(serframe)
    result = skt.run_frame(deserframe, replace_locals={'c': 100, 'b': 35})
    assert result == 136
    print("Test 'exclude_locals_greenlet' passed")

def test_exclude_locals_current_frame():
    global calls
    calls = 0
    exclude_locals = {'exclude_locals', 'g'}
    frm_bytes = exclude_locals_current_frame_fn(13, exclude_locals)
    result = skt.deserialize_frame(frm_bytes, run=True, replace_locals={'g': 8})
    print(f"The result is {result}")
    assert result == 23

    calls = 0
    exclude_locals = {'exclude_locals', 'c'}
    frm_bytes = exclude_locals_current_frame_fn(13, exclude_locals)
    result = skt.deserialize_frame(frm_bytes, run=True, replace_locals={'c': 100})
    print(f"The result is {result}")
    assert result == 106

    calls = 0
    exclude_locals = {'exclude_locals', 0}
    frm_bytes = exclude_locals_current_frame_fn(13, exclude_locals)
    result = skt.deserialize_frame(frm_bytes, replace_locals={0: 25}, run=True)
    print(f"The result is {result}")
    assert result == 31


    print("Test 'exclude_locals_current_frame' passed")

def test_exclude_locals():
    test_exclude_locals_greenlet()
    test_exclude_locals_current_frame()

test_copy_then_serialize()
test_combined_copy_serialize()
test_for_loop()
test_greenlet()
test_replace_locals()
test_exclude_locals()