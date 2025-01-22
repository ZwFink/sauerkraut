import sauerkraut as skt
calls = 0

def test1_fn(c):
    global calls
    calls += 1
    g = 4
    frm_copy = skt.copy_frame()
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


def test2_fn(c):
    global calls
    calls += 1
    g = 4
    frame_bytes = skt.copy_frame(serialize=True)
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
                frm_copy = skt.copy_frame(serialize=True)
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
    code = skt.deserialize_frame(read_frame, run=True)
    iters_run = skt.run_frame(code)

    assert iters_run == 18


test_copy_then_serialize()
test_combined_copy_serialize()
test_for_loop()
