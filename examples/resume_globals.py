import sauerkraut as skt
import greenlet
import numpy as np


def main():
    with open('serialized_globals_example.bin', 'rb') as f:
        serialized = f.read()
    result = skt.deserialize_frame(serialized, run=True)
    print(result)

if __name__ == "__main__":
    main()
