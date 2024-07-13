#include <linux/kernel.h>
#include <linux/sched.h>
#include <asm/uaccess.h>
#include <linux/errno.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/slab.h>   // For kmalloc and kfree
#include <linux/string.h> // For string functions
#include <linux/helper.h>
//#include <linux/jiffies.h>
#include <linux/delay.h>
#include <linux/init.h>


#define FAIL -1
#define SUCCESS 0
#define DEBUG 1

typedef struct mpi_poll_entry {
    pid_t pid;
    char incoming;
} poll_entry_t;

typedef struct message
{
    pid_t sender_pid;
    size_t size;
    char *data;
    list_t ptr;
} message_t;

/**
 * @param pid : the sender of the msg.
 * @return : 1 - if current has incomming msg from pid. 0 - else
 */
int is_incoming_msg_from(pid_t pid) 
{
    struct task_struct *task = current;
    struct list_head *pos;
    message_t *msg;

    list_for_each(pos, &task->comm_channel) {
        msg = list_entry(pos, message_t, ptr);
        if (msg->sender_pid == pid) {
            return 1;
        }
    }
    return 0;
}

/**
*   @brief: Check all entries of the poll_pids array (indices 0 to npids-1).
*   For each entry poll_pids[i], if there’s an incoming message from poll_pids[i].pid, 
*   set poll_pids[i].incoming to 1. Otherwise, set it to zero. If none of the given PIDs 
*   had an incoming message, go to sleep until a message arrives (and report it in
*   the correct poll_pids entry) or until timeout expires.
*
*   @return:  i. on failure (no messages reported): -1
*             ii.on success: The number of entries in poll_pids with incoming=1. 
*                poll_pids should be updated accordingly.
*
*   @errno: i. “ETIMEDOUT” (Polling timed out): No message arrived before timeout expired.
*           ii. “EFAULT” (Bad address): Error copying poll_pids from or to user space.
*           iii. "ENOMEM" (No memory): Error allocating memory.
*           iv. “EINVAL” (Invalid argument) npids<1 or timeout<0.
*           v. “EPERM” (Operation not permitted): The current process isn’t registered for MPI.
*
*/
int sys_mpi_poll(struct mpi_poll_entry* poll_pids, int npids, int timeout)
{
    debug_print("*** ------------ into polling msg --------------- *** \n");
   
    struct task_struct *c_task  = current;
    //poll_entry_t *poll_pids;

    //checking args
    if(npids < 1 || timeout < 0)
    {
        if(DEBUG) printk(KERN_ERR "Error: npids is %d and timeout is %d\n", npids, timeout);
        return -EINVAL;
    }

    debug_print("*** here1 *** \n");
    //checking if registered for MPI.
    if(!c_task->comm_on)
    {
        if(DEBUG)printk(KERN_ERR "process %d has not registered!\n", c_task->pid);
        return -EPERM;
    }
    debug_print("*** here2 *** \n");
    //copy struct to kernel space 
    poll_entry_t *poll_pids_ker = our_kmalloc(npids * sizeof(poll_entry_t) , GFP_KERNEL);
    if (!poll_pids_ker) {
        return -ENOMEM;
    }

    debug_print("*** here3 *** \n");
    if (copy_from_user(poll_pids_ker , poll_pids , npids * sizeof(poll_entry_t))) {
        our_kfree(poll_pids_ker);
        return -EFAULT;
    }
    
    debug_print("*** here4 *** \n");
    //count the number of senders from list + update list
    int i = 0;
    int num_of_senders = 0;
    for (i = 0 ; i < npids ; i++) { //for each pid in list
        pid_t sender_pid = poll_pids_ker[i].pid;
        if (is_incoming_msg_from(sender_pid)) {
            num_of_senders++;
            poll_pids_ker[i].incoming = 1;
        }
        else {
            poll_pids_ker[i].incoming = 0;
        }
    }
    debug_print("*** here5 *** \n");    
    //if not 0 return skip this section
    //if 0 begin
    if (num_of_senders == 0)
    {
        debug_print("*** here7 *** \n");
        //update the task struct to for the updated poll list.
        if (c_task->poll_list != NULL)
            our_kfree(c_task->poll_list);

        c_task->poll_list_size = npids;
        c_task->poll_list = our_kmalloc(npids * sizeof(int) , GFP_KERNEL);
        if (!c_task->poll_list) {
            our_kfree(poll_pids_ker);
            return -ENOMEM;
        }

        debug_print("*** here8 *** \n");

        for (i = 0 ; i < npids ; i++) {
            c_task->poll_list[i] = poll_pids_ker[i].pid;
        }
        //go to sleep with timeout
        debug_print("*** here9 *** \n");
        unsigned long timeout_jiffies = (timeout * HZ);
        set_current_state(TASK_INTERRUPTIBLE);
        schedule_timeout(timeout_jiffies);
        //woken up by either timeout or other proc  
        //check for messages again
        debug_print("*** here10 *** \n");
        num_of_senders = 0;
        pid_t sender_pid;
        for (i = 0 ; i < npids ; i++) { //for each pid in list
            sender_pid = poll_pids_ker[i].pid;
            if (is_incoming_msg_from(sender_pid)) {
                num_of_senders++;
                poll_pids_ker[i].incoming = 1;
            }
            else {
                poll_pids_ker[i].incoming = 0;
            }
        }
        num_of_senders = (num_of_senders == 0) ? -1 : num_of_senders;

        debug_print("*** here11 *** \n");
        //free list
        c_task->poll_list_size = 0;
        if(c_task->poll_list != NULL) {
            our_kfree(c_task->poll_list);
            c_task->poll_list = NULL;
        }
    }
    
    debug_print("*** here12 *** \n");
    if (DEBUG) { //the returned struct is
        printk(KERN_ERR "Returning the struct:\n"); 
        int k;
        for (k = 0 ; k < npids ; k++) {
            printk(KERN_ERR "PID is: %d , Char is: %c\n", poll_pids_ker[k].pid , poll_pids_ker[k].incoming);
        }
    }

    //copy struct back to user space
    if (copy_to_user(poll_pids, poll_pids_ker, npids* sizeof(poll_entry_t))) {
        our_kfree(poll_pids_ker);
        return -EFAULT;
    }
    our_kfree(poll_pids_ker);
    //return the number of processes
    
    if (DEBUG) { printk(KERN_ERR "Returning with value: %d\n", num_of_senders); }
    debug_print("*** ------------ Done with polling msg --------------- *** \n");

    return num_of_senders;
}
