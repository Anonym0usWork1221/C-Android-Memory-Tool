#pragma
#include<cstddef>
#include<cstdint>
#include<pthread.h>

struct MAPS {
    long int addr;
    long int taddr;
    int type;
    struct MAPS *next;
};

struct RESULT {
    long int addr;
    struct RESULT *next;
 };

struct FREEZE {
    long int addr; // address
    char *value; // value
    int type; // type
    struct FREEZE *next; // pointer to the next node
};

typedef int BOOLEAN;
typedef struct MAPS *PMAPS; //A linked list that stores maps
typedef struct RESULT *PRES; // linked list to store results
typedef struct FREEZE *PFREEZE; // Linked list to store frozen data

typedef int TYPE;
typedef int RANGE;
typedef int COUNT;
typedef int COLOR;
typedef long int OFFSET;
typedef long int ADDRESS;
typedef char PACKAGENAME;

typedef int64_t QWORD;
typedef int32_t DWORD;
typedef int16_t WORD;
typedef int8_t BYTE;
typedef float FLOAT;
typedef double DOUBLE;

enum type {
     TYPE_DWORD,
     TYPE_FLOAT,
     TYPE_DOUBLE,
     TYPE_WORD,
     TYPE_BYTE,
     TYPE_QWORD,
};

enum Range {
     ALL, // all memory
     B_BAD, // B memory
     C_ALLOC, // Ca memory
     C_BSS, // Cb memory
     C_DATA, // Cd memory
     C_HEAP, // Ch memory
     JAVA_HEAP, // Jh memory
     A_ANONMYOUS, // A memory
     CODE_SYSTEM, // Xs memory
     STACK, // S memory
     ASHMEM // As memory
};

enum Color {
     COLOR_SILVERY, // silver
     COLOR_RED, // Red
     COLOR_GREEN, // Green
     COLOR_YELLOW, // Yellow
     COLOR_DARK_BLUE, // Blue
     COLOR_PINK, // Pink
     COLOR_SKY_BLUE, // Sky Blue
     COLOR_WHITE // White
};

#define MODE_ROOT "MODE_ROOT"
#define MODE_NO_ROOT "MODE_NO_ROOT"
#define TRUE 1
#define FALSE 0
#define LEN sizeof(struct MAPS)
#define FRE sizeof(struct FREEZE)


class MemoryTool
{
public:
    PMAPS Res = NULL; // Global buff (where data is saved)
    PFREEZE value_freeze = NULL; // For storing frozen data
    PFREEZE pEnd = NULL;
    PFREEZE pNew = NULL;
    int FreezeCount = 0; // Number of frozen data
    int Freeze = 0; // switch
    pthread_t pth;
    char bm[64]; // Package names
    long int delay = 30000; // Freeze delay, default 30000us
    int process_handle; //mem process_handle
    int ResCount = 0; // number of results
    int gs = 0;
    int MemorySearchRange = 0; // 0 for all
    int ms = 0;

    int SetTextColor(int);
    int getPID(char [64]); // get pid
	void initXMemoryTools(char *, char *);
    int SetSearchRange(int); // Set search scope
    PMAPS readmaps(int);
    PMAPS readmaps_all(); // read maps file
    PMAPS readmaps_bad(); // read maps file
    PMAPS readmaps_c_alloc(); // read maps file
    PMAPS readmaps_c_bss(); // read maps file
    PMAPS readmaps_c_data(); // read maps file
    PMAPS readmaps_c_heap(); // read maps file
    PMAPS readmaps_java_heap(); // read maps file
    PMAPS readmaps_a_anonmyous(); // read maps file
    PMAPS readmaps_code_system(); // read maps file
    PMAPS readmaps_stack(); // read maps file
    PMAPS readmaps_ashmem(); // read maps file

    void BaseAddressSearch(char*, int , long int); // base search
    PMAPS BaseAddressSearch_DWORD(DWORD, long int, PMAPS); // DWORD
    PMAPS BaseAddressSearch_FLOAT(FLOAT , long int, PMAPS ); // FLOAT
    PMAPS BaseAddressSearch_DOUBLE(DOUBLE , long int , PMAPS); // DOUBLE
    PMAPS BaseAddressSearch_WORD(WORD, long int, PMAPS); // WORD
    PMAPS BaseAddressSearch_BYTE(BYTE, long int, PMAPS); // BYTE
    PMAPS BaseAddressSearch_QWORD(QWORD, long int, PMAPS); // QWORD


    void RangeMemorySearch(char*, char *, int); // range search
    PMAPS RangeMemorySearch_DWORD(DWORD, DWORD, PMAPS); // DWORD
    PMAPS RangeMemorySearch_FLOAT(FLOAT, FLOAT, PMAPS); // FLOAT
    PMAPS RangeMemorySearch_DOUBLE(DOUBLE, DOUBLE, PMAPS); // WORD
    PMAPS RangeMemorySearch_WORD(WORD, WORD, PMAPS); // WORD
    PMAPS RangeMemorySearch_BYTE(BYTE, BYTE, PMAPS); // BYTE
    PMAPS RangeMemorySearch_QWORD(QWORD, QWORD, PMAPS); // QWORD

    void MemorySearch(char*, int); // Type search, where value needs to pass in an address
    PMAPS MemorySearch_DWORD(DWORD, PMAPS); // Memory Search DWORD
    PMAPS MemorySearch_FLOAT(FLOAT, PMAPS); // Memory search FLOAT
    PMAPS MemorySearch_DOUBLE(DOUBLE, PMAPS); // Memory Search DOUBLE
    PMAPS MemorySearch_WORD(WORD, PMAPS); // Memory Search WORD
    PMAPS MemorySearch_BYTE(BYTE, PMAPS); // Memory Search BYTE
    PMAPS MemorySearch_QWORD(QWORD, PMAPS); // Memory Search QWORD

    void MemoryOffset(char *, long int, int ); // search offset
    PMAPS MemoryOffset_DWORD(DWORD , long int, PMAPS); // search offset DWORD
    PMAPS MemoryOffset_FLOAT(FLOAT , long int, PMAPS); // search offset FLOAT
    PMAPS MemoryOffset_DOUBLE(DOUBLE , long int , PMAPS); // search offset DOUBLE
    PMAPS MemoryOffset_WORD(WORD , long int , PMAPS); // search offset WORD
    PMAPS MemoryOffset_BYTE(BYTE , long int , PMAPS); // search offset BYTE
    PMAPS MemoryOffset_QWORD(QWORD , long int, PMAPS); // search offset QWORD

    void RangeMemoryOffset(char *, char *, long int, int); // range offset
    PMAPS RangeMemoryOffset_DWORD(DWORD, DWORD , long int, PMAPS); // range offset DWORD
    PMAPS RangeMemoryOffset_FLOAT(FLOAT , FLOAT, long int , PMAPS); // range offset FLOAT
    PMAPS RangeMemoryOffset_DOUBLE(DOUBLE, DOUBLE, long int, PMAPS); // range offset DOUBLE
    PMAPS RangeMemoryOffset_WORD(WORD, WORD, long int, PMAPS); // range offset WORD
    PMAPS RangeMemoryOffset_BYTE(BYTE, BYTE, long int, PMAPS); // range offset BYTE
    PMAPS RangeMemoryOffset_QWORD(QWORD, QWORD, long int, PMAPS); // range offset QWORD

    void MemoryWrite(char *, long int, int); // memory write
    int MemoryWrite_DWORD(DWORD, PMAPS, long int); // memory write DWORD
    int MemoryWrite_FLOAT(FLOAT, PMAPS, long int); // memory write FLOAT
    int MemoryWrite_DOUBLE(DOUBLE, PMAPS, long int); // memory write DOUBLE
    int MemoryWrite_WORD(WORD, PMAPS, long int); // memory write WORD
    int MemoryWrite_BYTE(BYTE, PMAPS, long int); // memory write BYTE
    int MemoryWrite_QWORD(QWORD, PMAPS, long int); // memory write QWORD


    int WriteAddress(long int, char *, int); // Modify the value in the address
    int WriteAddress_DWORD(ADDRESS, DWORD);
    int WriteAddress_FLOAT(ADDRESS, FLOAT);
    int WriteAddress_DOUBLE(ADDRESS, DOUBLE);
    int WriteAddress_WORD(ADDRESS, WORD);
    int WriteAddress_BYTE(ADDRESS, BYTE);
    int WriteAddress_QWORD(ADDRESS, QWORD);

    void PrintResults(); // Print the contents of Res
    void ClearResults(); // Clear the linked list and free up space
    void ClearMaps(PMAPS); // Clear maps
    int GetResultCount(); //Get the number of search results

    int isapkinstalled(char *); // Check if the app is installed
    int isapkrunning(char *); // Check if the app is running
    int killprocess(char *); // kill the process
    char GetProcessState(char *); // Get process state
    int killGG(); // kill the gg modifier
    int killXs(); // kill xs
    int uninstallapk(char *); // silently remove software
    int installapk(char *); // silently uninstall the software
    int rebootsystem(); // reboot the system (phone)
    int PutDate(); // output system date
    int GetDate(char *); // Get system time

    PMAPS GetResults(); // Get the result, return the head pointer
    int AddFreezeItem_All(char *, int , long int); // Freeze all results
    int AddFreezeItem(long int , char *, int , long int); // add frozen data
    int AddFreezeItem_DWORD(long int , char *); // DWORD
    int AddFreezeItem_FLOAT(long int, char *); // FLOAT
    int AddFreezeItem_DOUBLE(long int, char *); // DOUBLE
    int AddFreezeItem_WORD(long int, char *); // WORD
    int AddFreezeItem_BYTE(long int, char *); // BYTE
    int AddFreezeItem_QWORD(long int, char *); // QWORD

    int RemoveFreezeItem(long int); // clear fixed freeze data
    int RemoveFreezeItem_All(); // clear all frozen data
    int StartFreeze(); // start freezing
    int StopFreeze(); // stop freezing
    int SetFreezeDelay(long int); // set the freeze delay
    int PrintFreezeItems(); // print the freeze list

    char *GetAddressValue(ADDRESS,int);
    DWORD GetAddressValue_DWORD(ADDRESS);
    FLOAT GetAddressValue_FLOAT(ADDRESS);
    DOUBLE GetAddressValue_DOUBLE(ADDRESS);
    WORD GetAddressValue_WORD(ADDRESS);
    BYTE GetAddressValue_BYTE(ADDRESS);
    QWORD GetAddressValue_QWORD(ADDRESS);

    char *getRightText(char *,char *,int);//Get the right text
    bool getMask(long int ,char *);//Judgment mask
    void MaskSearch(char *, int );//Mask search

    bool snowpd(long int);
    long int GetModuleBase(char *,char * ,int);
    void FreezeThread();
};


 