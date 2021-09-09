#include "ppnarg.h"

#define FIRST(a,...) a
#define REST(a,...) __VA_ARGS__
#define STET(...) __VA_ARGS__
#define CAT(a,b) a ## b
#define LABEL(label,offset) unsigned int label = offset; 
#define LABEL1(a)           LABEL(FIRST a, _)
#define LABEL2(a,b)         LABEL1(a) LABEL(FIRST b, FIRST a +PP_NARG(REST a))
#define LABEL3(a,b,c)       LABEL2(a,b) LABEL(FIRST c, FIRST b +PP_NARG(REST b))
#define LABEL4(a,b,c,d)     LABEL3(a,b,c) LABEL(FIRST d, FIRST c +PP_NARG(REST c))
#define LABEL5(a,b,c,d,e)   LABEL4(a,b,c,d) LABEL(FIRST e, FIRST d +PP_NARG(REST d))
#define LABEL6(a,b,c,d,e,f) LABEL5(a,b,c,d,e) LABEL(FIRST f, FIRST e +PP_NARG(REST e))
#define LABEL7(a,b,c,d,e,f,g) LABEL6(a,b,c,d,e,f) LABEL(FIRST g, FIRST f +PP_NARG(REST f))
#define LABEL_(...) CAT(LABEL, __VA_ARGS__)
#define LABELS(...) LABEL_( STET(PP_NARG(__VA_ARGS__)) ) ( __VA_ARGS__ )
#define BYTE(a) ++_,  *ptr++ = (a);
#define BYTE1(a) BYTE(a)
#define BYTE2(a,b) BYTE1(a) BYTE(b)
#define BYTE3(a,b,c) BYTE2(a,b) BYTE(c)
#define BYTE4(a,b,c,d) BYTE3(a,b,c) BYTE(d)
#define BYTE5(a,b,c,d,e) BYTE4(a,b,c,d) BYTE(e)
#define BYTE6(a,b,c,d,e,f) BYTE5(a,b,c,d,e) BYTE(f)
#define BYTE7(a,b,c,d,e,f,g) BYTE6(a,b,c,d,e,f) BYTE(g)
#define BYTE8(a,b,c,d,e,f,g,h) BYTE7(a,b,c,d,e,f,g) BYTE(h)
#define BYTE9(a,b,c,d,e,f,g,h,i) BYTE8(a,b,c,d,e,f,g,h) BYTE(i)
#define BYTE10(a,b,c,d,e,f,g,h,i,j) BYTE9(a,b,c,d,e,f,g,h,i) BYTE(j)
#define BYTE11(a,b,c,d,e,f,g,h,i,j,k) BYTE10(a,b,c,d,e,f,g,h,i,j) BYTE(k)
#define BYTE12(a,b,c,d,e,f,g,h,i,j,k,l) BYTE11(a,b,c,d,e,f,g,h,i,j,k) BYTE(l)
#define BYTE13(a,b,c,d,e,f,g,h,i,j,k,l,m) BYTE12(a,b,c,d,e,f,g,h,i,j,k,l) BYTE(m)
#define BYTE14(a,b,c,d,e,f,g,h,i,j,k,l,m,n) BYTE13(a,b,c,d,e,f,g,h,i,j,k,l,m) BYTE(n)
#define BYTE15(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o) BYTE14(a,b,c,d,e,f,g,h,i,j,k,l,m,n) BYTE(o)
#define BYTE16(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p) BYTE15(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o) BYTE(p)
#define BYTE17(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q) BYTE16(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p) BYTE(q)
#define BYTE18(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r) BYTE17(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q) \
                                                    BYTE(r)
#define BYTE19(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s) BYTE17(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q)\
                                                      BYTE2(r,s)
#define BYTE20(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t)BYTE17(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q)\
                                                      BYTE3(r,s,t)
#define BYTE_(...) CAT(BYTE, __VA_ARGS__)
#define BYTES(...) BYTE_( STET(PP_NARG(__VA_ARGS__)) ) ( __VA_ARGS__ )
#define PHRASE(code) BYTES( REST code )
#define PHRASE1(a)         PHRASE(a)
#define PHRASE2(a,b)       PHRASE(a) PHRASE(b)
#define PHRASE3(a,b,c)     PHRASE(a) PHRASE2(b,c)
#define PHRASE4(a,b,c,d)   PHRASE(a) PHRASE3(b,c,d)
#define PHRASE5(a,b,c,d,e) PHRASE(a) PHRASE4(b,c,d,e)
#define PHRASE6(a,b,c,d,e,f) PHRASE(a) PHRASE5(b,c,d,e,f)
#define PHRASE7(a,b,c,d,e,f,g) PHRASE(a) PHRASE6(b,c,d,e,f,g)
#define PHRASE_(...) CAT(PHRASE, __VA_ARGS__)
#define PHRASES(...) PHRASE_( STET(PP_NARG(__VA_ARGS__)) ) ( __VA_ARGS__ )

#define CODE(n, ...) \
  { \
  char name[] = # n ; \
  unsigned int _ = 0; \
  LABELS( __VA_ARGS__ ) \
  PHRASES( __VA_ARGS__ ) \
  }

#define OR(k,m,r,g) k+OR,m##r##g
#define MOV(k,m,r,g) k+MOV,m##r##g
#define DEC_(m,r) DEC,m##r

CODE(cmove,  (L1,  POP(CX), POP(DX), POP(AX), PUSH(SI), CLD, OR(,R,CX,CX), JZ, L4-_),
             (L2,  MOV(,R,SI,AX), MOV(,R,DI,DX)),
             (L3,  BYTE+MOVS, DEC_(R,CX), JNZ, L3-_),
             (L4,  POP(SI)))

CODE(lit,    (LIT, LODS, PUSH(AX))
#ifdef TRACE 
           , (PrtNum,    TEST(R,AX,AX), JGE, NotNeg-_, \
                         PUSH(AX), MOVAX(0xE00+'-'), INT(10), POP(AX), NEG(R,AX)),
             (NotNeg,    MOVBX(10), XOR(,R,CX,CX)), 
             (NextDigit, INC(CX), XOR(DX,DX), DIV(BX), PUSH(DX), TEST(AX,AX), JNZ, NextDigit-_),
             (PutDigit,  POP(AX), ADDAX(0xE00+'0'), INT(10), LOOP, PutDigit-_)
#endif
)

CODE(lit,    (LIT,      LODS, PUSH(AX))
#ifdef TRACE
           , (stub,     sJMP, CODE-_),
             (divisors, DW(10000), DW(1000), DW(100), DW(10)),
             (CODE,     MOVBX(divisors), XOR(,R,DX,DX),
                        DIV(Z,BX_), ADDAX(0xE00+'0'), INT(10)),
             (L1,       MOV(,R,AX,DX), DIV(B,BX_),2, ADDAX(0xE000+'0'), INT(10),
                        MOV(,R,AX,DX), DIV(B,BX_),4, ADDAX(0xE000+'0'), INT(10)),
             (L2,       MOV(,R,AX,DX), DIV(B,BX_),6, ADDAX(0xE000+'0'), INT(10),
	                MOD(,R,AX,DX), ADDAX(0xE00+'0'), INT(10))
#endif
)

CODE(lit,    (LIT,      LODS, PUSH(AX))
#ifdef TRACE
    , (tracelit, MOVBX(divisors), XOR(,R,DX,DX), MOVCX(5)),
      (digit,    DIV(Z,BX_), ADDAX(0xE00+'0'), INT(10),
                 MOV(,R,AX,DX), ADDI(BX,2), LOOP, digit-_,
                 NEXT)
      (divisors, DW(10000), DW(1000), DW(100), DW(10), DW(1)),
#endif
)

CODE(lit,    (LIT,      LODS, PUSH(AX))
#ifdef TRACE
    , (tracelit, XOR(,R,DX,DX), MOVCX(5),
                 TEST(R,AX,AX), JGE,notneg,
                 MOVI(AX,0xE00+'-'), INT(10), NEG(R,AX), JNO, notneg,
                 INC(R,DX)),
      (notneg,   MOVI(BX,10000), DIV(R,BX), CALL, print,
                 MOVI(BX,1000), MOV(,R,AX,DX), DIV(R,BX), CALL, print,
                 MOVI(BX,100), MOV(,R,AX,DX), DIV(R,BX), CALL, print,
                 MOVI(BX,10), MOV(,R,AX,DX), DIV(R,BX), CALL, print,
                 MOV(,R,AX,DX), CALL, print,
                 NEXT),
      (print,    ADDAX(0xE00'0'), INT(10), RET)
#endif
)
