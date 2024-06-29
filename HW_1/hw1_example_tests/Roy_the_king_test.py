#!/usr/bin/python

import os
import pyMpi
import errno

def test1():
    
    father_pid = os.getpid()
    pyMpi.register()
    pid = os.fork()
    if pid == 0:  # Child process
        pyMpi.send(father_pid, 'Message 1')
        os._exit(0)  # Exit child process
    else:  # Parent process
        _, status = os.waitpid(pid, 0)  # Wait for child process to finish
        message = pyMpi.receive(pid, 100)
        print(message)
        assert (message == "Message 1")


if __name__ == "__main__":
    test1()
