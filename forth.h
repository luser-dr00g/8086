//http://www.forth.org/fig-forth/fig-forth_8086-8088_ver_10.pdf
//http://forthfiles.net/ting/sysguidefig.pdf
//ftp://ftp.oldskool.org/pub/misc/temp/8086_family_Users_Manual.pdf

#include "applyx.h"
#include "asm8086.h"

#define CODE(letters,...) _COUNT(LETTERS letters __VA_ARGS__)
#define WORD(letters,...) _COUNT(LETTERS letters JMP(DOCOL), __VA_ARGS__)
#define LETTERS(...) PP_NARG(__VA_ARGS__),APPLYXn(CHARIFY, __VA_ARGS__)
#define _FINIS(...) __VA_ARGS__
#define _COUNT(...) __VA_ARGS__,PP_NARG(__VA_ARGS__)
#define STRINGIFY(x) #x
#define CHARIFY(x) *STRINGIFY(x),

#define w   BX
#define ip  SI
#define psp SP
#define rsp BP
#define x   AX
#define TOS_in_memory

#define NEXT       LODS, JMP_(R,AX)
#define PUSHRSP(r) LEA(,B,BP,BP_),minus(4), MOV(,Z,r,BP_)
#define POPRSP(r)  MOV(F,Z,r,BP_), LEA(,B,BP,BP_),4
#define minus(x)   ( 1+0xff^( x ) )

static inline int
forth(char *mem){
  unsigned char image[] = {
    0,
    CODE((e,n,t,e,r), PUSHRSP(SI), ADDAX,4,0, MOV(,R,DI,AX), NEXT),
    CODE((e,x,i,t),   POPRSP(SI), NEXT),
    CODE((l,i,t),     LODS, PUSH(AX), NEXT),
    CODE((d,r,o,p),   POP(AX), NEXT),
    CODE((s,w,a,p),   POP(AX), POP(BX), PUSH(AX), PUSH(BX), NEXT),
    CODE((d,u,p),     MOV(,R,BP,SP), MOV(,B,AX,BP_),0, PUSH(AX), NEXT),
    CODE((o,v,e,r),   MOV(,R,BP,SP), MOV(,B,AX,BP_),2, PUSH(AX), NEXT),
    CODE((r,o,t),     POP(AX), POP(BX), POP(CX), PUSH(BX), PUSH(AX), PUSH(CX), NEXT),
    CODE((-,r,o,t),   POP(AX), POP(BX), POP(CX), PUSH(AX), PUSH(CX), PUSH(BX), NEXT),
    CODE((2,d,r,o,p), POP(AX), POP(AX), NEXT),
    CODE((2,d,u,p),   MOV(,R,BP,SP), MOV(,B,AX,BP_),0, MOV(,B,BX,BP_),2, PUSH(BX), PUSH(AX), NEXT),
    CODE((2,s,w,a,p), POP(AX), POP(BX), POP(CX), POP(DX),PUSH(BX),PUSH(AX),PUSH(DX),PUSH(CX),NEXT),
    CODE((?,d,u,p),   MOV(,R,BP,SP), MOV(,B,AX,BP_),0, TEST(R,AX,AX), JZ,1, PUSH(AX), NEXT),
    CODE((1,+),       MOV(,R,BP,SP), INC_(B,BP_),0, NEXT),
    CODE((1,-),       MOV(,R,BP,SP), DEC_(B,BP_),0, NEXT),
    CODE((+),         POP(AX), MOV(,R,BP,SP), ADD(F,B,AX,BP_),0, NEXT),
    CODE((-),         POP(AX), MOV(,R,BP,SP), SUB(F,B,AX,BP_),0, NEXT),
    CODE((*),         POP(AX),POP(BX), IMUL(R,BX), PUSH(AX), NEXT),
    CODE((!),         POP(BX), POP(AX), MOV(F,Z,AX,BX_), NEXT),
    CODE((@),         POP(BX), MOV(,R,AX,BX), PUSH(AX), NEXT),
    CODE((+,!),       POP(BX), POP(AX), ADD(F,Z,AX,BX_), NEXT),
    CODE((-,!),       POP(BX), POP(AX), SUB(F,Z,AX,BX_), NEXT),
    WORD((d,o,u,b,l,e), DUP, PLUS, EXIT),
  };
  return  memcpy(mem, image, sizeof image);
}
