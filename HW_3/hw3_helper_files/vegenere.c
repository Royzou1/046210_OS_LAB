/* vegenere.c: Example char device module.
 *
 */
/* Kernel Programming */
#define MODULE
#define LINUX
#define __KERNEL__

#include <linux/kernel.h>  	
#include <linux/module.h>
#include <linux/fs.h>       		
#include <asm/uaccess.h>
#include <linux/errno.h>  
#include <asm/segment.h>
#include <asm/current.h>
#include <linux/slab.h>
//#include <linux/mutex.h>

#include "vegenere.h"

#define MY_DEVICE "vegenere"

#define UPPER_OFFSET 0
#define LOWER_OFFSET 26
#define ZERO_OFFSET 52
#define ABC_LEN 62

#define INVALID -1
#define SUCCESS 0
MODULE_AUTHOR("Anonymous");
MODULE_LICENSE("GPL");

/* globals */
int my_major = 0; /* will hold the major # of my device driver */
size_t key_len = 1; // Length of the encryption key
int debug_mode = 0; // Debug mode flag
//struct mutex my_mutex; // Mutex for synchronization

struct file_operations my_fops = {
    .open = my_open,
    .release = my_release,
    .read = my_read,
    .ioctl = my_ioctl,
    .write = my_write,
    .llseek = my_llseek
};

int init_module(void)
{
   // This function is called when inserting the module using insmod

    my_major = register_chrdev(my_major, MY_DEVICE, &my_fops);

    if (my_major < 0)
    {
	printk(KERN_WARNING "can't get dynamic major\n");
	return my_major;
    }
    return 0;
}


void cleanup_module(void)
{
    // This function is called when removing the module using rmmod

    unregister_chrdev(my_major, MY_DEVICE);
    return;
}


int my_open(struct inode *inode, struct file *filp)
{
    filp->private_data = kmalloc(sizeof(loff_t), GFP_KERNEL);
    if (!filp->private_data) {
        return -ENOMEM;
    }
    *(loff_t *)filp->private_data = 0;

    return 0;
}


int my_release(struct inode *inode, struct file *filp)
{
    kfree(filp->private_data);
    return 0;
}

ssize_t my_read(struct file *filp, char *buf, size_t count, loff_t *f_pos)
{
    ssize_t bytes_read = 0;

    if (*f_pos >= data_size) {
        return 0;
    }

    if (*f_pos + count > data_size) {
        count = data_size - *f_pos;
    }

   // mutex_lock(&my_mutex);

    translate_msg(buffer ,  key_len ,  1, 0);//what key offset need to be here?


    *f_pos += count;
    bytes_read = count;

   // mutex_unlock(&my_mutex);

    return bytes_read;
}

int my_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
    char *new_key;
    size_t new_key_len;

    switch(cmd)
    {
    case SET_KEY:
        if (!arg) {
            return -EINVAL;
        }

        new_key_len = strlen((char *)arg);
        new_key = kmalloc(new_key_len, GFP_KERNEL);
        if (!new_key) {
            return -ENOMEM;
        }

        if (copy_from_user(new_key, (char __user *)arg, new_key_len)) {
            kfree(new_key);
            return -EBADF;
        }

        //mutex_lock(&my_mutex);
        if (key) {
            kfree(key);
        }
        key = new_key;
        key_len = new_key_len;
      //  mutex_unlock(&my_mutex);

        return 0;
    case RESET:
      //  mutex_lock(&my_mutex);
        if (buffer) {
            kfree(buffer);
        }
        buffer_size = PAGE_SIZE;
        buffer = kmalloc(buffer_size, GFP_KERNEL);
        if (!buffer) {
          //  mutex_unlock(&my_mutex);
            return -ENOMEM;
        }
        data_size = 0;

        if (key) {
            kfree(key);
            key = NULL;
            key_len = 0;
        }
     //   mutex_unlock(&my_mutex);

        return 0;
    case DEBUG:
      //  mutex_lock(&my_mutex);
        debug_mode = arg;
     //   mutex_unlock(&my_mutex);

        return 0;
    default:
	return -ENOTTY;
    }
}
loff_t my_llseek(struct file *filp, loff_t offset, int whence)
{
    loff_t new_pos;

    switch(whence) {
    case 0: // SEEK_SET
        new_pos = offset;
        break;
    case 1: // SEEK_CUR
        new_pos = *(loff_t *)filp->private_data + offset;
        break;
    case 2: // SEEK_END
        new_pos = data_size + offset;
        break;
    default:
        return -EINVAL;
    }

    if (new_pos < 0) {
        new_pos = 0;
    }

    if (new_pos > data_size) {
        new_pos = data_size;
    }

    *(loff_t *)filp->private_data = new_pos;

    return new_pos;
}

ssize_t my_write(struct file *filp, const char *buf, size_t buf_len, loff_t *f_pos)
{
    //checking args
    if (!buf || !flip || !f_pos) {
        return -EFAULT;
    }
    if (buf_len < 1) {
        return -EINVAL;
    }

    //copy to buff
    ssize_t bytes_written = 0;
    loff_t curr_fpos = *f_pos
    /*(GPT HELP) copy into buff min(buf_len , curr_fpos to EOF) bytes */
    /*(GPT HELP) add \0 to end what we read (it can be shorter than buf)*/
    
    //translate
    translate_msg(buf , f_pos , 0) //we want to decript
    return bytes_written;
}


int find_offset(char *p) {
    return  ((*p >= '0') && (*p <= '9')) ? *p - '0' + ZERO_OFFSET :
            ((*p >= 'A') && (*p <= 'Z')) ? *p - 'A' + UPPER_OFFSET :
            ((*p >= 'a') && (*p <= 'z')) ? *p - 'a' + LOWER_OFFSET :
            INVALID;
}

int translate_char(char *p, int key_offset, int to_encrypt) {
    int p_old_offset = find_offset(p);
    if (p_old_offset == INVALID)
        return INVALID;
    int p_new_offset =  (to_encrypt) ? (p_old_offset + key_offset) % ABC_LEN :
                                    (p_old_offset - key_offset) % ABC_LEN;
    *p =    (p_new_offset < LOWER_OFFSET) ? 'A' + p_new_offset : //UPPER
            (p_new_offset < ZERO_OFFSET) ? 'a' + p_new_offset - LOWER_OFFSET:
            '0' + p_new_offset - ZERO_OFFSET;
    return SUCCESS;
}

void translate_msg(char *str, int key_offset, int to_encrypt) {
    char* ptr = str;
    int temp_key = (key_offset - 1) % key_size; 
    while (*ptr) {
        translate_char(ptr, temp_key + 1 , to_encrypt);
        temp_key = (temp_key + 1) % key_size;
        ptr++;
    }
    return;
}




////////////////////////////////////////////////////////////////////////////
/* vegenere.c: Example char device module.
 *
 */
/* Kernel Programming */
#define MODULE
#define LINUX
#define __KERNEL__

#include <linux/kernel.h>  	
#include <linux/module.h>
#include <linux/fs.h>       		
#include <asm/uaccess.h>
#include <linux/errno.h>  
#include <asm/segment.h>
#include <asm/current.h>
#include <linux/slab.h>
//#include <linux/mutex.h>

#include "vegenere.h"

#define MY_DEVICE "vegenere"

#define UPPER_OFFSET 0
#define LOWER_OFFSET 26
#define ZERO_OFFSET 52
#define ABC_LEN 62

#define INVALID -1
#define SUCCESS 0
MODULE_AUTHOR("Anonymous");
MODULE_LICENSE("GPL");

/* globals */
int my_major = 0; /* will hold the major # of my device driver */
//char *key = NULL; // Encryption key
size_t key_len = 0; // Length of the encryption key
char *buffer = NULL; // Buffer for storing data
size_t buffer_size = 0; // Size of the buffer
size_t data_size = 0; // Amount of data in the buffer
int debug_mode = 0; // Debug mode flag
//struct mutex my_mutex; // Mutex for synchronization


struct file_operations my_fops = {
    .open = my_open,
    .release = my_release,
    .read = my_read,
    .ioctl = my_ioctl,
    .write = my_write,
    .llseek = my_llseek
};

int init_module(void)
{
   // This function is called when inserting the module using insmod

    my_major = register_chrdev(my_major, MY_DEVICE, &my_fops);

    if (my_major < 0)
    {
	printk(KERN_WARNING "can't get dynamic major\n");
	return my_major;
    }

    buffer_size = PAGE_SIZE; // Initial buffer size
    buffer = kmalloc(buffer_size, GFP_KERNEL);
    if (!buffer) {
        unregister_chrdev(my_major, MY_DEVICE);
        printk(KERN_WARNING "could not allocate buffer\n");
        return -ENOMEM;
    }

    //mutex_init(&my_mutex);

    return 0;
}


void cleanup_module(void)
{
    // This function is called when removing the module using rmmod

    unregister_chrdev(my_major, MY_DEVICE);

    if (buffer) {
        kfree(buffer);
    }

    if (key) {
        kfree(key);
    }

    return;
}


int my_open(struct inode *inode, struct file *filp)
{
    filp->private_data = kmalloc(sizeof(loff_t), GFP_KERNEL);
    if (!filp->private_data) {
        return -ENOMEM;
    }
    *(loff_t *)filp->private_data = 0;

    return 0;
}


int my_release(struct inode *inode, struct file *filp)
{
    kfree(filp->private_data);
    return 0;
}

ssize_t my_read(struct file *filp, char *buf, size_t count, loff_t *f_pos)
{
    ssize_t bytes_read = 0;
    //size_t i;

    if (!buffer) {
        return -EFAULT;
    }

    if (!key) {
        return -EINVAL;
    }

    if (*f_pos >= data_size) {
        return 0;
    }

    if (*f_pos + count > data_size) {
        count = data_size - *f_pos;
    }

   // mutex_lock(&my_mutex);

    translate_msg(buffer ,  key_len ,  1, 0);//what key offset need to be here?


    *f_pos += count;
    bytes_read = count;

   // mutex_unlock(&my_mutex);

    return bytes_read;
}

int my_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
    char *new_key;
    size_t new_key_len;

    switch(cmd)
    {
    case SET_KEY:
        if (!arg) {
            return -EINVAL;
        }

        new_key_len = strlen((char *)arg);
        new_key = kmalloc(new_key_len, GFP_KERNEL);
        if (!new_key) {
            return -ENOMEM;
        }

        if (copy_from_user(new_key, (char __user *)arg, new_key_len)) {
            kfree(new_key);
            return -EBADF;
        }

        //mutex_lock(&my_mutex);
        if (key) {
            kfree(key);
        }
        key = new_key;
        key_len = new_key_len;
      //  mutex_unlock(&my_mutex);

        return 0;
    case RESET:
      //  mutex_lock(&my_mutex);
        if (buffer) {
            kfree(buffer);
        }
        buffer_size = PAGE_SIZE;
        buffer = kmalloc(buffer_size, GFP_KERNEL);
        if (!buffer) {
          //  mutex_unlock(&my_mutex);
            return -ENOMEM;
        }
        data_size = 0;

        if (key) {
            kfree(key);
            key = NULL;
            key_len = 0;
        }
     //   mutex_unlock(&my_mutex);

        return 0;
    case DEBUG:
      //  mutex_lock(&my_mutex);
        debug_mode = arg;
     //   mutex_unlock(&my_mutex);

        return 0;
    default:
	return -ENOTTY;
    }
}
loff_t my_llseek(struct file *filp, loff_t offset, int whence)
{
    loff_t new_pos;

    switch(whence) {
    case 0: // SEEK_SET
        new_pos = offset;
        break;
    case 1: // SEEK_CUR
        new_pos = *(loff_t *)filp->private_data + offset;
        break;
    case 2: // SEEK_END
        new_pos = data_size + offset;
        break;
    default:
        return -EINVAL;
    }

    if (new_pos < 0) {
        new_pos = 0;
    }

    if (new_pos > data_size) {
        new_pos = data_size;
    }

    *(loff_t *)filp->private_data = new_pos;

    return new_pos;
}

ssize_t my_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos)
{
    ssize_t bytes_written = 0;
    size_t i;

    if (!buffer) {
        return -EFAULT;
    }

    if (!key) {
        return -EINVAL;
    }

  //  mutex_lock(&my_mutex);

    if (*f_pos + count > buffer_size) {
        char *new_buffer = krealloc(buffer, *f_pos + count, GFP_KERNEL);
        if (!new_buffer) {
      //      mutex_unlock(&my_mutex);
            printk(KERN_WARNING "could not reallocate buffer\n");
            return -ENOMEM;
        }
        buffer = new_buffer;
        buffer_size = *f_pos + count;
    }

    for (i = 0; i < count; i++) {
        char c;
        if (get_user(c, buf + i)) {
      //      mutex_unlock(&my_mutex);
            return -EBADF;
        }
        if (!debug_mode) {
            int key_offset = find_offset(&key[i % key_len]);
            if (key_offset == INVALID) {
       //         mutex_unlock(&my_mutex);
                return -EFAULT;
            }
            translate_char(&c, key_offset, 1);
        }
        buffer[*f_pos + i] = c;
    }

    *f_pos += count;
    data_size = max(data_size, *f_pos);
    bytes_written = count;

 //   mutex_unlock(&my_mutex);

    return bytes_written;
}


int find_offset(char *p) {
    return  ((*p >= '0') && (*p <= '9')) ? *p - '0' + ZERO_OFFSET :
            ((*p >= 'A') && (*p <= 'Z')) ? *p - 'A' + UPPER_OFFSET :
            ((*p >= 'a') && (*p <= 'z')) ? *p - 'a' + LOWER_OFFSET :
            INVALID;
}

int translate_char(char *p, int key_offset, int to_encrypt) {
    int p_old_offset = find_offset(p);
    if (p_old_offset == INVALID)
        return INVALID;
    int p_new_offset =  (to_encrypt) ? (p_old_offset + key_offset) % ABC_LEN :
                                    (p_old_offset - key_offset) % ABC_LEN;
    *p =    (p_new_offset < LOWER_OFFSET) ? 'A' + p_new_offset : //UPPER
            (p_new_offset < ZERO_OFFSET) ? 'a' + p_new_offset - LOWER_OFFSET:
            '0' + p_new_offset - ZERO_OFFSET;
    return SUCCESS;
}

void translate_msg(char *str, int key_offset, int to_encrypt) {
    char* ptr = str;
    int temp_key = (key_offset - 1) % key_size; 
    while (*ptr) {
        translate_char(ptr, temp_key + 1 , to_encrypt);
        temp_key = (temp_key + 1) % key_size;
        ptr++;
    }
    return;
}