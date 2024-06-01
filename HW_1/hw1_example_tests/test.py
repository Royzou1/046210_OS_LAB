#!/usr/bin/python

import os
import pyMpi

def test1():
    """simple test to invoke the syscalls with no error."""
    
    pid = os.getpid()

    # Register
    pyMpi.register()
    
    # Send ourselves some messages. Python automatically sends the size.
    pyMpi.send(pid, "Message 1")
    pyMpi.send(pid, "Message 2")
    
    # Receive one message. Python allocates and returns the buffer.
    message = pyMpi.receive(pid, 100)

    # Make sure the first message was received
    assert (message == "Message 1")

if __name__ == "__main__":
    test1()
