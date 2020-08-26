#include "undefs.h"
#define LEA(to,m,r,r_m) to+0x8d,MRM(m,r,r_m)
#define MOV(to,m,r,r_m) to+0x8b,MRM(m,r,r_m)
#define ADD(to,m,r,r_m) to+0x03,MRM(m,r,r_m)
#define SUB(to,m,r,r_m) to+0x2b,MRM(m,r,r_m)
#define     F -2
#define    MRM(m,r,r_m) 0##m##r##r_m
#define        Z 0
#define        B 1
#define        W 2
#define        R 3
#define          AX 0
#define          CX 1
#define          DX 2
#define          BX 3
#define          SP 4
#define          BP 5
#define          SI 6
#define          DI 7
#define            BX_SI 0
#define            BX_DI 1
#define            BP_SI 2
#define            BP_DI 3
#define            SI_   4
#define            DI_   5
#define            BP_   6
#define            BX_   7
#define TEST(m,r,r_m) 0x85,MRM(m,r,r_m)
#define IMUL(m,  r_m) 0xf7,MRM(m,5,r_m)
#define INC_(m,  r_m) 0xff,MRM(m,0,r_m)
#define DEC_(m,  r_m) 0xff,MRM(m,1,r_m)
#define JMP_(m,  r_m) 0xff,MRM(m,5,r_m)
#define POP(r)  0x58+r
#define PUSH(r) 0x50+r
#define ADDAX   0x05
#define LODS    0xAD
#define JZ      0x74
#define HALT    0xF4
#define INT(no) 0xCD,0x##no
#define MOVAXI(a,b) 0xB8,a,b
#define MOVSII(a,b) 0xBE,a,b
#define CALL    0xE8
