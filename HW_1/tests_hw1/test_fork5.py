#!/usr/bin/python

import os
import pyMpi
import time

def test_fork5():

    parent_pid = os.getpid()
    pyMpi.register()
    first_child_pid = os.fork()
    if first_child_pid > 0:
        # Parent process
        second_child_pid = os.fork()
        if second_child_pid > 0:
            # Parent process
            pyMpi.send(first_child_pid, str(second_child_pid))
            print("Parent sent Second Child's pid to First Child.")
            pyMpi.send(first_child_pid, "I am your Parent!")
            print("Parent sent message to First Child.")
            time.sleep(3)
        elif second_child_pid == 0:
            # Second child process
            pyMpi.send(first_child_pid, "I am your Younger Sibling!")
            print("Younger Sibling sent message to Older Sibling.")
            time.sleep(2)
            message = pyMpi.receive(first_child_pid, 100)
            print("Younger Sibling received message from Older Sibling:")
            print(message)
        else:
            # Fork failed
            print("Fork failed!")
    elif first_child_pid == 0:
        # First child process
        time.sleep(1)
        younger_sibling_pid = pyMpi.receive(parent_pid, 100)
        print("First Child received Younger Sibling's pid from Parent:")
        print(younger_sibling_pid)
        message = pyMpi.receive(int(younger_sibling_pid), 100)
        print("Older Sibling received message from Younger Sibling:")
        print(message)
        message = pyMpi.receive(parent_pid, 100)
        print("First Child received message from Parent:")
        print(message)
        pyMpi.send(int(younger_sibling_pid), "I am your Older Sibling!")
        print("Older Sibling sent message to Younger Sibling.")
    else:
        # Fork failed
        print("Fork failed!")

if __name__ == "__main__":
    test_fork5()