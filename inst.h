#ifndef INST_
#define INST_
#define bool int

//================================
char regTable[32][6];
char instTable[64][6];
char funcTable[64][6];

int * inst;
int reg[32];
unsigned int memory[256];

unsigned int SP;
unsigned int iPC;
unsigned int PC;
unsigned int HI;
unsigned int LO;

int m_hi;
int m_lo;

int memNum;
int instNum;
int cycle;

int errorList[5];
//================================

extern int readInst(int index);

extern int isOverflow(int a, int b);

extern void printErr(void);

extern void printRpt(int dest, int _dest, int index);

#endif