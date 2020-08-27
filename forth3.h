
#include "asm8086.h"
#include "fdict.h"
/*	 W   = AX     PSP = SP
	 X   = BX     RSP = BP
	 IP  = SI     TOS_in_memory              */
#define NEXT       	LODS, MOV(,R,BX,AX), MOV(,Z,BX,BX_), JMP_(R,BX)
#define PUSHRSP(r) 	LEA(,B,BP,BP_),minus(4), MOV(F,B,r,BP_),0
#define POPRSP(r)  	MOV(,B,r,BP_),0, LEA(,B,BP,BP_),4
#define minus(x)	1+(0xff^x)

static inline int
forth(char *start){
  char *p = start;
  { UC x[] = { HALT, 00, 00 }; memcpy( p, x, sizeof x ); p += 16; } 
  HEADLESS(enter,  enter, PUSHRSP(SI), ADDAX,2,0, MOV(,R,SI,AX))
  CODE(exit,   exit,      POPRSP(SI))
  CODE(emit,   emit,      POP(DX), MOVAXI(00,02), INT(21))
  CODE(key,    key,       MOVAXI(00,01), INT(21), PUSH(AX))
  CODE(lit,    lit,       LODS, PUSH(AX))
  CODE(drop,   drop,      POP(AX))
  CODE(swap,   swap,      POP(AX), POP(BX), PUSH(AX), PUSH(BX))
  CODE(dup,    dup,       MOV(,R,BX,SP), MOV(,B,AX,BX_),0, PUSH(AX))
  CODE(over,   over,      MOV(,R,BX,SP), MOV(,B,AX,BX_),2, PUSH(AX))
  CODE(rot,    rot,       POP(AX), POP(BX), POP(CX), PUSH(BX), PUSH(AX), PUSH(CX))
  CODE(-rot,   nrot,      POP(AX), POP(BX), POP(CX), PUSH(AX), PUSH(CX), PUSH(BX))
  CODE(2drop,  2drop,     POP(AX), POP(AX))
  CODE(2dup,   2dup,      MOV(,R,BX,SP), MOV(,B,AX,BX_),0, MOV(,B,CX,BX_),2, PUSH(AX), PUSH(CX))
  CODE(1+,     oneplus,   MOV(,R,BX,SP), INC_(B,BX_),0)
  CODE(1-,     oneminus,  MOV(,R,BX,SP), DEC_(B,BX_),0)
  CODE(+,      plus,      POP(AX), MOV(,R,BX,SP), ADD(F,B,AX,BX_),0)
  CODE(-,      minus,     POP(AX), MOV(,R,BX,SP), SUB(F,B,AX,BX_),0)
  CODE(*,      star,      POP(AX), POP(BX), IMUL(R,BX), PUSH(AX))
  CODE(!,      bang,      POP(BX), POP(AX), MOV(F,Z,AX,BX_))
  CODE(@,      at,        POP(BX), MOV(,Z,AX,BX_), PUSH(AX))
  CODE(+!,     plusbang,  POP(BX), POP(AX), ADD(F,Z,AX,BX_))
  CODE(-!,     minusbang, POP(BX), POP(AX), SUB(F,Z,AX,BX_))
  CODE(bye,    bye,       HALT)
  WORD(double, double,    c_dup, c_plus)
  WORD(dubdub, dubdub,    c_double, c_double)
  WORD(run,    run,       c_lit, 79, c_emit, c_lit, 75, c_emit, 
                          c_lit, 5, c_double, c_emit, c_bye)
  /*
  printf("enter:%x exit:%x emit:%x lit:%x dup:%x plus:%x bye:%x double:%x run:%x\n", 
         c_enter, c_exit, c_emit, c_lit, c_dup, c_plus, c_bye, c_double, c_run ); /**/
  US init = c_run + 2;
  { UC x[] = { MOVBPI( 0x00,0x20 ), MOVSII(init%0x100,init/0x100), NEXT };
    memcpy( start, x, sizeof x ); }
  return  0;
}
