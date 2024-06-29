#include <linux/kernel.h>
#include <linux/sched.h>
#include <asm/uaccess.h>
#include <linux/errno.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/slab.h>   // For kmalloc and kfree
#include <linux/string.h> // For string functions


#define FAIL -1
#define SUCCESS 0
#define DEBUG 1


static int check_messages(struct task_struct *task, pid_t pid) {
    struct list_head *pos;
    message_t *msg;
    list_for_each(pos, &task->comm_channel) {
        msg = list_entry(pos, message_t, ptr);
        if (msg->sender_pid == pid) {
            return 1; // Message found
        }
    }
    return 0; // No message found
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
int mpi_poll(struct mpi_poll_entry* poll_pids, int npids, int timeout)
{
    task_struct_t *c_task  = current;
    //poll_entry_t *poll_pids;
    int messages_found = 0;

    //checking args
    if(npids < 1 || timeout < 0)
    {
        if(DEBUG) printk(KERN_ERR "Error: npids is %d and timeout is %d\n", npids, timeout);
        return -EINVAL;
    }

    //checking if registered for MPI.
    if(!c_task->comm_on)
    {
        if(DEBUG)printk(KERN_ERR "process %d has not registered!\n", c_task->pid);
        return -EPERM;
    }

    //copy struct to kernel space 
    poll_entry_t *poll_pids_ker = kmalloc(npids * sizeof(poll_entry_t) , GFP_KERNEL);
    if (!poll_pid_ker) {
        return -ENOMEM;
    }
    if (copy_from_user(poll_pids_ker , poll_pids , npids * sizeof(poll_entry_t))) {
        kfree(poll_pids_ker);
        return -EFAULT;
    }
    
    //updating the list
    for(int i = 0; i < npids; i++)
    {
        task_struct_t *task_pid = find_task_by_vpid(poll_pids_ker[i]->pid);
        if(task_pid == NULL)
        {
            if(DEBUG) printk(KERN_ERR "Error: task struct of process %d not found\n",poll_pids[i]->pid);
            return FAIL;
        }
        if(check_messages(poll_pids ,poll_pids_ker[i]->pid))
        {
            poll_pids_ker[i]->incoming = 1;
            messages_found++;
        }
        else
        {
            poll_pids_ker[i]->incoming = 0;
        }
    }
    


    if (messages_found == 0) {
        // No messages found, go to sleep
        int ret;
        DEFINE_WAIT(wait);

        prepare_to_wait(&mpi_wait_queue, &wait, TASK_INTERRUPTIBLE);
        ret = schedule_timeout(timeout);

        if (ret == 0) {
            // Timeout expired
            finish_wait(&mpi_wait_queue, &wait);
            kfree(poll_pids_ker);
            return -ETIMEDOUT;
        }

        // Check again for incoming messages after wake up
        messages_found = 0;
        for (int i = 0; i < npids; i++) {
            task_struct_t *task = find_task_by_vpid(poll_pids_ker[i].pid);
            if (task && check_messages(task, c_task->pid)) {
                poll_pids_ker[i].incoming = 1;
                messages_found++;
            } else {
                poll_pids_ker[i].incoming = 0;
            }
        }
        finish_wait(&mpi_wait_queue, &wait);
    }

    //copy struct to user space
    if (copy_to_user(poll_pids, poll_pids_ker, npids* sizeof(poll_entry_t))) {
        kfree(poll_pids_ker);
        return -EFAULT;
    }
    kfree(poll_pids_ker);

}