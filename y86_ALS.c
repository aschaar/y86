/* 
    Project #4 
    CEG 4350-01
    Abigail Schaar

    This is the source of code to simulate the y86 emulator. It uses the
    basic structure that was provided with some tweaks regarding print 
    statements and the clearing and setting of conditional flags.

    First, the program reads in a user supplied binary file and puts the
    read bytes into the program memory. Then, the program switches to 
    decoding and executing the instructions that were in the original 
    binary file. 

    Once the program is finished, it will print out the registers, the
    conditional flags and the memory of the program.
*/

#include"y86_ALS.h"
#define FALSE 0
#define TRUE 1
#define MEMSIZE 2000

char p[2000]; /* This is memory */
int programLength;

int littleEndian = TRUE;
int num_words;
int *eax, *ecx, *edx, *ebx, *esi, *edi, *esp, *ebp, pc;
char codes; /* status, condition codes[3]; control and status flags */

/* Program start */
int main(int argc, char ** argv)
{   
    setup();
    char *input = argv[1];
    FILE * f;

    /* 
    TODO 1: read the file in as a binary file 
    */
    f = fopen(input, "rb"); 
    printf("Opened file %s\n", input);
    //parsing the file
    parse(f);
    printf("Parsed %s\n", input);

    //going through the codes to emulate the program
    decode();
    
    //print the memory
    printMemory(0);

    return 0;
}

void setup()
{
    //allocating all of the registers and setting them to 0 for setup
    eax = malloc(sizeof(int*));
    ecx = malloc(sizeof(int*));
    edx = malloc(sizeof(int*));
    ebx = malloc(sizeof(int*));
    esi = malloc(sizeof(int*));
    edi = malloc(sizeof(int*));
    esp = malloc(sizeof(int*));
    ebp = malloc(sizeof(int*));
    *eax = *ecx = *edx = *ebx = *esi = *edi = *esp = *ebp = pc = 0;
}

/* Parses the file into byte-size characters ***************************
************************************************************************/
int parse(FILE * f)
{
    programLength = 0;
    int i = 0;

    if (f == 0)
    {
        perror("Cannot open input file\n");
        exit(1);
    }
    else
    {
        //reading the file into memory
        programLength = fread(p, 1, 2000, f);

        //printing out the bytes that were read in
        for (int i = 0; i < programLength; i++) {
            printf("%x ", p[i] & 0xff); 
        }

    }
    
    printf("\n");
    fclose(f);
    return 0;
}

/* Decodes the string of byte-sized characters and executes them **********
*************************************************************************/
int decode()
{
    /* 
    TODO 2: Implement this function so it reads from a binary file 
    rather than from ASCII characters that represent binary digits like 
    it does now 
    */

    //setting the AOK status to then start the decoding
    setAOK();

    for (pc=0; pc<programLength; )
    {
        printf("\n0x%x:\t ", pc & 0xff);

        switch (p[pc] & 0xf0)
        {
            case 0x00:
            {
                /* l=2, halt */
                if ((p[pc] & 0x0f) == 0x0)
                {
                    printf("%x\t\t", p[pc]);
                    halt();
                } else
                {
                    setINS();
                    error("Error interpreting halt at pc=%x", pc);

                }
                break;
            }
            case 0x10:
            {
                /* l=2, nop */
                if ((p[pc] & 0x0f) == 0x0)
                {
                    printf("%x\t\t", p[pc]);
                    nop();
                } else
                {
                    setINS();
                    error("Error interpreting nop at pc=%x", pc);
                }
                break;
            }
            case 0x20:
            {
                //checking if the program has exceeded the program memory
                if((pc+1) > MEMSIZE){
                    setADR();
                    error("Error grabbing register for mov", pc);
                }

                printf("%x %x \t\t", p[pc]&0xff, p[pc+1]&0xff);
                char reg = p[pc+1];

                /* l=4, mov 
                    rrmovl rA, rb     20 rArB
                    cmovle rA, rb     21 rArB
                    cmovl rA, rb      22 rArB
                    cmove rA, rB      23 rArB
                    cmovne rA, rB     24 rArB
                    cmovge rA, rB     25 rArB
                    cmovg rA, rB      26 rArB
                */
                if ((p[pc]&0x0f) == 0x0)
                {
                    rrmovl(reg);
                }
                else if ((p[pc]&0x0f) == 0x1)
                {
                    cmovle(reg);
                }
                else if ((p[pc]&0x0f) == 0x2)
                {
                    cmovl(reg);
                }
                else if ((p[pc]&0x0f) == 0x3)
                {
                    cmove(reg);
                }
                else if ((p[pc]&0x0f) == 0x4)
                {
                    cmovne(reg);
                }
                else if ((p[pc]&0x0f) == 0x5)
                {
                    cmovge(reg);
                }
                else if ((p[pc]&0x0f) == 0x6)
                {
                    cmovg(reg);
                } else 
                {
                    setINS();
                    error("Error interpreting mov at pc=%x", pc);
                }
                break;
            }
            case 0x30:
            {
                /* l=8, irmovl */
                /* irmovl V, rb      30 FrB V[4] */
               if ((p[pc] & 0x0f) == 0x0)
               {
                    if ((pc+5) > MEMSIZE){
                        setADR();
                        error("Error grabbing value for irmovl", pc);
                    } else {
                        printf("%x %x %x %x %x %x\t", p[pc]&0xff, p[pc+1]&0xff, 
                            p[pc+2]&0xff, p[pc+3]&0xff, p[pc+4]&0xff, p[pc+5]&0xff);
                        int val = getVal(p[pc+2], p[pc+3], p[pc+4], p[pc+5]);
                        
                        irmovl(val, p[pc+1]);
                    }
                
               } else
               {
                    setINS();
                    error("Error interpreting irmovl at pc=%x", pc);
               }
               break;
            }
            case 0x40:
            {
                /* l=8, rmmovl */
                /* rmmovl rA, D(rB)  40 rArB D[4] */
                if ((p[pc]&0x0f) == 0x0)
                {
                    if ((pc+5) > MEMSIZE){
                        setADR();
                        error("Error grabbing data for rmmovl", pc);
                    } else {
                        printf("%x %x %x %x %x %x\t", p[pc]&0xff, p[pc+1]&0xff, 
                            p[pc+2]&0xff, p[pc+3]&0xff, p[pc+4]&0xff, p[pc+5]&0xff);                    
                        int data = getVal(p[pc+2], p[pc+3], p[pc+4], p[pc+5]);
                        rmmovl(p[pc+1], data);
                    }
                } else
                {
                    setINS();
                    error("Error interpreting rmmovl at pc=%x", pc);
                }
                break;
            }
            case 0x50:
            {
                /* l=8, mrmovl */
                /*     mrmovl D(rB), rA     50 rArB D[4] */
                if ((p[pc]&0x0f) == 0x0)
                {
                    if ((pc+5) > MEMSIZE){
                        setADR();
                        error("Error grabbing data for mrmovl", pc);
                    } else {
                        printf("%x %x %x %x %x %x\t", p[pc]&0xff, p[pc+1]&0xff, 
                            p[pc+2]&0xff, p[pc+3]&0xff, p[pc+4]&0xff, p[pc+5]&0xff);                    
                        int data = getVal(p[pc+2], p[pc+3], p[pc+4], p[pc+5]);
                        mrmovl(p[pc+1], data);
                    }
                } else
                {
                    setINS();
                    error("Error interpreting mrmovl at pc=%x", pc);
                }
               break;
            }
            case 0x60:
            {
                if ((pc+1) > MEMSIZE){
                    setADR();
                    error("Error grabbing register for OPl", pc);
                }
                
                printf("%x %x \t\t", p[pc]&0xff, p[pc+1]&0xff);
                char reg = p[pc+1];

                /* l=4, op */
                /* 
                addl rA, rB       60 rArB
                subl rA, rB       61 rArB
                andl rA, rB       62 rArB
                xorl rA, rB       63 rArB
                */
                if ((p[pc]&0x0f) == 0x0)
                {
                    addl(reg);
                } else if ((p[pc]&0x0f) == 0x1)
                {
                    subl(reg);
                } else if ((p[pc]&0x0f) == 0x2)
                {
                    andl(reg);
                } else if ((p[pc]&0x0f) == 0x3)
                {
                    xorl(reg);
                } else
                {
                    setINS();
                    error("Problem parsing op at pc=%x", pc);
                }
                break;
            }
            case 0x70:
            {
                if ((pc+4) > MEMSIZE){
                    setADR();
                    error("Error grabbing data for jmp", pc);
                }

                printf("%x %x %x %x %x\t", p[pc]&0xff, p[pc+1]&0xff, 
                    p[pc+2]&0xff, p[pc+3]&0xff, p[pc+4]&0xff);                    
                int dest = getVal(p[pc+1], p[pc+2], p[pc+3], p[pc+4]);
                
                /* l=8, jmps
                jmp Dest          70 Dest[4]
                jle Dest          71 Dest[4]
                jl Dest           72 Dest[4]
                je Dest           73 Dest[4]
                jne Dest          74 Dest[4]
                jge Dest          75 Dest[4]
                jg Dest           76 Dest[4]
                */
                if ((p[pc]&0x0f) == 0x0)
                {
                    jmp(dest);
                } else if ((p[pc]&0x0f) == 0x1)
                {
                    jle(dest);
                } else if ((p[pc]&0x0f) == 0x2)
                {
                    jl(dest);
                } else if ((p[pc]&0x0f) == 0x3)
                {
                    je(dest);
                } else if ((p[pc]&0x0f) == 0x4)
                {
                    jne(dest);
                } else if ((p[pc]&0x0f) == 0x5)
                {
                    jge(dest);
                } else if ((p[pc]&0x0f) == 0x6)
                {
                    jg(dest);
                } else 
                {
                    setINS();
                    error("Error interpreting jump at pc=%x", pc);
                }
                break;
            }
            case 0x80:
            {
                /* l=8, call */
                /*     call  80 Dest[4] */
                if ((p[pc]&0x0f) == 0x0)
                {
                    if ((pc+4) > MEMSIZE){
                        setADR();
                        error("Error grabbing data for call", pc);
                    } else {
                        printf("%x %x %x %x %x\t", p[pc]&0xff, p[pc+1]&0xff, 
                            p[pc+2]&0xff, p[pc+3]&0xff, p[pc+4]&0xff);                    
                        int dest = getVal(p[pc+1], p[pc+2], p[pc+3], p[pc+4]);
                        call(dest);
                    }                
                } else 
                {
                    setINS();
                    error("Error interpreting call at pc=%x", pc);
                }
                break;
            }
            case 0x90:
            {
                /* l=2, ret */
                /*     ret   90 */
               if ((p[pc]&0x0f) == 0x0)
               {
                    printf("%x  \t\t", p[pc]&0xff);
                    ret();
               } else
               {
                    setINS();
                    error("Error interpreting ret at pc=%x", pc);
               }
               break;
            }
            case 0xa0:
            {
                /* l=4, pushl */
                /* pushl rA          A0 rAF */
                if ((p[pc]&0x0f) == 0x0)
                {
                    if ((pc+1) > MEMSIZE){
                        setADR();
                        error("Error grabbing pc for pushl", pc);
                    } else {
                        printf("%x %x \t\t", p[pc]&0xff, p[pc+1]&0xff);
                        pushl(p[pc+1]&0xf0);
                    }
                } else 
                {   
                    setINS();
                    error("Error interpreting pushl at pc=%x", pc);
                }
               break;
            }
            case 0xb0:
            {
                /* l=4, popl */
                /* popl rA           B0 rAF */
                if ((p[pc]&0x0f) == 0x0)
                {
                    if ((pc+1) > MEMSIZE){
                        setADR();
                        error("Error grabbing pc for popl", pc);
                    } else {
                        printf("%x %x \t\t", p[pc]&0xff, p[pc+1]&0xff);
                        popl(p[pc+1]&0xf0);
                    }
                } else
                {
                    setINS();
                    error("Error interpreting pushl at pc=%x", pc);
                }
                break;
            }
            default:
            {
                setINS();
                error("Error interpreting instruction at pc=%x", pc);
            }
        }
    }

    printRegisters();
    return 0;

}//end of decode()

/*
    x | ZF | SF | OF |  1 | 1 | 1 | 1
    1 AOK  Normal operation
    2 HLT  halt instruction encountered
    3 ADR  Invalid address encountered
    4 INS  Invalid instruction encountered
    *************************************
    ZF  zero flag
    SF  sign flag
    OF  overflow flag
*/

/* Zero flag operations */
void setZF()
{  
    codes = codes | 64; /* sets the 01000000 flag */
    printf("(ZF)");
}

void clearZF()
{
    codes = codes & 0xBF; /* clears 10111111 flag BF */
}

int getZF()
{
    if ((codes & 64) == 64)
        return TRUE;
    return FALSE;
}

void clearFlags()
{
    clearSF();
    clearZF();
    clearOF();
}

/* Sign Flag operations */
void setSF()
{
    codes = codes | 32; /* sets the 00100000 flag */
    printf("(SF)");
}

void clearSF()
{
    codes = codes & 0xDF;
}

int getSF()
{
    if ((codes & 32) == 32)
        return TRUE;
    return FALSE;
}


/* Overflow flag operations */
void setOF()
{
    codes = codes | 16; /* sets the 00010000 flag */
    printf("(OF)");
}

void clearOF()
{
    codes = codes & 0xEF;
}

int getOF()
{
    if ((codes & 16) == 16)
        return TRUE;
    return FALSE;
}

/* Status code operations (last four bits) */
void setAOK()
{
    clearStatus();
    codes = codes | 1; /* sets 00000001 */
    printf("(set status = AOK)");
}

void setHLT()
{
    clearStatus();
    codes = codes | 2; /* sets 00000010 */
    printf("(set status = HLT)");
}

void setADR()
{
    clearStatus();
    codes = codes | 3; /* sets 00000011 */
    printf("(set status = ADR)");
}

void setINS()
{
    clearStatus();
    codes = codes | 4; /* sets 00000010 */
    printf("(set status = INS)");  
}

int getStatus()
{
    return codes & 0xf0;
}

void clearStatus()
{
    codes = codes & (255 - 15); /*11110000 - clears last four bits */
}

/**
**  Gets an integer value from four bytes (based on big 
**   endian or little endian encoding)
**/
int getVal(char a, char b, char c, char d)
{
    int val;
    if (littleEndian)
    {
        val = a;
        val = val | b<<8;
        val = val | c<<16;
        val = val | d<<24;
    } else /* big endian */
    {
        val = d;
        val = val | c<<8;
        val = val | b<<16;
        val = val | a<<24;
    }
    return val;
}

/* 
    Computes the register from the first part of the byte 
    passed in. Returns a pointer to the register given 
    character code as input.
    0  eax; 1 ecx; 2 edx; 3  ebx
    4  esp; 5 ebp; 6 esi; 7  edi
    F  No register
*/
int * r1(char a)
{
    switch (a & 0xf0)
    {
        case 0x00:  return eax;
        case 0x10:  return ecx;
        case 0x20:  return edx;
        case 0x30:  return ebx;
        case 0x40:  return esp;
        case 0x50:  return ebp;
        case 0x60:  return esi;
        case 0x70:  return edi;
        case 0xf0:  return 0; 
        default:
            error("Error determining register value. pc=%x", pc);
    }
    return eax; /* shouldn't be hit */
}

/* 
    Computes the register from the second part of the byte 
    passed in. Returns a pointer to the register given 
    character code as input.
    0  eax; 1 ecx; 2 edx; 3  ebx
    4  esp; 5 ebp; 6 esi; 7  edi
    F  No register
*/
int * r2(char a)
{
    switch (a & 0x0f)
    {
        case 0x00:  return eax;
        case 0x01:  return ecx;
        case 0x02:  return edx;
        case 0x03:  return ebx;
        case 0x04:  return esp;
        case 0x05:  return ebp;
        case 0x06:  return esi;
        case 0x07:  return edi;
        case 0x0f:  return 0; 
        default:
            error("Error determining register value. pc=%x", pc);
    }
    return eax; /* shouldn't be hit */
}

/*
*  Prints out the values of the registers */
void printRegisters()
{
    printf("\n\nRegisters: ");
    printf("EAX:%x  " , *eax & 0xff);
    printf("ECX:%x  " , *ecx & 0xff);
    printf("EDX:%x  ", *edx & 0xff);
    printf("EBX:%x  ", *ebx & 0xff);
    printf("ESP:%x  ", *esp & 0xff);
    printf("EBP:%x  ", *ebp & 0xff);
    printf("ESI:%x  ", *esi & 0xff);
    printf("EDI:%x  ", *edi & 0xff);
    printf("PC:%x \n", pc & 0xff);

    printf("Flags: ");
    printf("ZF:%x ", getZF());
    printf("SF:%x ", getSF());
    printf("OF:%x \n", getOF());

}

/* Generates an error then exits the program 
*/ 
void error(char * words, int pc)
{
    printf("%s",words);
    //printing the memory at to show where the invalid instruction was taken
    printMemory(pc);
    exit(1);
}

/* Assembly instructions */

/**     
*    halt              00 */
void halt()
{
    printf("halt");
    printRegisters();
    setHLT();
    pc+=1;
    //printing out all of the memory
    printMemory(0);
    exit(0);
}

/**  nop               10  */
void nop()
{
    printf("nop");
    pc+=1;
}

/**  rrmovl rA, rb     20 rArB   */
void rrmovl(char reg)
{
    int * src = r1(reg);
    int * dst = r2(reg);
    *dst = *src;
    printf("rrmovl %x, %x", *src, *dst);
    pc+=2;
}

/**     cmovle rA, rb     21 rArB  */
void cmovle(char reg)
{
    int * src = r1(reg);
    int * dst = r2(reg);
    if ((getZF() == 1) || getSF() != getOF())
    {
        *dst = *src;
        printf("cmovle %x, %x (moved)", *src, *dst);
        clearFlags();
    } else 
        printf("cmovle %x, %x (not moved)", *src, *dst);
    pc+=2;
}

/**      cmovl rA, rb      22 rArB   */
void cmovl(char reg)
{
    int * src = r1(reg);
    int * dst = r2(reg);
    if (getSF() != getOF())
    {
        *dst = *src;
        printf("cmovl %x, %x (moved)", *src, *dst);
        clearFlags();
    } else
        printf("cmovl %x, %x (not moved)", *src, *dst);
    pc+=2;
}

/**     cmove rA, rB      23 rArB  */
void cmove(char reg)
{
    int * src = r1(reg);
    int * dst = r2(reg);
    if (getZF() == 1)
    {
        *dst = *src;   
        printf("cmove %x, %x (moved)", *src, *dst);
        clearFlags();    
    } else
        printf("cmove %x, %x (not moved)", *src, *dst);    
    pc+=2;
}

/*
    Executes move if not equal to by checking the 
    condition codes. If true, the src will be put in dst 
    If not, the pc will be moved past the instruction
    and continue sequentially in the memory. 

    @param char: the value containing the src and dst
*/
/**     cmovne rA, rB     24 rArB  */
void cmovne(char reg)
{
    /* TODO 3: Implement the cmovne instruction */
    
    //grabbing src and dst registers
    int * src = r1(reg);
    int * dst = r2(reg);

    //checking if the operation should happen
    if(getZF() == 0){
        *dst = *src;
        printf("cmovne %x, %x (moved)", *src, *dst);
        clearFlags();
    } else {
        printf("cmovne %x, %x (not moved)", *src, *dst);
    }
    //moving the pc    
    pc+=2;
}

/*
    Executes move if greater than or equal to by checking the 
    condition codes. If true, the src will be put in dst 
    If not, the pc will be moved past the instruction
    and continue sequentially in the memory. 

    @param char: the value containing the src and dst
*/
/**     cmovge rA, rB     25 rArB  */
void cmovge(char reg)
{
    /* TODO 4: Implement the cmovge instruction */

    //grabbing src and dst registers
    int * src = r1(reg);
    int * dst = r2(reg);

    //checking if the operation should happen
    if(getSF() == getOF()){
        *dst = *src;
        printf("cmovge %x, %x (moved)", *src, *dst);
        clearFlags();
    } else { 
        printf("cmovge %x, %x (not moved)", *src, *dst);
    }
    //moving the pc
    pc+=2;
}

/*
    Executes move if greater than by checking the 
    condition codes. If true, the src will be put in dst 
    If not, the pc will be moved past the instruction
    and continue sequentially in the memory. 

    @param char: the value containing the src and dst
*/
/*     cmovg rA, rB      26 rArB   */
void cmovg(char reg)
{
    /* TODO 5: Implement the cmovg instruction */

    //grabbing src and dst registers
    int * src = r1(reg);
    int * dst = r2(reg);

    //checking if the operation should happen
    if((getSF() == getOF()) && (getZF() == 0)){
        *dst = *src;
        printf("cmovg %x, %x (moved)", *src, *dst);
        clearFlags();
    } else { 
        printf("cmovg %x, %x (not moved)", *src, *dst);
    }
    //moving the pc
    pc+=2;
}

/**     irmovl V, rb      30 FrB Va Vb Vc Vd  */
void irmovl(int val, char reg)
{
    int *rB = r2(reg);
    *rB = val;
    printf("irmovl rB, %x", *rB);
    pc+=6;
}

/*
    Executes a register->memory move. It grabs the
    value to be put in the register from the memory
    and sticks it there.

    @param char: the register containing src and dst
    @param int: the offset of the register value

*/
/**     rmmovl rA, D(rB)     40 rArB Da Db Dc Dd  */
void rmmovl(char reg, int offset)
{
    int * rA = r1(reg);
    int * rB = r2(reg);
    p[*rB + offset] = *rA;
    printf("rmmovl rA, %x(%x)", offset, *rB);
    pc+=6;
}

/*
    Executes a memory->register move. It grabs the
    value to be put in the register from the memory
    and sticks it there.

    @param char: the register containing src and dst
    @param int: the offset of the register value

*/
/**     mrmovl D(rB), rA      50 rArB Da Db Dc Dd  */
void mrmovl(char reg, int offset)
{
    /* TODO 6: Implement the mrmovl instruction */
    
    //grabbing the source and destination registers
    int * rA = r1(reg);
    int * rB = r2(reg);

    //performing the operation 
    *rA = p[*rB + offset];

    //printing the results and moving the pc
    printf("mrmovl D(rB), %x", *rA); 
    pc+=6;
}

/** 
    This function serves to set the conditional flags
    (OF - overflow, SF - sign flag, or ZF - zero flag)
    based off of the values of src(a) and dst(b) and 
    the result of the operation. 

    @param a: the src value
    @param b: the dst value
    @param result: the value of the operation
    @param isAdd: integer flag indicating addition or
                    subtraction 
*/
void setFlags(int a, int b, int result, int isAdd)
{
    /* TODO 7: Implement the setFlags function */
    
    //clearing the flags before setting them based off the conditions
    clearFlags();

    //isAdd to indicate addition or subtraction
    if(isAdd == 1){ //if it's addition
        //(positive) + (positive) = (negative)
        if((a > 0) && (b > 0) && (result < 0)){
            setOF();
        //(negative) + (negative) = (positive or 0)
        } else if ((a < 0) && (b < 0) && (result >= 0)) {
            setOF();
        }
    } else if(isAdd == 2){ //if it's subtraction
        //(negative) - (positive) = (positive) 
        if((a > 0) && (b < 0) && (result > 0)) { 
            setOF();
        //(positive or 0) - (negative) = (negative)
        } else if((a < 0) && (b >= 0) && (result < 0)) {
            setOF();
        }
    } 

    //setting the zero or sign flag if applicable
    if(result == 0){
        setZF();
    } else if(result < 0){
        setSF();
    }

}//end of setFlags()

/*
    Executes an addition operation on the src and dst registers.
    As a result of this operation, the sign, zero, or overflow 
    flag might be set, depending on the result.

    @param char : the register containing both the src and dst
*/
/**     addl rA, rB          60 rArB  */
void addl(char reg)
{
    int * src = r1(reg);
    int * dst = r2(reg);
    int tmp = *dst;
    *dst = *dst + *src;
    printf("addl rA, rB: (%x)", *dst);  
    setFlags(*src,tmp,*dst, 1);
    pc+=2;
}

/*
    Executes a subtraction operation on the src and dst registers.
    As a result of this operation, the sign, zero, or overflow 
    flag might be set, depending on the result.

    @param char : the register containing both the src and dst
*/
/**      subl rA, rB        61 rArB  */
void subl(char reg)
{
    /* TODO 8: Implement the subl instruction */

    //grabbing the src and dst registers
    int * src = r1(reg);
    int * dst = r2(reg);
    
    //storing the original value of the dst reg
    int tmp = *dst;

    //preforming the operation
    *dst = *dst - *src;
    
    //printing the results, setting flags, and moving pc
    printf("subl rA, rB: (%x)", *dst);
    setFlags(*src, tmp, *dst, 2);
    pc+=2;
}

/*
    Executes an and operation on the src and dst registers.
    It utilizes the '&'' c operation. As a result of this 
    operation, the sign, zero, or overflow flag might be set,
    depending on the result.

    @param char : the register containing both the src and dst
*/
/**     andl rA, rB       62 rArB  */
void andl(char reg)
{
    /* TODO 9: Implement the andl instruction */

    //grabbing the src and dst registers
    int * src = r1(reg);
    int * dst = r2(reg);

    //storing the orginal value of the dst reg
    int tmp = *dst;

    //peforming the operation 
    *dst = *dst & *src;

    //printing the results, setting flags, and moving pc
    printf("andl rA, rB: (%x)", *dst);
    setFlags(*src, tmp, *dst, 0);
    pc+=2;
}

/*
    Executes an exclusive or on the src and dst registers.
    It utilizes the '^'' c operation. As a result of this 
    operation, the sign or overflow flag might be set,
    depending on the result.

    @param char : the register containing both the src and dst
*/
/**     xorl rA, rB       63 rArB  */
void xorl(char reg)
{
    /* TODO 10: Implement the xorl instruction */

    //grabbing the src and dst 
    int * src = r1(reg);
    int * dst = r2(reg);

    //storing the original value of the dst reg
    int tmp = *dst;

    //performing the operation
    *dst = *dst ^ *src;

    //printing the results, setting flags, and moving pc
    printf("xorl rA, rB: (%x)", *dst);
    setFlags(*src, tmp, *dst, 0);
    pc+=2;
}

/**     jmp Dest          70 Da Db Dc Dd  */
void jmp(int dest)
{
    printf("jmp %x", dest);
    pc = dest;
    printf(" (pc=%x)", dest);
}

/*
    Executes jump if less than or equal to by checking the 
    condition codes. If true, the pc will be moved to the 
    destination. If not, the pc will be moved past the 
    destination and sequentially in the memory. 

    @param dest: the destination to go to if the condition
                    is true
*/
/**     jle Dest          71 Da Db Dc Dd  */
void jle(int dest)
{
    /* TODO 11: Implement the jle instruction */ 
    
    printf("jle %x", dest);
    
    //checking if the jump can execute
    if((getZF() == 1) || (getSF() != getOF())){
        pc = dest;
        printf(" (pc=%x)", dest);
        clearFlags();
    } else {
        printf(" (not taken)");
        //moving program counter 5 to skip the destination
        pc+=5;
    }
}

/*
    Executes jump if less than by checking the condition
    codes. If true, the pc will be moved to the destination.
    If not, the pc will be moved past the destination and 
    sequentially in the memory. 

    @param dest: the destination to go to if the condition
                    is true
*/
/**     jl Dest           72 Da Db Dc Dd  */
void jl(int dest)
{
    /* TODO 12: Implement the jl instruction */
    
    printf("jl %x", dest);
    
    //checking if the jmp should be taken
    if(getSF() != getOF()){
        pc = dest;
        printf(" (pc=%x)", dest);
        clearFlags();
    } else {
        printf(" (not taken)");
        //moving program counter 5 to skip the dest
        pc+=5;
    }
}

/**     je Dest           73 Da Db Dc Dd  */
void je(int dest)
{
    printf("je %x", dest);
    if (getZF() == 1)
    {
        pc = dest;
        printf(" (pc=%x)", dest);
        clearFlags();
    } else
    {
        printf(" (not taken)");
        pc+=5;
    }
}

/**     jne Dest          74 Da Db Dc Dd  */
void jne(int dest)
{
    printf("jne %x", dest);
    if (getZF() == 0)
    {
        pc = dest;
        printf(" (pc=%x)", pc&0xff);
        clearFlags();
    } else
    {
        printf(" (not taken)");
        pc+=5;
    }
}

/*
    Executes jump if greater than or equal to by
    checking the condition flags. If true, the pc 
    will be moved to the new destination. If not
    the pc will be moved sequentially in the 
    memory.

    @param int: the destination to move the pc to
*/
/**     jge Dest          75 Da Db Dc Dd  */
void jge(int dest)
{
    /* TODO 13: Implement the jge instruction */
    
    printf("jge %x", dest);

    //checking if the jmp should be taken
    if(getSF() == getOF()){
        pc = dest;
        printf(" (pc=%x)", pc&0xff);
        clearFlags();
    } else {
        printf(" (not taken)");
        //moving program counter 5 to skip the dest
        pc+=5;
    }
}

/*
    Executes the jump if greater than by checking the
    condition flags. If the condition is true, the 
    pc will be moved to the corresponding destination.
    If not, the pc will be moved sequentially in the 
    memory.

    @param int: the destination to move the pc to 
*/
/**     jg Dest           76 Da Db Dc Dd  */
void jg(int dest)
{
    /* TODO jg: Implement the jg instruction */
    
    printf("jf %x", dest);
    //checking if the jmp should be taken
    if((getSF() == getOF()) && (getZF() == 0)){
        pc = dest;
        printf(" (pc=%x", pc&0xff);
        clearFlags();
    } else {
        printf(" (not taken)");
        //moving program counter 5 to skip the dest
        pc+=5;
    }
}

/* TODO 15: Verify all the other instructions work correctly */

/*     call              80 Da Db Dc Dd  */
void call(int addr)
{
    printf("call %x", addr);
    *esp = *esp - 0x4; /* make entry on stack */
    p[*esp] = pc;     /* push old pc onto stack */
    pc = addr;
}

/*     ret               90   */
void ret()
{
    printf("ret");
    pc = p[*esp];     /* restore pc from stack */
    *esp = *esp + 0x4; /* move esp back down */
    pc += 5;  /* for how many bytes it took to call */
}

/*     pushl rA          A0 rAF   */
void pushl(char reg)
{
    int * rA = r1(reg);
    *esp = *esp - 0x4;    /* make entry on the stack */
    p[*esp] = *rA;      /* put rA into it */
    printf("pushl %x", *rA);
    pc+=2;
}

/*      popl rA           B0 rAF   */
void popl(char reg)
{
    int * rA = r1(reg);
    *rA = p[*esp];      /* pull top stack value into rA */
    *esp = *esp + 0x4;  /* remove entry from the stack */
    printf("popl %x", *rA);
    pc+=2;
}

/* TODO: 16: Make sure printMemory prints the ending memory on the screen properly */
int printMemory(int start)
{
    printf("\nMemory:");
    //looping through the starting position to the end of the program
    for (int i=start; i<programLength; i++)
    {
        //ensuring that the memory prints out in a nice block
        if (i%14 == 0)
            printf("\n");
        printf("%x  ", p[i] & 0xFF);
    }

    printf("\n");
    return 0;
}