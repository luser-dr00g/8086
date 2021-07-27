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
#define             Z_    6
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
#define SHR(m,   r_m) 0xd1,MRM(m,5,r_m)
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
#define INC(r)    0x40+r
#define DEC(r)    0x48+r
#define PUSH(r)   0x50+r
#define POP(r)    0x58+r
#define ADDAX(imm) 0x05,LITTLEENDIAN(imm)
#define SUBAX(imm) 0x2d,LITTLEENDIAN(imm)
#define ADDAL(imm) 0x04,imm
#define MOVS      0xa5
#define CMPS      0xa7
#define LODS      0xad
#define SCAS      0xaf
#define JZ        0x74
#define JNZ       0x75
#define JS        0x78
#define JNS       0x79
#define JL        0x7C
#define JGE       0x7D
#define JLE       0x7E
#define JG        0x7F
#define JMP       0xE9
#define sJMP      0xEB
#define LAHF      0x9f
#define HALT      0xF4
#define CWD       0x99
#define CLD       0xfc
#define STD       0xfd
#define UP        CLD
#define DOWN      STD
#define STI       0xfb
#define CLI       0xfa
#define INT(no)   0xCD,0x##no
#define LITTLEENDIAN(w) (w)%0x100,(w)/0x100
#define MOVI(r,a) 0xb8+r,LITTLEENDIAN(a)
#define MOVBI(r,a) 0xb0+r,a
#define CALL      0xE8
#define ESC(v)    0xd8+v
#define IRET      0xcf
