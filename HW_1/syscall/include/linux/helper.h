<<<<<<< HEAD
#ifndef _LINUX_HELPER_H
#define _LINUX_HELPER_H

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>

#define MAX_PTRS 250
#define DEBUG 1
#define DEBUG_MEM 1

extern int kmallocs_alive;
extern void *ptr_malloced[MAX_PTRS];
extern void *ptr_freed[MAX_PTRS];

void debug_print(char* str);

void *our_kmalloc(size_t size, int flags);

void our_kfree(void *ptr);

void mem_stats();
#endif
=======
#ifndef _LINUX_HELPER_H
#define _LINUX_HELPER_H

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>

#define MAX_PTRS 250
#define DEBUG 1
#define DEBUG_MEM 1

extern int kmallocs_alive;
extern void *ptr_malloced[MAX_PTRS];
extern void *ptr_freed[MAX_PTRS];

void debug_print(char* str);

void *our_kmalloc(size_t size, int flags);

void our_kfree(void *ptr);

void mem_stats();
#endif
>>>>>>> 3ec69fe274ab06ce39e6357348d35a15d3f0e956
