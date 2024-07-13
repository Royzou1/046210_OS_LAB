<<<<<<< HEAD
#include "linux/helper.h"

int kmallocs_alive = 0;
void *ptr_malloced[MAX_PTRS] = {0};
void *ptr_freed[MAX_PTRS] = {0};

void debug_print(char* str) {
    if (DEBUG) {
        printk(KERN_ERR "DEBUG: %s\n", str); 
    }
}

void *our_kmalloc(size_t size, int flags)
{
    static int i = 0;
    void* ptr = kmalloc(size, flags);
    if (ptr) {
        kmallocs_alive++;
        ptr_malloced[i] = ptr;
        i++;
        if (i >= MAX_PTRS)
            printk(KERN_ERR "*****INCREASE MAX_PTR********"); 
        if (DEBUG_MEM) {
            printk(KERN_ERR "MEM_DEBUG: kmalloc: %p\n", ptr); 
        }
    }
    return ptr;
}

void our_kfree(void *ptr)
{
    static int k = 0;
    if (DEBUG_MEM) {
        printk(KERN_ERR "MEM_DEBUG: Kfree: %p\n", ptr); 
    }
    kfree(ptr);
    kmallocs_alive--;
    ptr_freed[k] = ptr;
    k++;
}

void mem_stats()
{
    if (DEBUG_MEM) 
    {        
        printk(KERN_ERR "STATS_MEM: kmallocs_alive are: %d\n", kmallocs_alive); 
        int prob_counter = 0;
        int i, j;
        for (i = 0; i < MAX_PTRS ; i++) {
            void *ptr = ptr_malloced[i];
            if (ptr != 0) {
                int malloced = 0;
                int freed = 0;
                for (j = 0; j < MAX_PTRS ; j++) {
                    if (ptr_malloced[j] == ptr)
                        malloced += 1; 
                }
                for (j = 0; j < MAX_PTRS ; j++) {
                    if (ptr_freed[j] == ptr)
                        freed += 1; 
                }
                if (malloced != freed) {
                    prob_counter++;
                    printk(KERN_ERR "STATS_MEM: The ptr: %p was malloced: %d times and freed %d times\n", ptr, malloced, freed);
                }
            }
        }
        if (prob_counter == 0)
            printk(KERN_ERR "STATS_MEM: ALL GOOD\n");
    }
}
=======
#include "linux/helper.h"

int kmallocs_alive = 0;
void *ptr_malloced[MAX_PTRS] = {0};
void *ptr_freed[MAX_PTRS] = {0};

void debug_print(char* str) {
    if (DEBUG) {
        printk(KERN_ERR "DEBUG: %s\n", str); 
    }
}

void *our_kmalloc(size_t size, int flags)
{
    static int i = 0;
    void* ptr = kmalloc(size, flags);
    if (ptr) {
        kmallocs_alive++;
        ptr_malloced[i] = ptr;
        i++;
        if (i >= MAX_PTRS)
            printk(KERN_ERR "*****INCREASE MAX_PTR********"); 
        if (DEBUG_MEM) {
            printk(KERN_ERR "MEM_DEBUG: kmalloc: %p\n", ptr); 
        }
    }
    return ptr;
}

void our_kfree(void *ptr)
{
    static int k = 0;
    if (DEBUG_MEM) {
        printk(KERN_ERR "MEM_DEBUG: Kfree: %p\n", ptr); 
    }
    kfree(ptr);
    kmallocs_alive--;
    ptr_freed[k] = ptr;
    k++;
}

void mem_stats()
{
    if (DEBUG_MEM) 
    {        
        printk(KERN_ERR "STATS_MEM: kmallocs_alive are: %d\n", kmallocs_alive); 
        int prob_counter = 0;
        int i, j;
        for (i = 0; i < MAX_PTRS ; i++) {
            void *ptr = ptr_malloced[i];
            if (ptr != 0) {
                int malloced = 0;
                int freed = 0;
                for (j = 0; j < MAX_PTRS ; j++) {
                    if (ptr_malloced[j] == ptr)
                        malloced += 1; 
                }
                for (j = 0; j < MAX_PTRS ; j++) {
                    if (ptr_freed[j] == ptr)
                        freed += 1; 
                }
                if (malloced != freed) {
                    prob_counter++;
                    printk(KERN_ERR "STATS_MEM: The ptr: %p was malloced: %d times and freed %d times\n", ptr, malloced, freed);
                }
            }
        }
        if (prob_counter == 0)
            printk(KERN_ERR "STATS_MEM: ALL GOOD\n");
    }
}
>>>>>>> 3ec69fe274ab06ce39e6357348d35a15d3f0e956
