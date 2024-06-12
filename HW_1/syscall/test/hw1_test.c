#include <stdio.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "mpi_api.h"

int main()
{
	int res = -1;
    pid_t pid = getpid();
    char message[100];

	// Register for MPI
	res = mpi_register();
	assert(res == 0);

	// Send ourselves some messages. Note that size does not include the NULL character.
	res = mpi_send(pid, "Message 1", 9);
	assert(res == 0);
	res = mpi_send(pid, "Message 2", 9);
	assert(res == 0);

	// Read a message
	res = mpi_receive(pid, message, 100);
	assert(res == 9);
    // Messages are read in order, so message should be 'Message 1'
    assert(message[8] == '1');

    // Exit, remaining message should be deleted
	return 0;
}
