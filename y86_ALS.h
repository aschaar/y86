/* 
	Project #4 
	CEG 4350-01
	Abigail Schaar

	This is the header file for the y86 emulator. It has all
	of the function declarations as well as the file imports
	to assist with the proper operation and execution of the 
	functions implemented in the .c file. 
*/

#include<stdio.h>
#include<math.h>
#include<stdlib.h>
#include<ctype.h>

void setup();
void error(char*, int);
void setZF();
void clearZF();
int getZF();
void setSF();
void clearSF();
int getSF();
void setOF();
void clearOF();
int getOF();
void setAOK();
void setHLT();
void setADR();
void setINS();
int getStatus();
void clearStatus();
int getVal(char,char,char,char);
int* r1(char);
int* r2(char);
void printRegisters();

void error(char*, int);
void halt();
void nop();
void rrmovl(char);
void cmovle(char);
void cmovl(char);
void cmove(char);
void cmovne(char);
void cmovge(char);
void cmovg(char);
void irmovl(int, char);
void rmmovl(char,int);
void mrmovl(char,int);
void addl(char);
void subl(char);
void andl(char);
void xorl(char);
void jmp(int);
void jle(int);
void jl(int);
void je(int);
void jne(int);
void jge(int);
void jg(int);
void call(int);
void ret();
void pushl(char);
void popl(char);

int printMemory(int);
int parse(FILE*);
int decode();
void clearFlags();
void setFlags(int,int,int,int);





