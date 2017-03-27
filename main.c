#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scan.h"
#include "inst.h"

int main(void)
{
	scanMem();
	scanInst();
	FILE * file = fopen("error_dump.rpt", "w");
	HI = 0;	LO = 0;	m_hi = 0; m_lo = 0;	fclose(file);
//==========================================	
	file = fopen("snapshot.rpt", "w");
	int index = 0;
	cycle = 0;
	PC = iPC;
	reg[29] = SP;
	//
	fprintf(file, "cycle 0\n");
	for (int i = 0; i < 32; i++) {
		fprintf(file, "$%02d: 0x%08X\n", i, reg[i]);
	}
	fprintf(file, "$HI: 0x%08X\n", HI);
	fprintf(file, "$LO: 0x%08X\n", LO);
	fprintf(file, "PC: 0x%08X\n\n\n", iPC);
	fclose(file);
	//
	while (index != instNum && index != -999)
		++cycle, PC += 4, index = readInst(index);
//==========================================
	free(inst);
	return 0;
}