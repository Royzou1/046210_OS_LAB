#!/usr/bin/python

import os
import pyMpi

def test1():
    father_pid = os.getpid()
    pyMpi.register()
    pid = os.fork()
    if pid == 0:  # Child process
        pyMpi.send(father_pid, 'Message 1')
        os._exit(0)  # Exit child process
    else:  # Parent process
        _, status = os.waitpid(pid, 0)  # Wait for child process to finish



if __name__ == "__main__":
    test1()
