#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/module.h>
#include <asm/current.h>
#include <asm/uaccess.h>
#include <linux/list.h>
#include <linux/kernel.h>
#include <linux/slab.h> // For kmalloc and kfree
#include <linux/errno.h> 


#define FAIL -1
#define SUCCESS 0
#define DEBUG 1

typedef struct message
{
    pid_t seder_pid;
    size_t size;
    char *data;
    list_t ptr;
} message_t;


/**
 * @brief - registers the following process to the communication.
 * 1. changes bool task_struct -> comm_on = true;
 * 2. reseting the task_struct -> comm_channel ; allocate mem and assign the
 *    head of the list (node)
 * @return FAIL: -1 ; SUCCESS: 0 
 * on failure, errno: ENOMEM
*/
int sys_mpi_register(void)
{
    task_t* p = current;
    pid_t pid =  p->pid;
    if(p == NULL)
    {
        if(DEBUG) printk(KERN_ERR "error in get task_struct on proccess %d \n", pid);
    }
    if(p->comm_on)
    {
        if(DEBUG)printk(KERN_ERR "the proccess %d is already registered\n", pid);
        return SUCCESS;
    }
    
    p->comm_on = 1;
   
    INIT_LIST_HEAD(&p->comm_channel);

    if (!list_empty(&p->comm_channel)) {
        if(DEBUG) printk(KERN_ERR "list isnt empty on init: %d\n", -ENOMEM);
        return -ENOMEM;
    }
   
    return SUCCESS;
}
    
