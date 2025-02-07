import os
import greenlet

# Get the directory containing the greenlet package
greenlet_dir = os.path.dirname(greenlet.__file__)
print(greenlet_dir, end='') 