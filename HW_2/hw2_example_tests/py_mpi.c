#include <Python.h>
#include "mpi_api.h"


static PyObject *
posix_error(void)
{
  return PyErr_SetFromErrno(PyExc_OSError);
}


static PyObject *
py_register(PyObject *self, PyObject *args)
{
  int status;

  status = mpi_register();

  if (status < 0)
    return posix_error();

  Py_INCREF(Py_None);
  return Py_None;
}


static PyObject *
py_send(PyObject *self, PyObject *args)
{
  int pid;
  char *message;
  int message_size;
  int status;

  if (!PyArg_ParseTuple(args, "is#", &pid, &message, &message_size))
    return NULL;

  status = mpi_send(pid, message, message_size);
  
  if (status < 0) {
    return posix_error();
  }

  Py_INCREF(Py_None);
  return Py_None;
}


static PyObject *
py_receive(PyObject *self, PyObject *args)
{
  int pid;
  int size;
  int status;
  char *message;
  PyObject* ret;

  if (!PyArg_ParseTuple(args, "ii", &pid, &size))
    return NULL;

  message = malloc(size);
  if (!message)
      return PyErr_NoMemory();

  status = mpi_receive(pid, message, size);

  if (status < 0) {
    free(message);
    return posix_error();
  }

  ret = Py_BuildValue("s#", message, status);
  free(message);
  return ret;
}

static PyObject *
py_poll(PyObject *self, PyObject *args)
{
    //
    printf("testing print py_poll\n");
    //
    PyObject *pids;
    int timeout, npids;
    struct mpi_poll_entry *poll_entries;
    int incoming_count, i, j;
    PyObject *incoming_pids;

    if (!PyArg_ParseTuple(args, "Oi", &pids, &timeout))
        return NULL;

    npids = PySequence_Size(pids);

    poll_entries = malloc(sizeof(struct mpi_poll_entry) * npids);
    if (!poll_entries)
        return PyErr_NoMemory();

    for (i=0; i<npids; i++) {
        poll_entries[i].pid = (int)PyInt_AsLong(PySequence_GetItem(pids, i));
    }

    incoming_count = mpi_poll(poll_entries, npids, timeout);
    printf("proc1 is: %d, incomming is: %d\n", poll_entries[0].pid , poll_entries[0].incoming);
    if (incoming_count < 0) {
        free(poll_entries);
        return posix_error();
    }

    //
    printf("proc1 is: %d, incomming is: %d\n", poll_entries[0].pid , poll_entries[0].incoming);
    //
    incoming_pids = PyTuple_New(incoming_count);
    if (!incoming_pids) {
        free(poll_entries);
       
        return NULL;
    }
    for (i=0, j=0; i<npids && j<incoming_count; i++) {
        if (poll_entries[i].incoming == 1) {
            if (PyTuple_SetItem(incoming_pids, j, PyInt_FromLong(poll_entries[i].pid)) != 0) {
                free(poll_entries);
                //
                printf("NULL exit : PyTuple\n");
                //
                return NULL;
            }
            j++;
        }
    }
    free(poll_entries);

    //
    printf("incomming pid == %p\n" , incoming_pids);
    //
    printf("proc1 is: %d, incomming is: %d\n", poll_entries[0].pid , poll_entries[0].incoming);
    //
    //


    return incoming_pids;
}

static PyMethodDef msgMethods[] = {
  {"register",  py_register, METH_VARARGS,
   "Register for MPI communication.\n"},
  {"send",  py_send, METH_VARARGS,
   "Send a message to another process.\nExample:\nsend(12, \"Hello!\")\n"},
  {"receive",  py_receive, METH_VARARGS,
   "Receive a message from another process.\nExample:\nreceive(12, 1000) # Read messages of length 1000 from process 12\n"},
  {"poll",  py_poll, METH_VARARGS,
   "Poll for incoming messages from the given PIDs.\nExample:\npids_with_messages = poll([1001,1002,1003], 10) # Wait up to 10s for messages from these PIDs\n"},
  {NULL, NULL, 0, NULL}
};


void
initpyMpi(void)
{
  (void) Py_InitModule("pyMpi", msgMethods);
}
