#!/usr/bin/python

import os
import pyMpi

# Check register, send and receive (one process)
def test_basics():
    pid = os.getpid()

    # Check failures before registration
    try:
        pyMpi.send(pid+1000, "Failed Message")
    except:
        print("Error: Process pid doesn't exist.")
    try:
        pyMpi.send(pid, "Failed Message")
    except:
        print("Error: Process pid isn't registered.")
    try:
        message = pyMpi.receive(pid, 100)
    except:
        print("Error: Process pid isn't registered.")
    
    # Register
    pyMpi.register()
    # Double registration
    pyMpi.register()

    # Check failures after registration and before sending messasges
    try:
        pyMpi.send(pid, "")
    except:
        print("Error: message_size < 1")
    try:
        message = pyMpi.receive(pid, 100)
    except:
        print("Error: No message found from pid.")

    # Sending messages:
    # Sending one message -> Trying to receive two
    pyMpi.send(pid, "Message 1")
    try: 
        message = pyMpi.receive(pid, 0)
    except:
        print("Error: message_size < 1")
    message = pyMpi.receive(pid, 100)
    assert (message == "Message 1")
    try:
        message = pyMpi.receive(pid, 100)
    except:
        print("Error: No message found from pid.")

    # Send->Send->Receive->Send->Receive->Receive->Receive
    pyMpi.send(pid, "Message 2")
    pyMpi.send(pid, "Message 3")
    message = pyMpi.receive(pid, 100)
    try:
        assert (message == "Message 1")
    except:
        print("Error: message != 'Message 1'.")
    assert (message == "Message 2")
    pyMpi.send(pid, "Message 4")
    message = pyMpi.receive(pid, 100)
    assert (message == "Message 3")
    message = pyMpi.receive(pid, 100)
    assert (message == "Message 4")
    try:
        message = pyMpi.receive(pid, 100)
    except:
        print("Error: No message found from pid.")
    
    # Receive a part of a message
    pyMpi.send(pid, "Half Message")
    message = pyMpi.receive(pid, 6)
    assert (message == "Half M")
    try:
        message = pyMpi.receive(pid, 6)
    except:
        print("Error: No message found from pid.")

    return 0

if __name__ == "__main__":
    test_basics()
