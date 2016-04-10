//**************CSE430 Project 3******************
//Created by Jiongming Fan, Martin Kuna, Chris Monken

#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <asm/pgtable.h>  //pte_none, pte-presemt or clear or bad
#include <swap.h>
#include <swapops.h>
#include <linux/highmem.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <asm/uaccess.h>
#include <linux/init>
#include <linux/module>

#define PAGE_SHIFT 12

asmlinkage long get_addr(int pid, unsigned long vr_addr)
{
    struct task_struct *task;

    unsigned long phy_page_frame;
    unsigned long offset;
    unsigned long shifted_frame;
    unsigned long phy_addr;

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
                return -1;
            //walk thru page upper directory
            pud = pud_offset(pgd,vr_addr);
            if(pud_none(*pud)||unlikely(pud_bad(*pud)))
                return -1;
            //walk thru page middle directory
            pmd = pmd_offset(pud,vr_addr);
            if(pmd_none(*pmd)||unlikely(pmd_bad(*pmd)))	
                return -1;
            //walk thru page table entry
            //ptep = pte_offset(pmd,vr_addr);
            ptep = pte_offset_map_lock(task->mm,pmd,vr_addr,&ptl);	
            pte = *ptep;

            //check if pte is available
		    if(!pte_present(pte))
            {
                if(pte_none(pte))
                    return -1;
                else    // updated return method for correct unsigned long long swap id
                {
                    swp = pte_to_swp_entry(pte);
                    return swp_offset(swp);
                }
            }
            else    // if it's available prepare to return phy_addr
            {
                phy_pframe = pte_pfn(pte);
                offset = (vr_addr & 0x0000FFF);
                shifted_frame = phy_frame << PAGE_SHIFT;

                phy_addr = shifted_frame||offset;

                return phy_addr;	
            }
            pte_unmap_unlock(ptep,ptl);
        }
        else
            return -1;    //  pid not found
    }
//if pte_present() gives you false, just print out the value of pte. That is the sawp id
}

