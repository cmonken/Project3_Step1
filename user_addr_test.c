// testing the kernel system call
// Created by Jiongming Fan
#include <linux/stdio.h>
#include <linux/unistd.h>

#define __NR_my_syscall 361

main(int argc, char * argv[])
{

	int pid = atoi(argv[1]);
	long vr_addr = atol(argv[2]);

	printf("Program PID = %d with virtual address of %d selected",pid,vr_addr);
	long phy_addr = syscall(__NR_my_syscall, pid , vr_addr);

	//dealing with kernel return values
	if(phy_addr == -1)
		printf("Could Not Found The Page Selected.NOT AVAILABLE\n");
	else 
		printf("PID: %d	Physical Address Found:0x%lx\n",pid,phy_addr);
	

	return 0;



}

