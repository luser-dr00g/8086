
#include "asm8086.h"
#include "fdict.h"
/*	 W   = AX     PSP = SP
	 X   = BX     RSP = BP
	 IP  = SI     TOS_in_memory              */
#define NEXT       	LODS, MOV(,R,BX,AX), MOV(,Z,BX,BX_), JMP_(R,BX)
#define PUSHRSP(r) 	LEA(,B,BP,BP_),minus(4), MOV(F,B,r,BP_),0
#define POPRSP(r)  	MOV(,B,r,BP_),0, LEA(,B,BP,BP_),4
#define minus(x)	1+(0xff^x)

static inline void nop_(){}

static inline int
forth(char *start){
char *p = start;
trace=0;
{ UC x[] = { HALT, 00, 00 };
  memcpy( p, x, sizeof x );
  p += 64; } //boot code and return stack area. stack pointer assumed well out of the way
HEADLESS(enter, enter, PUSHRSP(SI), ADDAX,2,0, MOV(,R,SI,AX))
HEADLESS(docon, docon, MOV(,R,BX,AX), MOV(,B,AX,BX_),2, PUSH(AX))
HEADLESS(dovar, dovar, MOV(,R,BX,AX), LEA(,B,AX,BX_),2, PUSH(AX))
CODE(exit,  c_exit,    POPRSP(SI))
CODE(emit,  emit,      POP(DX), MOVAXI(00,02), INT(21))
CODE(key,   key,       MOVAXI(00,01), INT(21), PUSH(AX))
CODE(0branch,zbranch,  POP(BX), LODS, SHL(R,AX), OR(,R,BX,BX), JNZ,2, ADD(,R,SI,AX))
CODE(branch,branch,    LODS, SHL(R,AX), ADD(,R,SI,AX))
CODE(lit,   lit,       LODS, PUSH(AX))
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
CODE(2drop, twodrop,   POP(AX), POP(AX))
CODE(2dup,  twodup,    MOV(,R,BX,SP),MOV(,B,AX,BX_),0, MOV(,B,CX,BX_),2, PUSH(AX),PUSH(CX))
CODE(0=,    zeq,       POP(BX), MOVAXI(0xff,0xff), OR(,R,BX,BX), JZ,2,INC_(R,AX), PUSH(AX))
CODE(0<,    zless,     POP(BX), MOVAXI(0xff,0xff), OR(,R,BX,BX), JL,2,INC_(R,AX), PUSH(AX))
CODE(0>,    zmore,     POP(BX), MOVAXI(0xff,0xff), OR(,R,BX,BX), JG,2,INC_(R,AX), PUSH(AX))
CODE(<,     less,      POP(CX),POP(BX), MOVAXI(0xff,0xff), CMP(,R,BX,CX), JL,2,INC_(R,AX), PUSH(AX))
CODE(>,     more,      POP(CX),POP(BX), MOVAXI(0xff,0xff), CMP(,R,BX,CX), JG,2,INC_(R,AX), PUSH(AX))
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
CODE(type,  type,      POP(CX), POP(BX), MOVAXI(00,02), 
                       BYTE+MOV(,Z,DL,BX_), INT(21), INC_(R,BX), DEC_(R,CX), JNZ,-10)
CODE(bye,   bye,       HALT)
WORD(0,     zero,      docon, 0)
WORD(1,     one,       docon, 1)
WORD(10,    ten,       docon, 10)
WORD(var,   var,       dovar, 42)
WORD(base,  base,      dovar, 10)
WORD(cr,    cr,        enter, lit,'\n', emit)
WORD(space, space,     enter, lit,' ', emit)
WORD(.neg,  dotneg,    enter, dup, zless, zbranch, 4, lit, '-', emit, minus)
WORD(.emit, dotemit,   enter, dup,  ten, less, zbranch, 5,
                                    lit, '0', add,
                                    branch, 5,
                                    lit, 'A', add, ten, sub,
                              emit)
WORD(.expand,dotexpand,enter, zero, swap,              // ... d n
                                base, at, divmod,      // ... d n%b n/b
                                rot, oneplus, swap,    // n%b d+1 n/b
                                dup, zeq, zbranch, -10,  drop) // n0 n1 ... nN N
WORD(.digits,dotdigits,enter,   swap, dotemit, oneminus, // ... nN-1 N-1  [nN=>output]
                                dup, zeq, zbranch, -7,  drop) // |-
WORD(.,     dot,       enter, dotneg, dotexpand,  //dup, dotemit,
                              dotdigits, space)
//WORD(digits,digits,    dovar, 0)
//WORD(.start,dotstart,  enter, zero, digits, bang)
//WORD(.neg,  dotneg,    enter, dup, zless, zbranch, 4, lit, '-', emit, minus)
//WORD(.emit, dotemit,   enter, lit, '0', add, emit)
//WORD(.expand,dotexpand,enter, base, at, divmod, one, digits, plusbang,
//                              dup, zeq, zbranch, -9)
//WORD(.digits,dotdigits,enter, drop,
//                              dotemit, one, digits, minusbang, 
//                              digits, at, zeq, zbranch, -9)
//WORD(.,     dot,       enter, dotstart,
//                              dotneg, 
//                              dotexpand, //digits, at, dotemit,
//                              dotdigits,
//                              space)
//WORD(.,     dot,       enter, dup, zless, zbranch, 4, lit, '-', emit, minus,
//                              ten, divmod, swap, lit, '0', add, emit,
//                              dup, zeq, zbranch, -11, space)
WORD(ok,    ok,        enter, lit,'O',emit, lit,'K',emit, cr)
WORD(here,  here,      enter, lit, 0)
WORD(allot, allot,     enter, lit, here+4, plusbang)
WORD(test0, test0,     enter, zero, dot,
                              one, dot, ten, dot, ok)
WORD(test1, test1,     enter, zero, zeq, dot,
                              one, zeq, dot, ok)
WORD(test2, test2,     enter, lit,12,            zless, dot,
                              lit,1+(0xffff^50), zless, dot,
                              lit,12,            zmore, dot,
                              lit,1+(0xffff^50), zmore, dot, ok)
WORD(test2a,test2a,    enter, one, ten, less, dot,
                              one, ten, more, dot, ok)
WORD(test3, test3,     enter, here, lit, 12, allot, 
                              lit, 12, type, ok)
WORD(test4, test4,     enter, var, dot, var, at, dot, ok)
WORD(test5, test5,     enter, ten, var, bang, var, at, dot, ok)
WORD(test6, test6,     enter, lit, 16, base, bang, var, dot, ok)
WORD(test,  test,      enter, test0, cr,
                              test1, cr,
                              test2, cr,
                              test2a, cr,
                              test3, cr,
                              test4, cr,
                              test5, cr,
                              test6, cr,
                              bye)
memcpy( start+here+4, (US[]){ p-start }, 2);
memcpy( p, "Hello world!", 12 );
nop_();
US init = test + 2;
{ UC x[] = { MOVBPI( 0x00,0x20 ), MOVSII(init%0x100,init/0x100), NEXT };
  memcpy( start, x, sizeof x ); }
trace=0;
return  0;}

