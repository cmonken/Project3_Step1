#include <unistd.h>
#include <linux/sched.h>
#include <string.h>
#include <stdio.h>
#define __NR_my_ps 359
struct my_mem_addr
{
	int task_pid;
	int present;
	unsigned long long phy_addr;
	unsigned long swap_id;
}my_mem_addr;

int main()
{	
    int pid;
    unsigned long mem_addr;
	int i=0;
	int num =0; 
	struct my_mem_addr userbuff[1];

	syscall (__NR_get_addr,pid,mem_addr);
	
    if (my_mem_addr[0].virt_addr != NULL)
	    printf("physical memory address: %llu\n", my_mem_addr[0].phy_addr);
    else if (my_mem_addr[0].swap_id != NULL)
	    printf("swap identifier: %lu\n", my_mem_addr[0].swap_id);
	else if (my_mem_addr[0].present == 1)
	    printf("not available\n");
	
	return 0;
}

