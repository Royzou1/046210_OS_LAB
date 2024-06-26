#include <linux/kernel.h>
#include <linux/sched.h>
#include <asm/uaccess.h>
#include <linux/errno.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/slab.h>   // For kmalloc and kfree
#include <linux/string.h> // For string functions
#include <linux/helper.h>


#define FAIL -1
#define SUCCESS 0
#define DEBUG 1


typedef struct message {
    pid_t sender_pid;
    size_t size;
    char *data;
    struct list_head ptr;
} message_t;

/**
 * @brief Check for message from process pid. 
 *        If there is, copy it to the buffer message with size message_size and 
 *        delete it from the incoming messages queue. Messages are processed in 
 *        the order they were received. If the message is longer than message_size,
 *        copy only the first message_size bytes and delete the message from the queue.
 * @return FAIL: -1 ; SUCCESS: The size of the string copied to message.
 * @errno: i. “EPERM” (Operation not permitted): The current process isn’t 
 *            registered for MPI communication
 *         ii. “EINVAL” (Invalid argument): message is NULL or message_size < 1
 *         iii. “EAGAIN” (Resource temporarily unavailable): No message found from pid
 *         iv. “EFAULT” (Bad address): error writing to user buffer
 */
int sys_mpi_receive(pid_t pid, char* message, ssize_t message_size) {
    printk(KERN_ERR "*** ------------ into receiving msg --------------- *** \n");
     
    struct task_struct *task = current;
        if (!task) {
            if (DEBUG) { printk(KERN_ERR "didn't find task_struct: %d\n", -ENOMEM); }
            return FAIL;
        }
        if (DEBUG) {
            printk(KERN_ERR "I am: %d, receiving from: %d\n", task->pid, pid);
        }
    //1 - not in communication
    if (task->comm_on == 0) {
        return -EPERM;
        //errno = EPERM;
        //return -1;
    }

    //2 - not valid
    if (message_size < 1 || message == NULL) {
        return -EINVAL;
    }

    message_t *msg;
    struct list_head *pos, *n;
    int copied_size;
    if (DEBUG) { printk(KERN_ERR "start to search in the list \n"); }
    // Iterate over the incoming messages list
    list_for_each_safe(pos, n, &task->comm_channel) {
        msg = list_entry(pos, message_t, ptr);
        if (DEBUG) { 
            printk(KERN_ERR "current sender is: %d\n",msg->sender_pid); 
        }

        if (msg->sender_pid == pid) {
            if (DEBUG) { printk(KERN_ERR "in if loop, found message with good pid\n"); }
            
            // Determine the number of bytes to copy
            copied_size = min(msg->size, (size_t)message_size);

            // Copy the message to the user buffer
            if (copy_to_user(message, msg->data, copied_size)) {
                return -EFAULT;
            }

            // Null-terminate the string if there's space
            message[copied_size] = '\0';
            

            // Remove the message from the list
            list_del(&msg->ptr);

            // Free the message data and structure
            our_kfree(msg->data);
            
            our_kfree(msg);
            return copied_size;  // Return the size of the copied message
        }
        if (DEBUG) { 
            printk(KERN_ERR "not the right sender"); 
        }
    }

    // No message found from the given pid
    if (DEBUG) { 
            printk(KERN_ERR "did not find the right sender"); 
        }
    return -EAGAIN; 
    }
    