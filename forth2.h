static int code();
static int word();
static int grow();

static int
forth( char *load_address ){
  unsigned char *image = NULL;
  unsigned int capacity;
  unsigned int offset = 0;
  offset += code( image, offset, capacity, "docol", PUSHRSP(SI), ADDAX,4,0, MOV(,R,DI,AX), NEXT );
  offset += word( image, offset, capacity, "double", "dup", "plus", "exit" );
}

static int
code( unsigned char *image, unsigned int offset, unsigned int capacity, char *name, ... ){
}

static int
word( unsigned char *image, unsigned int offset, unsigned int capacity, char *name, ... ){
}
