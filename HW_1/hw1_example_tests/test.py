#!/usr/bin/python

import os
import pyMpi
import errno
import time

def test0():
    print("-------------------------starting test0--------------------------\n")
    father_pid = os.getpid()
    pyMpi.register()
    pid = os.fork()
    
    if pid == 0:  # Child process
        ret_val = pyMpi.send(os.getpid(), "son to son")
        msg = pyMpi.receive(os.getpid(), 100)
        print(msg)
        os._exit(0)
    else:  # Parent process
        ret_val = pyMpi.send(father_pid, "father to father")
        msg = pyMpi.receive(father_pid, 100)
        print(msg)
        _, status = os.waitpid(pid, 0)  # Wait for child process to finish

def test1():
    print("-----------------------starting test1----------------------------\n")
    father_pid = os.getpid()
    pyMpi.register()
    pid = os.fork()
    if pid == 0:  # Child process
        time.sleep(1)
        message = pyMpi.receive(father_pid, 100)
        print(message)
        os._exit(0)  # Exit child process
    else:  # Parent process
        ret_val = pyMpi.send(pid, "father to son")
        _, status = os.waitpid(pid, 0)  # Wait for child process to finish
        
def test2():
    print("-----------------------starting test2----------------------------\n")
    father_pid = os.getpid()
    pyMpi.register()
    pid = os.fork()
    if pid == 0:  # Child process
        ret_val = pyMpi.send(father_pid, "son to papa")
        os._exit(0)  # Exit child process
    else:  # Parent process
        time.sleep(1)
        message = pyMpi.receive(pid, 100)
        print(message)
        _, status = os.waitpid(pid, 0)  # Wait for child process to finish

def test3():
    print("-----------------------starting test3----------------------------\n")
    father_pid = os.getpid()
    pyMpi.register()
    #Child 1
    pid1 = os.fork()
    pid2 = os.fork()
    if pid1 == 0:  # Child process 1
        pyMpi.send(father_pid, "son: 1, msg: 1")
        time.sleep(2)
        pyMpi.send(father_pid, "son: 1, msg: 2")
        time.sleep(2)
        pyMpi.send(father_pid, "son: 1, msg: 3")
        time.sleep(2)
        pyMpi.send(father_pid, "son: 1, msg: 4")
        os._exit(0)  # Exit child process
    elif pid2 == 0: # Second child
        time.sleep(1)
        pyMpi.send(father_pid, "son:2,  msg: 1")
        time.sleep(2)
        pyMpi.send(father_pid, "son:2,  msg: 2")
        time.sleep(2)
        pyMpi.send(father_pid, "son:2,  msg: 3")
        time.sleep(2)
        pyMpi.send(father_pid, "son:2,  msg: 4")
        os._exit(0)  # Exit child process
    else:
        _, status = os.waitpid(pid1, 0)  # Wait for child1 to finish
        _, status = os.waitpid(pid2, 0)  # Wait for child2 to finish
        message = pyMpi.receive(pid2, 100)
        print(message)
        message = pyMpi.receive(pid2, 100)
        print(message)
        message = pyMpi.receive(pid1, 100)
        print(message)
        message = pyMpi.receive(pid2, 100)
        print(message)
        message = pyMpi.receive(pid1, 100)
        print(message)
        message = pyMpi.receive(pid1, 100)
        print(message)
        message = pyMpi.receive(pid2, 100)
        print(message)
        message = pyMpi.receive(pid1, 100)
        print(message)

def basic():
    print("-----------------------starting basic----------------------------\n")
    pid = os.getpid()

    # Check failures before registration
    print(104)
    try:
        pyMpi.send(pid + 1000, "Failed Message")
    except:
        print("Error: Process pid doesn't exist.")
        
    print(110)
    try:
        pyMpi.send(pid, "Failed Message")
    except:
        print("Error: Process pid isn't registered.")
        
    print(116)
    try:
        message = pyMpi.receive(pid, 100)
    except:
        print("Error: Process pid isn't registered.")
    print(121)
    
    # Register
    pyMpi.register()
    # Double registration
    pyMpi.register()

    # Check failures after registration and before sending messasges
    try:
        pyMpi.send(pid, "")
    except:
        print("Error: message_size < 1")
    print(133)
    try:
        message = pyMpi.receive(pid, 100)
    except:
        print("Error: No message found from pid.")
    print(138)
    # Sending messages:
    # Sending one message -> Trying to receive two
    pyMpi.send(pid, "Message 1")
    try: 
        message = pyMpi.receive(pid, 0)
    except:
        print("Error: message_size < 1")
    print(146)
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

def test_fork1():
    print("----------------------- fork1 ----------------------------\n")
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
        
def test_fork2():
    print("-----------------------starting fork2----------------------------\n")
    child_pid = os.fork()
    if child_pid > 0:
        # Parent process
        time.sleep(1)
        pyMpi.register()
        try:
            message = pyMpi.receive(parent_pid, 100)
        except:
            print("gdcsage from child.")
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

def test_fork3():
    print("-----------------------starting fork3----------------------------\n")
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

def test_fork4():
    print("-----------------------starting fork4----------------------------\n")
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

def test_fork5():
    print("-----------------------starting fork5----------------------------\n")
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
    print("********************starting new tests**************************\n")
    test0()
    test1()
    test2()
    test3()

