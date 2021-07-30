#include "ppnarg.h"

#define NAMESTRING(...) # __VA_ARGS__
#define PHRASE(code,more) \
  { \
    unsigned int so_far = _; \
    unsigned char bytes[] = { code }; \
    memcpy( ptr, bytes, sizeof bytes ); \
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


CODE(this, (MOVAL,37,JMP,-4))
           
CODE(this, (MOVAL,37,JMP,-4),
           (MOVAL,36,HALT) )

CODE(cmove, (POP(CX), POP(DX), POP(AX), PUSH(SI), CLD, OR(,R,CX,CX), JZ, 9),
              (MOV(,R,SI,AX), MOV(,R,DI,DX)),
              (BYTE+MOVS, DEC_(R,CX), JNZ, -5),
              (POP(SI)))

CODE(plusminus, (POP(BX), OR(,R,BX,BX), JS, 9),
                  (POP(AX), OR(,R,AX,AX), JNS, 2),
                  (NEG(R,AX), JMP, 7),
                  (POP(AX), OR(,R,AX,AX), JS, 2),
                  (NEG(R,AX), PUSH(AX)))

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
