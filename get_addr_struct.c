//**************CSE430 Project 3******************
//Created by Jiongming Fan, Martin Kuna, Chris Monken

#include <linux/kernel.h>
#include <linux/init>
#include <linux/syscalls.h>
#include <asm/pgtable.h>  //pte_none, pte-presemt or clear or bad
#include <swap.h>
#include <swapops.h>
#include <linux/highmem.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <asm/uaccess.h>
#include <linux/module>

#define PAGE_SHIFT 12

struct my_mem_addr
{
	int present;    // 1 = phy addr, 2 = swap id, 3 = not available
	unsigned long long return_data;
}my_mem_addr;

asmlinkage long get_addr(int pid, unsigned long vr_addr)
{
    struct task_struct *task;
    struct my_mem_addr kernel_buff[1];

    unsigned long phy_page_frame;
    unsigned long offset;
    unsigned long shifted_frame;
    unsigned long phy_addr;
    int num = 0;
    int len = 0;
    int found = 0;
    int i = 1, n = 0;

    pgd_t *pgd;
    pud_t *pud;
    pmd_t *pmd;
    pte_t *ptep,pte;
    spinlock_t *ptl;
    swp_entry_t swp;

    for_each_process(task)
    {
        if(pid == task->pid) // found the matching pid, checking
        {
            //walk thru  page global directory
            pgd = pgd_offset(task->mm, vr_addr);
            if(pgd_none(*pgd)||unlikely(pgd_bad(*pgd)))
                kernel_buffer[0].present = 3;    // not found
            //walk thru page upper directory
            pud = pud_offset(pgd,vr_addr);
            if(pud_none(*pud)||unlikely(pud_bad(*pud)))
                kernel_buffer[0].present = 3;    // not found
            //walk thru page middle directory
            pmd = pmd_offset(pud,vr_addr);
            if(pmd_none(*pmd)||unlikely(pmd_bad(*pmd)))	
                kernel_buffer[0].present = 3;    // not found
            //walk thru page table entry
            //ptep = pte_offset(pmd,vr_addr);
            ptep = pte_offset_map_lock(task->mm,pmd,vr_addr,&ptl);	
            pte = *ptep;

            //check if pte is available
		    if(!pte_present(pte))
            {
                if(pte_none(pte))
                    kernel_buffer[0].present = 3;    // not found
                else    // updated return method for correct unsigned long long swap id
                {
                    swp = pte_to_swp_entry(pte);
                    kernel_buffer[0].return_data = swp_offset(swp);
                    kernel_buffer[0].present = 2;    // swap id present
                }
            }
            else    // if it's available prepare to return phy_addr
            {
                phy_pframe = pte_pfn(pte);
                offset = (vr_addr & 0x0000FFF);
                shifted_frame = phy_frame << PAGE_SHIFT;
                phy_addr = shifted_frame||offset;
                kernel_buffer[0].return_data = phy_addr;
                kernel_buffer[0].present = 1;    // phy addr present
            }
            pte_unmap_unlock(ptep,ptl);
        }
        else
            kernel_buffer[0].present = 3;         // pid not found
    }
    len = sizeof(my_mem_addr);
    n = copy_to_user(address,kernel_buff,len);

    return i;
//if pte_present() gives you false, just print out the value of pte. That is the sawp id
}

