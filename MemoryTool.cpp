#include "MemoryTool.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <dirent.h>
#include <thread>
#include <cstring>
#include <cstddef>
#include <cstdint>
#include <pthread.h>

using namespace std;

void MemoryTool::initXMemoryTools(char *b, char *mode) {
	strcpy(bm, b);
	if (strcmp(mode, MODE_ROOT)==0) {
		if (getuid() != 0) {
			system("echo starting");
			exit(1);
		}
	}
	system("echo 0 > /proc/sys/fs/inotify/max_user_watches");
	//system("echo 0 > /proc/sys/fs/inotify/max_queued_events");
	pid_t pid = getPID(b);
	if (pid == 0) {
		printf("\033[31;1m");
		puts("Failed to get process!");
		exit(1);
	}
	char lj[64];
	sprintf(lj, "/proc/%d/mem", pid);
	process_handle = open(lj, O_RDWR);
	if (process_handle == -1) {
		printf("\033[31;1m");
		puts("Failed to get mem!");
		exit(1);
	}
	lseek(process_handle, 0, SEEK_SET);
}

int MemoryTool::GetResultCount() {
	return gs;
}

bool MemoryTool::snowpd(long int a2) {
	return a2 <= 7999 && a2 > 31;
}

DWORD MemoryTool::GetAddressValue_DWORD(ADDRESS addr) {
	DWORD buf;
	pread64(process_handle, &buf, sizeof(DWORD), addr);
	return buf;
}

FLOAT MemoryTool::GetAddressValue_FLOAT(ADDRESS addr) {
	FLOAT buf;
	pread64(process_handle, &buf, sizeof(FLOAT), addr);
	return buf;
}

DOUBLE MemoryTool::GetAddressValue_DOUBLE(ADDRESS addr) {
	DOUBLE buf;
	pread64(process_handle, &buf, sizeof(DOUBLE), addr);
	return buf;
}

WORD MemoryTool::GetAddressValue_WORD(ADDRESS addr) {
	WORD buf;
	pread64(process_handle, &buf, sizeof(WORD), addr);
	return buf;
}

BYTE MemoryTool::GetAddressValue_BYTE(ADDRESS addr) {
	BYTE buf;
	pread64(process_handle, &buf, sizeof(BYTE), addr);
	return buf;
}

QWORD MemoryTool::GetAddressValue_QWORD(ADDRESS addr) {
	QWORD buf;
	pread64(process_handle, &buf, sizeof(QWORD), addr);
	return buf;
}

char* MemoryTool::GetAddressValue(ADDRESS addr,int type) {
	char *str = (char *)malloc(sizeof(char));
	DWORD i;
	FLOAT f;
	DOUBLE d;
	WORD w;
	BYTE b;
	QWORD q;
	switch (type) {
		case TYPE_DWORD:
			i = GetAddressValue_DWORD(addr);
			sprintf(str,"%d",i);
			break;
		case TYPE_FLOAT:
			f = GetAddressValue_FLOAT(addr);
			sprintf(str,"%e",f);
			break;
		case TYPE_DOUBLE:
			d = GetAddressValue_DOUBLE(addr);
			sprintf(str,"%e",d);
			break;
		case TYPE_WORD:
			w = GetAddressValue_WORD(addr);
			sprintf(str,"%d",w);
			break;
		case TYPE_BYTE:
			b = GetAddressValue_BYTE(addr);
			sprintf(str,"%d",b);
			break;
		case TYPE_QWORD:
			q = GetAddressValue_QWORD(addr);
			sprintf(str,"%ld",q);
			break;
		default:
			sprintf(str,"%s","");
			printf("\033[32;1mYou chose an unknown type!\n");
			break;
	}
	return str;
}

char* MemoryTool::getRightText(char *dst,char *src,int n) {
	char *p = src;
	char *q = dst;
	int len = strlen(src);
	if(n>len) n=len;
	p+=(len-n);
	while(*(q++)=*(p++));
	return dst;
}

bool MemoryTool::getMask(long int addr ,char *def) {
	char *str = (char *)malloc(sizeof(char));
	sprintf(str,"%lX",addr);
	char *dest;
	char* s = getRightText(dest,str,strlen(def));
	if (strcmp(s, def) == 0) {
		return true;
	}
	return false;
}

void MemoryTool::MaskSearch(char *def, int type) {
	PMAPS e, n;
	e = n = (PMAPS) malloc(LEN);
	PMAPS pBuff = n;
	int iCount = 0;
	PMAPS pTemp = Res;
	for (int i = 0; i < ResCount; i++) {
		long int addr = pTemp->addr;
		bool value = getMask(addr, def);
		if (value == true) {
			iCount ++;
			n->addr = addr;
			n->type = type;
			if (iCount == 1) {
				n->next = NULL;
				e = n;
				pBuff = n;
			} else {
				n->next = NULL;
				e->next = n;
				e = n;
			}
			n = (PMAPS) malloc(LEN);
		}
		pTemp = pTemp->next;
	}
	ResCount = iCount;
	gs = iCount;
	Res = pBuff;
}

int MemoryTool::SetTextColor(COLOR color) {
	switch (color) {
		case COLOR_SILVERY:
			printf("\033[30;1m");
			break;
		case COLOR_RED:
			printf("\033[31;1m");
			break;
		case COLOR_GREEN:
			printf("\033[32;1m");
			break;
		case COLOR_YELLOW:
			printf("\033[33;1m");
			break;
		case COLOR_DARK_BLUE:
			printf("\033[34;1m");
			break;
		case COLOR_PINK:
			printf("\033[35;1m");
			break;
		case COLOR_SKY_BLUE:
			printf("\033[36;1m");
			break;
		case COLOR_WHITE:
			printf("\033[37;1m");
			break;
		default:
			printf("\033[37;1m");
			break;
	}
	return 0;
}

long int MemoryTool::GetModuleBase(char *module,char * flags,int MapSize) {
	PMAPS pHead = NULL;
	PMAPS pNew = NULL;
	PMAPS pEnd = NULL;
	pEnd = pNew = (PMAPS) malloc(MapSize);
	FILE *fp;
	int flag = 1;
	char lj[64], buff[256];
	int pid = getPID(bm);
	sprintf(lj, "/proc/%d/maps", pid);
	fp = fopen(lj, "r");
	if (fp == NULL) {
		puts("Memory read failed!");
		return 0;
	}
	while (!feof(fp)) {
		fgets(buff, sizeof(buff), fp);
		if (strstr(buff, flags) != NULL && !feof(fp) && strstr(buff, module)) {
			sscanf(buff, "%lx-%lx", &pNew->addr, &pNew->taddr);
			flag = 1;
		} else {
			flag = 0;
		}
		if (flag == 1) {
			pNew->next = NULL;
			pEnd = pNew;
			pHead = pNew;
			pNew = (PMAPS) malloc(MapSize);
		}
	}
	free(pNew);
	fclose(fp);
	return pHead->addr;
}

int MemoryTool::getPID(PACKAGENAME * PackageName) {
	DIR *dir = NULL;
	struct dirent *ptr = NULL;
	FILE *fp = NULL;
	char filepath[256];
	char filetext[128];
	dir = opendir("/proc");
	if (NULL != dir) {
		while ((ptr = readdir(dir)) != NULL) {
			if ((strcmp(ptr->d_name, ".") == 0) || (strcmp(ptr->d_name, "..") == 0))
				continue;
			if (ptr->d_type != DT_DIR)
				continue;
			sprintf(filepath, "/proc/%s/cmdline", ptr->d_name);
			fp = fopen(filepath, "r");
			if (NULL != fp) {
				fgets(filetext, sizeof(filetext), fp);
				if (strcmp(filetext, PackageName) == 0) {
					// puts(filepath);
					// printf("packagename:%s\n",filetext);
					break;
				}
				fclose(fp);
			}
		}
	}
	if (readdir(dir) == NULL) {
		return 0;
	}
	closedir(dir);
	return atoi(ptr->d_name);
}

void MemoryTool::PrintResults() {
	PMAPS temp = Res;
	const char *type = (char *)malloc(sizeof(char));
	for (int i = 0; i < ResCount; i++) {
		switch (temp->type) {
			case TYPE_DWORD:
				type = "DWORD";
				break;
			case TYPE_FLOAT:
				type = "FLOAT";
				break;
			case TYPE_DOUBLE:
				type = "DOUBLE";
				break;
			case TYPE_WORD:
				type = "WORD";
				break;
			case TYPE_BYTE:
				type = "BYTE";
				break;
			case TYPE_QWORD:
				type = "QWORD";
				break;
			default:
				type = "";
				break;
		}

		printf("\e[37;1mAddr:\e[32;1m0x%lX  \e[37;1mType:\e[36;1m%s\n",temp->addr,type);
		//printf("addr:\e[32;1m0x%lX type:%s\n", temp->addr,type);
		temp = temp->next;
	}
}

void MemoryTool::ClearResults()	{
	PMAPS pHead = Res;
	PMAPS pTemp = pHead;
	int i;
	for (i = 0; i < ResCount; i++) {
		pTemp = pHead;
		pHead = pHead->next;
		free(pTemp);
	}
}

void MemoryTool::BaseAddressSearch(char *value, TYPE type, ADDRESS BaseAddr) {
	PMAPS pHead = NULL;
	PMAPS pMap = NULL;
	gs = 0;
	switch (MemorySearchRange) {
		case ALL:
			pMap = readmaps(ALL);
			break;
		case B_BAD:
			pMap = readmaps(B_BAD);
			break;
		case C_ALLOC:
			pMap = readmaps(C_ALLOC);
			break;
		case C_BSS:
			pMap = readmaps(C_BSS);
			break;
		case C_DATA:
			pMap = readmaps(C_DATA);
			break;
		case C_HEAP:
			pMap = readmaps(C_HEAP);
			break;
		case JAVA_HEAP:
			pMap = readmaps(JAVA_HEAP);
			break;
		case A_ANONMYOUS:
			pMap = readmaps(A_ANONMYOUS);
			break;
		case CODE_SYSTEM:
			pMap = readmaps(CODE_SYSTEM);
			break;
		case STACK:
			pMap = readmaps(STACK);
			break;
		case ASHMEM:
			pMap = readmaps(ASHMEM);
			break;
		default:
			printf("\033[32;1mYou Select A NULL Type!\n");
			break;
	}
	if (pMap == NULL) {
		puts("map error");
		return;
	}
	switch (type) {
		case TYPE_DWORD:
			pHead = BaseAddressSearch_DWORD(atoi(value), BaseAddr, pMap);
			break;
		case TYPE_FLOAT:
			pHead = BaseAddressSearch_FLOAT(atof(value), BaseAddr, pMap);
			break;
		case TYPE_DOUBLE:
			pHead = BaseAddressSearch_DOUBLE(atof(value), BaseAddr, pMap);
			break;
		case TYPE_WORD:
			pHead = BaseAddressSearch_WORD(atoi(value), BaseAddr, pMap);
			break;
		case TYPE_BYTE:
			pHead = BaseAddressSearch_BYTE(atoi(value), BaseAddr, pMap);
			break;
		case TYPE_QWORD:
			pHead = BaseAddressSearch_QWORD(atoi(value), BaseAddr, pMap);
			break;
		default:
			printf("\033[32;1mYou Select A NULL Type!\n");
			break;
	}
	if (pHead == NULL) {
        puts("Search memory failed!");
		return (void)0;
	}
	ResCount = gs;
	Res = pHead;
}

PMAPS MemoryTool::BaseAddressSearch_DWORD(DWORD value, ADDRESS BaseAddr, PMAPS pMap) {
	//*gs = 0;
	// printf("BaseAddress:%lX\n",BaseAddr);
	PMAPS e, n;
	e = n = (PMAPS) malloc(LEN);
	PMAPS pBuff = n;
	int iCount = 0;
	long int c, ADDR;
	void *BUF[8];
	PMAPS pTemp = pMap;
	while (pTemp != NULL) {
		c = (pTemp->taddr - pTemp->addr) / 4096;
		for (int j = 0; j < c; j++) {
			ADDR = pTemp->addr + j * 4096 + BaseAddr;
			pread64(process_handle, BUF, 8, ADDR);
			if (*(DWORD *)&BUF[0] == value) {
				iCount++;
				gs += 1;
				n->addr = ADDR;
				n->type = TYPE_DWORD;
				// printf("addr:%lx,val:%d,buff=%d\n",n->addr,value,buff[i]);
				if (iCount == 1) {
					n->next = NULL;
					e = n;
					pBuff = n;
				} else {
					n->next = NULL;
					e->next = n;
					e = n;
				}
				n = (PMAPS) malloc(LEN);
			}
		}
		pTemp = pTemp->next;
	}
	return pBuff;
}

PMAPS MemoryTool::BaseAddressSearch_DOUBLE(DOUBLE value, ADDRESS BaseAddr, PMAPS pMap) {
	//*gs = 0;
	// printf("BaseAddress:%lX\n",BaseAddr);
	PMAPS e, n;
	e = n = (PMAPS) malloc(LEN);
	PMAPS pBuff = n;
	int iCount = 0;
	long int c, ADDR;
	void *BUF[8];
	PMAPS pTemp = pMap;
	while (pTemp != NULL) {
		c = (pTemp->taddr - pTemp->addr) / 4096;
		for (int j = 0; j < c; j++) {
			ADDR = pTemp->addr + j * 4096 + BaseAddr;
			pread64(process_handle, BUF, 8, ADDR);
			if (*(DOUBLE *)&BUF[0] == value) {
				iCount++;
				gs += 1;
				n->addr = ADDR;
				n->type = TYPE_DOUBLE;
				// printf("addr:%lx,val:%d,buff=%d\n",n->addr,value,buff[i]);
				if (iCount == 1) {
					n->next = NULL;
					e = n;
					pBuff = n;
				} else {
					n->next = NULL;
					e->next = n;
					e = n;
				}
				n = (PMAPS) malloc(LEN);
			}
		}
		pTemp = pTemp->next;
	}
	return pBuff;
}

PMAPS MemoryTool::BaseAddressSearch_FLOAT(FLOAT value, ADDRESS BaseAddr, PMAPS pMap) {
	//*gs = 0;
	PMAPS e, n;
	e = n = (PMAPS) malloc(LEN);
	PMAPS pBuff = n;
	long int c, ADDR;
	int iCount = 0;
	void *BUF[8];
	PMAPS pTemp = pMap;
	while (pTemp != NULL) {
		c = (pTemp->taddr - pTemp->addr) / 4096;
		for (int j = 0; j < c; j++) {
			ADDR = pTemp->addr + j * 4096 + BaseAddr;
			pread64(process_handle, BUF, 8, ADDR);
			if (*(FLOAT *)&BUF[0] == value) {
				iCount++;
				gs += 1;
				n->addr = ADDR;
				n->type = TYPE_FLOAT;
				// printf("addr:%lx,val:%d,buff=%d\n",n->addr,value,buff[i]);
				if (iCount == 1) {
					n->next = NULL;
					e = n;
					pBuff = n;
				} else {
					n->next = NULL;
					e->next = n;
					e = n;
				}
				n = (PMAPS) malloc(LEN);
			}
		}
		pTemp = pTemp->next;
	}
	return pBuff;
}

PMAPS MemoryTool::BaseAddressSearch_WORD(WORD value, ADDRESS BaseAddr, PMAPS pMap) {
	//*gs = 0;
	PMAPS e, n;
	e = n = (PMAPS) malloc(LEN);
	PMAPS pBuff = n;
	long int c, ADDR;
	int iCount = 0;
	void *BUF[8];
	PMAPS pTemp = pMap;
	while (pTemp != NULL) {
		c = (pTemp->taddr - pTemp->addr) / 4096;
		for (int j = 0; j < c; j++) {
			ADDR = pTemp->addr + j * 4096 + BaseAddr;
			pread64(process_handle, BUF, 8, ADDR);
			if (*(WORD *)&BUF[0] == value) {
				iCount++;
				gs += 1;
				n->addr = ADDR;
				n->type = TYPE_WORD;
				// printf("addr:%lx,val:%d,buff=%d\n",n->addr,value,buff[i]);
				if (iCount == 1) {
					n->next = NULL;
					e = n;
					pBuff = n;
				} else {
					n->next = NULL;
					e->next = n;
					e = n;
				}
				n = (PMAPS) malloc(LEN);
			}
		}
		pTemp = pTemp->next;
	}
	return pBuff;
}

PMAPS MemoryTool::BaseAddressSearch_QWORD(QWORD value, ADDRESS BaseAddr, PMAPS pMap) {
	//*gs = 0;
	PMAPS e, n;
	e = n = (PMAPS) malloc(LEN);
	PMAPS pBuff = n;
	long int c, ADDR;
	int iCount = 0;
	void *BUF[8];
	PMAPS pTemp = pMap;
	while (pTemp != NULL) {
		c = (pTemp->taddr - pTemp->addr) / 4096;
		for (int j = 0; j < c; j++) {
			ADDR = pTemp->addr + j * 4096 + BaseAddr;
			pread64(process_handle, BUF, 8, ADDR);
			if (*(QWORD *)&BUF[0] == value) {
				iCount++;
				gs += 1;
				n->addr = ADDR;
				n->type = TYPE_QWORD;
				// printf("addr:%lx,val:%d,buff=%d\n",n->addr,value,buff[i]);
				if (iCount == 1) {
					n->next = NULL;
					e = n;
					pBuff = n;
				} else {
					n->next = NULL;
					e->next = n;
					e = n;
				}
				n = (PMAPS) malloc(LEN);
			}
		}
		pTemp = pTemp->next;
	}
	return pBuff;
}

PMAPS MemoryTool::BaseAddressSearch_BYTE(BYTE value, ADDRESS BaseAddr, PMAPS pMap) {
	//*gs = 0;
	
	PMAPS e, n;
	e = n = (PMAPS) malloc(LEN);
	PMAPS pBuff = n;
	long int c, ADDR;
	int iCount = 0;
	void *BUF[8];
	PMAPS pTemp = pMap;
	while (pTemp != NULL) {
		c = (pTemp->taddr - pTemp->addr) / 4096;
		for (int j = 0; j < c; j++) {
			ADDR = pTemp->addr + j * 4096 + BaseAddr;
			pread64(process_handle, BUF, 8, ADDR);
			if (*(BYTE *)&BUF[0] == value) {
				iCount++;
				gs += 1;
				n->addr = ADDR;
				n->type = TYPE_BYTE;
				// printf("addr:%lx,val:%d,buff=%d\n",n->addr,value,buff[i]);
				if (iCount == 1) {
					n->next = NULL;
					e = n;
					pBuff = n;
				} else {
					n->next = NULL;
					e->next = n;
					e = n;
				}
				n = (PMAPS) malloc(LEN);
			}
		}
		pTemp = pTemp->next;
	}
	return pBuff;
}

void MemoryTool::RangeMemorySearch(char *from_value, char *to_value, TYPE type) {
	PMAPS pHead = NULL;
	PMAPS pMap = NULL;
	gs = 0;
	switch (MemorySearchRange) {
		case ALL:
			pMap = readmaps(ALL);
			break;
		case B_BAD:
			pMap = readmaps(B_BAD);
			break;
		case C_ALLOC:
			pMap = readmaps(C_ALLOC);
			break;
		case C_BSS:
			pMap = readmaps(C_BSS);
			break;
		case C_DATA:
			pMap = readmaps(C_DATA);
			break;
		case C_HEAP:
			pMap = readmaps(C_HEAP);
			break;
		case JAVA_HEAP:
			pMap = readmaps(JAVA_HEAP);
			break;
		case A_ANONMYOUS:
			pMap = readmaps(A_ANONMYOUS);
			break;
		case CODE_SYSTEM:
			pMap = readmaps(CODE_SYSTEM);
			break;
		case STACK:
			pMap = readmaps(STACK);
			break;
		case ASHMEM:
			pMap = readmaps(ASHMEM);
			break;
		default:
			printf("\033[32;1mYou Select A NULL Type!\n");
			break;
	}
	if (pMap == NULL) {
		puts("map error");
		return;
	}
	switch (type) {
		case TYPE_DWORD:
			if (atoi(from_value) > atoi(to_value))
				pHead = RangeMemorySearch_DWORD(atoi(to_value), atoi(from_value), pMap);
			else
				pHead = RangeMemorySearch_DWORD(atoi(from_value), atoi(to_value), pMap);
			break;
		case TYPE_FLOAT:
			if (atof(from_value) > atof(to_value))
				pHead = RangeMemorySearch_FLOAT(atof(to_value), atof(from_value), pMap);
			else
				pHead = RangeMemorySearch_FLOAT(atof(from_value), atof(to_value), pMap);
			break;
		case TYPE_DOUBLE:
			if (atof(from_value) > atof(to_value))
				pHead = RangeMemorySearch_DOUBLE(atof(to_value), atof(from_value), pMap);
			else
				pHead = RangeMemorySearch_DOUBLE(atof(from_value), atof(to_value), pMap);
			break;
		case TYPE_WORD:
			if (atoi(from_value) > atoi(to_value))
				pHead = RangeMemorySearch_WORD(atoi(to_value), atoi(from_value), pMap);
			else
				pHead = RangeMemorySearch_WORD(atoi(from_value), atoi(to_value), pMap);
			break;
		case TYPE_BYTE:
			if (atoi(from_value) > atoi(to_value))
				pHead = RangeMemorySearch_BYTE(atoi(to_value), atoi(from_value), pMap);
			else
				pHead = RangeMemorySearch_BYTE(atoi(from_value), atoi(to_value), pMap);
			break;
		case TYPE_QWORD:
			if (atoi(from_value) > atoi(to_value))
				pHead = RangeMemorySearch_QWORD(atoi(to_value), atoi(from_value), pMap);
			else
				pHead = RangeMemorySearch_QWORD(atoi(from_value), atoi(to_value), pMap);
			break;
		default:
			printf("\033[32;1mYou Select A NULL Type!\n");
			break;
	}
	if (pHead == NULL) {
		puts("Range search memory failed!");
		return;
	}
	ResCount = gs;
	Res = pHead;// Res pointer points to linked list
}

PMAPS MemoryTool::RangeMemorySearch_DWORD(DWORD from_value, DWORD to_value, PMAPS pMap) {	// DWORD
	pid_t pid = getPID(bm);
	if (pid == 0) {
		puts("Failed to get process!");
		return NULL;
	}
	//*gs = 0;
	PMAPS pTemp = NULL;
	pTemp = pMap;
	PMAPS n, e;
	e = n = (PMAPS) malloc(LEN);
	PMAPS pBuff;
	pBuff = n;
	int iCount = 0;
	int c;
	DWORD buff[1024] = { 0 };
	memset(buff, 0, 4);
	while (pTemp != NULL) {
		c = (pTemp->taddr - pTemp->addr) / 4096;
		for (int j = 0; j < c; j++) {
			pread64(process_handle, buff, 0x1000, pTemp->addr + j * 4096);
			for (int i = 0; i < 1024; i++) {
				if (buff[i] >= from_value && buff[i] <= to_value) {
					iCount++;
					gs += 1;
					n->addr = (pTemp->addr) + (j * 4096) + (i * 4);
					n->type = TYPE_DWORD;
					if (iCount == 1) {
						n->next = NULL;
						e = n;
						pBuff = n;
					} else {
						n->next = NULL;
						e->next = n;
						e = n;
					}
					n = (PMAPS) malloc(LEN);
				}
			}
		}
		pTemp = pTemp->next;
	}
	free(n);
	return pBuff;
}

PMAPS MemoryTool::RangeMemorySearch_FLOAT(FLOAT from_value, FLOAT to_value, PMAPS pMap) {	// FLOAT
	pid_t pid = getPID(bm);
	if (pid == 0) {
		puts("Failed to get process!");
		return NULL;
	}
	//*gs = 0;
	PMAPS pTemp = NULL;
	pTemp = pMap;
	PMAPS n, e;
	e = n = (PMAPS) malloc(LEN);
	PMAPS pBuff;
	pBuff = n;
	int iCount = 0;	
	int c;
	FLOAT buff[1024] = { 0 };
	while (pTemp->next != NULL) {
		c = (pTemp->taddr - pTemp->addr) / 4096;
		for (int j = 0; j < c; j += 1) {
			pread64(process_handle, buff, 0x1000, pTemp->addr + (j * 4096));
			for (int i = 0; i < 1024; i += 1) {
				if (buff[i] >= from_value && buff[i] <= to_value) {
					iCount++;
					gs += 1;
					n->addr = (pTemp->addr) + (j * 4096) + (i * 4);
					n->type = TYPE_FLOAT;
					if (iCount == 1) {
						n->next = NULL;
						e = n;
						pBuff = n;
					} else {
						n->next = NULL;
						e->next = n;
						e = n;
					}
					n = (PMAPS) malloc(LEN);
				}
				// printf("buff[%d]=%f\n",l,buff[l]);
				// usleep(1);
			}
			// memset(buff,0,4);
		}
		pTemp = pTemp->next;
	}
	free(n);
	return pBuff;
}

PMAPS MemoryTool::RangeMemorySearch_DOUBLE(DOUBLE from_value, DOUBLE to_value, PMAPS pMap) {	// FLOAT
	pid_t pid = getPID(bm);
	if (pid == 0) {
		puts("Failed to get process!");
		return NULL;
	}
	//*gs = 0;
	PMAPS pTemp = NULL;
	pTemp = pMap;
	PMAPS n, e;
	e = n = (PMAPS) malloc(LEN);
	PMAPS pBuff;
	pBuff = n;
	int iCount = 0;
	int c;
	DOUBLE buff[1024] = { 0 };
	while (pTemp->next != NULL) {
		c = (pTemp->taddr - pTemp->addr) / 4096;
		for (int j = 0; j < c; j += 1) {
			pread64(process_handle, buff, 0x1000, pTemp->addr + (j * 4096));
			for (int i = 0; i < 1024; i += 1) {
				if (buff[i] >= from_value && buff[i] <= to_value) {
					iCount++;
					gs += 1;
					n->addr = (pTemp->addr) + (j * 4096) + (i * 4);
					n->type = TYPE_DOUBLE;
					if (iCount == 1) {
						n->next = NULL;
						e = n;
						pBuff = n;
					} else {
						n->next = NULL;
						e->next = n;
						e = n;
					}
					n = (PMAPS) malloc(LEN);
				}
				// printf("buff[%d]=%f\n",l,buff[l]);
				// usleep(1);
			}
			// memset(buff,0,4);
		}
		pTemp = pTemp->next;
	}
	free(n);
	return pBuff;
}

PMAPS MemoryTool::RangeMemorySearch_WORD(WORD from_value, WORD to_value, PMAPS pMap) {	// FLOAT
	pid_t pid = getPID(bm);
	if (pid == 0) {
		puts("Failed to get process!");
		return NULL;
	}
	//*gs = 0;
	PMAPS pTemp = NULL;
	pTemp = pMap;
	PMAPS n, e;
	e = n = (PMAPS) malloc(LEN);
	PMAPS pBuff;
	pBuff = n;
	int iCount = 0;	
	int c;
	WORD buff[1024] = { 0 };
	while (pTemp->next != NULL) {
		c = (pTemp->taddr - pTemp->addr) / 4096;
		for (int j = 0; j < c; j += 1) {
			pread64(process_handle, buff, 0x1000, pTemp->addr + (j * 4096));
			for (int i = 0; i < 1024; i += 1) {
				if (buff[i] >= from_value && buff[i] <= to_value) {
					iCount++;
					gs += 1;
					n->addr = (pTemp->addr) + (j * 4096) + (i * 4);
					n->type = TYPE_WORD;
					if (iCount == 1) {
						n->next = NULL;
						e = n;
						pBuff = n;
					} else {
						n->next = NULL;
						e->next = n;
						e = n;
					}
					n = (PMAPS) malloc(LEN);
				}
				// printf("buff[%d]=%f\n",l,buff[l]);
				// usleep(1);
			}
			// memset(buff,0,4);
		}
		pTemp = pTemp->next;
	}
	free(n);
	return pBuff;
}

PMAPS MemoryTool::RangeMemorySearch_BYTE(BYTE from_value, BYTE to_value, PMAPS pMap) {	// FLOAT
	pid_t pid = getPID(bm);
	if (pid == 0) {
		puts("Failed to get process!");
		return NULL;
	}
	//*gs = 0;
	PMAPS pTemp = NULL;
	pTemp = pMap;
	PMAPS n, e;
	e = n = (PMAPS) malloc(LEN);
	PMAPS pBuff;
	pBuff = n;
	int iCount = 0;
	int c;
	BYTE buff[1024] = { 0 };
	while (pTemp->next != NULL) {
		c = (pTemp->taddr - pTemp->addr) / 4096;
		for (int j = 0; j < c; j += 1) {
			pread64(process_handle, buff, 0x1000, pTemp->addr + (j * 4096));
			for (int i = 0; i < 1024; i += 1) {
				if (buff[i] >= from_value && buff[i] <= to_value) {
					iCount++;
					gs += 1;
					n->addr = (pTemp->addr) + (j * 4096) + (i * 4);
					n->type = TYPE_BYTE;
					if (iCount == 1) {
						n->next = NULL;
						e = n;
						pBuff = n;
					} else {
						n->next = NULL;
						e->next = n;
						e = n;
					}
					n = (PMAPS) malloc(LEN);
				}
				// printf("buff[%d]=%f\n",l,buff[l]);
				// usleep(1);
			}
			// memset(buff,0,4);
		}
		pTemp = pTemp->next;
	}
	free(n);
	return pBuff;
}

PMAPS MemoryTool::RangeMemorySearch_QWORD(QWORD from_value, QWORD to_value, PMAPS pMap) {	// FLOAT
	pid_t pid = getPID(bm);
	if (pid == 0) {
		puts("Failed to get process!");
		return NULL;
	}
	//*gs = 0;
	PMAPS pTemp = NULL;
	pTemp = pMap;
	PMAPS n, e;
	e = n = (PMAPS) malloc(LEN);
	PMAPS pBuff;
	pBuff = n;
	int iCount = 0;
	int c;
	QWORD buff[1024] = { 0 };
	while (pTemp->next != NULL) {
		c = (pTemp->taddr - pTemp->addr) / 4096;
		for (int j = 0; j < c; j += 1) {
			pread64(process_handle, buff, 0x1000, pTemp->addr + (j * 4096));
			for (int i = 0; i < 1024; i += 1) {
				if (buff[i] >= from_value && buff[i] <= to_value) {
					iCount++;
					gs += 1;
					n->addr = (pTemp->addr) + (j * 4096) + (i * 4);
					n->type = TYPE_QWORD;
					if (iCount == 1) {
						n->next = NULL;
						e = n;
						pBuff = n;
					} else {
						n->next = NULL;
						e->next = n;
						e = n;
					}
					n = (PMAPS) malloc(LEN);
				}
				// printf("buff[%d]=%f\n",l,buff[l]);
				// usleep(1);
			}
			// memset(buff,0,4);
		}
		pTemp = pTemp->next;
	}
	free(n);
	return pBuff;
}

void MemoryTool::MemorySearch(char *value, TYPE type) {
	PMAPS pHead = NULL;
	PMAPS pMap = NULL;
	gs = 0;
	switch (MemorySearchRange) {
		case ALL:
			pMap = readmaps(ALL);
			break;
		case B_BAD:
			pMap = readmaps(B_BAD);
			break;
		case C_ALLOC:
			pMap = readmaps(C_ALLOC);
			break;
		case C_BSS:
			pMap = readmaps(C_BSS);
			break;
		case C_DATA:
			pMap = readmaps(C_DATA);
			break;
		case C_HEAP:
			pMap = readmaps(C_HEAP);
			break;
		case JAVA_HEAP:
			pMap = readmaps(JAVA_HEAP);
			break;
		case A_ANONMYOUS:
			pMap = readmaps(A_ANONMYOUS);
			break;
		case CODE_SYSTEM:
			pMap = readmaps(CODE_SYSTEM);
			break;
		case STACK:
			pMap = readmaps(STACK);
			break;
		case ASHMEM:
			pMap = readmaps(ASHMEM);
			break;
		default:
			printf("\033[32;1mYou Select A NULL Type!\n");
			break;
	}
	if (pMap == NULL) {
		puts("map error");
		return (void)0;
	}
	switch (type) {
		case TYPE_DWORD:
			pHead = MemorySearch_DWORD(atoi(value), pMap);
			break;
		case TYPE_FLOAT:
			pHead = MemorySearch_FLOAT(atof(value), pMap);
			break;
		case TYPE_DOUBLE:
			pHead = MemorySearch_DOUBLE(atof(value), pMap);
			break;
		case TYPE_WORD:
			pHead = MemorySearch_WORD(atoi(value), pMap);
			break;
		case TYPE_BYTE:
			pHead = MemorySearch_BYTE(atoi(value), pMap);
			break;
		case TYPE_QWORD:
			pHead = MemorySearch_QWORD(atoi(value), pMap);
			break;
		default:
			printf("\033[32;1mYou Select A NULL Type!\n");
			break;
	}
	if (pHead == NULL) {
		puts("Failed to get process!");
		return;
	}
	ResCount = gs;
	Res = pHead;
}

PMAPS MemoryTool::MemorySearch_DWORD(DWORD value, PMAPS pMap) {
	pid_t pid = getPID(bm);
	if (pid == 0) {
		puts("Failed to get process!");
		return NULL;
	}
	//*gs = 0;
	PMAPS pTemp = NULL;
	pTemp = pMap;
	PMAPS n, e;
	e = n = (PMAPS) malloc(LEN);
	PMAPS pBuff;
	pBuff = n;
	int iCount = 0;
	int c;
	DWORD buff[1024] = { 0 };
	//memset(buff, 0, 4);
	while (pTemp != NULL) {
		c = (pTemp->taddr - pTemp->addr) / 4096;
		for (int j = 0; j < c; j++) {
			pread64(process_handle, buff, 0x1000, pTemp->addr + j * 4096);
			for (int i = 0; i < 1024; i++) {
				if (buff[i] == value) {
					iCount++;
					gs += 1;
					n->addr = (pTemp->addr) + (j * 4096) + (i * 4);
					n->type = TYPE_DWORD;
					// printf("addr:%lx,val:%d,buff=%d\n",n->addr,value,buff[i]);
					if (iCount == 1) {
						n->next = NULL;
						e = n;
						pBuff = n;
					} else {
						n->next = NULL;
						e->next = n;
						e = n;
					}
					n = (PMAPS) malloc(LEN);
				}
			}
		}
		pTemp = pTemp->next;
	}
	free(n);
	return pBuff;
}

PMAPS MemoryTool::MemorySearch_FLOAT(FLOAT value, PMAPS pMap) {
	pid_t pid = getPID(bm);
	if (pid == 0) {
		puts("Failed to get process!");
		return NULL;
	}
	//*gs = 0;
	PMAPS pTemp = NULL;
	pTemp = pMap;
	PMAPS n, e;
	e = n = (PMAPS) malloc(LEN);
	PMAPS pBuff;
	pBuff = n;
	int iCount = 0;
	int c;
	FLOAT buff[1024] = { 0 };
	while (pTemp->next != NULL) {
		c = (pTemp->taddr - pTemp->addr) / 4096;
		for (int j = 0; j < c; j += 1) {
			pread64(process_handle, buff, 0x1000, pTemp->addr + (j * 4096));
			for (int i = 0; i < 1024; i += 1) {
				if (buff[i] == value) {
					iCount++;
					gs += 1;
					n->addr = (pTemp->addr) + (j * 4096) + (i * 4);
					n->type = TYPE_FLOAT;
					if (iCount == 1) {
						n->next = NULL;
						e = n;
						pBuff = n;
					} else {
						n->next = NULL;
						e->next = n;
						e = n;
					}
					n = (PMAPS) malloc(LEN);
				}
				// printf("buff[%d]=%f\n",l,buff[l]);
				// usleep(1);
			}
			// memset(buff,0,4);
		}
		pTemp = pTemp->next;
	}
	free(n);
	return pBuff;
}

PMAPS MemoryTool::MemorySearch_DOUBLE(DOUBLE value, PMAPS pMap) {
	pid_t pid = getPID(bm);
	if (pid == 0) {
		puts("Failed to get process!");
		return NULL;
	}
	//*gs = 0;
	PMAPS pTemp = NULL;
	pTemp = pMap;
	PMAPS n, e;
	e = n = (PMAPS) malloc(LEN);
	PMAPS pBuff;
	pBuff = n;
	int iCount = 0;
	int c;
	DOUBLE buff[1024] = { 0 };
	while (pTemp->next != NULL) {
		c = (pTemp->taddr - pTemp->addr) / 4096;
		for (int j = 0; j < c; j += 1) {
			pread64(process_handle, buff, 0x1000, pTemp->addr + (j * 4096));
			for (int i = 0; i < 1024; i += 1) {
				if (buff[i] == value) {
					iCount++;
					gs += 1;
					n->addr = (pTemp->addr) + (j * 4096) + (i * 4);
					n->type = TYPE_DOUBLE;
					if (iCount == 1) {
						n->next = NULL;
						e = n;
						pBuff = n;
					} else {
						n->next = NULL;
						e->next = n;
						e = n;
					}
					n = (PMAPS) malloc(LEN);
				}
				// printf("buff[%d]=%f\n",l,buff[l]);
				// usleep(1);
			}
			// memset(buff,0,4);
		}
		pTemp = pTemp->next;
	}
	free(n);
	return pBuff;
}

PMAPS MemoryTool::MemorySearch_WORD(WORD value, PMAPS pMap) {
	pid_t pid = getPID(bm);
	if (pid == 0) {
		puts("Failed to get process!");
		return NULL;
	}
	//*gs = 0;
	PMAPS pTemp = NULL;
	pTemp = pMap;
	PMAPS n, e;
	e = n = (PMAPS) malloc(LEN);
	PMAPS pBuff;
	pBuff = n;
	int iCount = 0;
	int c;
	WORD buff[1024] = { 0 };
	while (pTemp->next != NULL) {
		c = (pTemp->taddr - pTemp->addr) / 4096;
		for (int j = 0; j < c; j += 1) {
			pread64(process_handle, buff, 0x1000, pTemp->addr + (j * 4096));
			n->type = TYPE_WORD;
			for (int i = 0; i < 1024; i += 1) {
				if (buff[i] == value) {
					iCount++;
					gs += 1;
					n->addr = (pTemp->addr) + (j * 4096) + (i * 4);
					if (iCount == 1) {
						n->next = NULL;
						e = n;
						pBuff = n;
					} else {
						n->next = NULL;
						e->next = n;
						e = n;
					}
					n = (PMAPS) malloc(LEN);
				}
				// printf("buff[%d]=%f\n",l,buff[l]);
				// usleep(1);
			}
			// memset(buff,0,4);
		}
		pTemp = pTemp->next;
	}
	free(n);
	return pBuff;
}

PMAPS MemoryTool::MemorySearch_BYTE(BYTE value, PMAPS pMap) {
	pid_t pid = getPID(bm);
	if (pid == 0) {
		puts("Failed to get process!");
		return NULL;
	}
	//*gs = 0;
	PMAPS pTemp = NULL;
	pTemp = pMap;
	PMAPS n, e;
	e = n = (PMAPS) malloc(LEN);
	PMAPS pBuff;
	pBuff = n;
	int iCount = 0;
	int c;
	BYTE buff[1024] = { 0 };
	while (pTemp->next != NULL) {
		c = (pTemp->taddr - pTemp->addr) / 4096;
		for (int j = 0; j < c; j += 1) {
			pread64(process_handle, buff, 0x1000, pTemp->addr + (j * 4096));
			for (int i = 0; i < 1024; i += 1) {
				if (buff[i] == value) {
					iCount++;
					gs += 1;
					n->addr = (pTemp->addr) + (j * 4096) + (i * 4);
					n->type = TYPE_BYTE;
					if (iCount == 1) {
						n->next = NULL;
						e = n;
						pBuff = n;
					} else {
						n->next = NULL;
						e->next = n;
						e = n;
					}
					n = (PMAPS) malloc(LEN);
				}
				// printf("buff[%d]=%f\n",l,buff[l]);
				// usleep(1);
			}
			// memset(buff,0,4);
		}
		pTemp = pTemp->next;
	}
	free(n);
	return pBuff;
}

PMAPS MemoryTool::MemorySearch_QWORD(QWORD value, PMAPS pMap) {
	pid_t pid = getPID(bm);
	if (pid == 0) {
		puts("Failed to get process!");
		return NULL;
	}
	//*gs = 0;
	PMAPS pTemp = NULL;
	pTemp = pMap;
	PMAPS n, e;
	e = n = (PMAPS) malloc(LEN);
	PMAPS pBuff;
	pBuff = n;
	int iCount = 0;
	int c;
	QWORD buff[1024] = { 0 };
	while (pTemp->next != NULL) {
		c = (pTemp->taddr - pTemp->addr) / 4096;
		for (int j = 0; j < c; j += 1) {
			pread64(process_handle, buff, 0x1000, pTemp->addr + (j * 4096));
			for (int i = 0; i < 1024; i += 1) {
				if (buff[i] == value) {
					iCount++;
					gs += 1;
					n->addr = (pTemp->addr) + (j * 4096) + (i * 4);
					n->type = TYPE_QWORD;
					if (iCount == 1) {
						n->next = NULL;
						e = n;
						pBuff = n;
					} else {
						n->next = NULL;
						e->next = n;
						e = n;
					}
					n = (PMAPS) malloc(LEN);
				}
				// printf("buff[%d]=%f\n",l,buff[l]);
				// usleep(1);
			}
			// memset(buff,0,4);
		}
		pTemp = pTemp->next;
	}
	free(n);
	return pBuff;
}

void MemoryTool::MemoryOffset(char *value, OFFSET offset, TYPE type) {
	PMAPS pHead = NULL;
	gs = 0;
	switch (type) {
		case TYPE_DWORD:
			pHead = MemoryOffset_DWORD(atoi(value), offset, Res);
			break;
		case TYPE_FLOAT:
			pHead = MemoryOffset_FLOAT(atof(value), offset, Res);
			break;
		case TYPE_DOUBLE:
			pHead = MemoryOffset_DOUBLE(atof(value), offset, Res);
			break;
		case TYPE_WORD:
			pHead = MemoryOffset_WORD(atoi(value), offset, Res);
			break;
		case TYPE_BYTE:
			pHead = MemoryOffset_BYTE(atoi(value), offset, Res);
			break;
		case TYPE_QWORD:
			pHead = MemoryOffset_QWORD(atoi(value), offset, Res);
			break;
		default:
			printf("\033[32;1mYou Select A NULL Type!\n");
			break;
	}
	if (pHead == NULL) {
		puts("Offset failed!");
		return (void)0;
	}
	ResCount = gs;
	ClearResults();	
	Res = pHead;
}

PMAPS MemoryTool::MemoryOffset_DWORD(DWORD value, OFFSET offset, PMAPS pBuff) {
	
	//*gs = 0;
	PMAPS pEnd = NULL;
	PMAPS pNew = NULL;
	PMAPS pTemp = pBuff;
	PMAPS BUFF = NULL;
	pEnd = pNew = (PMAPS) malloc(LEN);
	BUFF = pNew;
	int iCount = 0;
	long int all;
	DWORD *buf = (DWORD *)malloc(sizeof(DWORD));
	int jg;
	while (pTemp != NULL) {
		all = pTemp->addr + offset;
		pread64(process_handle, buf, 4, all);
		jg = *buf;
		if (jg == value) {
			iCount++;
			gs += 1;
			pNew->addr = pTemp->addr;
			pNew->type = TYPE_DWORD;
			if (iCount == 1) {
				pNew->next = NULL;
				pEnd = pNew;
				BUFF = pNew;
			} else {
				pNew->next = NULL;
				pEnd->next = pNew;
				pEnd = pNew;
			}
			pNew = (PMAPS) malloc(LEN);
			if (ResCount == 1) {
				free(pNew);
				close(process_handle);
				return BUFF;
			}
		}
		/* else { printf("jg:%d,value:%d\n",jg,value); } */
		pTemp = pTemp->next;
	}
	free(pNew);
	return BUFF;
}

PMAPS MemoryTool::MemoryOffset_FLOAT(FLOAT value, OFFSET offset, PMAPS pBuff) {
	
	//*gs = 0;
	PMAPS pEnd = NULL;
	PMAPS pNew = NULL;
	PMAPS pTemp = pBuff;
	PMAPS BUFF = NULL;
	pEnd = pNew = (PMAPS) malloc(LEN);
	BUFF = pNew;
	int iCount = 0;	
	long int all;
	FLOAT *buf = (FLOAT *)malloc(sizeof(FLOAT));
	// int buf[16];
	FLOAT jg;
	while (pTemp != NULL) {
		all = pTemp->addr + offset;
		//pread64(process_handle, buf, sizeof(FLOAT), all);
		pread64(process_handle, buf, 4, all);
		jg = *buf;
		if (jg == value) {
			iCount++;
			gs += 1;
			pNew->addr = pTemp->addr;
			pNew->type = TYPE_FLOAT;
			if (iCount == 1) {
				pNew->next = NULL;
				pEnd = pNew;
				BUFF = pNew;
			} else {
				pNew->next = NULL;
				pEnd->next = pNew;
				pEnd = pNew;
			}
			pNew = (PMAPS) malloc(LEN);
			if (ResCount == 1) {
				free(pNew);
				close(process_handle);
				return BUFF;
			}
		}
		/* else { printf("jg:%e,value:%e\n",jg,value); } */
		pTemp = pTemp->next;
	}
	free(pNew);
	return BUFF;
}

PMAPS MemoryTool::MemoryOffset_DOUBLE(DOUBLE value, OFFSET offset, PMAPS pBuff) {
	
	//*gs = 0;
	PMAPS pEnd = NULL;
	PMAPS pNew = NULL;
	PMAPS pTemp = pBuff;
	PMAPS BUFF = NULL;
	pEnd = pNew = (PMAPS) malloc(LEN);
	BUFF = pNew;
	int iCount = 0;
	long int all;
	DOUBLE *buf = (DOUBLE *)malloc(sizeof(DOUBLE));
	// int buf[16]; 
	DOUBLE jg;
	while (pTemp != NULL) {
		all = pTemp->addr + offset;
		pread64(process_handle, buf, 4, all);
		jg = *buf;
		if (jg == value) {
			iCount++;
			gs += 1;
			pNew->addr = pTemp->addr;
			pNew->type = TYPE_DOUBLE;
			if (iCount == 1) {
				pNew->next = NULL;
				pEnd = pNew;
				BUFF = pNew;
			} else {
				pNew->next = NULL;
				pEnd->next = pNew;
				pEnd = pNew;
			}
			pNew = (PMAPS) malloc(LEN);
			if (ResCount == 1) {
				free(pNew);
				close(process_handle);
				return BUFF;
			}
		}
		/* else { printf("jg:%e,value:%e\n",jg,value); } */
		pTemp = pTemp->next;
	}
	free(pNew);
	return BUFF;
}

PMAPS MemoryTool::MemoryOffset_WORD(WORD value, OFFSET offset, PMAPS pBuff) {
	
	PMAPS pEnd = NULL;
	PMAPS pNew = NULL;
	PMAPS pTemp = pBuff;
	PMAPS BUFF = NULL;
	pEnd = pNew = (PMAPS) malloc(LEN);
	BUFF = pNew;
	int iCount = 0;	
	long int all;			
	WORD *buf = (WORD *)malloc(sizeof(WORD));
	WORD jg;
	while (pTemp != NULL) {
		all = pTemp->addr + offset;	
		pread64(process_handle, buf, 4, all);
		jg = *buf;
		if (jg == value) {
			iCount++;
			gs += 1;
			pNew->addr = pTemp->addr;
			pNew->type = TYPE_WORD;
			if (iCount == 1) {
				pNew->next = NULL;
				pEnd = pNew;
				BUFF = pNew;
			} else {
				pNew->next = NULL;
				pEnd->next = pNew;
				pEnd = pNew;
			}
			pNew = (PMAPS) malloc(LEN);
			if (ResCount == 1) {
				free(pNew);
				close(process_handle);
				return BUFF;
			}
		}
		/* else { printf("jg:%e,value:%e\n",jg,value); } */
		pTemp = pTemp->next;
	}
	free(pNew);
	return BUFF;
}

PMAPS MemoryTool::MemoryOffset_BYTE(BYTE value, OFFSET offset, PMAPS pBuff) {
	
	PMAPS pEnd = NULL;
	PMAPS pNew = NULL;
	PMAPS pTemp = pBuff;
	PMAPS BUFF = NULL;
	pEnd = pNew = (PMAPS) malloc(LEN);
	BUFF = pNew;
	int iCount = 0;
	long int all;
	BYTE *buf = (BYTE *)malloc(sizeof(BYTE));

	BYTE jg;
	while (pTemp != NULL) {
		all = pTemp->addr + offset;
		pread64(process_handle, buf, 4, all);
		jg = *buf;
		if (jg == value) {
			iCount++;
			gs += 1;
			pNew->addr = pTemp->addr;
			pNew->type = TYPE_BYTE;
			if (iCount == 1) {
				pNew->next = NULL;
				pEnd = pNew;
				BUFF = pNew;
			} else {
				pNew->next = NULL;
				pEnd->next = pNew;
				pEnd = pNew;
			}
			pNew = (PMAPS) malloc(LEN);
			if (ResCount == 1) {
				free(pNew);
				close(process_handle);
				return BUFF;
			}
		}
		/* else { printf("jg:%e,value:%e\n",jg,value); } */
		pTemp = pTemp->next;
	}
	free(pNew);
	return BUFF;
}

PMAPS MemoryTool::MemoryOffset_QWORD(QWORD value, OFFSET offset, PMAPS pBuff) {
	
	PMAPS pEnd = NULL;
	PMAPS pNew = NULL;
	PMAPS pTemp = pBuff;
	PMAPS BUFF = NULL;
	pEnd = pNew = (PMAPS) malloc(LEN);
	BUFF = pNew;
	int iCount = 0;
	long int all;				
	QWORD *buf = (QWORD *)malloc(sizeof(QWORD));
	QWORD jg;
	while (pTemp != NULL) {
		all = pTemp->addr + offset;
		pread64(process_handle, buf, 4, all);
		jg = *buf;
		if (jg == value) {
			iCount++;
			gs += 1;
			
			pNew->addr = pTemp->addr;
			pNew->type = TYPE_QWORD;
			if (iCount == 1) {
				pNew->next = NULL;
				pEnd = pNew;
				BUFF = pNew;
			} else {
				pNew->next = NULL;
				pEnd->next = pNew;
				pEnd = pNew;
			}
			pNew = (PMAPS) malloc(LEN);
			if (ResCount == 1) {
				free(pNew);
				close(process_handle);
				return BUFF;
			}
		}
		/* else { printf("jg:%e,value:%e\n",jg,value); } */
		pTemp = pTemp->next;
	}
	free(pNew);
	return BUFF;
}


void MemoryTool::RangeMemoryOffset(char *from_value, char *to_value, OFFSET offset, TYPE type) {
	PMAPS pHead = NULL;
	gs = 0;
	switch (type) {
		case TYPE_DWORD:
			if (atoi(from_value) > atoi(to_value))
				pHead = RangeMemoryOffset_DWORD(atoi(to_value), atoi(from_value), offset, Res);
			else
				pHead = RangeMemoryOffset_DWORD(atoi(from_value), atoi(to_value), offset, Res);
			break;
		case TYPE_FLOAT:
			if (atof(from_value) > atof(to_value))
				pHead = RangeMemoryOffset_FLOAT(atof(to_value), atof(from_value), offset, Res);
			else
				pHead = RangeMemoryOffset_FLOAT(atof(from_value), atof(to_value), offset, Res);
			break;
		case TYPE_DOUBLE:
			if (atof(from_value) > atof(to_value))
				pHead = RangeMemoryOffset_DOUBLE(atof(to_value), atof(from_value), offset, Res);
			else
				pHead = RangeMemoryOffset_DOUBLE(atof(from_value), atof(to_value), offset, Res);
			break;
		case TYPE_WORD:
			if (atoi(from_value) > atoi(to_value))
				pHead = RangeMemoryOffset_WORD(atoi(to_value), atoi(from_value), offset, Res);
			else
				pHead = RangeMemoryOffset_WORD(atoi(from_value), atoi(to_value), offset, Res);
			break;
		case TYPE_BYTE:
			if (atoi(from_value) > atoi(to_value))
				pHead = RangeMemoryOffset_BYTE(atoi(to_value), atoi(from_value), offset, Res);
			else
				pHead = RangeMemoryOffset_BYTE(atoi(from_value), atoi(to_value), offset, Res);
			break;
		case TYPE_QWORD:
			if (atoi(from_value) > atoi(to_value))
				pHead = RangeMemoryOffset_QWORD(atoi(to_value), atoi(from_value), offset, Res);
			else
				pHead = RangeMemoryOffset_QWORD(atoi(from_value), atoi(to_value), offset, Res);
			break;
		default:
			printf("\033[32;1mYou Select A NULL Type!\n");
			break;
	}
	if (pHead == NULL) {
		puts("Offset failed!");
		return (void)0;
	}
	ResCount = gs;
	ClearResults();
	Res = pHead;
}

PMAPS MemoryTool::RangeMemoryOffset_DWORD(DWORD from_value, DWORD to_value, OFFSET offset, PMAPS pBuff) {
	
	//*gs = 0;					
	PMAPS pEnd = NULL;
	PMAPS pNew = NULL;
	PMAPS pTemp = pBuff;
	PMAPS BUFF = NULL;
	pEnd = pNew = (PMAPS) malloc(LEN);
	BUFF = pNew;
	int iCount = 0;		
	long int all;				
	DWORD *buf = (DWORD *)malloc(sizeof(DWORD));	
	DWORD jg;
	while (pTemp != NULL) {
		all = pTemp->addr + offset;
		pread64(process_handle, buf, 4, all);
		jg = *buf;
		if (jg >= from_value && jg <= to_value) {
			iCount++;
			gs += 1;
			pNew->addr = pTemp->addr;
			pNew->type = TYPE_DWORD;
			if (iCount == 1) {
				pNew->next = NULL;
				pEnd = pNew;
				BUFF = pNew;
			} else {
				pNew->next = NULL;
				pEnd->next = pNew;
				pEnd = pNew;
			}
			pNew = (PMAPS) malloc(LEN);
			if (ResCount == 1) {
				free(pNew);
				close(process_handle);
				return BUFF;
			}
		}
		/* else { printf("jg:%d,value:%d\n",jg,value); } */
		pTemp = pTemp->next;
	}
	free(pNew);
	return BUFF;
}

PMAPS MemoryTool::RangeMemoryOffset_FLOAT(FLOAT from_value, FLOAT to_value, OFFSET offset, PMAPS pBuff) {
	
	//*gs = 0;				
	PMAPS pEnd = NULL;
	PMAPS pNew = NULL;
	PMAPS pTemp = pBuff;
	PMAPS BUFF = NULL;
	pEnd = pNew = (PMAPS) malloc(LEN);
	BUFF = pNew;
	int iCount = 0;		
	long int all;				
	FLOAT *buf = (FLOAT *)malloc(sizeof(FLOAT));	
	// int buf[16]; /
	FLOAT jg;
	while (pTemp != NULL) {
		all = pTemp->addr + offset;	
		pread64(process_handle, buf, 4, all);
		jg = *buf;
		if (jg >= from_value && jg <= to_value) {
			iCount++;
			gs += 1;
			pNew->addr = pTemp->addr;
			pNew->type = TYPE_FLOAT;
			if (iCount == 1) {
				pNew->next = NULL;
				pEnd = pNew;
				BUFF = pNew;
			} else {
				pNew->next = NULL;
				pEnd->next = pNew;
				pEnd = pNew;
			}
			pNew = (PMAPS) malloc(LEN);
			if (ResCount == 1) {
				free(pNew);
				close(process_handle);
				return BUFF;
			}
		}
		/* else { printf("jg:%e,value:%e\n",jg,value); } */
		pTemp = pTemp->next;
	}
	free(pNew);
	return BUFF;
}

PMAPS MemoryTool::RangeMemoryOffset_DOUBLE(DOUBLE from_value, DOUBLE to_value, OFFSET offset, PMAPS pBuff) {
	
	//*gs = 0;				
	PMAPS pEnd = NULL;
	PMAPS pNew = NULL;
	PMAPS pTemp = pBuff;
	PMAPS BUFF = NULL;
	pEnd = pNew = (PMAPS) malloc(LEN);
	BUFF = pNew;
	int iCount = 0;		
	long int all;			
	DOUBLE *buf = (DOUBLE *)malloc(sizeof(DOUBLE));	
	// int buf[16];
	DOUBLE jg;
	while (pTemp != NULL) {
		all = pTemp->addr + offset;
		pread64(process_handle, buf, 4, all);
		jg = *buf;
		if (jg >= from_value && jg <= to_value) {
			iCount++;
			gs += 1;
			pNew->addr = pTemp->addr;
			pNew->type = TYPE_DOUBLE;
			if (iCount == 1) {
				pNew->next = NULL;
				pEnd = pNew;
				BUFF = pNew;
			} else {
				pNew->next = NULL;
				pEnd->next = pNew;
				pEnd = pNew;
			}
			pNew = (PMAPS) malloc(LEN);
			if (ResCount == 1) {
				free(pNew);
				close(process_handle);
				return BUFF;
			}
		}
		/* else { printf("jg:%e,value:%e\n",jg,value); } */
		pTemp = pTemp->next;
	}
	free(pNew);
	return BUFF;
}

PMAPS MemoryTool::RangeMemoryOffset_WORD(WORD from_value, WORD to_value, OFFSET offset, PMAPS pBuff) {
	
	//*gs = 0;
	PMAPS pEnd = NULL;
	PMAPS pNew = NULL;
	PMAPS pTemp = pBuff;
	PMAPS BUFF = NULL;
	pEnd = pNew = (PMAPS) malloc(LEN);
	BUFF = pNew;
	int iCount = 0;		
	long int all;
	WORD *buf = (WORD *)malloc(sizeof(WORD));	
	// int buf[16];
	WORD jg;
	while (pTemp != NULL) {
		all = pTemp->addr + offset;
		pread64(process_handle, buf, 4, all);
		jg = *buf;
		if (jg >= from_value && jg <= to_value) {
			iCount++;
			gs += 1;
			// printf("Modify,addr:%lx\n",all);
			pNew->addr = pTemp->addr;
			pNew->type = TYPE_WORD;
			if (iCount == 1) {
				pNew->next = NULL;
				pEnd = pNew;
				BUFF = pNew;
			} else {
				pNew->next = NULL;
				pEnd->next = pNew;
				pEnd = pNew;
			}
			pNew = (PMAPS) malloc(LEN);
			if (ResCount == 1) {
				free(pNew);
				close(process_handle);
				return BUFF;
			}
		}
		/* else { printf("jg:%e,value:%e\n",jg,value); } */
		pTemp = pTemp->next;
	}
	free(pNew);
	return BUFF;
}

PMAPS MemoryTool::RangeMemoryOffset_BYTE(BYTE from_value, BYTE to_value, OFFSET offset, PMAPS pBuff) {
	
	//*gs = 0;
	PMAPS pEnd = NULL;
	PMAPS pNew = NULL;
	PMAPS pTemp = pBuff;
	PMAPS BUFF = NULL;
	pEnd = pNew = (PMAPS) malloc(LEN);
	BUFF = pNew;
	int iCount = 0;		
	long int all;				
	BYTE *buf = (BYTE *)malloc(sizeof(BYTE));	
	// int buf[16]; 
	BYTE jg;
	while (pTemp != NULL) {
		all = pTemp->addr + offset;	
		pread64(process_handle, buf, 4, all);
		jg = *buf;
		if (jg >= from_value && jg <= to_value) {
			iCount++;
			gs += 1;
			pNew->addr = pTemp->addr;
			pNew->type = TYPE_BYTE;
			if (iCount == 1) {
				pNew->next = NULL;
				pEnd = pNew;
				BUFF = pNew;
			} else {
				pNew->next = NULL;
				pEnd->next = pNew;
				pEnd = pNew;
			}
			pNew = (PMAPS) malloc(LEN);
			if (ResCount == 1) {
				free(pNew);
				close(process_handle);
				return BUFF;
			}
		}
		/* else { printf("jg:%e,value:%e\n",jg,value); } */
		pTemp = pTemp->next;
	}
	free(pNew);
	return BUFF;
}

PMAPS MemoryTool::RangeMemoryOffset_QWORD(QWORD from_value, QWORD to_value, OFFSET offset, PMAPS pBuff) {
	
	//*gs = 0;
	PMAPS pEnd = NULL;
	PMAPS pNew = NULL;
	PMAPS pTemp = pBuff;
	PMAPS BUFF = NULL;
	pEnd = pNew = (PMAPS) malloc(LEN);
	BUFF = pNew;
	int iCount = 0;		
	long int all;				
	QWORD *buf = (QWORD *)malloc(sizeof(QWORD));	
	// int buf[16];
	QWORD jg;
	while (pTemp != NULL) {
		all = pTemp->addr + offset;	
		pread64(process_handle, buf, 4, all);
		jg = *buf;
		if (jg >= from_value && jg <= to_value) {
			iCount++;
			gs += 1;
			pNew->addr = pTemp->addr;
			pNew->type = TYPE_QWORD;
			if (iCount == 1) {
				pNew->next = NULL;
				pEnd = pNew;
				BUFF = pNew;
			} else {
				pNew->next = NULL;
				pEnd->next = pNew;
				pEnd = pNew;
			}
			pNew = (PMAPS) malloc(LEN);
			if (ResCount == 1) {
				free(pNew);
				close(process_handle);
				return BUFF;
			}
		}
		/* else { printf("jg:%e,value:%e\n",jg,value); } */
		pTemp = pTemp->next;
	}
	free(pNew);
	return BUFF;
}

void MemoryTool::MemoryWrite(char *value, OFFSET offset, TYPE type) {
	switch (type) {
		case TYPE_DWORD:
			MemoryWrite_DWORD(atoi(value), Res, offset);
			break;
		case TYPE_FLOAT:
			MemoryWrite_FLOAT(atof(value), Res, offset);
			break;
		case TYPE_DOUBLE:
			MemoryWrite_DOUBLE(atof(value), Res, offset);
			break;
		case TYPE_WORD:
			MemoryWrite_WORD(atoi(value), Res, offset);
			break;
		case TYPE_BYTE:
			MemoryWrite_BYTE(atoi(value), Res, offset);
			break;
		case TYPE_QWORD:
			MemoryWrite_QWORD(atoi(value), Res, offset);
			break;
		default:
			printf("\033[32;1mYou Select A NULL Type!\n");
			break;
	}
	// ClearResults();//cleer list
}
int MemoryWrite_DWORD(DWORD value, PMAPS pBuff, OFFSET offset) {
	
	PMAPS pTemp = NULL;
	pTemp = pBuff;
	int i;
	for (i = 0; i < ResCount; i++) {
		pwrite64(process_handle, &value, 4, pTemp->addr + offset);
		if (pTemp->next != NULL)
			pTemp = pTemp->next;
	}
	return 0;
}

int MemoryTool::MemoryWrite_FLOAT(FLOAT value, PMAPS pBuff, OFFSET offset) {
	
	PMAPS pTemp = NULL;
	pTemp = pBuff;
	int i;
	for (i = 0; i < ResCount; i++) {
		pwrite64(process_handle, &value, 4, pTemp->addr + offset);
		if (pTemp->next != NULL)
			pTemp = pTemp->next;
	}
	return 0;
}

int MemoryTool::MemoryWrite_DOUBLE(DOUBLE value, PMAPS pBuff, OFFSET offset) {
	
	PMAPS pTemp = NULL;
	int process_handle;
	pTemp = pBuff;
	int i;
	for (i = 0; i < ResCount; i++) {
		pwrite64(process_handle, &value, 4, pTemp->addr + offset);
		if (pTemp->next != NULL)
			pTemp = pTemp->next;
	}
	return 0;
}

int MemoryTool::MemoryWrite_WORD(WORD value, PMAPS pBuff, OFFSET offset) {
	
	PMAPS pTemp = NULL;
	pTemp = pBuff;
	int i;
	for (i = 0; i < ResCount; i++) {
		pwrite64(process_handle, &value, 4, pTemp->addr + offset);
		if (pTemp->next != NULL)
			pTemp = pTemp->next;
	}
	return 0;
}

int MemoryTool::MemoryWrite_BYTE(BYTE value, PMAPS pBuff, OFFSET offset) {
	
	PMAPS pTemp = NULL;
	pTemp = pBuff;
	int i;
	for (i = 0; i < ResCount; i++) {
		pwrite64(process_handle, &value, 4, pTemp->addr + offset);
		if (pTemp->next != NULL)
			pTemp = pTemp->next;
	}
	return 0;
}

int MemoryTool::MemoryWrite_QWORD(QWORD value, PMAPS pBuff, OFFSET offset) {
	
	PMAPS pTemp = NULL;
	pTemp = pBuff;
	int i;
	for (i = 0; i < ResCount; i++) {
		pwrite64(process_handle, &value, 4, pTemp->addr + offset);
		if (pTemp->next != NULL)
			pTemp = pTemp->next;
	}
	return 0;
}

int MemoryTool::WriteAddress(ADDRESS addr, char *value, TYPE type) {
	switch (type) {
		case TYPE_DWORD:
			WriteAddress_DWORD(addr, atoi(value));
			break;
		case TYPE_FLOAT:
			WriteAddress_FLOAT(addr, atof(value));
			break;
		case TYPE_DOUBLE:
			WriteAddress_DOUBLE(addr, atof(value));
			break;
		case TYPE_WORD:
			WriteAddress_WORD(addr, atoi(value));
			break;
		case TYPE_BYTE:
			WriteAddress_BYTE(addr, atoi(value));
			break;
		case TYPE_QWORD:
			WriteAddress_QWORD(addr, atoi(value));
			break;
		default:
			printf("\033[32;1mYou Select A NULL Type!\n");
			break;
	}
	return 0;
}

int MemoryTool::WriteAddress_DWORD(ADDRESS addr, DWORD value) {
	
	pwrite64(process_handle, &value, 4, addr);
	return 0;
}

int MemoryTool::WriteAddress_FLOAT(ADDRESS addr, FLOAT value) {
	
	pwrite64(process_handle, &value, 4, addr);
	return 0;
}

int MemoryTool::WriteAddress_DOUBLE(ADDRESS addr, DOUBLE value) {
	
	pwrite64(process_handle, &value, 4, addr);
	return 0;
}

int MemoryTool::WriteAddress_WORD(ADDRESS addr, WORD value) {
	
	pwrite64(process_handle, &value, 4, addr);
	return 0;
}

int MemoryTool::WriteAddress_BYTE(ADDRESS addr, BYTE value) {
	
	pwrite64(process_handle, &value, 4, addr);
	return 0;
}

int MemoryTool::WriteAddress_QWORD(ADDRESS addr, QWORD value) {
	pwrite64(process_handle, &value, 4, addr);
	return 0;
}

int MemoryTool::isapkinstalled(PACKAGENAME * bm) {
	char LJ[128];
	sprintf(LJ, "/data/data/%s/", bm);
	DIR *dir;
	dir = opendir(LJ);
	if (dir == NULL) {
		return 0;
	} else {
		return 1;
	}
}

int MemoryTool::isapkrunning(PACKAGENAME * bm) {
	DIR *dir = NULL;
	struct dirent *ptr = NULL;
	FILE *fp = NULL;
	char filepath[50];
	char filetext[128];
	dir = opendir("/proc/");	
	if (dir != NULL) {
		while ((ptr = readdir(dir)) != NULL) {
			if ((strcmp(ptr->d_name, ".") == 0) || (strcmp(ptr->d_name, "..") == 0))
				continue;
			if (ptr->d_type != DT_DIR)
				continue;
			sprintf(filepath, "/proc/%s/cmdline", ptr->d_name);
			fp = fopen(filepath, "r");
			if (NULL != fp) {
				fgets(filetext, sizeof(filetext), fp);
				if (strcmp(filetext, bm) == 0) {
					closedir(dir);
					return 1;
				}
				fclose(fp);
			}
		}
	}
	closedir(dir);			
	return 0;
}

int MemoryTool::uninstallapk(PACKAGENAME * bm) {
	char ml[128];
	sprintf(ml, "pm uninstall %s", bm);
	system(ml);
	system("clear");
	return 0;
}

int MemoryTool::installapk(char *lj) {
	char ml[128];
	sprintf(ml, "pm install %s", lj);
	system(ml);
	system("clear");
	return 0;
}

int MemoryTool::killprocess(PACKAGENAME * bm) {
	int pid = getPID(bm);
	if (pid == 0) {
		return -1;
	}
	char ml[32];
	sprintf(ml, "kill %d", pid);
	system(ml);					// 杀掉进程
	return 0;
}

char MemoryTool::GetProcessState(PACKAGENAME * bm) {
	/*
    D uninterruptible sleep state (usually IO processes); R
    running, in an interruptible queue; S
    In dormant state, quiescent state; T
    Stop or be tracked, suspend execution; W
    into memory swap (not valid since kernel 2.6); X
    Dead process; Z zombie process does not exist but cannot be eliminated temporarily; W:
    Not enough memory pages to allocate WCHAN
    waiting process resource; <: high priority process N:
    Low priority process L: has memory page allocated and locked in memory
    (instant system or subject to A I/O), i.e., some pages are locked into memory s
    the leader of the process (with child processes under it); l
    multi-process (using CLONE_THREAD, similar to NPTL pthreads); +
    Process group in the background; */

	int pid = getPID(bm);
	if (pid == 0) {
		return 0;
	}
	FILE *fp;
	char lj[64];
	char buff[64];
	char zt;
	char zt1[16];
	sprintf(lj, "/proc/%d/status", pid);
	fp = fopen(lj, "r");
	if (fp == NULL) {
		return 0;
	}

	while (!feof(fp)) {
		fgets(buff, sizeof(buff), fp);
		if (strstr(buff, "State")) {
			sscanf(buff, "State: %c", &zt);
			// printf("state:%c\n",zt);
			// sleep(1);
			// puts("emmmm");
			break;
		}
	}
	// putchar(zt);
	// puts(zt2);
	fclose(fp);
	// puts("loopopp");
	return zt;
}

int MemoryTool::rebootsystem() {
	return system("su -c 'reboot'");
}

int MemoryTool::PutDate() {
	return system("date +%F-%T");
}

int MemoryTool::GetDate(char *date) {
	FILE *fp;
	system("date +%F-%T > log.txt");
	if ((fp = fopen("log.txt", "r")) == NULL) {
		return 0;
	}
	fscanf(fp, "%s", date);	
	remove("log.txt");
	return 1;
}

int MemoryTool::killGG() {
	// There is a folder named GG-**** in /data/data/[GG modifier package name]/files/
    // If there is this folder, get the package name mentioned above and kill the GG modifier
	
    DIR *dir = NULL;
	DIR *dirGG = NULL;
	struct dirent *ptr = NULL;
	struct dirent *ptrGG = NULL;
	char filepath[256];			// The size is arbitrary, the path that can install the cmdline file can be
	char filetext[128];			
	dir = opendir("/data/data");
	// puts("killing GG");
	int flag = 1;
	if (dir != NULL) {
		while (flag && (ptr = readdir(dir)) != NULL) {
			if ((strcmp(ptr->d_name, ".") == 0) || (strcmp(ptr->d_name, "..") == 0))
				continue;
			if (ptr->d_type != DT_DIR)
				continue;
			sprintf(filepath, "/data/data/%s/files", ptr->d_name);
			dirGG = opendir(filepath);
			if (dirGG != NULL) {
				while ((ptrGG = readdir(dirGG)) != NULL) {
					if ((strcmp(ptrGG->d_name, ".") == 0) || (strcmp(ptr->d_name, "..") == 0))
						continue;
					if (ptrGG->d_type != DT_DIR)
						continue;
					if (strstr(ptrGG->d_name, "GG")) {
						int pid;	// pid
						pid = getPID(ptr->d_name);
						if (pid == 0)	
							continue;
						else
							killprocess(ptr->d_name);
					}
				}
			}
		}
	}
	closedir(dir);	
	closedir(dirGG);
	return 0;
}

int MemoryTool::killXs() {
	DIR *dir = NULL;
	struct dirent *ptr = NULL;
	char filepath[256];
	char filetext[128];	
	dir = opendir("/data/data");
	FILE *fp = NULL;
	if (NULL != dir) {
		while ((ptr = readdir(dir)) != NULL) {
			if ((strcmp(ptr->d_name, ".") == 0) || (strcmp(ptr->d_name, "..") == 0))
				continue;
			if (ptr->d_type != DT_DIR)
				continue;
			// /data/data/%s/lib/libxscript.so
			sprintf(filepath, "/data/data/%s/lib/libxscript.so", ptr->d_name);
			fp = fopen(filepath, "r");
			if (fp == NULL)
				continue;
			else {
				killprocess(ptr->d_name);
			}
			// killprocess(ptr->d_name);
		}
	}
	closedir(dir);
	return 0;
}

void MemoryTool::FreezeThread() {
	int pid;
	pid = getPID(bm);
	if (pid == 0) {
		puts("Error -1");
		return;
	}
	DWORD buf_i;
	FLOAT buf_f;
	DOUBLE buf_d;
	WORD buf_w;
	BYTE buf_b;
	QWORD buf_q;
	if (process_handle == -1) {
		puts("Error -2");
		return;
	}
	PFREEZE pTemp = value_freeze;
	while (Freeze == 1) {
		pid = getPID(bm);
		if (pid == 0) {
			puts("Game is Not Run");
			exit(1);
			break;
		}
		for (int i = 0; i < FreezeCount; i++) {
			switch (pTemp->type) {
				case TYPE_DWORD:
					buf_i = atoi(pTemp->value);
					pwrite64(process_handle, &buf_i, 4, pTemp->addr);
					break;
				case TYPE_FLOAT:
					buf_f = atof(pTemp->value);
					pwrite64(process_handle, &buf_f, 4, pTemp->addr);
					break;
				case TYPE_DOUBLE:
					buf_d = atof(pTemp->value);
					pwrite64(process_handle, &buf_d, 4, pTemp->addr);
					break;
				case TYPE_WORD:
					buf_w = atoi(pTemp->value);
					pwrite64(process_handle, &buf_w, 4, pTemp->addr);
					break;
				case TYPE_BYTE:
					buf_b = atoi(pTemp->value);
					pwrite64(process_handle, &buf_b, 4, pTemp->addr);
					break;
				case TYPE_QWORD:
					buf_q = atoi(pTemp->value);
					pwrite64(process_handle, &buf_q, 4, pTemp->addr);
					break;
				default:
					break;
			}
			pTemp = pTemp->next;
			usleep(delay);
		}
		pTemp = value_freeze;
	}
	return;
}

PMAPS MemoryTool::GetResults() {
	if (Res == NULL) {
		return NULL;
	} else {
		return Res;	
	}
}

int MemoryTool::AddFreezeItem_All(char *Value, TYPE type, OFFSET offset) {
	if (ResCount == 0) {
		return -1;
	}
	PMAPS pTemp = Res;
	for (int i = 0; i < ResCount; i++) {
		switch (type) {
			case TYPE_DWORD:
				AddFreezeItem(pTemp->addr, Value, TYPE_DWORD, offset);
				break;
			case TYPE_FLOAT:
				AddFreezeItem(pTemp->addr, Value, TYPE_FLOAT, offset);
				break;
			case TYPE_DOUBLE:
				AddFreezeItem(pTemp->addr, Value, TYPE_DOUBLE, offset);
				break;
			case TYPE_WORD:
				AddFreezeItem(pTemp->addr, Value, TYPE_WORD, offset);
				break;
			case TYPE_BYTE:
				AddFreezeItem(pTemp->addr, Value, TYPE_BYTE, offset);
				break;
			case TYPE_QWORD:
				AddFreezeItem(pTemp->addr, Value, TYPE_QWORD, offset);
				break;
			default:
				SetTextColor(COLOR_SKY_BLUE);
				puts("You Choose a NULL type");
				break;
		}
		pTemp = pTemp->next;
	}
	return 0;
}

int MemoryTool::AddFreezeItem(ADDRESS addr, char *value, TYPE type, OFFSET offset) {
	switch (type) {
		case TYPE_DWORD:
			AddFreezeItem_DWORD(addr + offset, value);
			break;
		case TYPE_FLOAT:
			AddFreezeItem_FLOAT(addr + offset, value);
			break;
		case TYPE_DOUBLE:
			AddFreezeItem_DOUBLE(addr + offset, value);
			break;
		case TYPE_WORD:
			AddFreezeItem_WORD(addr + offset, value);
			break;
		case TYPE_BYTE:
			AddFreezeItem_BYTE(addr + offset, value);
			break;
		case TYPE_QWORD:
			AddFreezeItem_QWORD(addr + offset, value);
			break;
		default:
			SetTextColor(COLOR_SKY_BLUE);
			puts("You Choose a NULL type");
			break;
	}
	return 0;
}

int MemoryTool::AddFreezeItem_DWORD(ADDRESS addr, char *value) {
	if (FreezeCount == 0) {
		value_freeze = pEnd = pNew = (PFREEZE) malloc(FRE);
		pNew->next = NULL;
		pEnd = pNew;
		value_freeze = pNew;
		pNew->addr = addr;
		pNew->type = TYPE_DWORD;
		pNew->value = value;
		FreezeCount += 1;
	} else {
		pNew = (PFREEZE) malloc(FRE);
		pNew->next = NULL;
		pEnd->next = pNew;
		pEnd = pNew;
		pNew->addr = addr;
		pNew->type = TYPE_DWORD;
		pNew->value = value;
		FreezeCount += 1;
	}
	return 0;
}

int MemoryTool::AddFreezeItem_FLOAT(ADDRESS addr, char *value) {
	if (FreezeCount == 0) {
		value_freeze = pEnd = pNew = (PFREEZE) malloc(FRE);
		pNew->next = NULL;
		pEnd = pNew;
		value_freeze = pNew;
		pNew->addr = addr;
		pNew->type = TYPE_FLOAT;
		pNew->value = value;
		FreezeCount += 1;
	} else {
		pNew = (PFREEZE) malloc(FRE);
		pNew->next = NULL;
		pEnd->next = pNew;
		pEnd = pNew;
		pNew->addr = addr;
		pNew->type = TYPE_FLOAT;
		pNew->value = value;
		FreezeCount += 1;
	}
	return 0;
}

int MemoryTool::AddFreezeItem_DOUBLE(ADDRESS addr, char *value) {
	if (FreezeCount == 0) {
		value_freeze = pEnd = pNew = (PFREEZE) malloc(FRE);
		pNew->next = NULL;
		pEnd = pNew;
		value_freeze = pNew;
		pNew->addr = addr;
		pNew->type = TYPE_DOUBLE;
		pNew->value = value;
		FreezeCount += 1;
	} else {
		pNew = (PFREEZE) malloc(FRE);
		pNew->next = NULL;
		pEnd->next = pNew;
		pEnd = pNew;
		pNew->addr = addr;
		pNew->type = TYPE_DOUBLE;
		pNew->value = value;
		FreezeCount += 1;
	}
	return 0;
}

int MemoryTool::AddFreezeItem_WORD(ADDRESS addr, char *value) {
	if (FreezeCount == 0) {
		value_freeze = pEnd = pNew = (PFREEZE) malloc(FRE);
		pNew->next = NULL;
		pEnd = pNew;
		value_freeze = pNew;
		pNew->addr = addr;
		pNew->type = TYPE_WORD;
		pNew->value = value;
		FreezeCount += 1;
	} else {
		pNew = (PFREEZE) malloc(FRE);
		pNew->next = NULL;
		pEnd->next = pNew;
		pEnd = pNew;
		pNew->addr = addr;
		pNew->type = TYPE_WORD;
		pNew->value = value;
		FreezeCount += 1;
	}
	return 0;
}

int MemoryTool::AddFreezeItem_BYTE(ADDRESS addr, char *value) {
	if (FreezeCount == 0) {
		value_freeze = pEnd = pNew = (PFREEZE) malloc(FRE);
		pNew->next = NULL;
		pEnd = pNew;
		value_freeze = pNew;
		pNew->addr = addr;
		pNew->type = TYPE_BYTE;
		pNew->value = value;
		FreezeCount += 1;
	} else {
		pNew = (PFREEZE) malloc(FRE);
		pNew->next = NULL;
		pEnd->next = pNew;
		pEnd = pNew;
		pNew->addr = addr;
		pNew->type = TYPE_BYTE;
		pNew->value = value;
		FreezeCount += 1;
	}
	return 0;
}

int MemoryTool::AddFreezeItem_QWORD(ADDRESS addr, char *value) {
	if (FreezeCount == 0) {
		value_freeze = pEnd = pNew = (PFREEZE) malloc(FRE);
		pNew->next = NULL;
		pEnd = pNew;
		value_freeze = pNew;
		pNew->addr = addr;	
		pNew->type = TYPE_QWORD;
		pNew->value = value;
		FreezeCount += 1;
	} else {
		pNew = (PFREEZE) malloc(FRE);
		pNew->next = NULL;
		pEnd->next = pNew;
		pEnd = pNew;
		pNew->addr = addr;
		pNew->type = TYPE_QWORD;
		pNew->value = value;
		FreezeCount += 1;
	}
	return 0;
}

int MemoryTool::RemoveFreezeItem(ADDRESS addr) {
	PFREEZE pTemp = value_freeze;
	PFREEZE p1 = NULL;
	PFREEZE p2 = NULL;
	for (int i = 0; i < FreezeCount; i++) {
		p1 = pTemp;
		p2 = pTemp->next;
		if (pTemp->addr == addr) {
			p1->next = p2;
			free(pTemp);
			FreezeCount -= 1;
			// printf("Freeze count:%d\n",FreezeCount);
            // break;//Prevent the address from freezing repeatedly, so it is not added, of course, it can also be added
		}
		pTemp = p2;
	}
	return 0;
}

int MemoryTool::RemoveFreezeItem_All() {
	PFREEZE pHead = value_freeze;
	PFREEZE pTemp = pHead;
	int i;
	for (i = 0; i < FreezeCount; i++) {
		pTemp = pHead;
		pHead = pHead->next;
		free(pTemp);
		FreezeCount -= 1;
	}
	free(value_freeze);
	FreezeCount -= 1;
	return 0;
}

int MemoryTool::StartFreeze() {
	if (Freeze == 1) {
		return -1;
	}
	//int a;
	//strcpy(Fbm, bm);	
	Freeze = 1;
	//pthread_create(&pth, NULL, FreezeThread, NULL);
	thread th1(&MemoryTool::FreezeThread);
	//th1.join();
	return 0;
}

int MemoryTool::StopFreeze() {
	Freeze = 0;
	return 0;
}

int MemoryTool::SetFreezeDelay(long int De) {
	delay = De;
	return 0;
}

int MemoryTool::PrintFreezeItems() {
	PFREEZE pTemp = value_freeze;
	for (int i = 0; i < FreezeCount; i++) {
		const char *type = (char *)malloc(sizeof(char));
		switch (pTemp->type) {
			case TYPE_DWORD:
				type = "DWORD";
				break;
			case TYPE_FLOAT:
				type = "FLOAT";
				break;
			case TYPE_DOUBLE:
				type = "DOUBLE";
				break;
			case TYPE_WORD:
				type = "WORD";
				break;
			case TYPE_BYTE:
				type = "BYTE";
				break;
			case TYPE_QWORD:
				type = "QWORD";
				break;
			default:
				type = "";
				break;
		}
		printf("FreezeAddr:0x%lX  Type:%s  Value:%s\n",pTemp->addr,type,pTemp->value);
		pTemp = pTemp->next;
	}
	return 0;
}


int MemoryTool::SetSearchRange(TYPE type) {
	switch (type) {
		case ALL:
			MemorySearchRange = 0;
			break;
		case B_BAD:
			MemorySearchRange = 1;
			break;
		case C_ALLOC:
			MemorySearchRange = 2;
			break;
		case C_BSS:
			MemorySearchRange = 3;
			break;
		case C_DATA:
			MemorySearchRange = 4;
			break;
		case C_HEAP:
			MemorySearchRange = 5;
			break;
		case JAVA_HEAP:
			MemorySearchRange = 6;
			break;
		case A_ANONMYOUS:
			MemorySearchRange = 7;
			break;
		case CODE_SYSTEM:
			MemorySearchRange = 8;
			break;
		case STACK:
			MemorySearchRange = 9;
			break;
		case ASHMEM:
			MemorySearchRange = 10;
			break;
		default:
			printf("\033[32;1mYou Select A NULL Type!\n");
			break;
	}
	return 0;
}

PMAPS MemoryTool::readmaps(TYPE type) {
	PMAPS pMap = NULL;
	switch (type) {
		case ALL:
			pMap = readmaps_all();
			break;
		case B_BAD:
			pMap = readmaps_bad();
			break;
		case C_ALLOC:
			pMap = readmaps_c_alloc();
			break;
		case C_BSS:
			pMap = readmaps_c_bss();
			break;
		case C_DATA:
			pMap = readmaps_c_data();
			break;
		case C_HEAP:
			pMap = readmaps_c_heap();
			break;
		case JAVA_HEAP:
			pMap = readmaps_java_heap();
			break;
		case A_ANONMYOUS:
			pMap = readmaps_a_anonmyous();
			break;
		case CODE_SYSTEM:
			pMap = readmaps_code_system();
			break;
		case STACK:
			pMap = readmaps_stack();
			break;
		case ASHMEM:
			pMap = readmaps_ashmem();
			break;
		default:
			printf("\033[32;1mYou Select A NULL Type!\n");
			break;
	}
	if (pMap == NULL) {
		return 0;
	}
	return pMap;
}

PMAPS MemoryTool::readmaps_all() {
	PMAPS pHead = NULL;
	PMAPS pNew;
	PMAPS pEnd;
	pEnd = pNew = (PMAPS) malloc(LEN);
	FILE *fp;
	int i = 0, flag = 1;
	char lj[64], buff[256];
	int pid = getPID(bm);
	sprintf(lj, "/proc/%d/maps", pid);
	fp = fopen(lj, "r");
	if (fp == NULL) {
		puts("Memory read failed!");
		return NULL;
	}
	while (!feof(fp)) {
		fgets(buff, sizeof(buff), fp);
		if (strstr(buff, "rw") != NULL && !feof(fp)) {
			sscanf(buff, "%lx-%lx", &pNew->addr, &pNew->taddr);
			flag = 1;
		} else {
			flag = 0;
		}
		if (flag == 1) {
			i++;
			if (i == 1) {
				pNew->next = NULL;
				pEnd = pNew;
				pHead = pNew;
			} else {
				pNew->next = NULL;
				pEnd->next = pNew;
				pEnd = pNew;
			}
			pNew = (PMAPS) malloc(LEN);	
		}
	}
	free(pNew);
	fclose(fp);
	return pHead;
}

PMAPS MemoryTool::readmaps_bad() {
	PMAPS pHead = NULL;
	PMAPS pNew = NULL;
	PMAPS pEnd = NULL;
	pEnd = pNew = (PMAPS) malloc(LEN);
	FILE *fp;
	int i = 0, flag = 1;
	char lj[64], buff[256];
	int pid = getPID(bm);
	sprintf(lj, "/proc/%d/maps", pid);
	fp = fopen(lj, "r");
	if (fp == NULL) {
		puts("Memory read failed!");
		return NULL;
	}
	while (!feof(fp)) {
		fgets(buff, sizeof(buff), fp);
		if (strstr(buff, "rw") != NULL && !feof(fp) && strstr(buff, "kgsl-3d0")) {
			sscanf(buff, "%lx-%lx", &pNew->addr, &pNew->taddr);
			flag = 1;
		} else {
			flag = 0;
		}
		if (flag == 1) {
			i++;
			if (i == 1) {
				pNew->next = NULL;
				pEnd = pNew;
				pHead = pNew;
			} else {
				pNew->next = NULL;
				pEnd->next = pNew;
				pEnd = pNew;
			}
			pNew = (PMAPS) malloc(LEN);	
		}
	}
	free(pNew);			
	fclose(fp);
	return pHead;
}

PMAPS MemoryTool::readmaps_c_alloc() {
	PMAPS pHead = NULL;
	PMAPS pNew = NULL;
	PMAPS pEnd = NULL;
	pEnd = pNew = (PMAPS) malloc(LEN);
	FILE *fp;
	int i = 0, flag = 1;
	char lj[64], buff[256];
	int pid = getPID(bm);
	sprintf(lj, "/proc/%d/maps", pid);
	fp = fopen(lj, "r");
	if (fp == NULL) {
		puts("Memory read failed!");
		return NULL;
	}
	while (!feof(fp)) {
		fgets(buff, sizeof(buff), fp);	
		if (strstr(buff, "rw") != NULL && !feof(fp) && strstr(buff, "[anon:libc_malloc]")) {
			sscanf(buff, "%lx-%lx", &pNew->addr, &pNew->taddr);
			flag = 1;
		} else {
			flag = 0;
		}
		if (flag == 1) {
			i++;
			if (i == 1) {
				pNew->next = NULL;
				pEnd = pNew;
				pHead = pNew;
			} else {
				pNew->next = NULL;
				pEnd->next = pNew;
				pEnd = pNew;
			}
			pNew = (PMAPS) malloc(LEN);
		}
	}
	free(pNew);				
	fclose(fp);				
	return pHead;
}

PMAPS MemoryTool::readmaps_c_bss() {
	PMAPS pHead = NULL;
	PMAPS pNew = NULL;
	PMAPS pEnd = NULL;
	pEnd = pNew = (PMAPS) malloc(LEN);
	FILE *fp;
	int i = 0, flag = 1;
	char lj[64], buff[256];
	int pid = getPID(bm);
	sprintf(lj, "/proc/%d/maps", pid);
	fp = fopen(lj, "r");
	if (fp == NULL) {
		puts("Memory read failed!");
		return NULL;
	}
	while (!feof(fp)) {
		fgets(buff, sizeof(buff), fp);
		if (strstr(buff, "rw") != NULL && !feof(fp) && strstr(buff, "[anon:.bss]")) {
			sscanf(buff, "%lx-%lx", &pNew->addr, &pNew->taddr);
			flag = 1;
		} else {
			flag = 0;
		}
		if (flag == 1) {
			i++;
			if (i == 1) {
				pNew->next = NULL;
				pEnd = pNew;
				pHead = pNew;
			} else {
				pNew->next = NULL;
				pEnd->next = pNew;
				pEnd = pNew;
			}
			pNew = (PMAPS) malloc(LEN);
		}
	}
	free(pNew);				
	fclose(fp);				
	return pHead;
}

PMAPS MemoryTool::readmaps_c_data() {
	PMAPS pHead = NULL;
	PMAPS pNew = NULL;
	PMAPS pEnd = NULL;
	pEnd = pNew = (PMAPS) malloc(LEN);
	FILE *fp;
	int i = 0, flag = 1;
	char lj[64], buff[256];
	int pid = getPID(bm);
	sprintf(lj, "/proc/%d/maps", pid);
	fp = fopen(lj, "r");
	if (fp == NULL) {
		puts("Memory read failed!");
		return NULL;
	}
	while (!feof(fp)) {
		fgets(buff, sizeof(buff), fp);
		if (strstr(buff, "rw") != NULL && !feof(fp) && strstr(buff, "/data/app/")) {
			sscanf(buff, "%lx-%lx", &pNew->addr, &pNew->taddr);
			flag = 1;
		} else {
			flag = 0;
		}
		if (flag == 1) {
			i++;
			if (i == 1) {
				pNew->next = NULL;
				pEnd = pNew;
				pHead = pNew;
			} else {
				pNew->next = NULL;
				pEnd->next = pNew;
				pEnd = pNew;
			}
			pNew = (PMAPS) malloc(LEN);
		}
	}
	free(pNew);
	fclose(fp);	
	return pHead;
}

PMAPS MemoryTool::readmaps_c_heap() {
	PMAPS pHead = NULL;
	PMAPS pNew = NULL;
	PMAPS pEnd = NULL;
	pEnd = pNew = (PMAPS) malloc(LEN);
	FILE *fp;
	int i = 0, flag = 1;
	char lj[64], buff[256];
	int pid = getPID(bm);
	sprintf(lj, "/proc/%d/maps", pid);
	fp = fopen(lj, "r");
	if (fp == NULL) {
		puts("Memory read failed!");
		return NULL;
	}
	while (!feof(fp)) {
		fgets(buff, sizeof(buff), fp);
		if (strstr(buff, "rw") != NULL && !feof(fp) && strstr(buff, "[heap]")) {
			sscanf(buff, "%lx-%lx", &pNew->addr, &pNew->taddr);
			flag = 1;
		} else {
			flag = 0;
		}
		if (flag == 1) {
			i++;
			if (i == 1) {
				pNew->next = NULL;
				pEnd = pNew;
				pHead = pNew;
			} else {
				pNew->next = NULL;
				pEnd->next = pNew;
				pEnd = pNew;
			}
			pNew = (PMAPS) malloc(LEN);
		}
	}
	free(pNew);				
	fclose(fp);				
	return pHead;
}

PMAPS MemoryTool::readmaps_java_heap() {
	PMAPS pHead = NULL;
	PMAPS pNew = NULL;
	PMAPS pEnd = NULL;
	pEnd = pNew = (PMAPS) malloc(LEN);
	FILE *fp;
	int i = 0, flag = 1;
	char lj[64], buff[256];
	int pid = getPID(bm);
	sprintf(lj, "/proc/%d/maps", pid);
	fp = fopen(lj, "r");
	if (fp == NULL) {
		puts("Memory read failed!");
		return NULL;
	}
	while (!feof(fp)) {
		fgets(buff, sizeof(buff), fp);
		if (strstr(buff, "rw") != NULL && !feof(fp) && strstr(buff, "/dev/ashmem/")) {
			sscanf(buff, "%lx-%lx", &pNew->addr, &pNew->taddr);
			flag = 1;
		} else {
			flag = 0;
		}
		if (flag == 1) {
			i++;
			if (i == 1) {
				pNew->next = NULL;
				pEnd = pNew;
				pHead = pNew;
			} else {
				pNew->next = NULL;
				pEnd->next = pNew;
				pEnd = pNew;
			}
			pNew = (PMAPS) malloc(LEN);
		}
	}
	free(pNew);		
	fclose(fp);				
	return pHead;
}

PMAPS MemoryTool::readmaps_a_anonmyous() {
	PMAPS pHead = NULL;
	PMAPS pNew = NULL;
	PMAPS pEnd = NULL;
	pEnd = pNew = (PMAPS) malloc(LEN);
	FILE *fp;
	int i = 0, flag = 1;
	char lj[64], buff[256];
	int pid = getPID(bm);
	sprintf(lj, "/proc/%d/maps", pid);
	fp = fopen(lj, "r");
	if (fp == NULL) {
		 puts("Memory read failed!");
		return NULL;
	}
	while (!feof(fp)) {
		fgets(buff, sizeof(buff), fp);
		if (strstr(buff, "rw") != NULL && !feof(fp) && (strlen(buff) < 42)) {
			sscanf(buff, "%lx-%lx", &pNew->addr, &pNew->taddr);
			flag = 1;
		} else {
			flag = 0;
		}
		if (flag == 1) {
			i++;
			if (i == 1) {
				pNew->next = NULL;
				pEnd = pNew;
				pHead = pNew;
			} else {
				pNew->next = NULL;
				pEnd->next = pNew;
				pEnd = pNew;
			}
			pNew = (PMAPS) malloc(LEN);	
		}
	}
	free(pNew);			
	fclose(fp);			
	return pHead;
}

PMAPS MemoryTool::readmaps_code_system() {
	PMAPS pHead = NULL;
	PMAPS pNew = NULL;
	PMAPS pEnd = NULL;
	pEnd = pNew = (PMAPS) malloc(LEN);
	FILE *fp;
	int i = 0, flag = 1;
	char lj[64], buff[256];
	int pid = getPID(bm);
	sprintf(lj, "/proc/%d/maps", pid);
	fp = fopen(lj, "r");
	if (fp == NULL) {
		puts("Memory read failed!");
		return NULL;
	}
	while (!feof(fp)) {
		fgets(buff, sizeof(buff), fp);
		if (strstr(buff, "rw") != NULL && !feof(fp) && strstr(buff, "/system")) {
			sscanf(buff, "%lx-%lx", &pNew->addr, &pNew->taddr);
			flag = 1;
		} else {
			flag = 0;
		}
		if (flag == 1) {
			i++;
			if (i == 1) {
				pNew->next = NULL;
				pEnd = pNew;
				pHead = pNew;
			} else {
				pNew->next = NULL;
				pEnd->next = pNew;
				pEnd = pNew;
			}
			pNew = (PMAPS) malloc(LEN);	
		}
	}
	free(pNew);				
	fclose(fp);			
	return pHead;
}

PMAPS MemoryTool::readmaps_stack() {
	PMAPS pHead = NULL;
	PMAPS pNew = NULL;
	PMAPS pEnd = NULL;
	pEnd = pNew = (PMAPS) malloc(LEN);
	FILE *fp;
	int i = 0, flag = 1;
	char lj[64], buff[256];
	int pid = getPID(bm);
	sprintf(lj, "/proc/%d/maps", pid);
	fp = fopen(lj, "r");
	if (fp == NULL) {
		puts("Memory read failed!");
		return NULL;
	}
	while (!feof(fp)) {
		fgets(buff, sizeof(buff), fp);
		if (strstr(buff, "rw") != NULL && !feof(fp) && strstr(buff, "[stack]")) {
			sscanf(buff, "%lx-%lx", &pNew->addr, &pNew->taddr);
			flag = 1;
		} else {
			flag = 0;
		}
		if (flag == 1) {
			i++;
			if (i == 1) {
				pNew->next = NULL;
				pEnd = pNew;
				pHead = pNew;
			} else {
				pNew->next = NULL;
				pEnd->next = pNew;
				pEnd = pNew;
			}
			pNew = (PMAPS) malloc(LEN);	
		}
	}
	free(pNew);					
	fclose(fp);				
	return pHead;
}

PMAPS MemoryTool::readmaps_ashmem() {
	PMAPS pHead = NULL;
	PMAPS pNew = NULL;
	PMAPS pEnd = NULL;
	pEnd = pNew = (PMAPS) malloc(LEN);
	FILE *fp;
	int i = 0, flag = 1;
	char lj[64], buff[256];
	int pid = getPID(bm);
	sprintf(lj, "/proc/%d/maps", pid);
	fp = fopen(lj, "r");
	if (fp == NULL) {
		puts("Memory read failed!");
		return NULL;
	}
	while (!feof(fp)) {
		fgets(buff, sizeof(buff), fp);
		if (strstr(buff, "rw") != NULL && !feof(fp) && strstr(buff, "/dev/ashmem/")
		        && !strstr(buff, "dalvik")) {
			sscanf(buff, "%lx-%lx", &pNew->addr, &pNew->taddr);
			flag = 1;
		} else {
			flag = 0;
		}
		if (flag == 1) {
			i++;
			if (i == 1) {
				pNew->next = NULL;
				pEnd = pNew;
				pHead = pNew;
			} else {
				pNew->next = NULL;
				pEnd->next = pNew;
				pEnd = pNew;
			}
			pNew = (PMAPS) malloc(LEN);
		}
	}
	free(pNew);	
	fclose(fp);
	return pHead;
}
