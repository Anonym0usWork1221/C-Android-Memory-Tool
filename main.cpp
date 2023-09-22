# include "MemoryTool.h"

int main(int arc, char argv[]) {
	/* 1. Package Name of the game
	2. ROOT_MODE (tells you are using rooted device (not reooted device not implimented yet))
	*/
	char game_package[] = "com.tencent.ig";
	MemoryTool memory_tool;
	memory_tool.initXMemoryTools(game_package, MODE_ROOT);
	
	// Initialize the search range to global (check other ranges in header file of memory tool)
	memory_tool.SetSearchRange(ALL);
	// Search for 1 in whole memory of type byte
	memory_tool.MemorySearch("1", TYPE_BYTE);
	// This is another usage if you want to write a value at given offset you can use below line else above line will write all the data in memory 
	memory_tool.MemoryOffset("2", 2, /*offset*/ TYPE_BYTE);
	// Finally write the value to the memory
	memory_tool.MemoryWrite("0", 2, /*offset*/ TYPE_BYTE);
	// clear results function clean all the offsets/values from memory
	memory_tool.ClearResults();
	return 0;
}
