#include "undefs.h"
#define  ADD(to,m,r,r_m) to+0x03,MRM(m,r,r_m)
#define  AND(to,m,r,r_m) to+0x23,MRM(m,r,r_m)
#define   OR(to,m,r,r_m) to+0x0b,MRM(m,r,r_m)
#define  SBB(to,m,r,r_m) to+0x1b,MRM(m,r,r_m)
#define  SUB(to,m,r,r_m) to+0x2b,MRM(m,r,r_m)
#define  XOR(to,m,r,r_m) to+0x33,MRM(m,r,r_m)
#define  CMP(to,m,r,r_m) to+0x3b,MRM(m,r,r_m)
#define  MOV(to,m,r,r_m) to+0x8b,MRM(m,r,r_m)
#define  LEA(to,m,r,r_m) to+0x8d,MRM(m,r,r_m)
#define    F -2
#define     MRM(m,r,r_m) 0##m##r##r_m
#define         Z 0
#define         B 1
#define         W 2
#define         R 3
#define           AX 0
#define           CX 1
#define           DX 2
#define           BX 3
#define           SP 4
#define           BP 5
#define           SI 6
#define           DI 7
#define             BX_SI 0
#define             BX_DI 1
#define             BP_SI 2
#define             BP_DI 3
#define             SI_   4
#define             DI_   5
#define             BP_   6
#define             BX_   7
#define BYTE -1
#define           AL 0
#define           CL 1
#define           DL 2
#define           BL 3
#define           AH 4
#define           CH 5
#define           DH 6
#define           BH 7
#define TEST(m,r,r_m) 0x85,MRM(m,r,r_m)
#define SHL(m,   r_m) 0xd1,MRM(m,4,r_m) 
#define SHRC(m,  r_m) 0xd3,MRM(m,5,r_m)
#define NOT(m,   r_m) 0xf7,MRM(m,2,r_m)
#define NEG(m,   r_m) 0xf7,MRM(m,3,r_m)
#define MUL(m,   r_m) 0xf7,MRM(m,4,r_m)
#define IMUL(m,  r_m) 0xf7,MRM(m,5,r_m)
#define DIV(m,   r_m) 0xf7,MRM(m,6,r_m)
#define IDIV(m,  r_m) 0xf7,MRM(m,7,r_m)
#define INC_(m,  r_m) 0xff,MRM(m,0,r_m)
#define DEC_(m,  r_m) 0xff,MRM(m,1,r_m)
#define JMP_(m,  r_m) 0xff,MRM(m,5,r_m)
#define PUSH(r)   0x50+r
#define POP(r)    0x58+r
#define ADDAX     0x05
#define LODS      0xad
#define JZ        0x74
#define JNZ       0x75
#define JS        0x78
#define JNS       0x79
#define JL        0x7C
#define JGE       0x7D
#define JLE       0x7E
#define JG        0x7F
#define LAHF      0x9f
#define HALT      0xF4
#define INT(no)   0xCD,0x##no
#define MOVAXI(a,b) 0xb8,a,b
#define MOVCXI(a,b) 0xb9,a,b
#define MOVSPI(a,b) 0xbc,a,b
#define MOVBPI(a,b) 0xbd,a,b
#define MOVSII(a,b) 0xbe,a,b
#define CALL    0xE8
