Virtual Memory Manager Implementation
## Why does this matter and how does it work?
Virtual memory management protects processes from interfering with each others' memory.
A process run on the CPU generates a virtual address, which can be any number within that OS' address space. 

Across from the CPU, in physical memory (RAM), lives the memory mapping table, which maps each virtual address (a number)
to another location in RAM (another number) where some sought after data lives. If we agree that each logical address
can fill one slot in our address space, and we need to account for all possible virtual addresses, then we need to create
a memory mapping table equal in size to our total address space (e.g., 2^32 for a 32-bit machine); this is a massive number and thus
we would have some humongous, cumbersome data structure within RAM. So the question now is, how do we, or can we, decrease the size
of this table? And the answer is yes; we can do this by mapping chunks of virtual addresses to chunks of physical addresses. These chunks
of virtual addresses are called pages and these chunks of physical addresses are called frames. So we can now refer to our memory mapping
table as our page table. 

But obviously there is a caveat here; I want the data at point 146, I don't want all of the data inside chunk 100-200. So what now?
We can write our logical addresses out so that they give us a bit more information. We can have the first X number of bits tell us
what page we want to access, and the rest of the bits give us an offset from that page. For example, if my virtual address is 0x304,
then I can say the first byte after the 'x' is my page, and the rest of the bytes are my offset. So when I see 0x304, I'm saying "Go to page 3
and go 4 bytes up"; if I had something like 0x345, I'm saying "Go to page 3 and go 45 bytes up".

If the memory mapping table is consulted and finds that there is no page in RAM that has the data we're looking for, then the data must be in disk,
this data must be read from disk into RAM, which is an **incredibly** slow process.

Almost there. A final data structure we can introduce is something called a **translation lookaside buffer** which is a very
pretentious computer science term for a cache. It sits inside the Memory Management Unit (MMU) on the CPU and is much quicker to access
than our page table due to its proximity to where all of our processing is being done. You can think of the TLB as a super small page table
that is way quicker to access than our other data structure, the page table, that lives in RAM.

OK, I think that's about everything. Let's look at the code.

## Code Flow
addressList.txt is a huge text file containing a bunch of numbers meant to represent different logical addresses.

Jacob Zamore
Source Files: vmmgr.c 
Compile Command: gcc vmmgr.c -o vmmgr
Run Command: ./vmmgr addressList.txt


