#include "asm8086.h"
#include "fdict.h"

/*	 W   = AX     PSP = SP
	 X   = BX     RSP = BP
	 IP  = SI     TOS_in_memory              */
// all HEADLESS and CODE entries end with NEXT

#define TNEXT 		LODS, PUSH(AX), 					\
              		SUBAX(MAX_NAME), MOV(,R,DX,AX), 			\
              		DEC(AX),MOV(,R,BX,AX),MOV(BYTE,Z,CL,BX_),XOR(BYTE,R,CH,CH), \
              		MOVI(AX,0x4000), MOVI(BX,1), INT(21),			\
			MOVI(AX,0x0E00+' '), INT(10), 				\
                        POP(AX), MOV(,R,BX,AX), MOV(,Z,BX,BX_), JMP_(R,BX)
#define _NEXT       	LODS, MOV(,R,BX,AX), MOV(,Z,BX,BX_), JMP_(R,BX)

#ifdef TRACE
#  define NEXT TNEXT
#else
#  define NEXT _NEXT
#endif

#define PUSHRSP(r) 	LEA(,B,BP,BP_),minus(4), MOV(F,B,r,BP_),0
#define POPRSP(r)  	MOV(,B,r,BP_),0, LEA(,B,BP,BP_),4
#define minus(x)	1+(0xff^x)
#define JMPAX(addr)     MOVI(AX,addr), JMP_(R,AX)
#define PUSHAX(val)     MOVI(AX,val), PUSH(AX)

static inline void nop_(){}

static inline int
forth(char *mem, char *start){
char *p = start;
unsigned link = 0;
enum flag { immediate = 1, smudged = 2 };
unsigned flags = 0;
//trace=1;
{ UC x[] = { HALT, 00, 00, 00, 00 };
  memcpy( p, x, sizeof x );
  p += 0x100; } //boot code and return stack area. stack pointer assumed well out of the way

p += 2; //some wiggle room
HEADLESS(enter, enter, PUSHRSP(SI), ADDAX(2), MOV(,R,SI,AX)) // begin high level WORD
HEADLESS(docon, docon, MOV(,R,BX,AX), MOV(,B,AX,BX_),2, PUSH(AX))
HEADLESS(dovar, dovar, MOV(,R,BX,AX), LEA(,B,AX,BX_),2, PUSH(AX))
HEADLESS(dostr, dostr, MOV(,R,BX,AX), MOV(,B,AX,BX_),2, MOV(,B,CX,BX_),4, PUSH(AX),PUSH(CX))

CODE(push,   push,     POP(BX), POP(AX), INC_(Z,BX_), 
                       MOV(,Z,DI,BX_), SHL(R,DI), ADD(,R,DI,BX),  MOV(F,Z,AX,DI_))
CODE(peek,   peek,     POP(BX),  MOV(,Z,DI,BX_), SHL(R,DI), ADD(,R,DI,BX),
                       MOV(,Z,AX,DI_), PUSH(AX))
CODE(pop,    pop,      POP(BX),  MOV(,Z,DI,BX_), SHL(R,DI), ADD(,R,DI,BX), 
                       MOV(,Z,AX,DI_), DEC_(Z,BX_), PUSH(AX))

CODE(execute,execute,  POP(BX), MOV(,R,AX,BX), MOV(,Z,BX,BX_), JMP_(R,BX))
CODE(exit,   c_exit,   POPRSP(SI)) // all WORD()s end with c_exit

//CODE(emit,   emit,     POP(DX), MOVI(AX,0x0200), INT(21))
//CODE(key,    key,      MOVI(AX,0x0100), INT(21), XOR(,R,BX,BX), MOV(BYTE,R,BL,AL), PUSH(BX))
CODE(emit,   emit,     POP(AX), MOVBI(AH,0x0E), INT(10))
CODE(key,    key,      MOVI(AX,0), INT(16), XOR(BYTE,R,AH,AH), PUSH(AX))

CODE(0branch,zbranch,  POP(BX), LODS, SHL(R,AX), OR(,R,BX,BX), JNZ,2, ADD(,R,SI,AX))
CODE(branch, branch,   LODS, SHL(R,AX), ADD(,R,SI,AX))
CODE(1branch,onbranch, POP(BX), LODS, SHL(R,AX), OR(,R,BX,BX), JZ, 2, ADD(,R,SI,AX))

#ifdef TRACE
CODE(lit,  lit,
     LODS, PUSH(AX), MOV(,R,BX,AX),
     OR(,R,BX,BX), JGE,7, MOVI(AX,0x0E00+'-'), INT(10), NEG(R,BX),
     MOV(,R,AX,BX), XOR(,R,DX,DX), MOVI(CX,10000), DIV(R,CX),
       OR(,R,AX,AX), JZ,6,  MOVBI(AH,0x0E), ADDAL(48), INT(10),
     MOV(,R,AX,DX), XOR(,R,DX,DX), MOVI(CX,1000), DIV(R,CX),
       OR(,R,AX,AX), JZ,6,  MOVBI(AH,0x0E), ADDAL(48), INT(10),
     MOV(,R,AX,DX), MOVBI(CL,100), XOR(BYTE,R,CH,CH), BYTE+DIV(R,CL),
       MOV(BYTE,R,DL,AH),
       OR(BYTE,R,AL,AL), JZ,6,  MOVBI(AH,0x0E), ADDAL(48), INT(10),
     MOV(BYTE,R,AL,DL), XOR(BYTE,R,AH,AH), MOVBI(CL,10), BYTE+DIV(R,CL),
       MOV(BYTE,R,DL,AH),
       OR(BYTE,R,AL,AL), JZ,6,  MOVBI(AH,0x0E), ADDAL(48), INT(10),
     MOVBI(AH,0x0E), MOV(BYTE,R,AL,DL), ADDAL(48), INT(10),
       MOVI(AX,0x0E00+' '), INT(10) )/**/
#else
CODE(lit,  lit,      LODS, PUSH(AX))
#endif

CODE(dup,    dup,      POP(AX), PUSH(AX), PUSH(AX))
CODE(2dup,   twodup,   POP(BX), POP(AX),  PUSH(AX), PUSH(BX),  PUSH(AX), PUSH(BX))
CODE(3dup,   threedup, POP(CX), POP(BX), POP(AX),
                       PUSH(AX), PUSH(BX), PUSH(CX),  PUSH(AX), PUSH(BX), PUSH(CX))
CODE(drop,   drop,     POP(AX))
CODE(2drop,  twodrop,  POP(AX), POP(AX))
CODE(swap,   swap,     POP(AX), POP(BX),  PUSH(AX), PUSH(BX))
CODE(2swap,  twoswap,  POP(DX),POP(CX),POP(BX),POP(AX),PUSH(CX),PUSH(DX),PUSH(AX),PUSH(BX))
CODE(over,   over,     MOV(,R,BX,SP), MOV(,B,AX,BX_),2, PUSH(AX))
CODE(rot,    rot,      POP(AX), POP(BX), POP(CX),  PUSH(BX), PUSH(AX), PUSH(CX))
CODE(-rot,   nrot,     POP(AX), POP(BX), POP(CX),  PUSH(AX), PUSH(CX), PUSH(BX))
CODE(pick,   pick,     POP(BX),  SHL(R,BX), MOV(,R,DI,SP), MOV(,Z,AX,BX_DI),  PUSH(AX))
CODE(2pick,  twopick,  MOV(,R,BX,SP), MOV(,B,AX,BX_),4, PUSH(AX))
CODE(3pick,  threepick,MOV(,R,BX,SP), MOV(,B,AX,BX_),6, PUSH(AX))
CODE(?dup,   qdup,     POP(AX),  OR(,R,AX,AX),  JZ, 1, PUSH(AX),  PUSH(AX))
CODE(>r,     to_r,     POP(AX),  PUSHRSP(AX))
CODE(r>,     from_r,   POPRSP(AX),  PUSH(AX))
CODE(r,      r,        MOV(,B,AX,BP_),0, PUSH(AX))
CODE(depth,  depth,    MOVI(AX,0xf000), MOV(,R,BX,SP), SUB(,R,AX,BX),SHR(R,AX),  PUSH(AX))

CODE(0<,     zless,  POP(BX),  MOVI(AX,0xffff), OR(,R,BX,BX), JL,2,INC_(R,AX),  PUSH(AX))
CODE(0=,     zeq,    POP(BX),  MOVI(AX,0xffff), OR(,R,BX,BX), JZ,2,INC_(R,AX),  PUSH(AX))
CODE(0>,     zmore,  POP(BX),  MOVI(AX,0xffff), OR(,R,BX,BX), JG,2,INC_(R,AX),  PUSH(AX))
CODE(<,      less,   POP(CX),POP(BX),MOVI(AX,0xffff),CMP(,R,BX,CX),JL,2,INC_(R,AX),PUSH(AX))
CODE(=,      eq,     POP(CX),POP(BX),MOVI(AX,0xffff),CMP(,R,BX,CX),JZ,2,INC_(R,AX),PUSH(AX))
CODE(>,      more,   POP(CX),POP(BX),MOVI(AX,0xffff),CMP(,R,BX,CX),JG,2,INC_(R,AX),PUSH(AX))
CODE(not,    not,    POP(AX),  NOT(R,AX),  PUSH(AX))
WORD(<>,     ne,     enter, eq, not)

CODE(1+,     oneplus,  MOV(,R,BX,SP), INC_(B,BX_),0)
CODE(2+,     twoplus,  MOV(,R,BX,SP), INC_(B,BX_),0, INC_(B,BX_),0)
CODE(1-,     oneminus, MOV(,R,BX,SP), DEC_(B,BX_),0)
CODE(2-,     twominus, MOV(,R,BX,SP), DEC_(B,BX_),0, DEC_(B,BX_),0)
CODE(+,      add,      POP(AX), POP(BX),  ADD(,R,AX,BX),  PUSH(AX))
CODE(-,      sub,      POP(AX),  MOV(,R,BX,SP), SUB(F,B,AX,BX_),0)
CODE(*,      mul,      POP(AX), POP(BX),  IMUL(R,BX),  PUSH(AX))
CODE(/,      div,      //INT(15),
                       POP(BX), POP(AX),  CWD, IDIV(R,BX),  PUSH(AX))//, INT(15))
CODE(mod,    mod,      POP(BX), POP(AX),  CWD, IDIV(R,BX),  PUSH(DX))
CODE(/mod,   divmod,   POP(BX), POP(AX),  CWD, IDIV(R,BX),  PUSH(DX), PUSH(AX))
CODE(u/mod,  udivmod,  POP(BX), POP(AX),  CWD, DIV(R,BX),   PUSH(DX), PUSH(AX))
CODE(*/mod,  muldivmod,POP(CX),POP(BX),POP(AX), IMUL(R,BX), IDIV(R,CX),  PUSH(DX), PUSH(AX))
CODE(*/,     muldiv,   POP(CX),POP(BX),POP(AX), IMUL(R,BX), IDIV(R,CX),  PUSH(AX))
CODE(+-,     plusminus,POP(BX), OR(,R,BX,BX), JS, 9,
                         POP(AX), OR(,R,AX,AX), JNS, 2, //(5)
                         NEG(R,AX),                     //(2)
                         JMP, 7,                        //(2)
                         POP(AX), OR(,R,AX,AX), JS, 2,  //(5)
                         NEG(R,AX),                     //(2)
                       PUSH(AX))

CODE(min,    min,      POP(BX), POP(AX),  CMP(,R,AX,BX), JL,2, MOV(,R,AX,BX),  PUSH(AX))
CODE(max,    max,      POP(BX), POP(AX),  CMP(,R,AX,BX), JG,2, MOV(,R,AX,BX),  PUSH(AX))
CODE(abs,    abs,      POP(AX),  OR(,R,AX,AX), JGE,2, NEG(R,AX),  PUSH(AX))
CODE(negate, negate,   POP(AX),  NEG(R,AX),  PUSH(AX))
CODE(and,    and,      POP(BX), POP(AX),  AND(,R,AX,BX),  PUSH(AX))
CODE(or,     or,       POP(BX), POP(AX),  OR(,R,AX,BX),   PUSH(AX))
CODE(xor,    xor,      POP(BX), POP(AX),  XOR(,R,AX,BX),  PUSH(AX))

CODE(!,      bang,     POP(BX), POP(AX),  MOV(F,Z,AX,BX_))
CODE(c!,     cbang,    POP(BX), POP(AX),  MOV(BYTE+F,Z,AL,BX_))
CODE(@,      at,       POP(BX),  MOV(,Z,AX,BX_), PUSH(AX))
CODE(c@,     cat,      POP(BX),  MOVI(AX,0), MOV(BYTE,Z,AL,BX_),  PUSH(AX))
CODE(+!,     plusbang, POP(BX), POP(AX),  ADD(F,Z,AX,BX_))
CODE(-!,     minusbang,POP(BX), POP(AX),  SUB(F,Z,AX,BX_))

CODE(type,   type,     POP(CX), POP(BX),  MOVI(AX,0x0200), 
                       OR(,R,CX,CX), JLE, 10,
                       MOV(BYTE,Z,DL,BX_), INT(21),  INC_(R,BX), DEC_(R,CX), JNZ, -10)
CODE(s=,     seq,      POP(CX), POP(DX), POP(BX), POP(AX),  PUSH(SI), CLD,
                       CMP(,R,CX,BX), MOVI(BX,0xffff), JZ, 4, 
                         INC_(R,BX), sJMP, 11,        //(4)
                       MOV(,R,SI,AX), MOV(,R,DI,DX),  //(4)
                         BYTE+CMPS, JNZ, -11,         //(3)
                         DEC_(R,CX), JNZ, -7,         //(4)
                       POP(SI), PUSH(BX))
CODE(cmove,   cmove,   //INT(15), 
                       POP(CX), POP(DX), POP(AX),  PUSH(SI), CLD,
                       OR(,R,CX,CX), JZ, 9,
                         MOV(,R,SI,AX), MOV(,R,DI,DX), //(4)
                         BYTE+MOVS, DEC_(R,CX), JNZ, -5, //(5)
                       POP(SI))//, INT(15)

CODE((do),    _do_,      POP(AX), PUSHRSP(AX),  POP(AX), PUSHRSP(AX))
CODE(i,       i,         MOV(,B,AX,BP_),4, PUSH(AX))
CODE(leave,   leave,     MOV(,B,AX,BP_),4, MOV(F,B,AX,BP_),0)
CODE((loop),  _loop_,    //INT(15),
                         INC_(B,BP_),4,     // inc loop count on return stack
                         MOV(,B,AX,BP_),4,  // load loop count
                         CMP(,B,AX,BP_),0, JGE, 7,  // jump LOOP1 if count >= limit
                                                    // add backward branch offset. jump END
                           LODS, SHL(R,AX), ADD(,R,SI,AX), sJMP, 4,
                           LODS, LEA(,B,BP,BP_),8)  // LOOP1: discard params from ret stack
                         //INT(15)) // END: advance ip
CODE((+loop), _plusloop_,//INT(15),
                         POP(AX), ADD(F,B,AX,BP_),4,               // add arg to loop count
                         OR(,R,AX,AX), JL, 31,                           // jump :3 if arg<0
                           MOV(,B,BX,BP_),4, CMP(,B,BX,BP_),0, JGE, 12,  //(8) jmp :2 if lim
                           LODS, SHL(R,AX), ADD(,R,SI,AX),               //(5) branch NEXT
                           NEXT,                                         //(7) 2:
                           LODS, LEA(,B,BP,BP_),8,                       //(4) done NEXT
                           NEXT,                                         //(7) 3:
                           MOV(,B,BX,BP_),4, CMP(,B,BX,BP_),0, JLE, -19, //(8) jmp :2 if lim
                         LODS, SHL(R,AX), ADD(,R,SI,AX))                 // branch NEXT
                         //INT(15))
CODE(trac,    trac,      INT(15))

WORD(true,    true,      docon, -1)
WORD(0,       zero,      docon, 0)
WORD(1,       one,       docon, 1)
WORD(2,       two,       docon, 2)
WORD(3,       three,     docon, 3)
WORD(4,       four,      docon, 4)
WORD(5,       five,      docon, 5)
WORD(10,      ten,       docon, 10)
WORD(base,    base,      dovar, 10)
WORD(hex,     hex,       enter, lit, 16, base, bang)
WORD(decimal, decimal,   enter, lit, 10, base, bang)
WORD(octal,   octal,     enter, lit, 8, base, bang)

WORD(bl,      bl,        docon, ' ')
WORD(nl,      nl,        docon, '\n')
WORD(cr,      cr,        enter, nl, emit)
WORD(space,   space,     enter, bl, emit)
WORD(spaces,  spaces,    enter, dup, zeq, zbranch, 2, twodrop, c_exit, //(6)
                                space, oneminus, branch, -10) //(4)
WORD(blanks,  blanks,    enter, dup, zeq, zbranch, 2, twodrop, c_exit, //(6)
                                over, bl, swap, cbang, //(4)
                                oneminus, swap, oneplus, swap, branch, -16) //(6)

WORD(.sign,   dotsign,   enter, //trac, 
                                dup, zless, zbranch, 4, lit, '-', emit, negate)//, trac)
WORD(.emit,   dotemit,   enter, dup,  ten, less, zbranch, 5,
                                  lit, '0', add, branch, 3,
                                  lit, 'A'-10, add,
                                emit)
WORD(.expand, dotexpand, enter, zero,swap,
                                  base, at, udivmod, rot, oneplus, swap,
                                  dup, zeq, zbranch, -10,
                                drop)
WORD(.digits, dotdigits, enter,   swap, dotemit, oneminus, dup, zeq, zbranch, -7,
                                drop)
WORD(u.,      udot,      enter, dotexpand, dotdigits, space)
WORD(.,       dot,       enter, dotsign, udot)
WORD(based-u.,based_udot,enter, base, at, to_r, base, bang, udot, from_r, base, bang)
WORD(b.,      bdot,      enter, two, based_udot)
WORD(o.,      odot,      enter, lit, 8, based_udot)
WORD(h.,      hdot,      enter, lit, 16, based_udot)
WORD(?,       quest,     enter, at, dot)
WORD(ok,      ok,        enter, lit,'O',emit, lit,'K',emit, cr)

WORD(csp,     csp,       dovar, 0)
CODE(!csp,    bangcsp,   MOV(F,Z,SP,Z_), (csp+2)%0x100, (csp+2)/0x100)
WORD(dp,      dp,        dovar, 0) // patched to end of dictionary later
WORD(latest,  latest,    docon, 0) // patched to last dictionary entry later
WORD(here,    here,      enter, dp, at)
WORD(allot,   allot,     enter, dp, plusbang)
WORD(count,   count,     enter, dup, oneplus, swap, cat)
WORD(nfan,    nfan,      enter, //dup, 
                                lit, offsetof( struct code_entry, name_len ), add, 
                                count)//cat,
                                //swap, lit, offsetof( struct code_entry, name ), add, swap)
WORD(cfa,     cfa,       enter, lit, offsetof( struct code_entry, code ), add)
WORD(lfa,     lfa,       enter, lit, offsetof( struct code_entry, code ), sub)
WORD(words,   words,     enter, zero, latest,
                                  swap, //dup, udot,       //(1/3)
                                  oneplus, swap,           //(2)
                                  dup, nfan, type, space, //(4)
                                  at,                      //(1)
                                  dup, zeq, zbranch, -12,  //(4)
                                drop, drop)

WORD(tib,     tib,       dovar, 0xf000 + 100)
WORD(>in,     to_in,     dovar, 0);
WORD(blk,     blk,       dovar, 0);
WORD(pad,     pad,       dovar, 0); p += 64;
#define BUFFER_SIZE 100
WORD(buffer,  buffer,    dostr, buffer+6, BUFFER_SIZE) p += BUFFER_SIZE;
WORD(setbuf,  setbuf,    enter, lit, buffer+4, bang, lit, buffer+2, bang)
WORD(resetbuf,resetbuf,  enter, lit, buffer+6, lit, BUFFER_SIZE, setbuf)

WORD(readline,readline,  enter, resetbuf, buffer, drop, dup, // a a
                                  key, //dup, udot,          // a a k
                                  swap, twodup, bang,        // a k a  (a!k)
                                  oneplus, swap,             // a a' k
                                  ten, eq, zbranch, -10,//-12,
                                lit, ' '|(' '<<8), over, oneminus, bang,
                                over, sub, setbuf)

WORD(expect,  expect,    enter, // addr n
                                over, add, over, // addr n+addr addr
                                _do_,
                                  key, dup, lit, nl, eq, zbranch, 6,
                                    leave, drop, bl, zero, branch, 1, //(6)
                                    dup, //(1)
                                  i, cbang, zero, i, oneplus, bang,
                                  //emit,
                                _loop_,
                                drop)
WORD(query,   query,     enter, tib, at, 80, expect, zero, to_in, bang)
CODE(enclose, enclose,   POP(AX), XOR(BYTE,R,AH,AH), POP(BX), PUSH(BX), CLD,
                         MOV(,R,DI,BX),
			   BYTE+SCAS, JZ, -3,
                         MOV(,R,CX,DI), DEC_(R,CX), SUB(,R,CX,BX), PUSH(CX),
                           CMP(BYTE,Z,AH,DI_), JNZ, 13, //(4)
                             MOV(,R,CX,DI), SUB(,R,CX,BX), PUSH(CX), PUSH(CX), //(6)
                             NEXT, //(7)
                           BYTE+SCAS, JNZ, -20, //(3)
                         MOV(,R,CX,DI), DEC_(R,CX), SUB(,R,CX,BX), PUSH(CX),
                         INC_(R,CX), PUSH(CX))

WORD(block,    block,      enter, lit, 0x100, mul)
WORD(word2,    word2,      enter, blk, at, zbranch, 5,
                                  blk, at, block, branch, 2, //(5)
                                  tib, at, //(2)
                                to_in, at, add, // c addr
                                swap, enclose, // addr n1 n2 n3
                                here, lit, 34, blanks,
                                to_in, plusbang, // addr n1 n2
                                over, sub, to_r, // addr n1  [n2-n1]
                                r, here, cbang,  // store length byte
                                add, here, oneplus, // addr+n1 here+1
                                from_r, cmove)
                   

WORD(findloop,findloop,  enter, nrot,  // latest adr n
                                  threedup, rot, nfan, //(3) // l a n a n a' n'
                                  //twodup, type, space,  //(3)
                                  seq, onbranch, 9, //(3) // l a n (b:a=a')
                                  rot, at, dup, zeq, onbranch, 4, //(6) //a n l@ (b:0=l@)
                                  nrot, branch,      // l a n
                                                //-3
                                                -15,  //(3)
                                  rot) //a n l(@*)|0
WORD(find,    find,      enter, latest, findloop,
                                dup, zeq, not, zbranch, 1,
                                  cfa)

WORD(pdelim1, pdelim1,   enter, //dup, dot,
                                rot, twodup, swap, sub, swap, drop, // del ad sp n-sp
                                nrot, add, swap)       // del ad+sp n-sp
WORD(pdelim,  pdelim,    enter, nrot, swap, zero,      // del n ad sp
                                  twodup, add, cat,    // del n ad sp ad[sp]
                                  swap, oneplus, swap, // del n ad sp++ ad[sp]
                                  four, pick, ne, zbranch, -11,
                                oneminus,              // del n ad sp
                                pdelim1)

WORD(pndelim2,pndelim2,  enter, //dup, dot, 
                                swap, to_r, rot, drop,  // n sp
                                twodup, sub, rot, drop, // sp n-sp
                                over, r, add, swap, setbuf, // sp   (buf[ad+sp n-sp])
                                from_r, swap,          // ad sp
                                dup, zmore, zbranch, 1, c_exit, drop, zero)
WORD(pndelim1,pndelim1,  enter, swap, zero,            // del n ad sp=0
                                  twodup, add, cat,    // del n ad sp ad[sp]
                                  swap, oneplus, swap, // del n ad sp++ ad[sp]
                                  four, pick, eq, zbranch, -11,
                                oneminus,              // del n ad sp
                                pndelim2)
WORD(pndelim, pndelim,   enter, dup, zmore, zbranch, 2, // del ad n
                                  pndelim1, c_exit,
                                drop, swap, drop, zero)

WORD(word,    word,      enter, buffer, rot, pdelim, pndelim)//, twodup, type, space)

WORD(parse,   parse,     enter, //twodup, type, space, 
                                buffer, dup, zmore, zbranch, 3, 
                                twodrop, bl, word)

WORD(warning, warning,   dovar, 0)
WORD(errout,  errout,    enter, 0) //patched to quit later
WORD((abort), _abort_,   enter, 0) //patched to abort later
WORD(error,   error,     enter, warning, at, zless, zbranch, 1, _abort_,
                                lit, 'E', emit, lit, 'R', emit, lit, 'R', emit,
                                dot, dot, dot, cr, errout)
WORD(?error,  qerror,    enter, swap, zbranch, 2, error, c_exit,
                                drop)

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

WORD(state,   state,     dovar, 0)
WORD(!link,   banglink,  enter, latest, over, bang)
WORD(!flags,  bangflags, enter, over, lit, offsetof(struct word_entry,flags), add, bang)
WORD(!name,   bangname,  enter, parse, dup, // lfa a n n
                                threepick,lit,offsetof(struct word_entry,name_len),
                                add, cbang,
                                twopick, lit, offsetof(struct word_entry,name),
                                add, swap, cmove)
WORD(!code,   bangcode,  enter, over, lit, offsetof(struct word_entry,code), add, bang)
WORD(!colon,  bangcolon, enter, lit, enter, bangcode)
WORD(!con,    bangcon,   enter, lit, docon, bangcode)
WORD(!var,    bangvar,   enter, lit, dovar, bangcode)
WORD(link!,   linkbang,  enter, lit, latest+2, bang) 
WORD(create,  create,    enter, here, 
                                lit, sizeof(struct word_entry), allot,
                                banglink)

  #define COMMA ,
WORD(COMMA,   comma,     enter, here, bang, two, allot)
WORD(compile, compile,   enter, from_r, dup, twoplus, to_r, at, comma)
WORD(],       rbracket,  enter, true, state, bang) //start compiling

flags=immediate; // IMMEDIATE WORDS
WORD([,       lbracket,  enter, zero, state, bang) //stop compiling, start executing

WORD(if,      if_,       enter, compile, zbranch, here, zero, comma,
                                two)
WORD(endif,   endif,     enter, two, eq, onbranch, 1, error,
                                here, over, twoplus, sub, two, div, swap, bang)
WORD(else,    else_,     enter, two, eq, onbranch, 1, error,
                                compile, branch, here, zero, comma,
                                swap, here, over, twoplus, sub, two, div, swap, bang,
                                two)
WORD(then,    then,      enter, endif)

WORD(begin,   begin,     enter, here,
                                one)
WORD(back,    back,      enter, here, twoplus, sub, two, div, comma)
WORD(until,   until,     enter, one, eq, onbranch, 1, error,
                                compile, zbranch, back)
WORD(again,   again,     enter, one, eq, onbranch, 1, error,
                                compile, branch, back)
WORD(while,   while_,    enter, if_, twoswap)
WORD(repeat,  repeat,    enter, again, endif)

WORD(do,      do_,       enter, compile, _do_,
                                here, 
                                three)
WORD(loop,    loop,      enter, three, eq, onbranch, 1, error,
                                compile, _loop_, back)
WORD(+loop,   plusloop,  enter, three, eq, onbranch, 1, error,
                                compile, _plusloop_, back)

WORD(."",     dotquote,  enter, lit, '"', word,
                                swap, oneplus, swap, oneminus, 
                                state, at, onbranch, 2,
                                  type, c_exit,
                                compile, branch,
                                dup, dup, one, and, sub, dup, two, div, comma,
                                here, swap, allot,
                                over, to_r, dup, to_r,
                                swap, cmove,
                                compile, lit, from_r, comma,
                                compile, lit, from_r, comma,
                                compile, type)
  ((struct word_entry *)(mem+link))->name_len = 2;
WORD((),  lparen,    enter, lit, ')', word, twodrop)
  ((struct word_entry *)(mem+link))->name_len = 1;

WORD(;,       semi,      enter, //lit, ';', emit,
                                compile, c_exit, //latest, dot, twodup, dot, dot,
                                latest, zero, bangflags, drop,
                                lbracket)
flags=0; // Back to REGULAR WORDS

WORD(:,       colon,     enter, //lit, ':', emit,
                                create, 
                                lit, smudged, bangflags, bangname, bangcolon, linkbang, 
                                rbracket)//, twodup, udot, udot)
WORD(constant,constant,  enter, create, 
                                zero, bangflags, bangname, bangcon, dup, linkbang,
                                swap, comma)
WORD(variable,variable,  enter, create,
                                zero, bangflags, bangname, bangvar, dup, linkbang,
                                swap, comma)

WORD(isimmed, isimmed,   enter, lit, (S)((I)offsetof(struct word_entry,flags) -
                                     offsetof(struct word_entry,code)), add, at,
                                lit, immediate, and)

WORD(complit, complit,   enter, //lit, 'L', emit, space, //threedup, dot, dot, dot,
                                compile, lit, comma) 

WORD(execomp, execomp,   enter, state, at, zeq, over, isimmed, or, onbranch, 2,
                                  comma, c_exit,
                                execute)

WORD(literal, literal,   enter, state, at, zbranch, 1,
                                  complit)
WORD(iexec,   iexec,     enter, nrot, drop, drop, // c
                                execomp) // c?

WORD(interpret,interpret,enter,   parse,  //twodup, type, space,//(1/4) // a' n'
                                  dup, zmore, zbranch, 8,       //(4)
                                  find, dup, zeq, onbranch, 5,  //(5) // a' n' c (b:0=c)
                                  iexec,                        //(1)
                                  branch, -13,                  //(2)
                                twodrop, c_exit,                //(2) // ...c?
                                drop, number, literal,          //(3) // ...num 
                                branch, -20)                    //(2)

WORD(accept,    accept,    enter, readline, interpret)
CODE(resetsp,   resetsp,   MOVI(SP,0xf000))
CODE(resetrsp,  resetrsp,  MOVI(BP,0x0100))
CODE(bye,     bye,       HALT)
WORD(quit,      quit,      enter, resetsp, accept, ok, branch, -4)
WORD(abort,     abort,     enter, resetrsp, lbracket, quit)
HEADLESS(cold,  cold,      MOVI(SI,abort+2))

memcpy( mem+errout+2, (US[]){ quit }, 2 );
memcpy( mem+_abort_+2, (US[]){ abort }, 2 );
memcpy( mem+dp+2, (US[]){ p-mem }, 2 );
memcpy( mem+latest+2, (US[]){ link }, 2 );

nop_();
{ UC x[] = { JMPAX(cold) }; memcpy( start, x, sizeof x ); } // boot code

char src[] = 
//          1         2         3         4         5         6
// 1234567890123456789012345678901234567890123456789012345678901234
  "                                                                "
  "                                                                "
  "                                                                "
  "                                                                "
;
p = mem + 0xC000;
memcpy( p, src, sizeof src );

if(trace){P("\n");}
//trace=0;
return  0;}

