
#include "mpi_api.h"

#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define DEBUG 1
#define FAIL -1
#define SUCCESS 0 

/*****************************global*****************************************/
bool registered = false;
/*******************function implementation**********************************/
struct msg {
    pid
    message_size
    msg
}

int mpi_register(void) {
    registered = true;
    pid_t my_pid = getpid();
    char* pipe_name[256];
    pipe_name += "pipe_";
    pipe_name += (char*)my_pid;
    struct stat st;

    if(stat(pipe_name, &st) == 0)
    {
        if(S_ISFIFO(st.st_mode))
        {
            if(DEBUG)printf("Named pipe %s exist. \n", pipe_name);
            return SUCCESS;
        }
        else
            printf("Named pipe %s does not exist.\n", pipe_name);
    }

    int res = mkfifo(pipe_name, 0666);
    if(res == 0) return SUCCESS;

    if(DEBUG) perror("mkfifo failed");

    if(errno == ENOMEM) 
        printf("something wrong dont care, roy says not enough memory\n");


}

int mpi_send(pid_t pid, char *message, ssize_t message_size)
{
    char pipe_name[256];
    pipe_name+="pipe_";
    pipe_name+=(char*)pid;

    ssize_t message_size = strlen(message) + 1;
    int fd = open(pipe_name, O_WRONLY);
    if (fd == -1) {
        perror("open");
        return FAIL;
    }
    ssize_t bytes_written = write(fd, message, message_size);
    if (bytes_written == -1) {
        perror("write");
        close(fd);
        return FAIL;
    }

    // Close the file descriptor for the named pipe
    close(fd);


    switch (errno) {
    case ESRCH:
        printf("Process pid doesn’t exist\n");
        return FAIL;

    case EPRM:
        printf("Operation not permitted\n");
        return FAIL;

    case EINVAL:
        printf("message is NULL or message_size < 1\n");
        return FAIL;

    case EFAULT:
        printf("Error copying message from user space\n");
        return FAIL;
    case DEFAULT:
        break;
    } 
    return SUCCESS;  
}


int mpi_receive(pid_t pid, char* message, ssize_t message_size)
{
    //check for msgs in my pipe.
}


