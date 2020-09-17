
#include "asm8086.h"
#include "fdict.h"
/*	 W   = AX     PSP = SP
	 X   = BX     RSP = BP
	 IP  = SI     TOS_in_memory              */
#define NEXT       	LODS, MOV(,R,BX,AX), MOV(,Z,BX,BX_), JMP_(R,BX)
#define PUSHRSP(r) 	LEA(,B,BP,BP_),minus(4), MOV(F,B,r,BP_),0
#define POPRSP(r)  	MOV(,B,r,BP_),0, LEA(,B,BP,BP_),4
#define minus(x)	1+(0xff^x)
#define MOVAX( val )    MOVAXI( (val)%0x100, (val)/0x100 )
#define JMPAX( addr )   MOVAX(addr), JMP_(R,AX)
#define PUSHAX( val )   MOVAX(val), PUSH(AX)

static inline void nop_(){}

static inline int
forth(char *start){
char *p = start;
unsigned link = 0;
trace=0;
{ UC x[] = { HALT, 00, 00 };
  memcpy( p, x, sizeof x );
  p += 0x100; } //boot code and return stack area. stack pointer assumed well out of the way
HEADLESS(enter, enter, PUSHRSP(SI), ADDAX,2,0, MOV(,R,SI,AX))
HEADLESS(docon, docon, MOV(,R,BX,AX), MOV(,B,AX,BX_),2, PUSH(AX))
HEADLESS(dovar, dovar, MOV(,R,BX,AX), LEA(,B,AX,BX_),2, PUSH(AX))
CODE(exec,   exec,     MOV(,R,BX,AX), PUSHRSP(SI), MOV(,R,SI,BX_),2)
CODE(execute,execute,  POP(BX), MOV(,R,AX,BX), MOV(,B,BX,BX_),0, JMP_(R,BX))
CODE(exit,   c_exit,   POPRSP(SI))
CODE(emit,   emit,     POP(DX), MOVAXI(00,02), INT(21))
CODE(key,    key,      MOVAXI(00,01), INT(21), XOR(,R,BX,BX), MOV(BYTE,R,BL,AL), PUSH(BX))
CODE(0branch,zbranch,  POP(BX), LODS, SHL(R,AX), OR(,R,BX,BX), JNZ,2, ADD(,R,SI,AX))
CODE(branch, branch,   LODS, SHL(R,AX), ADD(,R,SI,AX))
CODE(1branch,onbranch, POP(BX), LODS, SHL(R,AX), OR(,R,BX,BX), JZ, 2, ADD(,R,SI,AX))
CODE(lit,    lit,      LODS, PUSH(AX))
CODE(depth,  depth,    MOVAXI(0x00,0xf0), MOV(,R,BX,SP), SUB(,R,AX,BX),SHR(R,AX), PUSH(AX))
CODE(dup,    dup,      POP(AX), PUSH(AX), PUSH(AX))
CODE(drop,   drop,     POP(AX))
CODE(swap,   swap,     POP(AX), POP(BX), PUSH(AX), PUSH(BX))
CODE(over,   over,     MOV(,R,BX,SP), MOV(,B,AX,BX_),2, PUSH(AX))
CODE(pick,   pick,     POP(BX), SHL(R,BX), MOV(,R,DI,SP), MOV(,Z,AX,BX_DI), PUSH(AX))
CODE(rot,    rot,      POP(AX), POP(BX), POP(CX), PUSH(BX), PUSH(AX), PUSH(CX))
CODE(-dup,   minusdup, POP(AX), OR(,R,AX,AX), JZ,1,PUSH(AX), PUSH(AX))
CODE(-rot,   nrot,     POP(AX), POP(BX), POP(CX), PUSH(AX), PUSH(CX), PUSH(BX))
CODE(>r,     to_r,     POP(AX), PUSHRSP(AX))
CODE(r>,     from_r,   POPRSP(AX), PUSH(AX))
CODE(r,      r,        MOV(,B,AX,BP_),0, PUSH(AX))
CODE(2drop,  twodrop,  POP(AX), POP(AX))
CODE(2dup,   twodup,   POP(BX), POP(AX), PUSH(AX), PUSH(BX), PUSH(AX), PUSH(BX))
CODE(3dup,   threedup, POP(CX), POP(BX), POP(AX),
                       PUSH(AX), PUSH(BX), PUSH(CX), PUSH(AX), PUSH(BX), PUSH(CX))
CODE(2swap,  twoswap,  POP(DX),POP(CX),POP(BX),POP(AX),PUSH(CX),PUSH(DX),PUSH(AX),PUSH(BX))
CODE(0=,     zeq,      POP(BX), MOVAXI(0xff,0xff), OR(,R,BX,BX), JZ,2,INC_(R,AX), PUSH(AX))
CODE(0<,     zless,    POP(BX), MOVAXI(0xff,0xff), OR(,R,BX,BX), JL,2,INC_(R,AX), PUSH(AX))
CODE(0>,     zmore,    POP(BX), MOVAXI(0xff,0xff), OR(,R,BX,BX), JG,2,INC_(R,AX), PUSH(AX))
CODE(=,      eq,   POP(CX),POP(BX),MOVAXI(0xff,0xff),CMP(,R,BX,CX),JZ,2,INC_(R,AX),PUSH(AX))
CODE(<,      less, POP(CX),POP(BX),MOVAXI(0xff,0xff),CMP(,R,BX,CX),JL,2,INC_(R,AX),PUSH(AX))
CODE(>,      more, POP(CX),POP(BX),MOVAXI(0xff,0xff),CMP(,R,BX,CX),JG,2,INC_(R,AX),PUSH(AX))
CODE(not,    not,      POP(AX), NOT(R,AX), PUSH(AX))
CODE(1+,     oneplus,  MOV(,R,BX,SP), INC_(B,BX_),0)
CODE(1-,     oneminus, MOV(,R,BX,SP), DEC_(B,BX_),0)
CODE(+,      add,      POP(AX), POP(BX), ADD(,R,AX,BX), PUSH(AX))
CODE(-,      sub,      POP(AX), MOV(,R,BX,SP), SUB(F,B,AX,BX_),0)
CODE(*,      mul,      POP(AX), POP(BX), IMUL(R,BX), PUSH(AX))
CODE(/,      div,      POP(BX), XOR(,R,DX,DX), POP(AX), IDIV(R,BX), PUSH(AX))
CODE(mod,    mod,      POP(BX), XOR(,R,DX,DX), POP(AX), IDIV(R,BX), PUSH(DX))
CODE(/mod,   divmod,   POP(BX), XOR(,R,DX,DX), POP(AX), IDIV(R,BX), PUSH(DX), PUSH(AX))
CODE(u/mod,  udivmod,  POP(BX), XOR(,R,DX,DX), POP(AX), DIV(R,BX), PUSH(DX), PUSH(AX))
CODE(*/mod,  muldivmod,POP(CX),POP(BX),POP(AX), IMUL(R,BX), IDIV(R,CX), PUSH(DX), PUSH(AX))
CODE(*/,     muldiv,   POP(CX), POP(BX), POP(AX), IMUL(R,BX), IDIV(R,CX), PUSH(AX))
CODE(min,    min,      POP(BX), POP(AX), CMP(,R,AX,BX), JL,2, MOV(,R,AX,BX), PUSH(AX))
CODE(max,    max,      POP(BX), POP(AX), CMP(,R,AX,BX), JG,2, MOV(,R,AX,BX), PUSH(AX))
CODE(abs,    abs,      POP(AX), OR(,R,AX,AX), JGE,2, NEG(R,AX), PUSH(AX))
CODE(minus,  minus,    POP(AX), NEG(R,AX), PUSH(AX))
CODE(and,    and,      POP(BX), POP(AX), AND(,R,AX,BX), PUSH(AX))
CODE(or,     or,       POP(BX), POP(AX), OR(,R,AX,BX), PUSH(AX))
CODE(xor,    xor,      POP(BX), POP(AX), XOR(,R,AX,BX), PUSH(AX))
CODE(!,      bang,     POP(BX), POP(AX), MOV(F,Z,AX,BX_))
CODE(@,      at,       POP(BX), MOV(,Z,AX,BX_), PUSH(AX))
CODE(+!,     plusbang, POP(BX), POP(AX), ADD(F,Z,AX,BX_))
CODE(-!,     minusbang,POP(BX), POP(AX), SUB(F,Z,AX,BX_))
CODE(type,   type,     POP(CX), POP(BX), MOVAXI(00,02), 
                       OR(,R,CX,CX), JLE, 10,
                       MOV(BYTE,Z,DL,BX_), INT(21), INC_(R,BX), DEC_(R,CX), JNZ, -10)
CODE(s=,     seq,      POP(CX), POP(DX), POP(BX), POP(AX), PUSH(SI), STD,
                       CMP(,R,CX,BX), MOVBXI(0xff,0xff), JZ, 4, 
                         INC_(R,BX), sJMP, 11,        //(4)
                       MOV(,R,SI,AX), MOV(,R,DI,DX),  //(4)
                         BYTE+CMPS, JNZ, -11,         //(3)
                         DEC_(R,CX), JNZ, -7,         //(4)
                       POP(SI), PUSH(BX))
CODE(bye,    bye,      HALT)
WORD(<>,      ne,        enter, eq, not)
WORD(0,       zero,      docon, 0)
WORD(1,       one,       docon, 1)
WORD(2,       two,       docon, 2)
WORD(3,       three,     docon, 3)
WORD(4,       four,      docon, 4)
WORD(5,       five,      docon, 5)
WORD(10,      ten,       docon, 10)
WORD(var,     var,       dovar, 42)
WORD(base,    base,      dovar, 10)
WORD(cr,      cr,        enter, lit,'\n', emit)
WORD(space,   space,     enter, lit,' ', emit)
WORD(c@,      cat,       enter, at, lit, 0xff, and)
WORD(.sign,   dotsign,   enter, dup, zless, zbranch, 4, lit, '-', emit, minus)
WORD(.emit,   dotemit,   enter, dup,  ten, less, zbranch, 5,
                                  lit, '0', add, branch, 3,
                                  lit, 'A'-10, add,
                                emit)
WORD(.expand, dotexpand, enter, zero,swap,
                                  base, at, udivmod, rot, oneplus, swap,
                                  dup, zeq, zbranch, -10,
                                drop)
WORD(.digits, dotdigits, enter,   swap, dotemit, oneminus,
                                  dup, zeq, zbranch, -7,
                                drop)
WORD(u.,      udot,      enter, dotexpand, dotdigits, space)
WORD(.,       dot,       enter, dotsign, udot)
WORD(ok,      ok,        enter, lit,'O',emit, lit,'K',emit, cr)
WORD(latest,  latest,    docon, 0)
WORD(here,    here,      docon, 0)
WORD(allot,   allot,     enter, lit, here+2, plusbang)
WORD(ename,   ename,     enter,   //dup, udot,
                                dup, lit, 2, add, at, lit, 0xff, and,
                                  //dup, udot,
                                swap, lit, 3, add, swap)
WORD(ecode,   ecode,     enter, lit, offsetof( struct code_entry, code ), add)
WORD(words,   words,     enter, zero, latest,
                                  swap, //dup, udot,       //(1/3)
                                  oneplus, swap,           //(2)
                                  dup, ename, type, space, //(4)
                                  at,                      //(1)
                                  dup, zeq, zbranch, -12,  //(4)
                                drop, drop)
WORD(readline,readline,  enter, here, dup,
                                  key, //dup, udot,
                                  swap, twodup, bang,
                                  oneplus, swap,
                                  ten, eq, zbranch, -10,//-12,
                                lit, ' '|(' '<<8), over, oneminus, bang,
                                over, sub)//, oneminus)
                                //key, drop)
WORD(findloop,findloop,  enter, nrot,  // latest adr n
                                  threedup, rot, ename, //(3) // l a n a n a' n'
                                  //twodup, type, space,  //(3)
                                  seq, onbranch, 9, //(3) // l a n (b:a=a')
                                  rot, at, dup, zeq, onbranch, 4, //(6) //a n l@ (b:0=l@)
                                  nrot, branch,      // l a n
                                                //-3
                                                -15,  //(3)
                                  rot) //a n l(@*)|0
WORD(find,    find,      enter, latest, findloop,
                                dup, zeq, not, zbranch, 2,
                                  ecode, c_exit)
WORD(pspace1, pspace1,   enter, //dup, dot,
                                rot, twodup, swap, sub, swap, drop, // ad sp n-sp
                                nrot, add, swap)       // ad+sp n-sp
WORD(pspace,  pspace,    enter, swap, zero,            // n ad sp
                                  twodup, add, cat,    // n ad sp ad[sp]
                                  swap, oneplus, swap, // n ad sp++ ad[sp]
                                  lit, ' ', ne, zbranch, -11,
                                oneminus,              // n ad sp
                                pspace1)
WORD(pnspace2,pnspace2,  enter, //dup, dot, 
                                swap, to_r,             // n sp
                                twodup, sub, rot, drop, // sp n-sp
                                over, r, add, swap,     // sp ad+sp n-sp
                                rot, from_r, swap,      // ad+sp n-sp ad sp
                                dup, zmore, zbranch, 1, c_exit, drop, zero)
WORD(pnspace1,pnspace1,  enter, swap, zero,            // n ad sp=0
                                  twodup, add, cat,    // n ad sp ad[sp]
                                  swap, oneplus, swap, // n ad sp++ ad[sp]
                                  lit, ' ', eq, zbranch, -11,
                                oneminus,              // n ad sp
                                pnspace2)
WORD(pnspace, pnspace,   enter, dup, zmore, zbranch, 2, 
                                  pnspace1, c_exit,
                                twodup, drop, zero)
WORD(parse,   parse,     enter, //twodup, type, space, 
                                dup, zmore, zbranch, 2, 
                                  pspace, pnspace)//, twodup, type, space)
WORD(error,   error,     enter, lit, 'E', emit, lit, 'R', emit, lit, 'R', emit,
                                dot, dot, dot,
                                bye)
WORD(numdec,  numdec,    enter, dup, lit, 'A', less, zbranch, 5,  // a n a@
                                  lit, '0', sub, branch, 3, //(5)
                                  lit, 'A'-10, sub,  //(3)
                                //dup, dot, space,
                                dup, base, at, less, zbranch, 1, c_exit,
                                error)
WORD(number,  number,    enter, drop, zero,                               // a n=0
                                  over, cat,                        //(2) // a n a@
                                  //dup, dot,
                                  dup, lit, ' ', eq, onbranch, 11,  //(6)
                                  numdec, swap, base, at, mul, add, //(6) // a n'
                                  swap, oneplus, swap,              //(3) // a' n'
                                  branch, -19,                      //(2)
                                drop, swap, drop)                         // n
WORD(iexec,   iexec,     enter, nrot, drop, drop, nrot, // c a n
                                to_r, to_r, execute, from_r, from_r) // ...c? a n
WORD(intrp,   intrp,     enter, readline, // a n
                                  parse, //twodup, type, space,      //(1/4) // a n a' n'
                                  dup, zmore, zbranch, 8,         //(4)
                                  find, dup, zeq, onbranch, 6, //(5) // a n a' n' c (b:0=c)
                                  iexec, //(1)
                                  branch, -13, //(2)
                                twodrop, twodrop, c_exit, //(3) // ...c?
                                drop, number, nrot, //(3) // ...num a n
                                branch, -21) //(2)
WORD(test0,   test0,     enter, zero, dot,
                                one, dot, ten, dot, ok)
WORD(test1,   test1,     enter, zero, zeq, dot,
                                one, zeq, dot, ok)
WORD(test2,   test2,     enter, lit,12,            zless, dot,
                                lit,1+(0xffff^50), zless, dot,
                                lit,12,            zmore, dot,
                                lit,1+(0xffff^50), zmore, dot, ok)
WORD(test2a,  test2a,    enter, one, ten, less, dot,
                                one, ten, more, dot, ok)
WORD(test3,   test3,     enter, here, lit, 12, allot, 
                                lit, 12, type, ok)
WORD(test4,   test4,     enter, var, dot, var, at, dot, ok)
WORD(test5,   test5,     enter, ten, var, bang, var, at, dot, ok)
WORD(test6,   test6,     enter, lit, 16, base, bang, var, dot, var, at, dot, ok)
WORD(test7,   test7,     enter, lit, -10, udot, ok)
WORD(test8,   test8,     enter, lit, 'w', emit, lit, 'o', emit, lit, 'r', emit,
                                lit, 'd', emit, lit, 's', emit, cr,  words, ok)
WORD(test9,   test9,     enter, readline, //twodup, udot, udot,
                                type, ok)
WORD(test10,  test10,    enter, readline, twodup, dup, dot, type, space,
                                latest, ename, twodup, dup, dot, type, space, 
                                seq, dot, cr)
WORD(test11,  test11,    enter, five, four, three, two, one,
                                zero, pick, dot,
                                one, pick, dot,
                                two, pick, dot, 
                                drop, drop, drop, drop, drop, ok)
WORD(test12,  test12,    enter, readline, find, dot, dot, dot, ok)
WORD(test13,  test13,    enter, readline, find, dup, dot,
                                dup, zeq, onbranch, 1,
                                  execute,
                                dot, ok)
WORD(test14,  test14,    enter, lit, 16, base, bang,
                                readline,
                                  parse, twodup, type, space,
                                  dup, zmore, zbranch, 4,
                                  drop, drop, branch, -12,
                                drop, drop,
                                ok)
WORD(test15,  test15,    enter, ten, base, bang,
                                intrp, ok, branch, -4)
WORD(test,    test,      enter,
                                //test0, test1, test2, test2a,
                                //test3, test4, test5, test6,
                                //test7, test8, //test9, cr, test10, cr,
                                //test11, //test12, cr, test13, cr,
                                //test14, cr,
                                test15,
                                bye)
int dummy = 0;
CODE(interpret, interpret, JMPAX(dummy))
CODE(accept,    accept,    JMPAX(interpret))
  //UC patch[] = { JMPAX(accept) }; memcpy(start+interpret, patch, sizeof patch );
  memcpy(start+interpret+1, (US[]){ accept }, sizeof(US));
HEADLESS(quit,  quit,      MOVBPI( 0x00,0x20 ), JMPAX(accept))
HEADLESS(abort, abort,     MOVSPI( 0x00,0xf0 ), JMPAX(quit))
HEADLESS(cold,  cold,      JMPAX(abort))
memcpy( start+here+2, (US[]){ p-start }, 2);
memcpy( start+latest+2, (US[]){ link }, 2);
memcpy( p, "Hello world!", 12 );
nop_();
US init = test + 2;
{ UC x[] = { MOVBPI( 0x00,0x01 ), MOVSII(init%0x100,init/0x100), NEXT };
  memcpy( start, x, sizeof x ); }
if(trace){P("\n");}
trace=0;
return  0;}
