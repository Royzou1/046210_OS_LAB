#!/usr/bin/python

from __future__ import division
from hwgrader.test_utils import memTrack
from hwgrader.test_utils import *
from hwgrader.utils import ParTextTestRunner, ParTestCase
import unittest
import shutil
import sys
import os
import errno
import pickle

EXTENSION_FILES = ('setup.py', 'py_mpi.c')
EXTENSION_LIB = 'pyMpi'
HEADER_FILE = 'mpi_api.h'
TEMP_FOLDER = 'temp'
EXT_SUCCESSFUL = True

class hw_test(ParTestCase):
    """Basic test"""

    def get_missing_pid(self):
        pid_list = os.popen('cd /proc; ls -d [1-9]* | sort -n').read().strip().split('\n')
        pid_list = [ int(p) for p in pid_list ]
        for missing_pid in range(1, 16000):
            if missing_pid not in pid_list:
                break
        return missing_pid

    def setUp(self):
        pass
    
    def tearDown(self):
        pass

    def test_header(self):
        """Verify that the header file compiled successfuly."""

        self.assert_(EXT_SUCCESSFUL, 'Failed compilation of %s.' % HEADER_FILE)
        
    def test_register(self):
        """ Test correct return values of register """
        pyMpi.register()

    def test_send1(self):
        """ Test correct return values of send: not registered"""
        self.errnoCheck(
            cmd=pyMpi.send,
            args=(os.getpid(), "hello1"),
            expected_errno=errno.EPERM,
            msg='send should fail if process is not registered')

    def test_send2(self):
        """ Test correct return values of send: target not registered"""
        pyMpi.register()
        self.errnoCheck(
            cmd=pyMpi.send,
            args=(1, "hello2"),
            expected_errno=errno.EPERM,
            msg='send should fail if target not registered') 
        
    def test_send3(self):
        """ Test correct return values of send: target doesn't exist """
        pyMpi.register()
        self.errnoCheck(
            cmd=pyMpi.send,
            args=(self.get_missing_pid(), "hello3 - here failed"),
            expected_errno=errno.ESRCH,
            msg='send should fail if target pid does not exist')
        
    #def test_send4(self):
    #    """ Test correct return values of send: bad message """
    #    pyMpi.register()
    #    self.errnoCheck(
    #        cmd=pyMpi.send_bad,
    #        args=(os.getpid(),),
    #        expected_errno=errno.EINVAL,
    #        msg='send should fail if message is NULL')
        
    def test_send5(self):
        """ Test correct return values of send: valid """
        pyMpi.register()
        pyMpi.send(os.getpid(), "Hello!")

    def test_receive1(self):
        """ Test correct return values of receive: not registered """
        self.errnoCheck(
            cmd=pyMpi.receive,
            args=(os.getpid(), 100),
            expected_errno=errno.EPERM,
            msg='receive should fail if process is not reigsterd')

    #def test_receive2(self):
    #    """ Test correct return values of receive: bad message size """
    #    pyMpi.register()
    #    pyMpi.send(os.getpid(), "Hello")
    #    self.errnoCheck(
    #        cmd=pyMpi.receive,
    #        args=(os.getpid(), -100),
    #        expected_errno=errno.EINVAL,
    #        msg='receive should fail if message_size is negative')

    def test_receive3(self):
        """ Test correct return values of receive: no message in queue from non-registerd process"""
        pyMpi.register()
        self.errnoCheck(
            cmd=pyMpi.receive,
            args=(1, 100),
            expected_errno=errno.EAGAIN,
            msg='receive should return EAGAIN when there is no incoming message')

    def test_receive4(self):
        """ Test correct return values of receive: no message in queue from non-existing process"""
        pyMpi.register()
        self.errnoCheck(
            cmd=pyMpi.receive,
            args=(self.get_missing_pid(), 100),
            expected_errno=errno.EAGAIN,
            msg='receive should return EAGAIN when there is no incoming message')

    def test_receive5(self):
        """ Test correct return values of receive: no message in queue from registered process"""
        pyMpi.register()
        self.errnoCheck(
            cmd=pyMpi.receive,
            args=(os.getpid(), 100),
            expected_errno=errno.EAGAIN,
            msg='receive should return EAGAIN when there is no incoming message')

    def test_receive6(self):
        """ Test correct return values of receive: buffer > message"""
        message = "Test message"
        pyMpi.register()
        pyMpi.send(os.getpid(), message)
        incoming_message = pyMpi.receive(os.getpid(), 100)

        assert (message == incoming_message)
        self.errnoCheck(
            cmd=pyMpi.receive,
            args=(os.getpid(), 100),
            expected_errno=errno.EAGAIN,
            msg='receive should remove message from queue after reading')

    def test_receive7(self):
        """ Test correct return values of receive: buffer < message"""
        message = "Test message"
        pyMpi.register()
        pyMpi.send(os.getpid(), message)
        incoming_message = pyMpi.receive(os.getpid(), 5)

        assert (message[:5] == incoming_message)
        self.errnoCheck(
            cmd=pyMpi.receive,
            args=(os.getpid(), 100),
            expected_errno=errno.EAGAIN,
            msg='receive should remove message from queue after reading')

    def test_fork1(self):
        """ Test child process inherits registered status and not the message queue """
        message = "Test test"
        pyMpi.register()
        pyMpi.send(os.getpid(), message)
        fork = tfork2()
        if fork.isChild:
            self.errnoCheck(
                cmd=pyMpi.receive,
                args=(fork.ppid, 100),
                expected_errno=errno.EAGAIN,
                msg='child should inherit an empty message queue')
            fork.exit()
        fork.wait()
        incoming_message = pyMpi.receive(fork.ppid, 100)
        assert (message == incoming_message)

    def test_fork2(self):
        """ Test messages are properly sent between father and child """
        message_pp = "Test test"
        message_pc = "Luke, I am your father"
        message_cp = "No!!!"
        pyMpi.register()
        fork = tfork2()
        if fork.isChild:
            fork.sync() # Wait for parent
            pyMpi.send(fork.ppid, message_cp)
            incoming_message = pyMpi.receive(fork.ppid, 100)
            assert (incoming_message == message_pc)
            self.errnoCheck(
                cmd=pyMpi.receive,
                args=(fork.ppid, 100),
                expected_errno=errno.EAGAIN,
                msg='child should have only one message from the parent')
            self.errnoCheck(
                cmd=pyMpi.receive,
                args=(fork.cpid, 100),
                expected_errno=errno.EAGAIN,
                msg='child should have no messages from itself')
            fork.exit()

        pyMpi.send(fork.ppid, message_pp)
        pyMpi.send(fork.cpid, message_pc)
        fork.release()
        fork.wait()
        incoming_message = pyMpi.receive(fork.cpid, 100)
        assert (incoming_message == message_cp)
        incoming_message = pyMpi.receive(fork.ppid, 100)
        assert (incoming_message == message_pp)
        self.errnoCheck(
            cmd=pyMpi.receive,
            args=(fork.ppid, 100),
            expected_errno=errno.EAGAIN,
            msg='parent should have only one message from itself')
        self.errnoCheck(
            cmd=pyMpi.receive,
            args=(fork.cpid, 100),
            expected_errno=errno.EAGAIN,
            msg='parent should have only one messages from the child')

    def test_fork3(self):
        """ Test messages are properly sent between separate processes where source is living"""
        message = "Test test"
        fork = tfork2()
        pyMpi.register()
        if fork.isChild:
            pyMpi.send(fork.ppid, message)
            fork.release()
            fork.sync()
            fork.exit()

        fork.sync()
        incoming_message = pyMpi.receive(fork.cpid, 100)
        fork.release()
        fork.wait()
        assert (incoming_message == message)

    def test_fork4(self):
        """ Test messages are properly sent between separate processes where source is dead """
        message = "Test test"
        fork = tfork2()
        pyMpi.register()
        if fork.isChild:
            pyMpi.send(fork.ppid, message)
            fork.exit()

        fork.wait()
        incoming_message = pyMpi.receive(fork.cpid, 100)
        assert (incoming_message == message)

    def test_message_order(self):
        """ Test messages are received in order + memory leaks """
        mm_track = memTrack()
        mm_track.start_track()
        message1 = "Message 1"
        message2 = "Message 2"
        try:
            fork = tfork2()
            if fork.isChild:
                pyMpi.register()
                pyMpi.send(fork.cpid, message1)
                pyMpi.send(fork.cpid, message2)
                incoming_message = pyMpi.receive(fork.cpid, 100)
                assert (incoming_message == message1)
                fork.exit()
            fork.wait()
        finally:
            mm_track.close()


def compile_extension(test_folder, submission_folder):
            
    global EXT_SUCCESSFUL
    
    #
    # Prepare a temporary folder with all necessary files.
    #
    temp_folder = os.path.join(test_folder, TEMP_FOLDER)
    os.chdir(test_folder)
    if os.path.exists(temp_folder):
        shutil.rmtree(temp_folder, ignore_errors=True)
    os.mkdir(temp_folder)
    
    shutil.copy(os.path.join(submission_folder, HEADER_FILE), temp_folder)
    for file in EXTENSION_FILES:
        shutil.copy(os.path.join(test_folder, file), temp_folder)
    
    #
    # Compile the extension module and import it into the modules namespace
    # Note:
    # I am saving the sys.argv because the run_setup script overwrites them
    # due to a bug
    #
    os.chdir(temp_folder)
    from distutils.core import run_setup
    save_argv = list(sys.argv)
    run_setup('setup.py', script_args=['build_ext', '-b', temp_folder])
    sys.argv = save_argv
    
    if os.path.exists(os.path.join(temp_folder, EXTENSION_LIB + '.so')):
        EXT_SUCCESSFUL = True
        #sys.path.append(temp_folder)
    else:
        EXT_SUCCESSFUL = False
        #sys.path.append(test_folder)

    #
    # Note,
    # In any case I use my extension.
    #
    sys.path.append(test_folder)
    os.chdir(test_folder)

    globals().update({EXTENSION_LIB: __import__(EXTENSION_LIB)})
    del sys.path[-1]


def suite(**args):
    
    #
    # First, compile the extension
    #
    test_folder = os.path.split(args['test_path'])[0]
    submission_folder = args['submission_path']
    compile_extension(test_folder, submission_folder)

    #
    # Return the test suite
    #
    return unittest.makeSuite(hw_test, prefix='test')


if __name__ == "__main__":


    script_path = os.path.abspath(sys.argv[0])
    test_folder = os.path.split(script_path)[0]
    submission_folder = test_folder
    
    #
    # Compile the extension
    #
    compile_extension(test_folder, submission_folder)

    #
    # Run the tests
    #
    unittest.main(testRunner=ParTextTestRunner())


