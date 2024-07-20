To compile the python extension put your 'mpi_api.h' header file in this
folder and type the following command in the terminal:

python setup.py build_ext -b .

If the compilation succeeds a new file will be created: 'pyMpi.so'.
This extension presents four functions that call your new system calls:
1) register
2) send
2) receive

You can use this functions in a python script or directly from the python
interpreter, type 'python' in the terminal and then the following commands:

>>>import pyMpi
>>>import os
>>>pyMpi.register()

The syntax of the command can be found by typing the following in the python
interpreter:

>>>import pyMpi
>>>help(pyMpi.send)

You can also use the ipython interpreter (you can find the rpm package in the
course website). After running ipython (type 'ipython' in the terminal) do:

[1] import pyMpi
[2] pyMpi.mpi_join?
