/* this code is exported from KSplice project as example of temporary mapping method in Linux Kernel */
/*
#include <linux/stop_machine.h>
int stop_machine(int (*fn)(void *), void *data, const struct cpumask *cpus);
                       ^
                This function should be in atomic context
Calling of stop_machine should be executed after map_writable (after vmap call).
                       
/*
 * map_writable creates a shadow page mapping of the range
 * [addr, addr + len) so that we can write to code mapped read-only.
 *
 * It is similar to a generalized version of x86's text_poke.  But
 * because one cannot use vmalloc/vfree() inside stop_machine, we use
 * map_writable to map the pages before stop_machine, then use the
 * mapping inside stop_machine, and unmap the pages afterwards.
 */

/*
 * free of the returned memory by vfree with page border alignment 
 */

static void *map_writable(void *addr, size_t len)
{
        void *vaddr;
        int nr_pages = DIV_ROUND_UP(offset_in_page(addr) + len, PAGE_SIZE);
        struct page **pages = kmalloc(nr_pages * sizeof(*pages), GFP_KERNEL);
        void *page_addr = (void *)((unsigned long)addr & PAGE_MASK);
        int i;

        if (pages == NULL)
                return NULL;

        for (i = 0; i < nr_pages; i++) {
                if (__module_address((unsigned long)page_addr) == NULL) {
                        pages[i] = virt_to_page(page_addr);
                        WARN_ON(!PageReserved(pages[i]));
                } else {
                        pages[i] = vmalloc_to_page(page_addr);
                }
                if (pages[i] == NULL) {
                        kfree(pages);
                        return NULL;
                }
                page_addr += PAGE_SIZE;
        }
        vaddr = vmap(pages, nr_pages, VM_MAP, PAGE_KERNEL);
        kfree(pages);
        if (vaddr == NULL)
                return NULL;
        return vaddr + offset_in_page(addr);
}
