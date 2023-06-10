#include <iostream>
#include "MemoryTools.h"

int main(int argc, char* argv[]) {
    /* 1. Package Name of the game
    2. ROOT_MODE (tells you are using rooted device (not reooted device not implimented yet))
    */
    MemoryTools memoryTools("com.tencent.ig", MODE_ROOT);
    
    // Initialize the search range to global (check other ranges in header file of memory tool)
    memoryTools.SetSearchRange(ALL);
    // Search for 1 in whole memory of type byte
    memoryTools.MemorySearch("1", TYPE_BYTE);
    // This is another usage if you want to write a value at given offset you can use below line else above line will write all the data in memory 
    memoryTools.MemoryOffset("2", 2, /*offset*/ TYPE_BYTE);
    // Finally write the value to the memory
    memoryTools.MemoryWrite("0", 2, /*offset*/ TYPE_BYTE);
    // clear results function clean all the offsets/values from memory
    memoryTools.ClearResults();

    return 0;
}
