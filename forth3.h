
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
CODE(exit,  exit,      POPRSP(SI))
CODE(emit,  emit,      POP(DX), MOVAXI(00,02), INT(21))
CODE(key,   key,       MOVAXI(00,01), INT(21), PUSH(AX))
CODE(0branch,zbranch,  POP(BX), LODS, SHL(R,AX), OR(,R,BX,BX), JNZ,2, ADD(,R,SI,AX))
CODE(branch,branch,    LODS, SHL(R,AX), ADD(,R,SI,AX))
CODE(lit,   lit,       LODS, PUSH(AX))
//CODE(dup,   dup,       MOV(,R,BX,SP), MOV(,B,AX,BX_),0, PUSH(AX))
CODE(dup,   dup,       POP(AX), PUSH(AX), PUSH(AX))
CODE(drop,  drop,      POP(AX))
CODE(swap,  swap,      POP(AX), POP(BX), PUSH(AX), PUSH(BX))
CODE(over,  over,      MOV(,R,BX,SP), MOV(,B,AX,BX_),2, PUSH(AX))
CODE(rot,   rot,       POP(AX), POP(BX), POP(CX), PUSH(BX), PUSH(AX), PUSH(CX))
CODE(-dup,  minusdup,  POP(AX), OR(,R,AX,AX), JZ,1,PUSH(AX), PUSH(AX))
CODE(-rot,  nrot,      POP(AX), POP(BX), POP(CX), PUSH(AX), PUSH(CX), PUSH(BX))
CODE(>r,    to_r,      POP(AX), PUSHRSP(AX))
CODE(r>,    from_r,    POPRSP(AX), PUSH(AX))
CODE(r,     r,         MOV(,B,AX,BP_),0, PUSH(AX))
CODE(2drop, 2drop,     POP(AX), POP(AX))
CODE(2dup,  2dup,      MOV(,R,BX,SP),MOV(,B,AX,BX_),0, MOV(,B,CX,BX_),2, PUSH(AX),PUSH(CX))
CODE(0=,    zeq,       POP(AX), OR(,R,AX,AX), MOVAXI(0xff,0xff), JZ,2,INC_(R,AX), PUSH(AX))
CODE(0<,    zless,     POP(AX), OR(,R,AX,AX), MOVAXI(0xff,0xff), JL,2,INC_(R,AX), PUSH(AX))
CODE(0>,    zmore,     POP(AX), OR(,R,AX,AX), MOVAXI(0xff,0xff), JG,2,INC_(R,AX), PUSH(AX))
CODE(not,   not,       POP(AX), NOT(R,AX), PUSH(AX))
CODE(1+,    oneplus,   MOV(,R,BX,SP), INC_(B,BX_),0)
CODE(1-,    oneminus,  MOV(,R,BX,SP), DEC_(B,BX_),0)
CODE(+,     add,       POP(AX), POP(BX), ADD(,R,AX,BX), PUSH(AX))
CODE(-,     sub,       POP(AX), MOV(,R,BX,SP), SUB(F,B,AX,BX_),0)
CODE(*,     mul,       POP(AX), POP(BX), IMUL(R,BX), PUSH(AX))
CODE(/,     div,       POP(BX), XOR(,R,DX,DX), POP(AX), IDIV(R,BX), PUSH(AX))
CODE(mod,   mod,       POP(BX), XOR(,R,DX,DX), POP(AX), IDIV(R,BX), PUSH(DX))
CODE(/mod,  divmod,    POP(BX), XOR(,R,DX,DX), POP(AX), IDIV(R,BX), PUSH(DX), PUSH(AX))
CODE(*/mod, muldivmod, POP(CX), POP(BX), POP(AX), IMUL(R,BX), IDIV(R,CX), PUSH(DX), PUSH(AX))
CODE(*/,    muldiv,    POP(CX), POP(BX), POP(AX), IMUL(R,BX), IDIV(R,CX), PUSH(AX))
CODE(max,   max,       POP(BX), POP(AX), CMP(,R,AX,BX), JL,2, MOV(,R,AX,BX), PUSH(AX))
CODE(min,   min,       POP(BX), POP(AX), CMP(,R,AX,BX), JG,2, MOV(,R,AX,BX), PUSH(AX))
CODE(abs,   abs,       POP(AX), OR(,R,AX,AX), JGE,2, NEG(R,AX), PUSH(AX))
CODE(minus, minus,     POP(AX), NEG(R,AX), PUSH(AX))
CODE(and,   and,       POP(BX), POP(AX), AND(,R,AX,BX), PUSH(AX))
CODE(or,    or,        POP(BX), POP(AX), OR(,R,AX,BX), PUSH(AX))
CODE(xor,   xor,       POP(BX), POP(AX), XOR(,R,AX,BX), PUSH(AX))
CODE(!,     bang,      POP(BX), POP(AX), MOV(F,Z,AX,BX_))
CODE(@,     at,        POP(BX), MOV(,Z,AX,BX_), PUSH(AX))
CODE(+!,    plusbang,  POP(BX), POP(AX), ADD(F,Z,AX,BX_))
CODE(-!,    minusbang, POP(BX), POP(AX), SUB(F,Z,AX,BX_))
CODE(bye,   bye,       HALT)
WORD(0,     zero,      c_lit, 0)
WORD(1,     one,       c_lit, 1)
WORD(10,    ten,       c_lit, 10)
WORD(double, double,   c_dup, c_add)
WORD(dubdub, dubdub,   c_double, c_double)
WORD(cr,    cr,        c_lit,'\n', c_emit)
WORD(space, space,     c_lit,' ', c_emit)
WORD(.,     dot,       c_dup, c_zless, c_zbranch, 4, c_lit, '-', c_emit, c_minus,
                       c_ten, c_divmod, c_swap, c_lit, '0', c_add, c_emit,
                       c_dup, c_zeq, c_zbranch, -11, c_space)
WORD(ok,    ok,        c_lit,'O',c_emit, c_lit,'K',c_emit, c_cr)
WORD(run,   run,       c_lit,0, c_zeq, c_dot,
                       c_lit,2, c_zeq, c_dot,
                       c_lit,12,            c_zless, c_dot,
                       c_lit,1+(0xffff^50), c_zless, c_dot,
                       c_lit,12,            c_zmore, c_dot,
                       c_lit,1+(0xffff^50), c_zmore, c_dot,
                       c_ok, c_bye)
US init = c_run + 2;
{ UC x[] = { MOVBPI( 0x00,0x20 ), MOVSII(init%0x100,init/0x100), NEXT };
memcpy( start, x, sizeof x ); }
return  0;}

