#!/usr/bin/python

import os
import errno
import pyMpi
import time

def test1():
    """ Verify a process waits until a new message arrives """
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
    poll_ret = pyMpi.poll([cpid], 10)
    e = time.time()
    elapsed = e - s
    # Elasped time should be about the same as the child's wait time
    assert (elapsed > 2 and elapsed < 4)
    assert poll_ret[0] == cpid

if __name__ == "__main__":
    test1()
