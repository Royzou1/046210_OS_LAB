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

typedef struct mpi_poll_entry {
    pid_t pid;
    char incoming;
} poll_entry_t;

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
 * @brief - Check all entries of the poll_pids array (indices 0 to npids-1). For each entry poll_pids[i], if there’s an
            incoming message from poll_pids[i].pid, set poll_pids[i].incoming to 1. Otherwise, set it to zero. If
            none of the given PIDs had an incoming message, go to sleep until a message arrives (and report it in
            the correct poll_pids entry) or until timeout expires.
            A message is considered available by the polling mechanism until mpi_receive is called, which receives
            the message and deletes it from the incoming messages queue. If multiple messages are available after
            waking up, report all of them.To put a process to sleep, you need to force the kernel not to schedule it. One possible way to do this is
            to change the status of the process to TASK_INTERRUPTIBLE and set a timeout after which it will be
            woken. Setting a timeout can be done using the schedule_timeout() function (inside the timer.c file).
            Note that this function calls the schedule() function therefore you should not call it from inside the
            schedule() function itself

 * @param - poll_pids - entry of processes
 *          npids - number of processes i the poll
 *          timeout - amount of time that afterwards we stop the poll
 * @return - on fail -1, on success the number of entries in poll_pids 
 *           with incoming = 1
 * @errno: i. “ETIMEDOUT” (Polling timed out): No message arrived before timeout expired.
           ii. “EFAULT” (Bad address): Error copying poll_pids from or to user space.
           iii. "ENOMEM" (No memory): Error allocating memory.
           iv. “EINVAL” (Invalid argument) npids<1 or timeout<0.
           v. “EPERM” (Operation not permitted): The current process isn’t registered for MPI.
 */
int mpi_poll(struct mpi_poll_entry * poll_pids, int npids, int timeout)
{
    struct task_struct *task = current;
    if(task->comm_on == 0)
    {
        debug_print("mpi_poll - operation not premmited");
        return -EPERM;
    }

    //check arguments
    if(poll_pids == NULL || npids <= 0 || timeout < 0)
    {
        debug_print("args in mpi_poll have problem");
        return -EINVAL;
    }

    // Copy data from user space to kernel space
        kernel_poll_pids = our_kmalloc(npids* sizeof(poll_entry_t), GFP_KERNEL);
        if (!kernel_poll_pids) {
            return -ENOMEM;
        }
    
        if (copy_from_user(kernel_poll_pids, poll_pids, npids * sizeof(poll_entry_t))) {
            kfree(kernel_poll_pids);
            return -EFAULT;
        }
    
    //Look for relavent mesegage and update list
        int num_of_senders = 0;
            for (int i= 0; i < npids ; i++) {
                poll_pids[i].incoming = is_incoming_msg_from(poll_pids[i].pid);
                num_of_senders = (poll_pids[i].incoming) ? 
                                num_of_senders++ : num_of_senders;
            }    
            // FIXME: add scedualer  
        

    // Copy data from kernel space back to user space
        if (copy_to_user(poll_pids, kernel_poll_pids, npids * sizeof(poll_entry_t))) {
            kfree(kernel_poll_pids);
            return -EFAULT;
        }
        our_kfree(kernel_poll_pids);

    //return val
        return (num_of_senders) ? num_of_senders : FAIL;
}

/******  gpt  ******* */
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/module.h>
#include <linux/slab.h>   // For kmalloc and kfree
#include <linux/string.h> // For string functions
#include <linux/uaccess.h> // For copy_from_user and copy_to_user
#include <linux/errno.h>
#include <linux/list.h>
#include <linux/jiffies.h> // For time conversions

#define FAIL -1
#define SUCCESS 0

typedef struct mpi_poll_entry {
    pid_t pid;
    char incoming;
} poll_entry_t;

/**
 * @param pid : the sender of the msg.
 * @return : 1 - if current has incoming msg from pid. 0 - else
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
 * @brief - Check all entries of the poll_pids array (indices 0 to npids-1). For each entry poll_pids[i], if there’s an
            incoming message from poll_pids[i].pid, set poll_pids[i].incoming to 1. Otherwise, set it to zero. If
            none of the given PIDs had an incoming message, go to sleep until a message arrives (and report it in
            the correct poll_pids entry) or until timeout expires.
            A message is considered available by the polling mechanism until mpi_receive is called, which receives
            the message and deletes it from the incoming messages queue. If multiple messages are available after
            waking up, report all of them. To put a process to sleep, you need to force the kernel not to schedule it. One possible way to do this is
            to change the status of the process to TASK_INTERRUPTIBLE and set a timeout after which it will be
            woken. Setting a timeout can be done using the schedule_timeout() function (inside the timer.c file).
            Note that this function calls the schedule() function therefore you should not call it from inside the
            schedule() function itself.

 * @param - poll_pids - entry of processes
 *          npids - number of processes in the poll
 *          timeout - amount of time that afterwards we stop the poll
 * @return - on fail -1, on success the number of entries in poll_pids 
 *           with incoming = 1
 * @errno: i. “ETIMEDOUT” (Polling timed out): No message arrived before timeout expired.
           ii. “EFAULT” (Bad address): Error copying poll_pids from or to user space.
           iii. "ENOMEM" (No memory): Error allocating memory.
           iv. “EINVAL” (Invalid argument) npids<1 or timeout<0.
           v. “EPERM” (Operation not permitted): The current process isn’t registered for MPI.
 */
int mpi_poll(struct mpi_poll_entry *poll_pids, int npids, int timeout)
{
    struct task_struct *task = current;
    if (task->comm_on == 0)
    {
        printk(KERN_ERR "mpi_poll - operation not permitted\n");
        return -EPERM;
    }

    // Check arguments
    if (poll_pids == NULL || npids <= 0 || timeout < 0)
    {
        printk(KERN_ERR "mpi_poll - invalid arguments\n");
        return -EINVAL;
    }

    // Allocate kernel memory for poll_pids
    poll_entry_t *kernel_poll_pids = kmalloc(npids * sizeof(poll_entry_t), GFP_KERNEL);
    if (!kernel_poll_pids)
    {
        return -ENOMEM;
    }

    // Copy data from user space to kernel space
    if (copy_from_user(kernel_poll_pids, poll_pids, npids * sizeof(poll_entry_t)))
    {
        kfree(kernel_poll_pids);
        return -EFAULT;
    }

    int num_of_senders = 0;
    unsigned long timeout_jiffies = msecs_to_jiffies(timeout);
    while (num_of_senders == 0 && timeout_jiffies)
    {
        num_of_senders = 0;
        for (int i = 0; i < npids; i++)
        {
            kernel_poll_pids[i].incoming = is_incoming_msg_from(kernel_poll_pids[i].pid);
            if (kernel_poll_pids[i].incoming)
            {
                num_of_senders++;
            }
        }

        if (num_of_senders == 0)
        {
            set_current_state(TASK_INTERRUPTIBLE);
            timeout_jiffies = schedule_timeout(timeout_jiffies);
        }
    }

    set_current_state(TASK_RUNNING);

    // Copy data from kernel space back to user space
    if (copy_to_user(poll_pids, kernel_poll_pids, npids * sizeof(poll_entry_t)))
    {
        kfree(kernel_poll_pids);
        return -EFAULT;
    }
    kfree(kernel_poll_pids);
    
    if (!num_of_senders)
        return FAIL;
    return num_of_senders;
}
