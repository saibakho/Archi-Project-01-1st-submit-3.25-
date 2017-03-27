#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "trans.h"
#include "inst.h"

int readInst(int index)	// four bytes
{
	int nextIndex = index+1;
	char buffer[32];	// character used
	char bin[32];
	intToBin(inst[index], bin, 32);
//===============================================
	int opcode 	= binToUnsInt(bin, 31, 26);
	// R-type
	int rs 		= binToUnsInt(bin, 25, 21);
	int rt 		= binToUnsInt(bin, 20, 16);
	int rd 		= binToUnsInt(bin, 15, 11);
	int shamt 	= binToUnsInt(bin, 10,  6);
	int funct	= binToUnsInt(bin,  5,  0);
	// I-type, J-type
	int immediate = binToInt(bin, 15, 0);	// warning for the extention from 16bit to 32bit (signed)
	int address   = binToUnsInt(bin, 25, 0);	//
//===============================================
	unsigned int _dest;
	unsigned int _hi = HI;
	unsigned int _lo = LO;
	long long mult;
	// errList init
	for (int i = 0; i < 5; i++)
		errorList[i] = 0;
//===============================================
	if ( opcode == 0 ) {
		_dest = reg[rd];
		// R-type
		switch(funct) {
			// default
			case 0x20 :		// add
				errorList[1] = isOverflow(reg[rt], reg[rs]);
				reg[rd] = reg[rt] + reg[rs];
				break;
			case 0x21 :		// addu
				reg[rd] = reg[rt] + reg[rs];
				break;
			case 0x22 :		// sub
				errorList[1] = isOverflow(-reg[rt], reg[rs]);
				reg[rd] = reg[rs] - reg[rt];
				break;
		//=========================================================================
			case 0x24 :		// and
				reg[rd] = reg[rs] & reg[rt];
				break;
			case 0x25 :		// or
				reg[rd] = reg[rs] | reg[rt];
				break;
			case 0x26 :		// xor
				reg[rd] = reg[rs] ^ reg[rt];
				break;
			case 0x27 :		// nor
				reg[rd] = ~( reg[rs] | reg[rt] );
				break;
			case 0x28 :		// nand
				reg[rd] = ~( reg[rs] & reg[rt] );
				break;
			case 0x2a :		// slt
				// warning for sign comparison
				reg[rd] = reg[rs] < reg[rt] ? 1 : 0 ;
				break;
		//=========================================================================
			// shift
			case 0x00 :		// sll
				reg[rd] = reg[rt] << shamt;
				break;
			case 0x02 :		// srl
				reg[rd] = reg[rt] >> shamt;
				break;
			case 0x03 :		// sra
				reg[rd] = reg[rt] << shamt;
				if ( (reg[rt] >> 31 ) == 1 )
					reg[rd] = reg[rd] | ( -1 << ( 32 - shamt ) );
				break;
		//=========================================================================
			// jr
			case 0x08 :
				nextIndex = ( reg[rs] - iPC ) / 4 ;
				PC = reg[rs];
				break;
		//=========================================================================
			// mult, multu
			case 0x18 :		// multi
				mult = reg[rs] * reg[rt];
				HI = mult >> 32;
				LO = mult & 0x00000000ffffffff;
				// err
				errorList[2] = m_hi && m_lo;
				m_hi = 1;	m_lo = 1;
				break;
			case 0x19 :		// multiu
				mult = (long long)((unsigned int)reg[rs]) * ((unsigned int)reg[rt]);
				HI = mult >> 32;
				LO = mult & 0x00000000ffffffff;
				// err
				errorList[2] = m_hi && m_lo;
				m_hi = 1;	m_lo = 1;
				break;
		//=========================================================================
			// mfhi, mflo
			case 0x10 :		// mfhi
				reg[rd] = HI;
				m_hi = 0;
				break;
			case 0x12 :		// mflo
				reg[rd] = LO;
				m_lo = 0;
				break;
		//=========================================================================
			default :
				printf("illegal instruction found at 0x%08X\n", PC);
				exit(1);
		}
		if (funct != 0x08 && funct != 0x18 && funct != 0x19 && inst[index] != 0x00000000) {
			printRpt(0, _dest, rd);
		} else if (funct == 0x18 || funct == 0x19) {
			printRpt(_hi, _lo, 999);
		} else {
			printRpt(555, 555, 555);
		}
	} else {
		_dest = reg[rt];
		switch(opcode) {
			// halt
			case 0x3f :
				nextIndex = -999;
				break;
			// J-type
			case 0x02 :		// j C
				nextIndex = ( address * 4 - iPC ) / 4 ;
				PC = address * 4;
				break;
			case 0x03 :		// jal C
				nextIndex = ( address * 4 - iPC ) / 4 ;
				_dest = reg[31];
				reg[31] = PC;
				PC =  address * 4;
				rt = 31;
				break;
			// I-type
			case 0x08 :		// addi
				errorList[1] = isOverflow(reg[rs], immediate);
				reg[rt] = reg[rs] + immediate;
				break;
			case 0x09 :		// addiu
				reg[rt] = reg[rs] + immediate;
				break;
			case 0x23 :		// lw
				errorList[1] = isOverflow(reg[rs], immediate);
				reg[rt] = memory[ reg[rs]/4 + immediate/4 ];
				// err
				errorList[3] = (reg[rs] + immediate) > 1023 ? 1 : 0 ;
				errorList[4] = (reg[rs]+immediate)%4 != 0 ? 1 : 0 ;
				break;
			case 0x21 :		// lh
				errorList[1] = isOverflow(reg[rs], immediate);
				reg[rt] = memory[ reg[rs]/4 + immediate/4 ];
				intToBin(reg[rt], buffer, 32);
				reg[rt] = immediate%4 == 2 ? binToInt(buffer, 15, 0) : binToInt(buffer, 31, 16);
				// err
				errorList[3] = (reg[rs] + immediate) > 1023 ? 1 : 0 ;
				errorList[4] = (reg[rs]+immediate)%2 != 0 ? 1 : 0 ;
				break;
			case 0x25 :		// lhu
				errorList[1] = isOverflow(reg[rs], immediate);
				reg[rt] = memory[ reg[rs]/4 + immediate/4 ];
				intToBin(reg[rt], buffer, 32);
				reg[rt] = immediate%4 == 2 ? binToUnsInt(buffer, 15, 0) : binToUnsInt(buffer, 31, 16);
				// err
				errorList[3] = (reg[rs] + immediate) > 1023 ? 1 : 0 ;
				errorList[4] = (reg[rs]+immediate)%2 != 0 ? 1 : 0 ;
				break;
			case 0x20 :		// lb
				errorList[1] = isOverflow(reg[rs], immediate);
				reg[rt] = memory[ reg[rs]/4 + immediate/4 ];
				intToBin(reg[rt], buffer, 32);
				switch (immediate%4) {
					case 0 :	reg[rt] = binToInt(buffer, 31, 24);	break;
					case 1 :	reg[rt] = binToInt(buffer, 23, 16);	break;
					case 2 :	reg[rt] = binToInt(buffer, 15,  8);	break;
					case 3 :	reg[rt] = binToInt(buffer,  7,  0);	break;
				}	// err
				errorList[3] = (reg[rs] + immediate) > 1023 ? 1 : 0 ;
				break;
			case 0x24 :		// lbu
				errorList[1] = isOverflow(reg[rs], immediate);
				reg[rt] = memory[ reg[rs]/4 + immediate/4 ];
				intToBin(reg[rt], buffer, 32);
				switch (immediate%4) {
					case 0 :	reg[rt] = binToUnsInt(buffer, 31, 24);	break;
					case 1 :	reg[rt] = binToUnsInt(buffer, 23, 16);	break;
					case 2 :	reg[rt] = binToUnsInt(buffer, 15,  8);	break;
					case 3 :	reg[rt] = binToUnsInt(buffer,  7,  0);	break;
				}
				errorList[3] = (reg[rs] + immediate) > 1023 ? 1 : 0 ;
				break;
			case 0x2b :		// sw
				memory[ reg[rs]/4 + immediate/4 ] = reg[rt];
				// err
				errorList[1] = isOverflow(reg[rs], immediate);
				errorList[3] = (reg[rs] + immediate) > 1023 ? 1 : 0 ;
				errorList[4] = (reg[rs]+immediate)%4 != 0 ? 1 : 0 ;
				break;
			case 0x29 :		// sh
				memory[ reg[rs]/4 + immediate/4 ] = (reg[rt] & 0x0000FFFF) << 16 ;
				// err
				errorList[1] = isOverflow(reg[rs], immediate);
				errorList[3] = (reg[rs] + immediate) > 1023 ? 1 : 0 ;
				errorList[4] = (reg[rs]+immediate)%2 != 0 ? 1 : 0 ;
				break;
			case 0x28 :		// sb
				memory[ reg[rs]/4 + immediate/4 ] = reg[rt] & 0x000000FF << 24 ;
				// err
				errorList[1] = isOverflow(reg[rs], immediate);
				errorList[3] = (reg[rs] + immediate) > 1023 ? 1 : 0 ;
				break;
			case 0x0f :		// lui
				reg[rt] = immediate << 16;
				printf("%d($%02d): 0x%08X\n", immediate, rt, reg[rt]);
				break;
			case 0x0c :		// andi
				reg[rt] = reg[rs] & immediate;
				break;
			case 0x0d :		// ori
				reg[rt] = reg[rs] | immediate;
				break;
			case 0x0e :		// nori
				reg[rt] = ~( reg[rs] | immediate );
				break;
			case 0x0a :		// slti
				// warning for sign comparison
				reg[rt] = reg[rs] < immediate ? 1 : 0 ;
				break;
			case 0x04 :		// beq
				errorList[1] = isOverflow(PC, immediate*4);
				if (reg[rs] == reg[rt])		nextIndex = nextIndex + immediate, PC = PC + immediate * 4;
				break;
			case 0x05 :		// bne
				errorList[1] = isOverflow(PC, immediate*4);
				if (reg[rs] != reg[rt])		nextIndex = nextIndex + immediate, PC = PC + immediate * 4;
				break;
			case 0x07 :		// bgtz
				errorList[1] = isOverflow(PC, immediate*4);
				if (reg[rs] > 0)	nextIndex = nextIndex + immediate, PC = PC + immediate * 4;
				break;
			default :
				printf("illegal instruction found at 0x%08X\n", PC);
				exit(1);
		}
		if (opcode != 0x3f && opcode != 0x02 && opcode != 0x04 && opcode != 0x05 &&
			opcode != 0x07 && opcode != 0x2b && opcode != 0x29 && opcode != 0x28)
			printRpt(reg[rt], _dest, rt);
		else if (opcode != 0x3f)
			printRpt(555, 555, 555);
	}
	return nextIndex;
}
int isOverflow(int a, int b)
{
	if (a > 0 && b > 0)
		return a + b < 0 ? 1 : 0 ;
	else if (a < 0 && b < 0)
		return a + b > 0 ? 1 : 0 ;
	else return 0;
}
void printErr(void)
{
	FILE * file = fopen("error_dump.rpt", "a+");
	for (int i = 0; i < 5; i++)
		if (errorList[i] == 1)
			switch(i) {
				case 0:	fprintf(file, "In cycle %d: Write $0 Error\n", cycle);				break;
				case 1:	fprintf(file, "In cycle %d: Number Overflow\n", cycle);				break;
				case 2:	fprintf(file, "In cycle %d: Overwrite HI-LO registers\n", cycle);	break;
				case 3:	fprintf(file, "In cycle %d: Address Overflow\n", cycle);			exit(1);
				case 4:	fprintf(file, "In cycle %d: Misalignment Error\n", cycle);			exit(1);
			}
	fclose(file);
}
void printRpt(int hilo, int _dest, int index)
{
	if (index == 0) {
		errorList[0] = 1;
		reg[0] = 0;
	}	printErr();
	FILE * file = fopen("snapshot.rpt", "a+");
	fprintf(file, "cycle %d\n", cycle);
	if (index == 999) {
		if ( hilo != HI)	fprintf(file, "$HI: 0x%08X\n", HI);
		if (_dest != LO)	fprintf(file, "$LO: 0x%08X\n", LO);
	} else if (index == 555) {
		// doing nothing
	} else {
		if (reg[index] != _dest)	fprintf(file, "$%02d: 0x%08X\n", index, reg[index]);
	}	fprintf(file, "PC: 0x%08X\n\n\n", PC);
	fclose(file);
}