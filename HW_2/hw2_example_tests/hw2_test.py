#!/usr/bin/python

import os
import errno
import pyMpi
import time

def testErrno() :
    print("**********************starting test Errno*************************")

    ppid = os.getpid()

    try:
        poll_ret = pyMpi.poll([ppid + 10], 5)
    except Exception, ex:
        assert (ex.errno == errno.EPERM)

    pyMpi.register()
    try:
        poll_ret = pyMpi.poll([ppid], 5)
    except Exception, ex:
        assert (ex.errno == errno.ETIMEDOUT)
    
    try:
        poll_ret = pyMpi.poll([], 5)
    except Exception, ex:
        assert (ex.errno == errno.EINVAL)

    try:
        poll_ret = pyMpi.poll([ppid], -1)
    except Exception, ex:
        assert (ex.errno == errno.EINVAL)
    print("**********************End of test Errno*************************")

    

def test1():
    print("**********************starting test 1******************************")
    """ Verify a process waits until a new message arrives """
    print("entering test1 verifing that when a message arrived the poll wont sleep and done")
    ppid = os.getpid()
    pyMpi.register()
    cpid = os.fork()
    if cpid == 0:
        # In child: Send a message to our parent after a short wait
        time.sleep(3)
        pyMpi.send(ppid, "Hello!")
        os._exit(0)
    # In parent: poll for new messages from the child. Python converts
    # struct mpi_poll_entry to a list of PIDs with incoming messages.
    s = time.time()
    poll_ret = pyMpi.poll([cpid], 15)
    print("after poll_ret ---")
    e = time.time()
    elapsed = e - s
    print('elapsed time is: ', elapsed)
    # Elasped time should be about the same as the child's wait time
    print("type of poll_ret is: ", type(poll_ret),"     ")
    assert (elapsed > 2 and elapsed < 4)
    print("poll_ret[0] is:", poll_ret[0])
    assert poll_ret[0] == cpid
    print("test1 completed succesfully")
    _, status = os.waitpid(cpid, 0)  # Wait for child1 to finish
    print("**********************end of test 1******************************")

def test2():
    """ The proc doest wake with nothing """
    print("**********************starting test 2******************************")
    print("--- process need to wait till timeout ---")
    ppid = os.getpid()
    pyMpi.register()

    cpid1 = os.fork()
    if cpid1 == 0:
        time.sleep(1)
        pyMpi.send(ppid, "Hello!")
        os._exit(0)

    cpid2 = os.fork()
    if cpid2 == 0:
        time.sleep(5)
        os._exit(0)

    s = time.time()
    try:
        poll_ret = pyMpi.poll([cpid2], 5)
    except Exception, ex:
        assert (ex.errno == errno.ETIMEDOUT)

    e = time.time()
    elapsed = e - s
    print(' time passed in test 2 is ' , elapsed)
    assert (elapsed > 4 and elapsed < 6)
    print(' time passed in test 2 is ' , elapsed)
    print('------ test 2 completed successfuly ---')
    _, status = os.waitpid(cpid1, 0)  # Wait for child1 to finish
    _, status = os.waitpid(cpid2, 0)  # Wait for child1 to finish
    print("**********************end of test 2******************************")
    
def test3():
    print("**********************starting test 3******************************")
    ppid = os.getpid()
    pyMpi.register()
    
    cpid1 = os.fork()
    if cpid1 == 0:
        pyMpi.send(ppid, "Hello1!")
        os._exit(0)
    
    cpid2 = os.fork()
    if cpid2 == 0:
        pyMpi.send(ppid, "Hello2!")
        os._exit(0)

    time.sleep(1)
    s = time.time()
    poll_ret = pyMpi.poll([cpid1 , cpid2], 5)
    e = time.time()
    elapsed = e - s
    assert (elapsed < 1)
    assert (poll_ret[0] == cpid1)
    assert (poll_ret[1] == cpid2)
    print("**********************end of test 3******************************")
    _, status = os.waitpid(cpid1, 0)  # Wait for child1 to finish
    _, status = os.waitpid(cpid2, 0)  # Wait for child1 to finish

def test4():
    print("**********************starting test 4******************************")
    """ not looking for all msges """
    ppid = os.getpid()
    pyMpi.register()
    cpid1 = os.fork()
    cpid2 = os.fork()
    if cpid1 == 0:
        time.sleep(1)
        pyMpi.send(ppid, "Hello1!")
        os._exit(0)
    if cpid2 == 0:
        time.sleep(3)
        pyMpi.send(ppid, "Hello2!")
        os._exit(0)
    s = time.time()
    poll_ret = pyMpi.poll([cpid2], 5)
    e = time.time()
    elapsed = e - s
    assert (elapsed > 2 and elapsed < 4)
    assert (poll_ret[0] == cpid2)
    _, status = os.waitpid(cpid1, 0)  # Wait for child1 to finish
    _, status = os.waitpid(cpid2, 0)  # Wait for child1 to finish
    print("**********************end of test 4******************************")

def test5():
    print("**********************starting test 5******************************")
    """ not looking for all msges """
    ppid = os.getpid()
    pyMpi.register()
    cpid1 = os.fork()
    if cpid1 == 0:
        pyMpi.send(ppid, "Hello1!")
        os._exit(0)
    _, status = os.waitpid(cpid1, 0)  # Wait for child1 to finish
    
    time.sleep(1)
    s = time.time()
    try:
        poll_ret = pyMpi.poll([cpid1], 5)
    except Exception, ex:
        print("The exception is: " , ex.errno)
    
    e = time.time()
    elapsed = e - s
    print('elapsed is: ' , elapsed)
    
    print("**********************end of test 5******************************")


def test6():
    print("**********************starting test 6******************************")
    """ Reciving fro,m dead proc and alive proc """
    ppid = os.getpid()
    pyMpi.register()
    cpid1 = os.fork()
    cpid2 = os.fork()
    if cpid1 == 0:
        pyMpi.send(ppid, "Hello1!")
        os._exit(0)
   
    if cpid2 == 0:
        pyMpi.send(ppid, "Hello2!")
        time.sleep(7)
        os._exit(0)
    
    _, status = os.waitpid(cpid1, 0)  # Wait for child1 to finish
    time.sleep(1)
    try:
        poll_ret = pyMpi.poll([cpid1, cpid2] , 5)
    except Exception, ex:
        print("The exception is: " , ex.errno)
    assert (poll_ret[0] == cpid1)
    assert (poll_ret[1] == cpid2)

    _, status = os.waitpid(cpid2, 0)  # Wait for child1 to finish
    print("**********************end of test 6******************************")

def test7():
    print("**********************starting test 7******************************")
    """ Reciving fro,m dead proc and alive proc """
    ppid = os.getpid()
    pyMpi.register()
    cpid1 = os.fork()
    cpid2 = os.fork()
    if cpid1 == 0:
        time.sleep(5)
        pyMpi.send(ppid, "Hello1!")
        os._exit(0)
   
    if cpid2 == 0:
        time.sleep(1)
        pyMpi.send(ppid, "Hello2!")
        os._exit(0)
    
    try:
        poll_ret = pyMpi.poll([cpid1, cpid2] , 2)
    except Exception, ex:
        print("The exception is: " , ex.errno)
    assert (poll_ret[0] == cpid2)
    pyMpi.receive(poll_ret[0] , 100)

    try:
        poll_ret = pyMpi.poll([cpid1, cpid2] , 3)
    except Exception, ex:
        assert (ex.errno == errno.ETIMEDOUT)
    
    try:
        poll_ret = pyMpi.poll([cpid1, cpid2] , 3)
    except Exception, ex:
        print("The exception is: " , ex.errno)
    assert (poll_ret[0] == cpid1)

    _, status = os.waitpid(cpid2, 0)  # Wait for child1 to finish
    print("**********************end of test 7******************************")

if __name__ == "__main__":
    print("**********************tests**************************************")
    testErrno()
    test1()
    test2()
    test3()
    test4()
    test5()
    test6()
    test7()
