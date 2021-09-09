#include "ppnarg.h"

#define NAMESTRING(...) # __VA_ARGS__
#define FIRST(a,...) a
#define REST(a,...) __VA_ARGS__
#define PHRASE(code,more) \
  { \
    unsigned int so_far = _; \
    unsigned int FIRST code = so_far; \
    unsigned char bytes[] = { REST code }; \
    memcpy( ptr, bytes, sizeof bytes ), ptr += sizeof bytes; \
    unsigned int _ = so_far + sizeof bytes; \
    more \
  }
#define CODE1(a) PHRASE(a,)
#define CODE2(a,b) PHRASE(a,PHRASE(b,))
#define CODE3(a,b,c) PHRASE(a,PHRASE(b,PHRASE(c,)))
#define CODE4(a,b,c,d) PHRASE(a, PHRASE(b, PHRASE(c, PHRASE(d,))))
#define CODE5(a,b,c,d,e) PHRASE(a, PHRASE(b, PHRASE(c, PHRASE(d, PHRASE(e,)))))
#define CODE_(...) CAT(CODE, __VA_ARGS__ )
#define STET(a) a
#define CAT(a,b) a ## b
#define CODEPHRASES(...) CODE_( STET(PP_NARG(__VA_ARGS__)) ) ( __VA_ARGS__ )
  
#define CODE(n, ...) \
  { \
  char name[] = NAMESTRING( n ); \
  unsigned int _ = 0; \
  CODEPHRASES( __VA_ARGS__ ) \
  }


CODE(this, (L1, MOVAL,37,JMP,-4))
           
CODE(this, (L1, MOVAL,37,JMP,-4),
           (L2, MOVAL,36,HALT) )

CODE(cmove, (L1, POP(CX), POP(DX), POP(AX), PUSH(SI), CLD, OR(,R,CX,CX), JZ, 9),
              (L2, MOV(,R,SI,AX), MOV(,R,DI,DX)),
              (L3, BYTE+MOVS, DEC_(R,CX), JNZ, -5),
              (L4, _-L3),
              (L5, POP(SI)))

CODE(plusminus, (L1, POP(BX), OR(,R,BX,BX), JS, 9),
                  (L2, POP(AX), OR(,R,AX,AX), JNS, 2),
                  (L3, NEG(R,AX), JMP, 7),
                  (L4, POP(AX), OR(,R,AX,AX), JS, 2),
                  (L5, NEG(R,AX), PUSH(AX)))

#define PREPEND_COUNT(tweak,...) tweak PP_NARG(__VA_ARGS__), __VA_ARGS__
#define APPEND_COUNT(tweak,...) __VA_ARGS__ tweak PP_NARG(__VA_ARGS__)
#define FORWARD(...) PREPEND_COUNT(, __VA_ARGS__ )
#define BACKWARD(...) APPEND_COUNT(-, __VA_ARGS__ )

#define MOV(k,m,r,g) k+MOV, m ## r ## g
#define DEC_(m,r) DEC, m ## r

unsigned char bytes = { POP(CX), POP(DX), POP(AX), PUSH(SI), CLD,
  OR(,R,CX,CX), JZ, FORWARD(
    MOV(,R,SI,AX), MOV(,R,DI,DX),
    BACKWARD( BYTE+MOVS, DEC_(R,CX), JNZ, 0)
  ), POP(SI) };
