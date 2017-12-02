#include"y86_ALS.h"
#define FALSE 0
#define TRUE 1

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
    f = fopen(input, "r"); 
    printf("Opened file %s\n", input);
    parse(f);
    printf("Parsed %s\n", input);
    decode();
    return 0;
}

void setup()
{
        /* get the file from arc */
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
    char c;
    programLength = 0;
    int i = 0;
    if (f == 0)
    {
        perror("Cannot open input file\n");
        return 1;
    }
    else
    { /* c0af   c 0af  c0 af  c0a f */
        while ((c = fgetc(f)) != EOF )
        {
            char t = fgetc(f);
            //c = toHex(c);
            //t = toHex(t);
            p[i] = c<<4 | t;
            i++; programLength++;
            printf("%x ", p[i] & 0xff);
        }
    }
    printf("\n");
    fclose(f);
    return 0;
}


char toHex(char x)
{
    if (x > '/' && x < ':') /* 0-9 */
        x = (x - 48);
    else if (x > '`' && x < 'g') /* a-f */
        x = (x-87);
    return x;
}






/* Decodes the string of byte-sized characters and executes them **********
*************************************************************************/
int decode()
{
    /*
    take the first char
        - determine how many operands
        - call function with right operands
    */

    /* 
    TODO 2: Implement this function so it reads from a binary file 
    rather than from ASCII characters that represent binary digits like 
    it does now 
    */

    for (pc=0; pc<programLength; )
    {
        printf("\n0x%x:\t ", pc&0xff);
        char a = p[pc] & 0xf0;
        char b = p[pc] & 0x0f;
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
                    error("Error interpreting nop at pc=%x", pc);
                }
                break;
            }
            case 0x20:
            {
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
                printf("%x %x %x %x %x %x\t", p[pc]&0xff, p[pc+1]&0xff, 
                    p[pc+2]&0xff, p[pc+3]&0xff, p[pc+4]&0xff, p[pc+5]&0xff);
                int val = getVal(p[pc+2], p[pc+3], p[pc+4], p[pc+5]);
                irmovl(val, p[pc+1]);
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
                    printf("%x %x %x %x %x %x\t", p[pc]&0xff, p[pc+1]&0xff, 
                        p[pc+2]&0xff, p[pc+3]&0xff, p[pc+4]&0xff, p[pc+5]&0xff);                    
                    int data = getVal(p[pc+2], p[pc+3], p[pc+4], p[pc+5]);
                    rmmovl(p[pc+1], data);
                } else
                {
                    setINS();
                    error("Error interpreting rrmovl at pc=%x", pc);
                }
                break;
            }
            case 0x50:
            {
                /* l=8, mrmovl */
                /*     mrmovl D(rB), rA     50 rArB D[4] */
                if ((p[pc]&0x0f) == 0x0)
                {
                    printf("%x %x %x %x %x %x\t", p[pc]&0xff, p[pc+1]&0xff, 
                        p[pc+2]&0xff, p[pc+3]&0xff, p[pc+4]&0xff, p[pc+5]&0xff);                    
                    int data = getVal(p[pc+2], p[pc+3], p[pc+4], p[pc+5]);
                    mrmovl(p[pc+1], data);
                } else
                {
                    setINS();
                    error("Error interpreting mrmovl at pc=%x", pc);
                }
               break;
            }
            case 0x60:
            {
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
                /* l=8, jmps */
                printf("%x %x %x %x %x\t", p[pc]&0xff, p[pc+1]&0xff, 
                    p[pc+2]&0xff, p[pc+3]&0xff, p[pc+4]&0xff);                    
                int dest = getVal(p[pc+1], p[pc+2], p[pc+3], p[pc+4]);
                /*
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
                    printf("%x %x %x %x %x\t", p[pc]&0xff, p[pc+1]&0xff, 
                        p[pc+2]&0xff, p[pc+3]&0xff, p[pc+4]&0xff);                    
                    int dest = getVal(p[pc+1], p[pc+2], p[pc+3], p[pc+4]);
                    call(dest);                
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
                    printf("%x %x \t\t", p[pc]&0xff, p[pc+1]&0xff);
                    pushl(p[pc+1]&0xf0);
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
                    printf("%x %x \t\t", p[pc]&0xff, p[pc+1]&0xff);
                    popl(p[pc+1]&0xf0);
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
}



/* Sample program (in the root directory as prog.o)
0x00:
    30f400010000 -> 30 f4 00 01 00 00
    30f500010000 -> 30 f5 00 01 00 00
    8024000000   -> 80 24 00 00 00 00
    00           -> 00

0x014:
    0d000000     -> 0d 00 00 00
    c0000000     -> c0 00 00 00 
    000b0000     -> 00 0b 00 00
    00a00000     -> 00 a0 00 00

0x024:
    a05f         -> a0 5f
    2045         -> 20 45
    30f004000000 -> 30 f0 04 00 00 00
    a00f         -> a0 0f
    30f214000000 -> 30 f2 14 00 00 00
    a02f         -> a0 2f
    8042000000   -> 80 42 00 00 00
    2054         -> 20 54
    b05f         -> b0 5f
    90           -> 90

0x42:
    a05f         -> a0 5f
    2045         -> 20 45
    501508000000 -> 50 15 08 00 00 00
    50250c000000 -> 50 25 0c 00 00 00
    6300         -> 63 00
    6222         -> 62 22
    7378000000   -> 73 78 00 00 00
    506100000000 -> 50 61 00 00 00 00 
    6060         -> 60 60 
    30f304000000 -> 30 f3 04 00 00 00 
    6031         -> 60 31
    30f3ffffffff -> 30 f3 ff ff ff ff
    6032         -> 60 32
    745b000000   -> 74 5b 00 00 00 
    2054         -> 20 54
    b05f         -> b0 5f
    90           -> 90

0x100: #stack starts here and grows to lower addresses)
*/


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
    codes = codes & 64; /* clears 10111111 flag */
    // if set, clear
    // if clear, clear
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
    codes = codes & 32;
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
    codes = codes & 16;
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
//        printf("      getVal: %x %x %x %x \n ",a,b,c,d);
        val = a;
        val = val | b<<8;
        val = val | c<<16;
        val = val | d<<24;
//        printf(": %x ", val);
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
//    printf("r2: %x, r2 & 0x0f: %x", a, (a & 0xf0)); 
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
//    printf("r2: %x, r2 & 0x0f %x", a, (a & 0x0f));
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
    printf("\nEAX:%x  " , *eax);
    printf("ECX:%x  " , *eax);
    printf("EDX:%x  ", *eax);
    printf("EBX:%x  ", *eax);
    printf("ESP:%x  ", *eax);
    printf("EBP:%x  ", *eax);
    printf("ESI:%x  ", *eax);
    printf("EDI:%x  ", *eax);
    printf("PC:%x \n", pc);
}

/* Generates an error then exits the program 
*/ 
void error(char * words, int pc)
{
    printf(words);
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
    if (getZF() || getSF() != getOF())
    {
        *dst = *src;
        printf("cmovle %x, %x (moved)", *src, *dst);
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
    } else
        printf("cmove %x, %x (not moved)", *src, *dst);    
    pc+=2;
}



/**     cmovne rA, rB     24 rArB  */
void cmovne(char reg)
{
    /* TODO 3: Implement the cmovne instruction */
}


/**     cmovge rA, rB     25 rArB  */
void cmovge(char reg)
{
    /* TODO 4: Implement the cmovge instruction */
}


/*     cmovg rA, rB      26 rArB   */
void cmovg(char reg)
{
    /* TODO 5: Implement the cmovg instruction */
}




/**     irmovl V, rb      30 FrB Va Vb Vc Vd  */
void irmovl(int val, char reg)
{
    int *rB = r2(reg);
    *rB = val;
    printf("irmovl rB, %x", *rB);
    pc+=6;
}



/**     rmmovl rA, D(rB)     40 rArB Da Db Dc Dd  */
void rmmovl(char reg, int offset)
{
    int * rA = r1(reg);
    int * rB = r2(reg);
    p[*rB + offset] = *rA;
    printf("rmmovl rA, %x(%x)", offset, *rB);
    pc+=6;
}




/**     mrmovl D(rB), rA      50 rArB Da Db Dc Dd  */
void mrmovl(char reg, int offset)
{
    /* TODO 6: Implement the mrmovl instruction */
}

/** Sets flags based on the last result */
void setFlags(int a, int b, int result, int isAdd)
{
    /* TODO 7: Implement the setFlags function */
    //set ZF - zero flag
    //setOF - overflow flag 
    //setSF - sign flag
}


/**     addl rA, rB          60 rArB  */
void addl(char reg)
{
    int * src = r1(reg);
    int * dst = r2(reg);
    int tmp = *dst;
    *dst = *dst + *src;
    printf("addl rA, rB: (%x)", *dst);
    setFlags(tmp, *src, *dst);
    pc+=2;
}


/**      subl rA, rB        61 rArB  */
void subl(char reg)
{
    /* TODO 8: Implement the subl instruction */
}


/**     andl rA, rB       62 rArB  */
void andl(char reg)
{
    /* TODO 9: Implement the andl instruction */
}


/**     xorl rA, rB       63 rArB  */
void xorl(char reg)
{
    /* TODO 10: Implement the xorl instruction */
}


/**     jmp Dest          70 Da Db Dc Dd  */
void jmp(int dest)
{
    printf("jmp %x", dest);
    pc = dest;
    printf(" (pc=%x)", dest);
    pc+=5;
}



/**     jle Dest          71 Da Db Dc Dd  */
void jle(int dest)
{
    /* TODO 11: Implement the jle instruction */
}


/**     jl Dest           72 Da Db Dc Dd  */
void jl(int dest)
{
    /* TODO 12: Implement the jl instruction */
}


/**     je Dest           73 Da Db Dc Dd  */
void je(int dest)
{
    printf("je %x", dest);
    if (getZF() == 1)
    {
        pc = dest;
        printf(" (pc=%x)", dest);
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
    } else
    {
        printf(" (not taken)");
        pc+=5;
    }
}



/**     jge Dest          75 Da Db Dc Dd  */
void jge(int dest)
{
    /* TODO 13: Implement the jge instruction */
}



/**     jg Dest           76 Da Db Dc Dd  */
void jg(int dest)
{
    /* TODO jg: Implement the jg instruction */
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
    *esp = *esp - 4;    /* make entry on the stack */
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
    int words_on_screen = 1000;

    for (int i=start; i<start+words_on_screen; i++)
    {
        if (i==(0%4))
            printf("");
        printf("%x\t", p[i]);
    }
    return 0;
}





/* Sample program (in directory as prog.o)
0x00:
    30f400010000 -> 30 f4 00 01 00 00
    30f500010000 -> 30 f5 00 01 00 00
    8024000000   -> 80 24 00 00 00 00
    00           -> 00

0x014:
    0d000000     -> 0d 00 00 00
    c0000000     -> c0 00 00 00 
    000b0000     -> 00 0b 00 00
    00a00000     -> 00 a0 00 00

0x024:
    a05f         -> a0 5f
    2045         -> 20 45
    30f004000000 -> 30 f0 04 00 00 00
    a00f         -> a0 0f
    30f214000000 -> 30 f2 14 00 00 00
    a02f         -> a0 2f
    8042000000   -> 80 42 00 00 00
    2054         -> 20 54
    b05f         -> b0 5f
    90           -> 90

0x42:
    a05f         -> a0 5f
    2045         -> 20 45
    501508000000 -> 50 15 08 00 00 00
    50250c000000 -> 50 25 0c 00 00 00
    6300         -> 63 00
    6222         -> 62 22
    7378000000   -> 73 78 00 00 00
    506100000000 -> 50 61 00 00 00 00 
    6060         -> 60 60 
    30f304000000 -> 30 f3 04 00 00 00 
    6031         -> 60 31
    30f3ffffffff -> 30 f3 ff ff ff ff
    6032         -> 60 32
    745b000000   -> 74 5b 00 00 00 
    2054         -> 20 54
    b05f         -> b0 5f
    90           -> 90


*/





