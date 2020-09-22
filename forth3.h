#include "asm8086.h"
#include "fdict.h"
/*	 W   = AX     PSP = SP
	 X   = BX     RSP = BP
	 IP  = SI     TOS_in_memory              */
#define NEXT       	LODS, MOV(,R,BX,AX), MOV(,Z,BX,BX_), JMP_(R,BX)
#define PUSHRSP(r) 	LEA(,B,BP,BP_),minus(4), MOV(F,B,r,BP_),0
#define POPRSP(r)  	MOV(,B,r,BP_),0, LEA(,B,BP,BP_),4
#define minus(x)	1+(0xff^x)
#define JMPAX( addr )   MOVAXI(addr), JMP_(R,AX)
#define PUSHAX( val )   MOVAXI(val), PUSH(AX)

static inline void nop_(){}

static inline int
forth(char *start){
char *p = start;
unsigned link = 0;
enum flag { immediate = 1, smudged = 2 };
unsigned flags = 0;
trace=0;
{ UC x[] = { HALT, 00, 00 };
  memcpy( p, x, sizeof x );
  p += 0x100; } //boot code and return stack area. stack pointer assumed well out of the way

HEADLESS(enter, enter, PUSHRSP(SI), ADDAX,2,0, MOV(,R,SI,AX))
HEADLESS(docon, docon, MOV(,R,BX,AX), MOV(,B,AX,BX_),2, PUSH(AX))
HEADLESS(dovar, dovar, MOV(,R,BX,AX), LEA(,B,AX,BX_),2, PUSH(AX))
HEADLESS(dostr, dostr, MOV(,R,BX,AX), MOV(,B,AX,BX_),2, MOV(,B,CX,BX_),4, PUSH(AX),PUSH(CX))

CODE(push,   push,     POP(BX), POP(AX), INC_(Z,BX_), 
                       MOV(,Z,DI,BX_), SHL(R,DI), ADD(,R,DI,BX),  MOV(F,Z,AX,DI_))
CODE(peek,   peek,     POP(BX),  MOV(,Z,DI,BX_), SHL(R,DI), ADD(,R,DI,BX),
                       MOV(,Z,AX,DI_), PUSH(AX))
CODE(pop,    pop,      POP(BX),  MOV(,Z,DI,BX_), SHL(R,DI), ADD(,R,DI,BX), 
                       MOV(,Z,AX,DI_), DEC_(Z,BX_), PUSH(AX))

CODE(execute,execute,  POP(BX), MOV(,R,AX,BX), MOV(,B,BX,BX_),0, JMP_(R,BX))
CODE(exit,   c_exit,   POPRSP(SI)) // all WORDs end with c_exit
CODE(emit,   emit,     POP(DX), MOVAXI(0x0200), INT(21))
CODE(key,    key,      MOVAXI(0x0100), INT(21), XOR(,R,BX,BX), MOV(BYTE,R,BL,AL), PUSH(BX))
CODE(0branch,zbranch,  POP(BX), LODS, SHL(R,AX), OR(,R,BX,BX), JNZ,2, ADD(,R,SI,AX))
CODE(branch, branch,   LODS, SHL(R,AX), ADD(,R,SI,AX))
CODE(1branch,onbranch, POP(BX), LODS, SHL(R,AX), OR(,R,BX,BX), JZ, 2, ADD(,R,SI,AX))
CODE(lit,    lit,      LODS, PUSH(AX))

CODE(dup,    dup,      POP(AX), PUSH(AX), PUSH(AX))
CODE(2dup,   twodup,   POP(BX), POP(AX), PUSH(AX), PUSH(BX), PUSH(AX), PUSH(BX))
CODE(3dup,   threedup, POP(CX), POP(BX), POP(AX),
                       PUSH(AX), PUSH(BX), PUSH(CX), PUSH(AX), PUSH(BX), PUSH(CX))
CODE(drop,   drop,     POP(AX))
CODE(2drop,  twodrop,  POP(AX), POP(AX))
CODE(swap,   swap,     POP(AX), POP(BX), PUSH(AX), PUSH(BX))
CODE(2swap,  twoswap,  POP(DX),POP(CX),POP(BX),POP(AX),PUSH(CX),PUSH(DX),PUSH(AX),PUSH(BX))
CODE(over,   over,     MOV(,R,BX,SP), MOV(,B,AX,BX_),2, PUSH(AX))
CODE(rot,    rot,      POP(AX), POP(BX), POP(CX), PUSH(BX), PUSH(AX), PUSH(CX))
CODE(-rot,   nrot,     POP(AX), POP(BX), POP(CX), PUSH(AX), PUSH(CX), PUSH(BX))
CODE(pick,   pick,     POP(BX), SHL(R,BX), MOV(,R,DI,SP), MOV(,Z,AX,BX_DI), PUSH(AX))
CODE(2pick,  twopick,  MOV(,R,BX,SP), MOV(,B,AX,BX_),4, PUSH(AX))
CODE(3pick,  threepick,MOV(,R,BX,SP), MOV(,B,AX,BX_),6, PUSH(AX))
CODE(?dup,   qdup,     POP(AX), OR(,R,AX,AX), JZ,1,PUSH(AX), PUSH(AX))
CODE(>r,     to_r,     POP(AX), PUSHRSP(AX))
CODE(r>,     from_r,   POPRSP(AX), PUSH(AX))
CODE(r,      r,        MOV(,B,AX,BP_),0, PUSH(AX))
CODE(depth,  depth,    MOVAXI(0xf000), MOV(,R,BX,SP), SUB(,R,AX,BX),SHR(R,AX), PUSH(AX))

CODE(0<,     zless,  POP(BX), MOVAXI(0xffff), OR(,R,BX,BX), JL,2,INC_(R,AX), PUSH(AX))
CODE(0=,     zeq,    POP(BX), MOVAXI(0xffff), OR(,R,BX,BX), JZ,2,INC_(R,AX), PUSH(AX))
CODE(0>,     zmore,  POP(BX), MOVAXI(0xffff), OR(,R,BX,BX), JG,2,INC_(R,AX), PUSH(AX))
CODE(<,      less,   POP(CX),POP(BX),MOVAXI(0xffff),CMP(,R,BX,CX),JL,2,INC_(R,AX),PUSH(AX))
CODE(=,      eq,     POP(CX),POP(BX),MOVAXI(0xffff),CMP(,R,BX,CX),JZ,2,INC_(R,AX),PUSH(AX))
CODE(>,      more,   POP(CX),POP(BX),MOVAXI(0xffff),CMP(,R,BX,CX),JG,2,INC_(R,AX),PUSH(AX))
CODE(not,    not,    POP(AX), NOT(R,AX), PUSH(AX))
WORD(<>,     ne,     enter, eq, not)

CODE(1+,     oneplus,  MOV(,R,BX,SP), INC_(B,BX_),0)
CODE(2+,     twoplus,  MOV(,R,BX,SP), INC_(B,BX_),0, INC_(B,BX_),0)
CODE(1-,     oneminus, MOV(,R,BX,SP), DEC_(B,BX_),0)
CODE(2-,     twominus, MOV(,R,BX,SP), DEC_(B,BX_),0)
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
CODE(c!,     cbang,    POP(BX), POP(AX), MOV(BYTE+F,Z,AL,BX_))
CODE(@,      at,       POP(BX), MOV(,Z,AX,BX_), PUSH(AX))
CODE(c@,     cat,      POP(BX), MOVAXI(0), MOV(BYTE,Z,AL,BX_), PUSH(AX))
CODE(+!,     plusbang, POP(BX), POP(AX), ADD(F,Z,AX,BX_))
CODE(-!,     minusbang,POP(BX), POP(AX), SUB(F,Z,AX,BX_))

CODE(type,   type,     POP(CX), POP(BX), MOVAXI(0x0200), 
                       OR(,R,CX,CX), JLE, 10,
                       MOV(BYTE,Z,DL,BX_), INT(21), INC_(R,BX), DEC_(R,CX), JNZ, -10)
CODE(s=,     seq,      POP(CX), POP(DX), POP(BX), POP(AX), PUSH(SI), STD,
                       CMP(,R,CX,BX), MOVBXI(0xffff), JZ, 4, 
                         INC_(R,BX), sJMP, 11,        //(4)
                       MOV(,R,SI,AX), MOV(,R,DI,DX),  //(4)
                         BYTE+CMPS, JNZ, -11,         //(3)
                         DEC_(R,CX), JNZ, -7,         //(4)
                       POP(SI), PUSH(BX))
//*
WORD(cmove,   cmove,     enter, // a n adr
                                swap, rot, // n adr a
                                  twopick, zeq, onbranch, 12, //(4)
                                  twodup, cat, swap, cbang, //(4)  // n adr a  (adr!a@)
                                  rot, oneminus, rot, oneplus, rot, oneplus, //(6) n- adr+ a+
                                  branch, -16, //(2)
                                drop, drop, drop)
				/**/

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
WORD(cr,      cr,        enter, lit,'\n', emit)
WORD(space,   space,     enter, lit,' ', emit)

WORD(.sign,   dotsign,   enter, dup, zless, zbranch, 4, lit, '-', emit, minus)
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
WORD(ok,      ok,        enter, lit,'O',emit, lit,'K',emit, cr)

WORD(dp,      dp,        dovar, 0) // patched to end of dictionary later
WORD(latest,  latest,    docon, 0) // patched to last dictionary entry later
WORD(here,    here,      enter, dp, at)
WORD(allot,   allot,     enter, dp, plusbang)
WORD(nfan,    nfan,      enter, dup, lit, offsetof( struct code_entry, name_len ), add, at,
                                swap, lit, offsetof( struct code_entry, name ), add, swap)
WORD(cfa,     cfa,       enter, lit, offsetof( struct code_entry, code ), add)
WORD(words,   words,     enter, zero, latest,
                                  swap, //dup, udot,       //(1/3)
                                  oneplus, swap,           //(2)
                                  dup, nfan, type, space, //(4)
                                  at,                      //(1)
                                  dup, zeq, zbranch, -12,  //(4)
                                drop, drop)

WORD(>in,     to_in,     dovar, 0);
WORD(pad,     pad,       dovar, 0);
p += 64;
#define BUFFER_SIZE 100
WORD(buffer,  buffer,    dostr, buffer+6, BUFFER_SIZE)
p += BUFFER_SIZE;
WORD(setbuf,  setbuf,    enter, lit, buffer+4, bang, lit, buffer+2, bang)
WORD(resetbuf,resetbuf,  enter, lit, buffer+6, lit, BUFFER_SIZE, setbuf)
WORD(readline,readline,  enter, resetbuf, buffer, drop, dup, // a a
                                  key, //dup, udot,          // a a k
                                  swap, twodup, bang,        // a k a  (a!k)
                                  oneplus, swap,             // a a' k
                                  ten, eq, zbranch, -10,//-12,
                                lit, ' '|(' '<<8), over, oneminus, bang,
                                over, sub, setbuf)

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
                                twodrop, lit, ' ', word)

CODE(bye,     bye,       HALT)
WORD(errout,  errout,    enter, bye) //patched to quit later
WORD(error,   error,     enter, lit, 'E', emit, lit, 'R', emit, lit, 'R', emit,
                                dot, dot, dot, cr,
                                errout)

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
                                add, bang,
                                twopick, lit, offsetof(struct word_entry,name),
                                add, swap, cmove)
WORD(!code,   bangcode,  enter, over, lit, offsetof(struct word_entry,code), add, bang)
WORD(!colon,  bangcolon, enter, lit, enter, bangcode)
WORD(!con,    bangcon,   enter, lit, docon, bangcode)
WORD(!var,    bangvar,   enter, lit, dovar, bangcode)
//WORD(param,   param,     enter, lit, offsetof(struct word_entry,code)+sizeof(US), add)
WORD(link!,   linkbang,  enter, lit, latest+2, bang) 
WORD(create,  create,    enter, here, 
                                lit, sizeof(struct word_entry), allot,
                                banglink)

WORD(bradr,   bradr,     dovar, 0)
p += 40;
WORD(doadr,   doadr,     dovar, 0)
p += 40;
WORD(beadr,   beadr,     dovar, 0)
p += 40;
WORD(whadr,   whadr,     dovar, 0)
p += 40;

#define COMMA ,
WORD(COMMA,   comma,     enter, //dup, dot, 
                                here, bang, two, allot)
WORD(],       rbracket,  enter, true, state, bang)

flags=immediate;
WORD([,       lbracket,  enter, zero, state, bang)
WORD(brsave,  brsave,    enter, here, bradr, push)
WORD(brpatch, brpatch,   enter, //here,
                                bradr, peek, sub, two, div, //dup, dot, cr,
                                  bradr, pop, bang)
WORD(if,      if_,       enter, //lit, 'I', emit, dup, dot,
                                lit, zbranch, comma,  //dup, dot,
                                brsave, zero, comma)
WORD(else,    else_,     enter, //lit, 'E', emit, dup, dot,
                                lit, branch, comma,  //dup, dot, cr,
                                here, brpatch,
                                brsave, zero, comma)
WORD(then,    then,      enter, //lit, 'T', emit,
                                here, twominus, brpatch)
WORD(endif,   endif,     enter, then)

WORD(besave,  besave,    enter, here, beadr, push)
WORD(whsave,  whsave,    enter, here, whadr, push)
WORD(bepatch, bepatch,   enter, beadr, peek, sub, two, div,
                                  beadr, pop, bang)
WORD(whpatch, whpatch,   enter, whadr, peek, sub, twominus, two, div,
                                  whadr, pop, bang)
WORD(bebrch,  bebrch,    enter, twoplus, beadr, pop, sub, two, div, minus,//dup,dot,cr,
                                comma)
WORD(begin,   begin,     enter, lit, 'B', emit, dup, //dot, cr,
                                besave)
WORD(until,   until,     enter, lit, 'U', emit, dup, //dot, cr,
                                lit, zbranch, comma, 
                                here, bebrch)
WORD(while,   while_,    enter, lit, 'W', emit, dup, //dot, cr,
                                lit, zbranch, comma,
                                whsave, zero, comma)
WORD(repeat,  repeat,    enter, lit, 'R', emit, dup, //dot, cr,
                                lit, branch, comma, 
                                here, bebrch,
                                whadr, twoplus, at, zbranch, 2,
                                  here, whpatch)

WORD(2trcom,  twotrcom,  enter, lit, to_r, comma, lit, to_r, comma)
WORD(2frcom,  twofrcom,  enter, lit, from_r, comma, lit, from_r, comma)
WORD(2drcom,  twodrcom,  enter, twofrcom, lit, twodrop, comma)
WORD(dobrch,  dobrch,    enter, here, twoplus, doadr, pop, sub, two, div, minus,//dup,dot,cr,
                                comma)
WORD(do,      do_,       enter, //lit, 'D', emit,
                                twotrcom,
                                here, doadr, push)
WORD(loop,    loop,      enter, //lit, 'O', emit,
                                twofrcom,
                                lit, oneplus, comma, 
                                lit, twodup, comma, lit, eq, comma,
                                lit, onbranch, comma, lit, 4, comma,
                                twotrcom,
                                lit, branch, comma, dobrch, twodrcom)
WORD(+loop,   plusloop,  enter, //lit, '+', emit,
                                twofrcom,
                                lit, rot, comma, lit, add, comma,
                                lit, twodup, comma, lit, more, comma,
                                lit, zbranch, comma, lit, 4, comma,
                                twotrcom,
                                lit, branch, comma, dobrch, twodrcom)
WORD(leave,   leave,     enter, //lit, 'X', emit,
                                twofrcom,
                                lit, drop, comma, lit, dup, comma, lit, oneminus, comma,
                                twotrcom)
WORD(i,       i_,        enter, //lit, 'i', emit,
                                twofrcom,
                                lit, dup, comma, lit, nrot, comma,
                                twotrcom)

WORD(."",     dotquote,  enter, lit, '"', word,
                                swap, oneplus, swap, oneminus, 
                                state, at, onbranch, 2,
                                  type, c_exit,
                                lit, branch, comma,
                                dup, dup, one, and, sub, dup, two, div, comma,
                                here, swap, allot,
                                over, to_r, dup, to_r,
                                swap, cmove,
                                lit, lit, comma, from_r, comma,
                                lit, lit, comma, from_r, comma,
                                lit, type, comma)
//memcpy( start+link+4, (US[]){ 2 }, sizeof(US) );
((struct word_entry *)(start+link))->name_len = 2;

WORD(;,       semi,      enter, //lit, ';', emit,
                                lit, c_exit, comma, //latest, dot, twodup, dot, dot,
                                lbracket)
flags=0;

WORD(:,       colon,     enter, //lit, ':', emit,
                                create, 
                                lit, smudged, bangflags, bangname, bangcolon, dup, linkbang, 
                                //dup, //param,
                                rbracket)//, twodup, udot, udot)
WORD(constant,constant,  enter, create, 
                                zero, bangflags, bangname, bangcon, dup, linkbang,
                                //dup, //param, rot, swap, bang, 
                                swap, comma)
WORD(variable,variable,  enter, create,
                                zero, bangflags, bangname, bangvar, dup, linkbang,
                                //dup, //param, rot, swap, bang,
                                swap, comma)

WORD(isimmed, isimmed,   enter, lit, (S)((I)offsetof(struct word_entry,flags) -
                                     offsetof(struct word_entry,code)), add, at,
                                lit, immediate, and)
WORD(compile, compile,   enter, //lit, 'C', emit, space,
                                //threedup, dot, dot, dot,
                                dup, isimmed, onbranch, 3, 
                                  comma, branch, 1,
                                  execute)
WORD(complit, complit,   enter, //lit, 'L', emit, space,
                                //threedup, dot, dot, dot,
                                lit, lit, comma, comma)
                                //swap, dup, lit, lit, swap, bang,
                                //twoplus, swap, over, bang,
                                //twoplus)
WORD(execomp, execomp,   enter, state, at, zbranch, 3, 
                                  compile, branch, 1,
                                  execute)

WORD(literal, literal,   enter, state, at, zbranch, 1,
                                  complit)
WORD(iexec,   iexec,     enter, nrot, drop, drop, // c
                                execomp) // c?

WORD(interpret,interpret,enter, //readline,                            // a n
                                  parse,  //twodup, type, space,//(1/4) // a' n'
                                  dup, zmore, zbranch, 8,       //(4)
                                  find, dup, zeq, onbranch, 5,  //(5) // a' n' c (b:0=c)
                                  iexec,                        //(1)
                                  branch, -13,                  //(2)
                                twodrop, c_exit,                //(2) // ...c?
                                drop, number, literal,          //(3) // ...num 
                                branch, -20)                    //(2)

WORD(accept,    accept,    enter, readline, interpret)
CODE(resetsp,   resetsp,   MOVSPI(0xf000))
CODE(resetrsp,  resetrsp,  MOVBPI(0x0100))
WORD(quit,      quit,      enter, resetsp, accept, ok, branch, -4)
WORD(abort,     abort,     enter, resetrsp, lbracket, quit)
HEADLESS(cold,  cold,      MOVSII(abort+2))

memcpy( start+errout+2, (US[]){ quit }, 2 );
memcpy( start+dp+2, (US[]){ p-start }, 2 );
memcpy( start+latest+2, (US[]){ link }, 2 );
memcpy( p, "Hello world!", 12 );

nop_();
{ UC x[] = { JMPAX(cold) }; memcpy( start, x, sizeof x ); } // boot code
if(trace){P("\n");}
trace=0;
return  0;}


     /*
WORD(e,       e,         enter, parse, type, space)
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

WORD(var,     var,       dovar, 42)
WORD(test4,   test4,     enter, var, dot, var, at, dot, ok)
WORD(test5,   test5,     enter, ten, var, bang, var, at, dot, ok)
WORD(test6,   test6,     enter, lit, 16, base, bang, var, dot, var, at, dot, ok)
WORD(test7,   test7,     enter, lit, -10, udot, ok)
WORD(test8,   test8,     enter, lit, 'w', emit, lit, 'o', emit, lit, 'r', emit,
                                lit, 'd', emit, lit, 's', emit, cr,  words, ok)
WORD(test9,   test9,     enter, readline, buffer, //twodup, udot, udot,
                                type, ok)

WORD(test10,  test10,    enter, readline, buffer, twodup, dup, dot, type, space,
                                latest, nfan, twodup, dup, dot, type, space, 
                                seq, dot, cr)
WORD(test11,  test11,    enter, five, four, three, two, one,
                                zero, pick, dot,
                                one, pick, dot,
                                two, pick, dot, 
                                drop, drop, drop, drop, drop, ok)
WORD(test12,  test12,    enter, readline, buffer, find, dot, dot, dot, ok)
WORD(test13,  test13,    enter, readline, buffer, find, dup, dot,
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
                                readline, interpret, ok, branch, -4)

WORD(test,    test,      enter,
                                test0, test1, test2, test2a,
                                test3, test4, test5, test6,
                                test7, test8, //test9, cr, test10, cr,
                                test11) //test12, cr, test13, cr,
                                //test14, cr, //test15, //bye)
				*/
