#ifndef _VEGENERE_H_
#define _VEGENERE_H_

#include <linux/ioctl.h>
#include <linux/types.h>

//
// Function prototypes
//


int my_open(struct inode *, struct file *);

int my_release(struct inode *, struct file *);

ssize_t my_read(struct file *, char *, size_t, loff_t *);

int my_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg);

/*our func*/
loff_t my_llseek(struct file *filp, loff_t offset, int whence);

ssize_t my_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos);

void translate_msg(char *str , int key_offset, int to_encrypt);

int translate_char(char *p , int key_offset, int to_encrypt);

int find_offset(char *p);
/*end of our func*/

#define MY_MAGIC 'r'
#define SET_KEY  _IOW(MY_MAGIC, 0, char*)
#define RESET  _IO(MY_MAGIC, 1)
#define DEBUG  _IOW(MY_MAGIC, 2, int)

#endif
