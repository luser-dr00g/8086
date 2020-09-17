typedef unsigned char UC;
typedef unsigned short US;
enum { MAX_NAME = 9, MAX_CODE_PARAM = 40, MAX_WORD_PARAM = 30 };

struct code_entry {
  US link;
  UC name_len;
  UC name[ MAX_NAME ];
  US code;
  UC param[ MAX_CODE_PARAM ];
};

struct headless_entry {
  US link;
  UC name_len;
  UC name[ MAX_NAME ];
  UC param[ MAX_CODE_PARAM ];
};

struct word_entry {
  US link;
  UC name_len;
  UC name[ MAX_NAME ];
  US code;
  US param[ MAX_WORD_PARAM ];
};

#define CODE(n, e, ...) 				\
  const US  e  = P_CODE_PTR; 				\
  {							\
    struct code_entry x = { 				\
      .link     = link, 				\
      .name_len = sizeof(  # n  ) - 1, 			\
      .name     = # n , 				\
      .code     = P_PARAM_PTR, 				\
      .param    = { __VA_ARGS__ , NEXT }		\
    };							\
    memcpy( p, &x, sizeof x ); 				\
    if(trace)printf("%s:%x ", #e, e);			\
    link = p - start;					\
    p += sizeof x; 					\
  } 							\
/*end CODE()*/

#define P_CODE_PTR ( p - start ) + offsetof( struct code_entry, code )
#define P_PARAM_PTR ( p - start ) + offsetof( struct code_entry, param )

#define HEADLESS(n, e, ...)				\
  const US  e  = P_HEADLESS_PTR;			\
  {							\
    struct headless_entry x = {				\
      .link     = link,					\
      .name_len = sizeof(  # n  ) - 1,			\
      .name     = # n ,					\
      .param    = { __VA_ARGS__ , NEXT }		\
    };							\
    memcpy( p, &x, sizeof x );				\
    if(trace)printf("%s:%x ", #e, e);			\
    link = p - start;					\
    p += sizeof x;					\
  }
/*end HEADLESS()*/

#define P_HEADLESS_PTR ( p - start ) + offsetof( struct headless_entry, param )

#define WORD(namestring, cname, codeword, ...) 		\
  const US  cname  = P_WORD_PTR;			\
  {							\
    struct word_entry x = { 				\
      .link     = link,		 			\
      .name_len = sizeof(  # namestring  ) - 1, 	\
      .name     = # namestring ,			\
      .code     = codeword ,	 			\
      .param    = { __VA_ARGS__ , c_exit } 		\
    }; 							\
    memcpy( p, &x, sizeof x ); 				\
    if(trace)printf("%s:%x ", #cname, cname);		\
    link = p - start;					\
    p += sizeof x; 					\
  } 							\
/*end WORD()*/

#define P_WORD_PTR ( p - start ) + offsetof( struct word_entry, code )
