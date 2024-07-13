#!/usr/bin/python

import os
import pyMpi
import time

def test_fork3():

    pyMpi.register()
    child_pid = os.fork()
    if child_pid > 0:
        # Parent process
        pyMpi.send(child_pid, "Hello Child!")
        print("Parent sent message to Child.")
        time.sleep(1)
        message_a = pyMpi.receive(child_pid, 100)
        message_b = pyMpi.receive(child_pid, 100)
        try:
            message_c = pyMpi.receive(child_pid, 100)
        except:
            print("Error: No message from Child.")
        print("Parent received messages from Child:")
        print(message_a)
        print(message_b)
    elif child_pid == 0:
        # Child process
        parent_pid = os.getppid()
        message = pyMpi.receive(parent_pid, 100)
        print("Child received message from Parent:")
        print(message)
        assert (message == "Hello Child!")
        pyMpi.send(parent_pid, "Hello Parent!")
        pyMpi.send(parent_pid, "How are you?")
        print("Child sent messages to Parent.")
        time.sleep(2)
    else:
        # Fork failed
        print("Fork failed!")

if __name__ == "__main__":
    test_fork3()