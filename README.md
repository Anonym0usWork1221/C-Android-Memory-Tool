C-Android-Memory-Tool
====
[![GitHub stars](https://img.shields.io/github/stars/Anonym0usWork1221/C-Android-Memory-Tool.svg)](https://github.com/Anonym0usWork1221/C-Android-Memory-Tool/stargazers)
[![GitHub forks](https://img.shields.io/github/forks/Anonym0usWork1221/C-Android-Memory-Tool.svg)](https://github.com/Anonym0usWork1221/C-Android-Memory-Tool/network/members)
[![GitHub issues](https://img.shields.io/github/issues/Anonym0usWork1221/C-Android-Memory-Tool.svg)](https://github.com/Anonym0usWork1221/C-Android-Memory-Tool/issues)
[![GitHub watchers](https://img.shields.io/github/watchers/Anonym0usWork1221/C-Android-Memory-Tool.svg)](https://github.com/Anonym0usWork1221/C-Android-Memory-Tool/watchers)
[![Python](https://img.shields.io/badge/language-cpp-blue.svg)](https://www.python.org)
[![GPT_LICENSE](https://img.shields.io/badge/license-MIT-red.svg)](https://opensource.org/licenses/)
![code size](https://img.shields.io/github/languages/code-size/Anonym0usWork1221/C-Android-Memory-Tool)

-----------

The MemoryTool is a C++ tool that provides functionality for reading and writing memory in a target process. It allows you to search for specific values in memory, modify memory addresses, freeze and unfreeze memory values, and perform various system-related operations. This documentation provides an overview of the tool's functionality, along with code snippets to demonstrate how to use each feature.

# Introduction
I don't know who initially wrote this memory tool. I have put in a lot of effort to correct non-working functions, fix errors, optimize the code, add proper documentation, and implement proper object-oriented programming structures to ensure the smooth running of the script. I have also added new functions.

Please note that this tool only works for rooted devices as the non-rooted (virtual environment) functionality has not been implemented yet.

If you encounter any non-working functions, please open an issue and report it.

*  Date   : 2023/06/02
*  Author : **__Abdul Moez__**
*  Version : 0.1
*  Study  : UnderGraduate in GCU Lahore, Pakistan
*  Repository  : https://github.com/Anonym0usWork1221/C-Android-Memory-Tool
*  Documentation: Starts From Below

# Samples
``main.cpp`` is the sample file to run the MemoryTool
### main.cpp File Explanation
The sample code provided demonstrates the usage of the memory tool. Let's go through it step by step:

```cpp
#include <iostream>
#include "MemoryTools.h"

int main(int argc, char *argv[]) {
    initXMemoryTools(
        /*process name*/
        "com.tencent.ig",
        /*mode root/no root*/
        MODE_ROOT
        /*MODE_NO_ROOT - for no root*/
    );
```

The code includes necessary headers and initializes the memory tool by calling initXMemoryTools(). It takes two parameters: the process name (in this case, "com.tencent.ig") and the mode ("MODE_ROOT" for rooted devices).

```cpp
    SetSearchRange(ALL);
```
The SetSearchRange() function sets the search range for memory operations. In this case, it is set to "ALL," which means searching in all memory ranges.

```cpp
    MemorySearch("1", TYPE_BYTE);
```
The MemorySearch() function is used to search for a specific value in memory. In this example, it searches for the value "1" of type TYPE_BYTE.

```cpp
    MemoryOffset("2", 2,/*offset*/ TYPE_BYTE);
```
The MemoryOffset() function is used to search for a specific value at an offset from a base address. It searches for the value "2" of type TYPE_BYTE with an offset of 2.

```cpp
    MemoryWrite("0", 2,/*offset*/ TYPE_BYTE);
```
The MemoryWrite() function is used to write a value to a specific memory address. In this example, it writes the value "0" of type TYPE_BYTE with an offset of 2.

```cpp
    ClearResults();
```
The ClearResults() function clears the linked list of results and frees up memory.
```cpp
    return 0;
}
```
The program ends and returns 0.

----
# Start of Memory Tool Documentation

## Table of Contents
1. Data Structures
2. Initialization
3. Reading Memory
4. Searching Memory
5. Writing Memory
6. Freezing Memory
7. System Operations
8. Utility Functions

## 1. Data Structures
The MemoryTool tool uses the following data structures:

### MAPS
```cpp
struct MAPS {
    long int addr;
    long int taddr;
    int type;
    struct MAPS *next;
};
```

* addr: The memory address.
* taddr: The target address.
* type: The type of memory.
* next: Pointer to the next MAPS structure.

### RESULT
```cpp
struct RESULT {
    long int addr;
    struct RESULT *next;
};
```

* addr: The memory address.
* next: Pointer to the next RESULT structure.

### FREEZE
```cpp
struct FREEZE {
    long int addr; // address
    char *value; // value
    int type; // type
    struct FREEZE *next; // pointer to the next node
};
```

* addr: The memory address.
* value: The value at the memory address.
* type: The type of memory.
* next: Pointer to the next FREEZE structure.

## 2. Initialization
To use the MemoryTool, you need to create an instance of the MemoryTool class. Here's how to initialize the tool:

```cpp 
MemoryTool memTool;
```

## 3. Reading Memory
The MemoryTool provides functions to read memory from the target process. The following functions are available:

### Read Maps File
```cpp
PMAPS readmaps(int pid);
```  
* pid: The process ID of the target process.
* Returns a linked list of MAPS structures containing information about the memory mappings in the target process.

### Read All Memory Maps
```cpp
PMAPS readmaps_all();
```
* Returns a linked list of MAPS structures for all memory mappings in the target process.

### Read Specific Memory Maps
```cpp
PMAPS readmaps_bad();
PMAPS readmaps_c_alloc();
PMAPS readmaps_c_bss();
PMAPS readmaps_c_data();
PMAPS readmaps_c_heap();
PMAPS readmaps_java_heap();
PMAPS readmaps_a_anonmyous();
PMAPS readmaps_code_system();
PMAPS readmaps_stack();
PMAPS readmaps_ashmem();
```

* Returns a linked list of MAPS structures for specific memory mappings in the target process. Each function corresponds to a specific memory range or type.

## 4. Searching Memory
The MemoryTool allows you to search for specific values in the memory of the target process. You can search for values within a specific range or across the entire memory. The following functions are available for memory search:

### Base Address Search
```cpp
void BaseAddressSearch(char* value, int type, long int address);
```
* value: The value to search for.
* type: The type of memory to search for (see type enum for options).
* address: The base address to start the search from.

### Range Memory Search
```cpp
void RangeMemorySearch(char* value, char* range, int type);
```
* value: The value to search for.
* range: The memory range to search in (see Range enum for options).
* type: The type of memory to search for (see type enum for options).

### Memory Search
```cpp
void MemorySearch(char* value, int type);
```
* value: The value to search for.
* type: The type of memory to search for (see type enum for options).

### Memory Offset Search
```cpp
void MemoryOffset(char* value, long int offset, int type);
```

* value: The value to search for.
* offset: The offset from the base address to search.
* type: The type of memory to search for (see type enum for options).

### Range Memory Offset Search
```cpp 
void RangeMemoryOffset(char* value, char* range, long int offset, int type);
```
* value: The value to search for.
* range: The memory range to search in (see Range enum for options).
* offset: The offset from the base address to search.
* type: The type of memory to search for (see type enum for options).

## 5. Writing Memory
The MemoryTool allows you to write values to memory addresses in the target process. The following functions are available for memory write:

### Memory Write
```cpp
void MemoryWrite(char* value, long int address, int type);
```
* value: The value to write to memory.
* address: The memory address to write the value to.
* type: The type of memory to write to (see type enum for options).
### Write Address
```cpp
int WriteAddress(long int address, char* value, int type);
```
* address: The memory address to write the value to.
* value: The value to write to memory.
* type: The type of memory to write to (see type enum for options).
* Returns 1 if the write operation is successful, 0 otherwise.

## 6. Freezing Memory
The MemoryTool allows you to freeze and unfreeze memory values in the target process. The freezing feature allows you to keep a memory value constant while the target process is running. The following functions are available for freezing memory:

### Start Freezing
```cpp
int StartFreeze();
```
* Starts the freezing process.

### Stop Freezing
```cpp
int StopFreeze();
```
* Stops the freezing process.

### Set Freeze Delay
```cpp
int SetFreezeDelay(long int delay);
```

* delay: The delay in microseconds between each freeze operation.
* Sets the delay between each freeze operation.

### Add Freeze Item
```cpp
int AddFreezeItem(long int address, char* value, int type, long int offset = 0);
```
* address: The memory address to freeze.
* value: The value to freeze.
* type: The type of memory to freeze (see type enum for options).
* offset: The offset from the base address to freeze (optional, default is 0).
* Adds a memory address and its frozen value to the freeze list.

### Remove Freeze Item
```cpp
int RemoveFreezeItem(long int address);
```
* address: The memory address to remove from the freeze list.
* Removes a memory address from the freeze list.

### Remove All Freeze Items
```cpp
int RemoveFreezeItem_All();
```
* Removes all memory addresses from the freeze list.

### Print Freeze Items
```cpp 
int PrintFreezeItems();
```
* Prints the current freeze list.

## 7. Getting Memory Values
The MemoryTool allows you to retrieve the values from memory addresses in the target process. The following functions are available for getting memory values:

### Get Address Value
```cpp
char* GetAddressValue(ADDRESS address, int type);
```

* address: The memory address to retrieve the value from.
* type: The type of memory value to retrieve (see type enum for options).
* Returns a character pointer containing the retrieved memory value.
### Get Address Value DWORD
```cpp
DWORD GetAddressValue_DWORD(ADDRESS address);
```
* address: The memory address to retrieve the DWORD value from.
* Returns the retrieved DWORD value.

### Get Address Value FLOAT
```cpp
FLOAT GetAddressValue_FLOAT(ADDRESS address);
```

* address: The memory address to retrieve the FLOAT value from.
* Returns the retrieved FLOAT value.

### Get Address Value DOUBLE
```cpp
DOUBLE GetAddressValue_DOUBLE(ADDRESS address);
```
* address: The memory address to retrieve the DOUBLE value from.
* Returns the retrieved DOUBLE value.

### Get Address Value WORD
```cpp
WORD GetAddressValue_WORD(ADDRESS address);
```
* address: The memory address to retrieve the WORD value from.
* Returns the retrieved WORD value.

### Get Address Value BYTE
```cpp
BYTE GetAddressValue_BYTE(ADDRESS address);
```
* address: The memory address to retrieve the BYTE value from.
* Returns the retrieved BYTE value.

### Get Address Value QWORD
```cpp
QWORD GetAddressValue_QWORD(ADDRESS address);
```
* address: The memory address to retrieve the QWORD value from.
* Returns the retrieved QWORD value.

## 8. Miscellaneous Functions
The MemoryTool provides additional miscellaneous functions for interacting with the target process:

### Get Result Count
```cpp
int GetResultCount();
```
* Returns the number of search results found.

### Print Results
```cpp
void PrintResults();
```
* Prints the contents of the search results.

### Clear Results
```cpp
void ClearResults();
```
* Clears the search results and frees up memory.

### Clear Maps
```cpp
void ClearMaps(PMAPS maps);
```
* maps: The pointer to the maps data structure to be cleared.
* Clears the maps data structure and frees up memory.

### Get Results
```cpp
PMAPS GetResults();
```

* Returns the pointer to the head of the search results.

### Get PID
```cpp
int getPID(char package[64]);
```

* package: The package name of the target process.
* Returns the process ID (PID) of the target process.
### Get Process State
```cpp
char GetProcessState(char* package);
```
* package: The package name of the target process.
* Returns the state of the target process.
### isapkinstalled
```cpp
int isapkinstalled(char* package);
```
* package: The package name of the app.
* Checks if the specified app is installed.

### isapkrunning
```cpp
int isapkrunning(char* package);
```
* package: The package name of the app.
* Checks if the specified app is running.

### killprocess
```cpp
int killprocess(char* package);
```
* package: The package name of the process to kill.
* Kills the specified process.


# Contributor

<a href = "https://github.com/Anonym0usWork1221/android-memorytool/graphs/contributors">
  <img src = "https://contrib.rocks/image?repo=Anonym0usWork1221/C-Android-Memory-Tool"/>
</a>


Assistance
----------
If you need assistance, you can ask for help on my mailing list:

* Email      : abdulmoez123456789@gmail.com

I also created a Discord group:

* Server     : https://discord.gg/RMNcqzmt9f
