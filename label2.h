#include "ppnarg.h"

#define FIRST(a,...) a
#define REST(a,...) __VA_ARGS__
#define STET(...) __VA_ARGS__
#define CAT(a,b) a ## b
#define LABEL(label,offset) unsigned int label = offset; 
#define LABEL1(a)         LABEL(FIRST a, _)
#define LABEL2(a,b)       LABEL1(a) LABEL(FIRST b, FIRST a +PP_NARG(REST a))
#define LABEL3(a,b,c)     LABEL2(a,b) LABEL(FIRST c, FIRST b +PP_NARG(REST b))
#define LABEL4(a,b,c,d)   LABEL3(a,b,c) LABEL(FIRST d, FIRST c +PP_NARG(REST c))
#define LABEL5(a,b,c,d,e) LABEL4(a,b,c,d) LABEL(FIRST e, FIRST d +PP_NARG(REST d))
#define LABEL_(...) CAT(LABEL, __VA_ARGS__)
#define LABELS(...) LABEL_( STET(PP_NARG(__VA_ARGS__)) ) ( __VA_ARGS__ )
#define BYTE1(a) ++_,  *ptr++ = (a);
#define BYTE2(a,b) BYTE1(a) BYTE1(b)
#define BYTE3(a,b,c) BYTE2(a,b) BYTE1(c)
#define BYTE4(a,b,c,d) BYTE3(a,b,c) BYTE1(d)
#define BYTE5(a,b,c,d,e) BYTE4(a,b,c,d) BYTE1(e)
#define BYTE6(a,b,c,d,e,f) BYTE5(a,b,c,d,e) BYTE1(f)
#define BYTE7(a,b,c,d,e,f,g) BYTE6(a,b,c,d,e,f) BYTE1(g)
#define BYTE8(a,b,c,d,e,f,g,h) BYTE7(a,b,c,d,e,f,g) BYTE1(h)
#define BYTE9(a,b,c,d,e,f,g,h,i) BYTE8(a,b,c,d,e,f,g,h) BYTE1(i)
#define BYTE_(...) CAT(BYTE, __VA_ARGS__)
#define BYTES(...) BYTE_( STET(PP_NARG(__VA_ARGS__)) ) ( __VA_ARGS__ )
#define PHRASE(code) BYTES( REST code )
#define PHRASE1(a)         PHRASE(a)
#define PHRASE2(a,b)       PHRASE(a) PHRASE(b)
#define PHRASE3(a,b,c)     PHRASE(a) PHRASE2(b,c)
#define PHRASE4(a,b,c,d)   PHRASE(a) PHRASE3(b,c,d)
#define PHRASE5(a,b,c,d,e) PHRASE(a) PHRASE4(b,c,d,e)
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

CODE(cmove, (L1, POP(CX), POP(DX), POP(AX), PUSH(SI), CLD, OR(,R,CX,CX), JZ, L4-_),
            (L2, MOV(,R,SI,AX), MOV(,R,DI,DX)),
            (L3, BYTE+MOVS, DEC_(R,CX), JNZ, L3-_),
            (L4, POP(SI)))
