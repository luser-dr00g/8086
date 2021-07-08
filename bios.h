static int load_bios( char *mem ){
  //write handlers in mem > 0xF000
  char *p = mem + 0xF002;

  U divzero = p - mem;
  { UC x[] = { ESC(1), 0x00, IRET }; memcpy( p, x, sizeof x ); p += sizeof x; }
  U step = p - mem;
  { UC x[] = { ESC(1), 0x01, IRET }; memcpy( p, x, sizeof x ); p += sizeof x; }
  U nmi = p - mem;
  { UC x[] = { ESC(1), 0x02, IRET }; memcpy( p, x, sizeof x ); p += sizeof x; }
  U breakpt = p - mem;
  { UC x[] = { ESC(1), 0x03, IRET }; memcpy( p, x, sizeof x ); p += sizeof x; }

  U vid = p - mem;
  { UC x[] = { ESC(1), 0x10, IRET }; memcpy( p, x, sizeof x ); p += sizeof x; }

  U tracer = p - mem;
  { UC x[] = { ESC(1), 0x15, IRET }; memcpy( p, x, sizeof x ); p += sizeof x; }
  U keyboard = p - mem;
  { UC x[] = { ESC(1), 0x16, IRET }; memcpy( p, x, sizeof x ); p += sizeof x; }

  U dos = p - mem;
  { UC x[] = { ESC(1), 0x21, IRET }; memcpy( p, x, sizeof x ); p += sizeof x; }

  //write interrupt table in mem = 0..0x3FF
  p = mem;
  { US x[] = { divzero,  0 }; memcpy( p, x, sizeof x ); p += sizeof x; }
  { US x[] = { step,     0 }; memcpy( p, x, sizeof x ); p += sizeof x; }
  { US x[] = { nmi,      0 }; memcpy( p, x, sizeof x ); p += sizeof x; }
  { US x[] = { breakpt,  0 }; memcpy( p, x, sizeof x ); p += sizeof x; }

  p = mem + 0x10 * 4;
  { US x[] = { vid,      0 }; memcpy( p, x, sizeof x ); p += sizeof x; }

  p = mem + 0x15 * 4;
  { US x[] = { tracer,   0 }; memcpy( p, x, sizeof x ); p += sizeof x; }
  { US x[] = { keyboard, 0 }; memcpy( p, x, sizeof x ); p += sizeof x; }

  p = mem + 0x21 * 4;
  { US x[] = { dos,      0 }; memcpy( p, x, sizeof x ); p += sizeof x; }

}

static int bios( UC vv[7] ){
  if(trace>1){puts("BIOS");}
  switch( vv[0] ){
  CASE 0x00: printf("div by zero trap\n"); P("ip:%" PRIxPTR, (U)wget(mem+*sp)); dump();
  CASE 0x10: switch(bget(ah)){ //video
             CASE 0x0E: x = bget(al);
                        strchr("\n\07\08\015", x)  ? fputc( x, stdout )
                                                : fputs( cp437tounicode( x ), stdout );
             }
  CASE 0x15: trace = 1 - trace;
  CASE 0x16: switch(bget(ah)){ //keyboard
             CASE 0x00: bput(al, fgetc(stdin));
             }
  }
}
