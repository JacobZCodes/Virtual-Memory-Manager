#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int totalReferences = 0;
int pageFaultRate = 0;
int TLBHitRate =0;
int pageTable[256];
int TLB[16][2]; // Page Number - Frame 
unsigned char RAM[256][256]; // u_char is 1 byte
int accessCount[16] = {0}; // for LRU algo
int getFrameTLB(int pageNum) {
    for (int i=0; i<16; ++i) {
        if (TLB[i][0] == pageNum) {
            return TLB[i][1];
        }
    }
    return -1;
}
int getFrame(int pageNum) {
    return pageTable[pageNum];
}

int findFreeFrame() {
    for (int i=0; i<256; ++i) {
        if (RAM[i][0] == 'x') {
            return i;
        }
    }
    printf("No found\n");
    return -1;
}

int findFreeIndex() {
    for (int i = 0; i< 16; ++i) {
        if (TLB[i][0] == -1) {
            return i;
        }
    }
    return -1;
}

int findTLBIndex(int frame) {
    for (int i=0; i<16; ++i) {
        if (TLB[i][1] == frame) {
            return i;
        }
    }
    printf("THIS SHOULD NEVER PRINT\n");
    return 1000;
}
void updateAccessCount(int index) {
    for (int i=0 ; i<16; ++i) {
        accessCount[i]++;
    }
    accessCount[index] = 0; // we just TLB hit this index, so shove it to front of access line
}

int findLRUIndex() {
    // Find highest num in access count
    int currMax = -1000;
    for (int i=0; i<16; ++i) {
        if (accessCount[i] > currMax) {
            currMax = accessCount[i];
        }
    }
    // once we've found currMax, find and return its index
    for (int i=0; i<16; ++i) {
        if (accessCount[i] == currMax) {
            return i;
        }
    }
    printf("THIS SHOULD NEVER PRINT!\n");
    return 10000;
}

void printTLB() {
    printf("********************\n");
    for (int i=0; i<16; ++i) {
        printf("%i   PageNum=%i     Frame=%i    \n", i, TLB[i][0], TLB[i][1]);
    }
    printf("********************\n");
}

void printTLBFrames() {
    printf("********************\n");
    printf("{ ");
    for (int i=0; i<16; ++i) {
        printf("%i ", TLB[i][1]);
    }
    printf("}");
    printf("********************\n");
}

int main(int argc, char** argv) {
    int c = 0;
    FILE *disk = fopen("BACKING_STORE.BIN", "rb");
    if (disk == NULL) {
        perror("Error opening disk file: ");
        return 1;
    }
    char* addressList = argv[1];
    FILE *fp = fopen(addressList, "r");
    if (fp == NULL) {
        perror("Error opening addresses: ");
        return 1;
    }
    char line[10];
    for (int i=0; i<256; ++i) { // Populate all of pagetable, TLB, and RAM as -1 to simulate empty
        pageTable[i] = -1;
    }
    for (int i = 0; i < 256; ++i) { 
        for (int j = 0; j < 256; ++j) {
            RAM[i][j] = 'x';
        }
    }
    for (int i = 0; i < 16; ++i) { 
        for (int j = 0; j < 2; ++j) {
            TLB[i][j] = -1;
        }
    }

    while(fgets(line, sizeof(line), fp) != NULL) {
        if (c>20) {
            exit(0);
        }
        // Get page number and offset
        int logicalAddress = atoi(line);
        int offset = logicalAddress & 0xFF;
        int pageNum = (logicalAddress >> 8) & 0xFF;
        // TLB hit -- update access here
        int tempFrame = getFrameTLB(pageNum);
        if (tempFrame != -1) {
            TLBHitRate++;
            updateAccessCount(findTLBIndex(tempFrame));
            printf("TLB Hit - Logical Address %i exists in RAM at %i, data is: %u\n", logicalAddress, (tempFrame * 256 + offset), RAM[tempFrame][offset]);
            continue;
        }
        // TLB miss - consult page table
        int frame = getFrame(pageNum);
        if (frame == -1) {
            pageFaultRate++;
            // Page fault -- 
            // find a free frame in RAM and write data from disk position
            // page num * 256, and write 256 bytes of data to RAM
            int freeFrame = findFreeFrame();
            fseek(disk, pageNum * 256, SEEK_SET);
            fread(RAM[freeFrame], sizeof(unsigned char), 256, disk);
            // Update page table
            pageTable[pageNum] = freeFrame;
            // Update TLB
            int TLBIndex = findFreeIndex();
            if (TLBIndex != -1) {
                TLB[TLBIndex][0] = pageNum;
                TLB[TLBIndex][1] = freeFrame;
            }
            else {
                // LRU to update TLB
                int LRUIndex = findLRUIndex();
                TLB[LRUIndex][0] = pageNum;
                TLB[LRUIndex][1] = freeFrame;
            }
            printf("Page Fault - Logical Address %i mapped to RAM at %i, written data is: %u\n", logicalAddress, (freeFrame * 256 + offset), RAM[freeFrame][offset]);
        }
        else {
            // Frame found in page table, so there's nothing to populate in RAM --
            // just print the data
            printf("Page Table Hit - Logical Address %i in RAM at %i, data is: %u\n", logicalAddress, (frame * 256 + offset), RAM[frame][offset]);
            
        }
        totalReferences++;
    }
    printf("Page Fault Rate: %i/%i\n", pageFaultRate, totalReferences);
    printf("TLB Hit Rate: %i/%i\n", TLBHitRate, totalReferences);
    return 0;
}

