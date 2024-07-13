<<<<<<< HEAD
#!/usr/bin/python

import os
import pyMpi
import errno

def test1():
    """Simple test to send and receive messages."""
    pid = os.getpid()
    
    # Register
    pyMpi.register()
    
    # Send ourselves some messages. Python automatically sends the size.
    pyMpi.send(pid, "Message 1")
    pyMpi.send(pid, "Message 2")
    pyMpi.send(pid, "Message 3")
    pyMpi.send(pid, "Message 4")
    
    # Receive messages and validate content
    message = pyMpi.receive(pid, 100)
    print(message)
    assert (message == "Message 1")
    
    message = pyMpi.receive(pid, 100)
    print(message)
    assert (message == "Message 2")
    
    message = pyMpi.receive(pid, 100)
    print(message)
    assert (message == "Message 3")
    
    message = pyMpi.receive(pid, 100)
    print(message)
    assert (message == "Message 4")

def test_read_no_msg():
    """Test reading with no messages and wrong sender PID."""
    pid = os.getpid()
    pyMpi.register()
    
    result = pyMpi.receive(pid, 100)
    assert result is None, "Test read no message: Failed - received {}".format(result)

    wrong_pid = pid + 1  # Assuming wrong_pid is not sending
    result = pyMpi.receive(wrong_pid, 100)
    assert result is None, "Test read wrong PID: Failed - received {}".format(result)

def test_register():
    """Test register functionality."""
    pid = os.getpid()
    pyMpi.register()

    # Registering again should have no effect
    pyMpi.register()
    print("Test register twice: Passed")

    pid = os.fork()
    if pid == 0:  # Child process
        pyMpi.register()
        pyMpi.send(os.getpid(), "Child Message")
        print("Child process registered and sent message")
        os._exit(0)
    else:  # Parent process
        os.wait()  # Wait for child to finish
        print("Parent process: Child process test complete")

def test_send():
    """Test sending messages with various error conditions."""
    invalid_pid = -1
    pyMpi.register()

    result = pyMpi.send(invalid_pid, "Test")
    assert result == -errno.ESRCH, "Test send ESRCH error: Failed - received {}".format(result)

    result = pyMpi.send(os.getpid(), None)
    assert result == -errno.EINVAL, "Test send EINVAL error (None): Failed - received {}".format(result)

    result = pyMpi.send(os.getpid(), "")
    assert result == -errno.EINVAL, "Test send EINVAL error (empty): Failed - received {}".format(result)

def test_receive():
    """Test receiving messages with various error conditions."""
    pid = os.getpid()
    pyMpi.register()
    
    result = pyMpi.receive(pid, 100)
    assert result is None, "Test receive EAGAIN error: Failed - received {}".format(result)

    result = pyMpi.receive(pid, 0)
    assert result is None, "Test receive EINVAL error: Failed - received {}".format(result)

def test_1proc():
    """Test message order and deletion in a single process."""
    pid = os.getpid()
    pyMpi.register()
    
    pyMpi.send(pid, "Message 1")
    pyMpi.send(pid, "Message 2")
    
    message1 = pyMpi.receive(pid, 100)
    print(message1)
    assert message1 == "Message 1"
    
    message2 = pyMpi.receive(pid, 100)
    print(message2)
    assert message2 == "Message 2"
    
    result = pyMpi.receive(pid, 100)
    assert result is None, "Test 1proc empty buffer: Failed - received {}".format(result)

def test_mul_proc():
    """Test multiple processes sending and receiving messages."""
    def sender(pid, message):
        pyMpi.send(pid, message)
    
    pyMpi.register()
    
    for i in range(2):
        pid = os.fork()
        if pid == 0:
            pyMpi.register()
            sender(os.getppid(), "Message from child {}".format(i))
            os._exit(0)
    
    os.wait()
    os.wait()
    
    parent_pid = os.getpid()
    
    received_messages = []
    for _ in range(2):
        received_messages.append(pyMpi.receive(parent_pid, 100))
    
    print("Received messages:", received_messages)
    
    assert "Message from child 0" in received_messages
    assert "Message from child 1" in received_messages

    result = os.system("kill -0 %d" % pid)
    assert result != 0, "Child process death handling: Failed - received {}".format(result)

# List of test functions to execute
tests = [
    test1,
    test_read_no_msg,
    test_register,
    test_send,
    test_receive,
    test_1proc,
    test_mul_proc
]

if __name__ == "__main__":
    for test in tests:
        pid = os.fork()
        if pid == 0:  # Child process
            test()
            os._exit(0)  # Exit child process
        else:  # Parent process
            _, status = os.waitpid(pid, 0)  # Wait for child process to finish

""""

import os
import sys



# Test functions that will be run in child processes
def test_function_1():
    # Simulate test function 1
    return 0

def test_function_2():
    # Simulate test function 2
    return 1

# Mapping from test function names to actual function objects
TEST_FUNCTIONS = {
    'test_function_1': test_function_1,
    'test_function_2': test_function_2,
    # Add more tests as needed
}

# Define your library with test functions and expected return values
EXIT_VAL = {
    'test_function_1': 0,
    'test_function_2': 1,
    # Add more tests as needed
}

def run_test(test_function):
    if test_function in TEST_FUNCTIONS:
        return_code = os.fork()
        if return_code == 0:
            # In child process
            exit_code = TEST_FUNCTIONS[test_function]()
            sys.exit(0)
        else:
            # In parent process
            _, status = os.wait()
            if os.WIFEXITED(status):
                return os.WEXITSTATUS(status)
            else:
                return -1  # handle abnormal termination
    else:
        print(f"Test function '{test_function}' not found.")
        return -1

def main():
    # Example usage
    tests = ['test_function_1', 'test_function_2', 'non_existing_test']

    for test in tests:
        print(f"Running test '{test}'...")
        actual_return = run_test(test)
        if test in TEST_FUNCTIONS and actual_return == EXIT_VAL[test]:
            print("Succeeded!")
        else:
            print("Failed!")
        

if __name__ == "__main__":
    main()
=======
#!/usr/bin/python

import os
import pyMpi
import errno

def test1():
    """Simple test to send and receive messages."""
    pid = os.getpid()
    
    # Register
    pyMpi.register()
    
    # Send ourselves some messages. Python automatically sends the size.
    pyMpi.send(pid, "Message 1")
    pyMpi.send(pid, "Message 2")
    pyMpi.send(pid, "Message 3")
    pyMpi.send(pid, "Message 4")
    
    # Receive messages and validate content
    message = pyMpi.receive(pid, 100)
    print(message)
    assert (message == "Message 1")
    
    message = pyMpi.receive(pid, 100)
    print(message)
    assert (message == "Message 2")
    
    message = pyMpi.receive(pid, 100)
    print(message)
    assert (message == "Message 3")
    
    message = pyMpi.receive(pid, 100)
    print(message)
    assert (message == "Message 4")

def test_read_no_msg():
    """Test reading with no messages and wrong sender PID."""
    pid = os.getpid()
    pyMpi.register()
    
    result = pyMpi.receive(pid, 100)
    assert result is None, "Test read no message: Failed - received {}".format(result)

    wrong_pid = pid + 1  # Assuming wrong_pid is not sending
    result = pyMpi.receive(wrong_pid, 100)
    assert result is None, "Test read wrong PID: Failed - received {}".format(result)

def test_register():
    """Test register functionality."""
    pid = os.getpid()
    pyMpi.register()

    # Registering again should have no effect
    pyMpi.register()
    print("Test register twice: Passed")

    pid = os.fork()
    if pid == 0:  # Child process
        pyMpi.register()
        pyMpi.send(os.getpid(), "Child Message")
        print("Child process registered and sent message")
        os._exit(0)
    else:  # Parent process
        os.wait()  # Wait for child to finish
        print("Parent process: Child process test complete")

def test_send():
    """Test sending messages with various error conditions."""
    invalid_pid = -1
    pyMpi.register()

    result = pyMpi.send(invalid_pid, "Test")
    assert result == -errno.ESRCH, "Test send ESRCH error: Failed - received {}".format(result)

    result = pyMpi.send(os.getpid(), None)
    assert result == -errno.EINVAL, "Test send EINVAL error (None): Failed - received {}".format(result)

    result = pyMpi.send(os.getpid(), "")
    assert result == -errno.EINVAL, "Test send EINVAL error (empty): Failed - received {}".format(result)

def test_receive():
    """Test receiving messages with various error conditions."""
    pid = os.getpid()
    pyMpi.register()
    
    result = pyMpi.receive(pid, 100)
    assert result is None, "Test receive EAGAIN error: Failed - received {}".format(result)

    result = pyMpi.receive(pid, 0)
    assert result is None, "Test receive EINVAL error: Failed - received {}".format(result)

def test_1proc():
    """Test message order and deletion in a single process."""
    pid = os.getpid()
    pyMpi.register()
    
    pyMpi.send(pid, "Message 1")
    pyMpi.send(pid, "Message 2")
    
    message1 = pyMpi.receive(pid, 100)
    print(message1)
    assert message1 == "Message 1"
    
    message2 = pyMpi.receive(pid, 100)
    print(message2)
    assert message2 == "Message 2"
    
    result = pyMpi.receive(pid, 100)
    assert result is None, "Test 1proc empty buffer: Failed - received {}".format(result)

def test_mul_proc():
    """Test multiple processes sending and receiving messages."""
    def sender(pid, message):
        pyMpi.send(pid, message)
    
    pyMpi.register()
    
    for i in range(2):
        pid = os.fork()
        if pid == 0:
            pyMpi.register()
            sender(os.getppid(), "Message from child {}".format(i))
            os._exit(0)
    
    os.wait()
    os.wait()
    
    parent_pid = os.getpid()
    
    received_messages = []
    for _ in range(2):
        received_messages.append(pyMpi.receive(parent_pid, 100))
    
    print("Received messages:", received_messages)
    
    assert "Message from child 0" in received_messages
    assert "Message from child 1" in received_messages

    result = os.system("kill -0 %d" % pid)
    assert result != 0, "Child process death handling: Failed - received {}".format(result)

# List of test functions to execute
tests = [
    test1,
    test_read_no_msg,
    test_register,
    test_send,
    test_receive,
    test_1proc,
    test_mul_proc
]

if __name__ == "__main__":
    for test in tests:
        pid = os.fork()
        if pid == 0:  # Child process
            test()
            os._exit(0)  # Exit child process
        else:  # Parent process
            _, status = os.waitpid(pid, 0)  # Wait for child process to finish

""""

import os
import sys



# Test functions that will be run in child processes
def test_function_1():
    # Simulate test function 1
    return 0

def test_function_2():
    # Simulate test function 2
    return 1

# Mapping from test function names to actual function objects
TEST_FUNCTIONS = {
    'test_function_1': test_function_1,
    'test_function_2': test_function_2,
    # Add more tests as needed
}

# Define your library with test functions and expected return values
EXIT_VAL = {
    'test_function_1': 0,
    'test_function_2': 1,
    # Add more tests as needed
}

def run_test(test_function):
    if test_function in TEST_FUNCTIONS:
        return_code = os.fork()
        if return_code == 0:
            # In child process
            exit_code = TEST_FUNCTIONS[test_function]()
            sys.exit(0)
        else:
            # In parent process
            _, status = os.wait()
            if os.WIFEXITED(status):
                return os.WEXITSTATUS(status)
            else:
                return -1  # handle abnormal termination
    else:
        print(f"Test function '{test_function}' not found.")
        return -1

def main():
    # Example usage
    tests = ['test_function_1', 'test_function_2', 'non_existing_test']

    for test in tests:
        print(f"Running test '{test}'...")
        actual_return = run_test(test)
        if test in TEST_FUNCTIONS and actual_return == EXIT_VAL[test]:
            print("Succeeded!")
        else:
            print("Failed!")
        

if __name__ == "__main__":
    main()
>>>>>>> 3ec69fe274ab06ce39e6357348d35a15d3f0e956
"""""