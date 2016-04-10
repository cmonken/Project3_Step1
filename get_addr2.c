/*  
 *  Fan, Martin Kuna, Christoper David Monken
 *  Project 3 Part 1
 *  CSE430: OPerating Systems
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>           // remove once change to syscall from module
//#include <linux/syscalls.h>     // use for syscall version
#include <linux/sched.h>

#include <asm/page.h>
#include <linux/mm.h>

#include <linux/printk.h>
#include <linux/jiffies.h>
#include <linux/tty.h>
#include <linux/uaccess.h>


/* 3.2��Describing a Page Table Entry
 * As mentioned, each entry is described by the structs pte_t, pmd_t and pgd_t for PTEs, PMDs and PGDs respectively.
 * Even though these are  often just unsigned integers, they are defined as structs for two reasons. The first is for
 * type protection so that they will not be used inappropriately. The second is for features like PAE on the x86 where
 * an additional 4 bits is used for addressing more than 4GiB of memory. To store the protection bits, pgprot_t is
 * defined which holds the relevant flags and is usually stored in the lower bits of a page table entry.
 *
 * For type casting, 4 macros are provided in asm/page.h, which takes the above types and returns the relevant part of
 * the structs. They are pte_val(), pmd_val(), pgd_val() and pgprot_val(). To reverse the type casting, 4 more macros
 * are provided __pte(), __pmd(), __pgd() and __pgprot().
 *
 * Where exactly the protection bits are stored is architecture dependent. For illustration purposes, we will examine
 * the case of an x86 architecture without PAE enabled but the same principles apply across architectures. On an x86
 * with no PAE, the pte_t is simply a 32 bit integer within a struct. Each pte_t points to an address of a page frame
 * and all the addresses pointed to are guaranteed to be page aligned. Therefore, there are PAGE_SHIFT (12) bits in that
 * 32 bit value that are free for status bits of the page table entry. A number of the protection and status bits are
 * listed in Table ?? but what bits exist and what they mean varies between architectures.
 *
 * Bit			Function
 * _PAGE_PRESENT	Page is resident in memory and not swapped out
 * _PAGE_PROTNONE	Page is resident but not accessable
 * _PAGE_RW		Set if the page may be written to
 * _PAGE_USER		Set if the page is accessible from user space
 * _PAGE_DIRTY		Set if the page is written to
 * _PAGE_ACCESSED	Set if the page is accessed
 * Table 3.1: Page Table Entry Protection and Status Bits
These bits are self-explanatory except for the _PAGE_PROTNONE which we will discuss further. On the x86 with Pentium III
 * and higher, this bit is called the Page Attribute Table (PAT) while earlier architectures such as the Pentium II had
 * this bit reserved. The PAT bit is used to indicate the size of the page the PTE is referencing. In a PGD entry, this
 * same bit is instead called the Page Size Exception (PSE) bit so obviously these bits are meant to be used in conjunction.
 *
 * As Linux does not use the PSE bit for user pages, the PAT bit is free in the PTE for other purposes. There is a requirement
 * for having a page resident in memory but inaccessible to the userspace process such as when a region is protected with
 * mprotect() with the PROT_NONE flag. When the region is to be protected, the _PAGE_PRESENT bit is cleared and the
 * _PAGE_PROTNONE bit is set. The macro pte_present() checks if either of these bits are set and so the kernel itself knows
 * the PTE is present, just inaccessible to userspace which is a subtle, but important point. As the hardware bit _PAGE_PRESENT
 * is clear, a page fault will occur if the page is accessed so Linux can enforce the protection while still knowing the page
 * is resident if it needs to swap it out or the process exits.
 */

/* 3.3��Using Page Table Entries
 * Macros are defined in <asm/pgtable.h> which are important for the navigation and examination of page table entries.
 * To navigate the page directories, three macros are provided which break up a linear address space into its component
 * parts. pgd_offset() takes an address and the mm_struct for the process and returns the PGD entry that covers the
 * requested address. pmd_offset() takes a PGD entry and an address and returns the relevant PMD. pte_offset() takes a
 * PMD and returns the relevant PTE. The remainder of the linear address provided is the offset within the page. The
 * relationship between these fields is illustrated in Figure 3.1.
 *
 * The second round of macros determine if the page table entries are present or may be used.
 * 
 * pte_none(), pmd_none() and pgd_none() return 1 if the corresponding entry does not exist;
 * pte_present(), pmd_present() and pgd_present() return 1 if the corresponding page table entries have the PRESENT bit set;
 * pte_clear(), pmd_clear() and pgd_clear() will clear the corresponding page table entry;
 * pmd_bad() and pgd_bad() are used to check entries when passed as input parameters to functions that may change the value
 * of the entries. Whether it returns 1 varies between the few architectures that define these macros but for those that
 * actually define it, making sure the page entry is marked as present and accessed are the two most important checks.
 * 
 * There are many parts of the VM which are littered with page table walk code and it is important to recognise it. A
 * very simple example of a page table walk is the function follow_page() in mm/memory.c. The following is an excerpt
 * from that function, the parts unrelated to the page table walk are omitted:
 *
 * 407         pgd_t *pgd;
 * 408         pmd_t *pmd;
 * 409         pte_t *ptep, pte;
 * 410 
 * 411         pgd = pgd_offset(mm, address);
 * 412         if (pgd_none(*pgd) || pgd_bad(*pgd))
 * 413                 goto out;
 * 414 
 * 415         pmd = pmd_offset(pgd, address);
 * 416         if (pmd_none(*pmd) || pmd_bad(*pmd))
 * 417                 goto out;
 * 418 
 * 419         ptep = pte_offset(pmd, address);
 * 420         if (!ptep)
 * 421                 goto out;
 * 422 
 * 423         pte = *ptep;
 */

struct my_mem_addr
{
	int task_pid;
	int present;
	unsigned long long phy_addr;
	unsigned long swap_id;
}my_mem_addr;

asmlinkage unsigned long long sys_get_addr(int pid, unsigned long address)
{
    struct my_mem_addr kernel_buff[1];
    struct task_struct *task;
    struct page *this_page;
    int num = 0;
    int len = 0;
    int found = 0;
    int i = 1, n = 0;

    for_each_process(task)
    {
        if(task->pid == input_pid)                // if the pid is found
        {
            this_page = follow_page_mask(task->mm->mmap, the_address, unsigned int flags, sizeof(page));  // need to determine what flags to use
            /* get the phy address or swap id */

            kernel_buffer[0].present = 0;         // set present flag to zero
        }else{                                    // otherwise if the pid is not found
            kernel_buffer[0].phy_addr = NULL;    // set virtual address to null
            kernel_buffer[0].swap_id = NULL;      // set swap id to null
            kernel_buffer[0].present = 1;         // set present flag to 1
        }
    }

    len = sizeof(my_mem_addr);
    n = copy_to_user(address,kernel_buff,len);

    return i;
}

