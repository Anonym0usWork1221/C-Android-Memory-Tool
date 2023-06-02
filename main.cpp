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

/*
    TYPE_DWORD,
    TYPE_FLOAT,
    TYPE_DOUBLE,
    TYPE_WORD,
    TYPE_BYTE,
    TYPE_QWORD,
*/

    SetSearchRange(ALL);
    MemorySearch("1", TYPE_BYTE);
    MemoryOffset("2", 2,/*offset*/ TYPE_BYTE);
    MemoryWrite("0", 2,/*offset*/ TYPE_BYTE);
    ClearResults();
    
    return 0;
}
