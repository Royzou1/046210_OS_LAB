#!/usr/bin/python

import os
import pyMpi
import time

def test_fork1():

    child_pid = os.fork()
    if child_pid > 0:
        # Parent process
        pyMpi.register()
        try:
            pyMpi.send(child_pid, "Hello Child!")
        except:
            print("Error: Child process pid isn't registered.")
        time.sleep(1)
    elif child_pid == 0:
        # Child process
        parent_pid = os.getppid()
        pyMpi.register()     
        try:
            message = pyMpi.receive(parent_pid, 100)
        except:
            print("Error: No message from parent.")
    else:
        # Fork failed
        print("Fork failed!")

if __name__ == "__main__":
    test_fork1()
