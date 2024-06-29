#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/module.h>
#include <asm/current.h>
#include <asm/uaccess.h>
#include <linux/list.h>
#include <linux/slab.h> // For kmalloc and kfree
#include <linux/errno.h> 
#include <linux/string.h> 

#define FAIL -1
#define SUCCESS 0
#define DEBUG 1

typedef struct message
{
    pid_t sender_pid;
    size_t size;
    char *data;
    list_t ptr;
} message_t;


/*
a. Description:
Send a message of size message_size to the process identified by pid.
b. Return value:
i. on error: -1
ii. on success: 0
c. On error errno should contain one of following values:
i. “ESRCH” (No such process): Process pid doesn’t exist
ii. “EPERM” (Operation not permitted): Either the sending process or pid isn’t registered
for MPI communication
iii. “EINVAL” (Invalid argument): message is NULL or message_size < 1
iv. “EFAULT” (Bad address): Error copying message from user space
*/
int sys_mpi_send(pid_t pid, char *message, ssize_t message_size)
{
    if (DEBUG) {
        printk(KERN_ERR "*** ------------ into sending msg --------------- *** \n");
        printk(KERN_ERR "the message is: %s , and it length is: %d\n", message, message_size);
    }
    struct task_struct* receiver = find_task_by_pid(pid);
    struct task_struct* sender = current;

    if (DEBUG) {
        printk(KERN_ERR "sender is: %d, receiver is %d",sender->pid, receiver->pid);
    }

    if (DEBUG) { ////////////
        printk(KERN_ERR "got both task structs \n");
    }
    pid_t my_pid = sender->pid;
    //receiver doesn't exist
    if(receiver == NULL)
    {
        if(DEBUG) printk(KERN_ERR "error in get task_struct on proccess %d \n", pid);
        return -ESRCH;
    }
    
    //sender doesn't exist?
    if(sender == NULL)
    {
        if(DEBUG) printk(KERN_ERR "error in get task_struct on proccess %d \n", my_pid);
        return -ESRCH;
    }

    if (DEBUG) { /////////////
        printk(KERN_ERR "reciever and sender do exist\n");
    }
    
    if(!receiver->comm_on)
    {
        if(DEBUG) printk(KERN_ERR "operation not permit on pid: %d \n", pid);
        return -EPERM;
    }
    if(!sender->comm_on)
    {
        if(DEBUG) printk(KERN_ERR "operation not permit on pid: %d \n", my_pid);
        return -EPERM;
    }

    if (DEBUG) { /////////////
        printk(KERN_ERR "Both comm are on! :)\n");
    }
    
    if(message == NULL || message_size < 1)
    {
        if(DEBUG) printk(KERN_ERR "message invalid on pid: %d \n", my_pid);
        return -EINVAL;
    }
    if (DEBUG) {
        printk(KERN_ERR "msg is valid\n");
    }
    message_t *new_msg;

    new_msg = kmalloc(sizeof(message_t), GFP_KERNEL);
    if (!new_msg) {
        return -ENOMEM;
        //?????????????????
    }

    if (DEBUG) { //////////////
        printk(KERN_ERR "Msg was alocated\n");
    }

    new_msg->sender_pid = sender->pid;
    new_msg->size = message_size; 
    new_msg->data = kmalloc(message_size + 1 , GFP_KERNEL);
    if (!new_msg->data) {
        kfree(new_msg);
        return -ENOMEM;
    }

    if (copy_from_user(new_msg->data, message, message_size)) {
        kfree(new_msg->data);
        kfree(new_msg);
        return -EFAULT;
    }
    new_msg->data[message_size] = '\0'; // Null-terminate the string

    if (DEBUG) { ////////////
        printk(KERN_ERR "msg was created\n");
    }

    INIT_LIST_HEAD(&new_msg->ptr);//why we need it? not in register?
    list_add_tail(&new_msg->ptr, &receiver->comm_channel);  // Add to the end of the list

    if (DEBUG) { ////////////
        printk(KERN_ERR "msg was sent\n");
        printk(KERN_ERR "current reciver list is...\n");

        message_t *msg;
        struct list_head *pos, *n;
        // Iterate over the incoming messages list
        list_for_each_safe(pos, n, &receiver->comm_channel) {
            msg = list_entry(pos, message_t, ptr);
            
            printk(KERN_ERR "msg: %s\n" , msg->data);
        }
    }

    printk(KERN_ERR "*** ------------ leaving sending msg --------------- *** \n");
    return SUCCESS;
}