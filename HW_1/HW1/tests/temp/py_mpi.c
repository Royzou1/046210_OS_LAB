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


static PyMethodDef msgMethods[] = {
  {"register",  py_register, METH_VARARGS,
   "Register for MPI communication.\n"},
  {"send",  py_send, METH_VARARGS,
   "Send a message to another process.\nExample:\nsend(12, \"Hello!\")\n"},
  {"receive",  py_receive, METH_VARARGS,
   "Receive a message from another process.\nExample:\nreceive(12, 1000) # Read messages of length 1000 from process 12\n"},
  {NULL, NULL, 0, NULL} 
};


void
initpyMpi(void)
{
  (void) Py_InitModule("pyMpi", msgMethods);
}
