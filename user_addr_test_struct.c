// testing the kernel system call
// Created by Jiongming Fan
#include <linux/stdio.h>
#include <linux/unistd.h>

#define __NR_my_syscall 361

struct my_mem_addr
{
	int present;    // 1 = phy addr, 2 = swap id, 3 = not available
	unsigned long long return_data;
}my_mem_addr;

main(int argc, char * argv[])
{

    int pid = atoi(argv[1]);
    long vr_addr = atol(argv[2]);
   	int i=0;
	int num =0; 
    struct my_mem_addr userbuff[1];


	printf("Program PID = %d with virtual address of %d selected",pid,vr_addr);
	long phy_addr = syscall(__NR_my_syscall, pid , vr_addr);

	//dealing with kernel return values
	if (my_mem_addr[0].present == 1)
    {
        // add conversion to hex before printing
		printf("PID: %d	Physical Address Found:0x%lx\n",pid,my_mem_addr[0].return_data);
	    printf("physical memory address: %llu\n", );
    }
    else if (my_mem_addr[0].present == 2)
    {
	    printf("PID: %d Swap Identifier Found: %llu\n", pid,my_mem_addr[0].return_data);
	}
	else // my_mem_addr[0].present == 3
	{
	    printf("Could Not Found The Page Selected.NOT AVAILABLE\n");
	}
	return 0;
}

