#!/usr/bin/python

import os
import pyMpi
import time

def test_fork2():

    child_pid = os.fork()
    if child_pid > 0:
        # Parent process
        time.sleep(1)
        pyMpi.register()
        try:
            message = pyMpi.receive(parent_pid, 100)
        except:
            print("Error: No message from child.")
    elif child_pid == 0:
        # Child process
        parent_pid = os.getppid()
        pyMpi.register()     
        try:
            pyMpi.send(parent_pid, "Hello Parent!")
        except:
            print("Error: Parent process pid isn't registered.")
        time.sleep(2)
    else:
        # Fork failed
        print("Fork failed!")

if __name__ == "__main__":
    test_fork2()
