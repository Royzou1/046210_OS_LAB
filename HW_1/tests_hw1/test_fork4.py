#!/usr/bin/python

import os
import pyMpi
import time

def test_fork4():

    grandparent_pid = os.getpid()
    pyMpi.register()
    child_pid = os.fork()
    if child_pid > 0:
        # Parent process
        time.sleep(1)
        my_grandchild_pid = pyMpi.receive(child_pid, 100)
        print("Grandparent received Grandchild's pid from Child:")
        print(my_grandchild_pid)
        message = pyMpi.receive(int(my_grandchild_pid), 100)
        print("Grandparent received message from Grandchild:")
        print(message)
        message = pyMpi.receive(child_pid, 100)
        print("Grandparent received message from Child:")
        print(message)
        pyMpi.send(int(my_grandchild_pid), "I am your Grandparent!")
        print("Grandparent sent message to Grandchild.")
        time.sleep(4)
    elif child_pid == 0:
        # Child process
        parent_pid = os.getpid()
        grandchild_pid = os.fork()
        if grandchild_pid > 0:
            # Child process
            pyMpi.send(grandparent_pid, str(grandchild_pid))
            print("Child sent Grandchild's pid to Parent.")
            pyMpi.send(grandparent_pid, "I am your Child!")
            print("Child sent message to Parent.")
            time.sleep(3)
        elif grandchild_pid == 0:
            # Grandchild process
            pyMpi.send(grandparent_pid, "I am your Grandchild!")
            print("Grandchild sent message to Grandparent.")
            time.sleep(2)
            message = pyMpi.receive(grandparent_pid, 100)
            print("Grandchild received message from Grandparent:")
            print(message)
        else:
            # Fork failed
            print("Fork failed!") 
    else:
        # Fork failed
        print("Fork failed!")

if __name__ == "__main__":
    test_fork4()