#include<ctype.h>
#include<inttypes.h>
#include<stdint.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/stat.h>
#include<sys/time.h>
#include<time.h>
#include<unistd.h>
#include"cp437.h"

#define P printf
#define R return
#define T typedef
#define CASE break;case
#define DEFAULT break;default
T intptr_t I; T uintptr_t U;
T short S; T unsigned short US;
T signed char C; T unsigned char UC; T void V;  // to make everything shorter
U o,w,d,f; // opcode, width, direction, extra temp variable (was for a flag, hence f)
U x,y,z;   // left operand, right operand, result
void *p;   // location to receive result
UC halt,debug=0,trace=0,reg[28],null[2],mem[0x100000]={ // operating flags, register memory, RAM
    1, 0300 /*(3<<6)*/,        // ADD ax,ax
    1, 0340 /*(3<<6)+(4<<3)*/, // ADD ax,sp
    3, 0340 /*(3<<6)+(4<<3)*/, // ADD sp,ax
    0xf4 //HLT
};
// memory map
// 0:0000-0:03ff interrupt vector table
// 0:0400-0:05ff dos global variables
// 0:0600-0:06ff forth boot code-> ... <-forth return stack
// 0:0700-0:1fb6 forth dictionary
// ...
// 0:C000-0:C3ff screen1
// 0:C400-0:C7ff screen2
// 0:C800-0:CBff screen3
// 0:CC00-0:CFff screen...
// ...
//       -0:F000 <- stack area
// 0:F000-0:F1ff bios interrupt handlers
// 0:F200-0:F264 forth tib (terminal input buffer)
// F:fff5-F:fffd rom release date
// F:fffe-F:fffe machine id

// register declaration and initialization
#define H(_)_(al)_(ah)_(cl)_(ch)_(dl)_(dh)_(bl)_(bh)
#define X(_)_(ax)     _(cx)     _(dx)     _(bx)     _(sp)_(bp)_(si)_(di)_(ip)_(fl)
#define SS(_)_(cs)_(ds)_(ss)_(es)
#define HD(_)UC*_;      // half-word regs declared as unsigned char *
#define XD(_)US*_;      // full-word regs declared as unsigned short *
#define HR(_)_=(UC*)(reg+i++);      // init and increment by one
#define XR(_)_=(US*)(reg+i);i+=2;   // init and increment by two
H(HD)X(XD)SS(XD)V init(){I i=0;H(HR)i=0;X(XR)SS(XR)}    // declare and initialize register pointers
enum { CF=1<<0, PF=1<<2, AF=1<<4, ZF=1<<6, SF=1<<7, TF=1<<8, IF=1<<9, DF=1<<10, OF=1<<11 };

#define HP(_)P(#_ ":%02x ",*_);     // dump a half-word reg as zero-padded hex
#define XP(_)P(#_ ":%04x ",*_);     // dump a full-word reg as zero-padded hex
V dump(){ //H(HP)P("\n");
    P("\n"); X(XP)
    if(trace)P("%s %s %s %s ",*fl&CF?"CA":"NC",*fl&OF?"OV":"NO",*fl&SF?"SN":"NS",*fl&ZF?"ZR":"NZ");
    P("\n");  // ^^^ crack flag bits into strings ^^^
}

// get and put into memory in a strictly little-endian format
I get_(void*p,U w){UC*c=p;R c[0]|(w?c[1]<<8:0);}
V put_(void*p,U x,U w){UC*c=p;c[0]=x;if(w)c[1]=x>>8;}
I bget(void*p){ R get_(p, 0); }
I wget(void*p){ R get_(p, 1); }
V bput(void*p,U x){ put_(p, x, 0); }
V wput(void*p,U x){ put_(p, x, 1); }
U qget(void*p, void*q){ R ( wget(p) << 16 ) | wget(q); };
U qput(void*p, void*q, U x){ R wput(p, x>>16), wput(q, x&0xffff), x; };

U segment(US *seg, US *adr){ R  ((U)wget(seg) << 4) | wget(adr); }
U cs_(US *adr){ R  segment( cs, adr ); }
U ds_(US *adr){ R  segment( ds, adr ); }
U ss_(US *adr){ R  segment( ss, adr ); }
U es_(US *adr){ R  segment( es, adr ); }

// get byte or word through cs:ip, incrementing ip
U inc(US*p){ U x; wput( p, x = 1 + wget( p ) ); R x; }
UC fetchb(){ U x = get_( mem + cs_(ip), 0 ); inc(ip);
             if(trace>1)P("%02llx(%03llo) ",(long long)x,(long long)x); R x; }
US fetchw(){I w=fetchb();R w|(fetchb()<<8);}

static int bios( UC vv[7] );
static int dos( UC vv[7] );
void escape( UC vv[7] ){
  if(trace>1)P( "%" PRIxPTR " ", (U)vv[0] );
  switch( vv[0] ){
  case 0x00:
  case 0x10:
  case 0x15:
  case 0x16: bios( vv );
  CASE 0x21: dos( vv );
  DEFAULT: P("unhandled escape code %02x\n", vv[0]);
  }
}

T struct rm{U mod,reg,r_m;}rm;      // the three fields of the mod-reg-r/m byte
rm mrm(U m){ R(rm){ (m>>6)&3, (m>>3)&7, m&7 }; }    // crack the mrm byte into fields
U decreg(U reg,U w){    // decode the reg field, yielding a uintptr_t to the register (byte/word)
    if (w)R (U)((US*[]){ax,cx,dx,bx,sp,bp,si,di}[reg]);
    else R (U)((UC*[]){al,cl,dl,bl,ah,ch,dh,bh}[reg]); }
U rs(US*x,US*y){ R get_(x,1)+get_(y,1); }  // fetch and sum two full-words
U decrm(rm r,U w){      // decode the r/m field, yielding uintptr_t to register or memory
    U x=(U[]){rs(bx,si),rs(bx,di),rs(bp,si),rs(bp,di),
              get_(si,1),get_(di,1),get_(bp,1),get_(bx,1)}[r.r_m];
    switch(r.mod){ CASE 0: if (r.r_m==6) R (U)(mem+fetchw());
                   CASE 1: x+=(I)(C)fetchb();
                   CASE 2: x+=(I)(S)fetchw();
                   CASE 3: R decreg(r.r_m,w); }
    R (U)(mem+x); }
U decseg(U sr){         // decode segment register
  R (U)((US*[]){es,cs,ss,ds}[sr&3]);
}

US*segov;
#define CLRSEGOV segov=0;
US*source(US*p,US*seg){ return segment(segov?segov:seg,p); }
US*dest(US*p,US*seg){ return segment(seg,p); }

V (*repstr)();
#define CLRREP repstr=(V(*)())0;

// opcode helpers
    // set d and w from o
#define DW  if(trace>1){ P("%s: ",__func__); } \
            d=!!(o&2); \
            w=o&1;
    // fetch mrm byte and decode, setting x and y as pointers to args and p ptr to dest
#define RMP rm r=mrm(fetchb());\
            x=decreg(r.reg,w); \
            y=decrm(r,w); \
            if(trace>1){ P("x:%" PRIdPTR " ",x); P("y:%" PRIdPTR " ",y); } \
            p=(void*)(d?x:y); \
            if(trace>1){ P("p:%" PRIdPTR " ",(U)p); }

    // fetch x and y values from x and y pointers
#define LDXY \
            x=get_((void*)x,w); \
            y=get_((void*)y,w); \
            if(trace>1){ P("x:%" PRIdPTR " ",x); P("y:%" PRIdPTR " ",y); }

    // normal mrm decode and load
#define RM  RMP LDXY

    // immediate to accumulator
#define IA x=(U)(p=w?(UC*)ax:al); \
           x=get_((void*)x,w); \
           y=w?fetchw():fetchb(); \
           d=1;

#define SETPF wput(fl, \
         wget(fl) | ( ((z)^(z>>1)^(z>>2)^(z>>3)^(z>>4)^(z>>5)^(z>>6)^(z>>7))&1 ?0:PF ) );
#define F(f) !!(wget(fl)&f) //test flag bit

    // flags set by logical operators
#define LOGFLAGS  wput(fl, ( (z&(w?0x8000:0x80))           ?SF:0) \
                         | ( (z&(w?0xffff:0xff))==0        ?ZF:0) );

    // additional flags set by math operators
#define MATHFLAGS(Overflow) *fl |= ( (z&(w?0xffff0000:0xff00))     ?CF:0) \
                                | (Overflow) \
                                | ( ((x^y^z)&0x10)                ?AF:0); \
                                SETPF

#define ADDFLAGS MATHFLAGS( ((    z  ^x)&(z^y)&(w?0x8000:0x80)) ?OF:0)
#define SUBFLAGS MATHFLAGS( (((d?x:y)^z)&(x^y)&(w?0x8000:0x80)) ?OF:0)

#define MULFLAGS  *fl &= ~(CF|OF); \
                  *fl |= (w?*dx:*ah)?CF|OF:0;

#define IMULFLAGS *fl &= ~(CF|OF); \
                  *fl |= (w?*ax&0x8000&&*dx==0xffff||!(*ax&0x8000)&&!*dx \
                           :*ah&0x80&&*ah==0xff||!(*ah&0x80)&&!*ah)?CF|OF:0;

    // store result to p ptr
#define RESULT \
        if(trace>1)P(w?"->%04llx ":"->%02llx ", (long long)z); \
        put_(p,z,w);

// operators, composed with helpers in the opcode table below
    // most of these macros will "enter" with x and y already loaded with operands
#define PUSH(x) *sp-=2,put_(mem+ss_(sp),*(x),1)
#define POP(x) *(x)=get_(mem+ss_(sp),1),*sp+=2
#define ADD z=x+y; LOGFLAGS ADDFLAGS RESULT
#define ADC x+=F(CF); ADD
#define SUB z=d?x-y:y-x; LOGFLAGS SUBFLAGS RESULT
#define SBB *(d?&y:&x)+=F(CF); SUB
#define CMP p=null; SUB
#define AND z=x&y; LOGFLAGS RESULT
#define  OR z=x|y; LOGFLAGS RESULT
#define XOR z=x^y; LOGFLAGS RESULT
#define INC(r) w=1; d=1; p=(V*)r; x=(S)*r; y=1; f=*fl&CF; ADD *fl=(*fl&~CF)|f;
#define DEC(r) w=1; d=1; p=(V*)r; x=(S)*r; y=1; f=*fl&CF; SUB *fl=(*fl&~CF)|f;
#define SEG(S) segov = S;
#define DAA if((*al&0xf)>9||F(AF)){*al+=6;STA}else CLA \
            if((*al&0xf0)>0x90||F(CF)){*al+=0x60;STC}else CLC \
            z=*al; LOGFLAGS SETPF
#define DAS if((*al&0xf)>9||F(AF)){f=*al<6;*al-=6; *fl|=f?CF:0; STA} \
            if(*al>0x9f||F(CF)){*al-=0x60; STC}else CLC \
            z=*al; LOGFLAGS SETPF
#define AAA if((*al&0xf)>9||F(AF)){*al+=6;*ah+=1;STA STC}else{CLA CLC} \
            *al&=0xf;
#define AAS if((*al&0xf)>9||F(AF)){*al-=6;*ah-=1;STA STC}else{CLA CLC} \
            *al&=0xf;
#define AAM f=fetchb();{I tal=*al;*ah=tal/f;*al=tal%f;} LOGFLAGS SETPF
#define AAD f=fetchb();{I tal=*al,tah=*ah;*al=(tal+tah*f)&0xff;*ah=0;} LOGFLAGS SETPF
#define J(c) U cf=F(CF),pf=F(PF),of=F(OF),sf=F(SF),zf=F(ZF); y=(S)(C)fetchb(); \
                  if(trace>1)P("<%d> ", c); \
                  if(c)*ip+=(S)y;
#define JN(c) J(!(c))
#define IMM(a,b) rm r=mrm(fetchb()); \
            p=(void*)(y=decrm(r,w)); \
            a \
            x=w?fetchw():fetchb(); \
            b \
            d=0; \
            y=get_((void*)y,w); \
            if(trace>1){ P("x:%" PRIdPTR " ",x); P("y:%" PRIdPTR " ",y); } \
            if(trace>1){ \
                P("%s ", (C*[]){"ADD","OR","ADC","SBB","AND","SUB","XOR","CMP"}[r.reg]); } \
            switch(r.reg){CASE 0:ADD \
                          CASE 1:OR \
                          CASE 2:ADC \
                          CASE 3:SBB \
                          CASE 4:AND \
                          CASE 5:SUB \
                          CASE 6:XOR \
                          CASE 7:CMP }
#define IMMIS IMM(w=0;,w=1;x=(S)(C)x;)
#define TEST z=x&y; LOGFLAGS ADDFLAGS
#define XCHG f=x;z=y; LDXY if(w){*(US*)f=y;*(US*)z=x;}else{*(UC*)f=y;*(UC*)z=x;}
#define MOV z=d?y:x; RESULT
#define MOVSEG rm r=mrm(fetchb()); \
               x=decseg(r.reg); \
               y=decrm(r,w); \
               z=*(US*)(d?x:y)=*(US*)(d?y:x);
#define LEA RMP z=((UC*)y)-mem; p=(void*)x; RESULT
#define NOP(n) (void)0;
#define AXCH(r) x=(U)ax; y=(U)(r); w=1; XCHG
#define CBW *ax=(S)(C)*al;
#define CWD z=(I)(S)*ax; *dx=-(z>>15);
#define CALL x=w?fetchw():(S)(C)fetchb(); PUSH(ip); (*ip)+=(S)x;
#define FARCALL PUSH(cs); PUSH(ip); FARJMP
#define WAIT
#define PUSHF PUSH(fl)
#define POPF POP(fl)
#define SAHF x=*fl; y=*ah; x=(x&~0xff)|y; *fl=x;
#define LAHF *ah=(UC)*fl;
#define mMOV if(d){ x=get_(mem+fetchw(),w); put_(ax,x,w); /*w?*ax=x:(*al=x);*/ } \
             else { put_(mem+fetchw(), get_(ax,w) /*w?*ax:*al*/,w ); }
#define Offset (w+1)*(1-d*2) //(w,d)=? (0,0)=1 (1,0)=2 (0,1)=-1 (1,1)=-2
#define MOVS put_(mem+es_(di),get_(mem+ds_(si),w),w); \
             d=!!(*fl&DF); *di+=Offset; *si+=Offset; \
             if(repstr)repstr();
#define CMPS x=(U)(mem+ds_(si)); y=(U)(mem+es_(di));\
             if(trace>1){ P("x:%" PRIdPTR " ",x); P("y:%" PRIdPTR " ",y); } \
             LDXY CMP \
             d=!!(*fl&DF); *di+=Offset; *si+=Offset; \
             if(repstr)repstr();
#define STOS put_(mem+es_(di),w?*ax:*al,w); \
             d=!!(*fl&DF); *di+=Offset; \
             if(repstr)repstr();
#define LODS if(w) *ax=get_(mem+ds_(si),w); else *al=get_(mem+ds_(si),w); \
             d=!!(*fl&DF); *si+=Offset; \
             if(repstr)repstr();
#define SCAS z=(x=w?*ax:*al)-(y=get_(mem+es_(di),w)); \
             d=!!(*fl&DF); *di+=Offset; \
             LOGFLAGS d=1; SUBFLAGS \
             if(repstr)repstr();
#define iMOVb(r) (*r)=fetchb();
#define iMOVw(r) if(trace>1)P("r:%" PRIxPTR " ",(U)r); (*r)=fetchw();
#define RET(v) POP(ip); if(v)*sp+=v*2;
#define LES
#define LDS
#define iMOVm if(w){iMOVw((US*)y)}else{iMOVb((UC*)y)}
#define fRET(v) POP(cs); RET(v)
#define INT(v) f = wget(mem+4*(x=v)); PUSH(fl); PUSH(cs); PUSH(ip); \
               *cs = wget(mem+4*x+2); \
               *ip = f; \
               if(trace>1)P("ip=%" PRIxPTR " ",(U)*ip);
#define INTO   if(*fl&OF){INT(4)}
#define IRET   POP(ip); POP(cs); POP(fl);
#define Shift rm r=mrm(fetchb()); \
              y=decrm(r,w); \
	      p=(void*)y; \
              y=get_((void*)y,w); \
	      if (trace>1)P("%s ", (C*[]){"ROL","ROR","RCL","RCR","SHL","SHR","SAR"}[r.reg]); \
              switch(r.reg){ \
	      CASE 0: ROL ; \
              CASE 1: ROR ; \
	      CASE 2: RCL ; \
              CASE 3: RCR ; \
	      CASE 4: SHL ; \
	      CASE 5: SHR ; \
              CASE 7: SAR ; \
              }
#define ROL z=y<<1|w?(y&0x8000)>>15:(y&0x80)>>7; RESULT
#define ROR z=y>>1|(y&1)<<(w?15:7); RESULT
#define RCL d=w?y>>15:y>>7; z=y<<1|!!(*fl&CF); *fl&=~CF; *fl|=d?CF:0; RESULT
#define RCR d=y&1; z=y>>1|(!!(*fl&CF))<<w?15:7; *fl&=~CF; *fl|=d?CF:0; RESULT
#define SHL z=y<<1; RESULT
#define SHR z=y>>1; RESULT
#define SAR z=(w?(I)(S)(US)y:(I)(C)(UC)y)>>1; RESULT
#define ShiftCL rm r=mrm(fetchb());
#define XLAT *al=mem[*bx+*al];
#define ESC(v) UC vv[7]={0}; for(int i=0; i<v; ++i) vv[i]=fetchb(); escape(vv);
#define LOOP y=(S)(C)fetchb(); if(--*cx) *ip+=(S)y;
#define LOOPZ y=(S)(C)fetchb(); if(--*cx&&*fl&ZF) *ip+=(S)y;
#define LOOPNZ y=(S)(C)fetchb(); if(--*cx&&!(*fl&ZF)) *ip+=(S)y;
#define JCXZ y=(S)(C)fetchb(); if(!*cx) *ip+=(S)y;
#define IN
#define OUT
#define INv
#define OUTv
#define JMP x=fetchw(); *ip+=(S)x;
#define sJMP x=(S)(C)fetchb(); *ip+=(S)x;
#define FARJMP x=fetchw(); y=fetchw(); *cs = y; *ip = x;
#define LOCK
V f_rep(){--*cx; if(*cx)--*ip; else CLRREP}
V f_repz(){--*cx; if(*cx&&*fl&ZF)--*ip; else CLRREP}
#define REP if(!*cx){fetchb();return;} repstr=f_rep;
#define REPZ if(!(*cx&&*fl&ZF)){fetchb();return;} repstr=f_repz;
#define HLT if(trace>1)P("HALT\n"); halt=1
#define CMC *fl=(*fl^CF);
#define NOT  z=~y; RESULT
#define NEG  z=w?-(S)y:-(C)y; RESULT
#define MUL  if(w){z=*ax*y; *ax=z; *dx=z>>16;} \
             else{z=*al*y; *ax=z;} MULFLAGS
#define IMUL if(w){z=(I)*ax*(I)y; *ax=z; *dx=z>>16;} \
             else{z=(I)*al*(I)y; *ax=z;} IMULFLAGS
#define DIV  if(!y){INT(0) return;} \
             if(w){d=*dx<<16|*ax; z=d/y; f=d%y; *ax=z; *dx=f;} \
             else{z=*ax/y; f=*ax%y; *al=z; *ah=f;} \
             //if(z>(w?0xffff:0xff)){INT(0) return;}
#define IDIV if(!y){INT(0) return;} \
             if(w){d=*dx<<16|*ax; z=(I)d/(S)y; f=(I)d%(S)y; *ax=z; *dx=f;} \
             else{z=(S)*ax/(UC)y; f=(S)*ax%(UC)y; *al=z; *ah=f;} \
             if((I)z>(w?0x7fff:0x7f) || (I)z<(w?-0x7fff:-0x7f)){INT(0) return;}
#define Grp1 rm r=mrm(fetchb()); \
             y=decrm(r,w); \
	     p=(V*)y; \
             y=get_((void*)y,w); \
             if(trace>1)P("%s ",(C*[]){"TEST","NOP","NOT","NEG","MUL","IMUL","DIV","IDIV"}[r.reg]); \
             switch(r.reg){CASE 0: x=w?fetchw():fetchb(); TEST; \
	                   CASE 1: NOP(-1); \
                           CASE 2: NOT; \
                           CASE 3: NEG; \
                           CASE 4: MUL; \
                           CASE 5: IMUL; \
                           CASE 6: DIV; \
                           CASE 7: IDIV; }
#define Grp2 rm r=mrm(fetchb()); \
             y=decrm(r,w); \
             if(trace>1)P("%s ",(C*[]){"INC","DEC","CALL","CALL","JMP","JMP","PUSH","NOP"}[r.reg]); \
             switch(r.reg){CASE 0: INC((S*)y); \
                           CASE 1: DEC((S*)y); \
                           CASE 2: CALL; \
                           CASE 3: CALL; \
                           CASE 4: /*JMP*/ wput(ip,wget((S*)y)); \
                           CASE 5: /*JMP interseg*/ wput(ip,wget((S*)y)); wput(cs,wget((S*)(y+2))); \
                           CASE 6: PUSH((S*)y); \
                           CASE 7: NOP(-2)}
#define CLC *fl=*fl&~CF;
#define STC *fl=*fl|CF;
#define CLI *fl=*fl&~IF;
#define STI *fl=*fl|IF;
#define CLD *fl=*fl&~DF;
#define STD *fl=*fl|DF;
#define CLA *fl=*fl&~AF;
#define STA *fl=*fl|AF;

// opcode table
// An x-macro table of pairs (a, b) where a becomes the name of a void function(void) which
// implements the opcode, and b comprises the body of the function (via further macro expansion)
#define OP(_)\
/*dw:bf                 wf                     bt                    wt   */ \
_(addbf, RM ADD)      _(addwf, RM ADD)       _(addbt,  RM ADD)     _(addwt, RM ADD)     /*00-03*/\
_(addbi, IA ADD)      _(addwi, IA ADD)       _(pushes, PUSH(es))   _(popes, POP(es))    /*04-07*/\
_(orbf,  RM OR)       _(orwf,  RM OR)        _(orbt,   RM OR)      _(orwt,  RM OR)      /*08-0b*/\
_(orbi,  IA OR)       _(orwi,  IA OR)        _(pushcs, PUSH(cs))   _(nop0, NOP(0))      /*0c-0f*/\
_(adcbf, RM ADC)      _(adcwf, RM ADC)       _(adcbt,  RM ADC)     _(adcwt, RM ADC)     /*10-13*/\
_(adcbi, IA ADC)      _(adcwi, IA ADC)       _(pushss, PUSH(ss))   _(popss, POP(ss))    /*14-17*/\
_(sbbbf, RM SBB)      _(sbbwf, RM SBB)       _(sbbbt,  RM SBB)     _(sbbwt, RM SBB)     /*18-1b*/\
_(sbbbi, IA SBB)      _(sbbwi, IA SBB)       _(pushds, PUSH(ds))   _(popds, POP(ds))    /*1c-1f*/\
_(andbf, RM AND)      _(andwf, RM AND)       _(andbt, RM AND)      _(andwt, RM AND)     /*20-23*/\
_(andbi, IA AND)      _(andwi, IA AND)       _(esseg, SEG(es))     _(daa, DAA)          /*24-27*/\
_(subbf, RM SUB)      _(subwf, RM SUB)       _(subbt, RM SUB)      _(subwt, RM SUB)     /*28-2b*/\
_(subbi, IA SUB)      _(subwi, IA SUB)       _(csseg, SEG(cs))     _(das, DAS)          /*2c-2f*/\
_(xorbf, RM XOR)      _(xorwf, RM XOR)       _(xorbt, RM XOR)      _(xorwt, RM XOR)     /*30-33*/\
_(xorbi, IA XOR)      _(xorwi, IA XOR)       _(ssseg, SEG(ss))     _(aaa, AAA)          /*34-37*/\
_(cmpbf, RM CMP)      _(cmpwf, RM CMP)       _(cmpbt, RM CMP)      _(cmpwt, RM CMP)     /*38-3b*/\
_(cmpbi, IA CMP)      _(cmpwi, IA CMP)       _(dsseg, SEG(ds))     _(aas, AAS)          /*3c-3f*/\
\
_(incax, INC(ax))     _(inccx, INC(cx))      _(incdx, INC(dx))     _(incbx, INC(bx))    /*40-43*/\
_(incsp, INC(sp))     _(incbp, INC(bp))      _(incsi, INC(si))     _(incdi, INC(di))    /*44-47*/\
_(decax, DEC(ax))     _(deccx, DEC(cx))      _(decdx, DEC(dx))     _(decbx, DEC(bx))    /*48-4b*/\
_(decsp, DEC(sp))     _(decbp, DEC(bp))      _(decsi, DEC(si))     _(decdi, DEC(di))    /*4c-4f*/\
_(pushax, PUSH(ax))   _(pushcx, PUSH(cx))    _(pushdx, PUSH(dx))   _(pushbx, PUSH(bx))  /*50-53*/\
_(pushsp, PUSH(sp))   _(pushbp, PUSH(bp))    _(pushsi, PUSH(si))   _(pushdi, PUSH(di))  /*54-57*/\
_(popax, POP(ax))     _(popcx, POP(cx))      _(popdx, POP(dx))     _(popbx, POP(bx))    /*58-5b*/\
_(popsp, POP(sp))     _(popbp, POP(bp))      _(popsi, POP(si))     _(popdi, POP(di))    /*5c-5f*/\
_(nop1,NOP(1))        _(nop2,NOP(2))         _(nop3,NOP(3))        _(nop4,NOP(4))       /*60=63*/\
_(nop5,NOP(5))        _(nop6,NOP(6))         _(nop7,NOP(7))        _(nop8,NOP(8))       /*64-67*/\
_(nop9,NOP(9))        _(nopA,NOP(A))         _(nopB,NOP(B))        _(nopC,NOP(C))       /*68-6b*/\
_(nopD,NOP(D))        _(nopE,NOP(E))         _(nopF,NOP(F))        _(nopG,NOP(G))       /*68-6f*/\
_(jo, J(of))          _(jno, JN(of))         _(jb, J(cf))          _(jnb, JN(cf))       /*70-73*/\
_(jz, J(zf))          _(jnz, JN(zf))         _(jbe, J(cf|zf))      _(jnbe, JN(cf|zf))   /*74-77*/\
_(js, J(sf))          _(jns, JN(sf))         _(jp, J(pf))          _(jnp, JN(pf))       /*78-7b*/\
_(jl, J(sf^of))       _(jnl_, JN(sf^of))     _(jle, J((sf^of)|zf))_(jnle,JN((sf^of)|zf))/*7c-7f*/\
\
_(immb, IMM(,))       _(immw, IMM(,))        _(immb1, IMM(,))      _(immis, IMMIS)      /*80-83*/\
_(testb, RM TEST)     _(testw, RM TEST)      _(xchgb, RMP XCHG)    _(xchgw, RMP XCHG)   /*84-87*/\
_(movbf, RM MOV)      _(movwf, RM MOV)       _(movbt, RM MOV)      _(movwt, RM MOV)     /*88-8b*/\
_(movsegf, MOVSEG)    _(lea, LEA)            _(movsegt, MOVSEG)  _(poprm,RM POP((US*)p))/*8c-8f*/\
_(nopH, NOP(H))       _(xchgac, AXCH(cx))    _(xchgad, AXCH(dx))   _(xchgab, AXCH(bx))  /*90-93*/\
_(xchgasp, AXCH(sp))  _(xchabp, AXCH(bp))    _(xchgasi, AXCH(si))  _(xchadi, AXCH(di))  /*94-97*/\
_(cbw, CBW)           _(cwd, CWD)            _(farcall, FARCALL)   _(wait, WAIT)        /*98-9b*/\
_(pushf, PUSHF)       _(popf, POPF)          _(sahf, SAHF)         _(lahf, LAHF)        /*9c-9f*/\
_(movalb, mMOV)       _(movaxw, mMOV)        _(movbal, mMOV)       _(movwax, mMOV)      /*a0-a3*/\
_(movsb, MOVS)        _(movsw, MOVS)         _(cmpsb, CMPS)        _(cmpsw, CMPS)       /*a4-a7*/\
_(testaib, IA TEST)   _(testaiw, IA TEST)    _(stosb, STOS)        _(stosw, STOS)       /*a8-ab*/\
_(lodsb, LODS)        _(lodsw, LODS)         _(scasb, SCAS)        _(scasw, SCAS)       /*ac-af*/\
_(movali, iMOVb(al))  _(movcli, iMOVb(cl))   _(movdli, iMOVb(dl))  _(movbli, iMOVb(bl)) /*b0-b3*/\
_(movahi, iMOVb(ah))  _(movchi, iMOVb(ch))   _(movdhi, iMOVb(dh))  _(movbhi, iMOVb(bh)) /*b4-b7*/\
_(movaxi, iMOVw(ax))  _(movcxi, iMOVw(cx))   _(movdxi, iMOVw(dx))  _(movbxi, iMOVw(bx)) /*b8-bb*/\
_(movspi, iMOVw(sp))  _(movbpi, iMOVw(bp))   _(movsii, iMOVw(si))  _(movdii, iMOVw(di)) /*bc-bf*/\
\
_(nopI, NOP(I))       _(nopJ, NOP(J))        _(reti, RET(fetchw())) _(retz, RET(0))     /*c0-c3*/\
_(les, LES)           _(lds, LDS)            _(movimb, RMP iMOVm)  _(movimw, RMP iMOVm) /*c4-c7*/\
_(nopK, NOP(K))       _(nopL, NOP(P))        _(freti, fRET(fetchw())) _(fretz, fRET(0)) /*c8-cb*/\
_(int3, INT(3))       _(inti, INT(fetchb())) _(into, INTO)         _(iret, IRET)        /*cc-cf*/\
_(shiftb, Shift)      _(shiftw, Shift)       _(shiftbv, ShiftCL)   _(shiftwv, ShiftCL)  /*d0-d3*/\
_(aam, AAM)           _(aad, AAD)            _(nopM, NOP(M))       _(xlat, XLAT)        /*d4-d7*/\
_(esc0, ESC(0))       _(esc1, ESC(1))        _(esc2, ESC(2))       _(esc3, ESC(3))      /*d8-db*/\
_(esc4, ESC(4))       _(esc5, ESC(5))        _(esc6, ESC(6))       _(esc7, ESC(7))      /*dc-df*/\
_(loopnz, LOOPNZ)     _(loopz, LOOPZ)        _(loop, LOOP)         _(jcxz, JCXZ)        /*e0-e3*/\
_(inb, IN)            _(inw, IN)             _(outb, OUT)          _(outw, OUT)         /*e4-e7*/\
_(call, w=1; CALL)    _(jmp, JMP)            _(farjmp, FARJMP)     _(sjmp, sJMP)        /*e8-eb*/\
_(invb, INv)          _(invw, INv)           _(outvb, OUTv)        _(outvw, OUTv)       /*ec-ef*/\
_(lock, LOCK)         _(nopN, NOP(N))        _(rep, REP)           _(repz, REPZ)        /*f0-f3*/\
_(hlt, HLT)           _(cmc, CMC)            _(grp1b, Grp1)        _(grp1w, Grp1)       /*f4-f7*/\
_(clc, CLC)           _(stc, STC)            _(cli, CLI)           _(sti, STI)          /*f8-fb*/\
_(cld, CLD)           _(std, STD)            _(grp2b, Grp2)        _(grp2w, Grp2)       /*fc-ff*/
#define OPF(a,b)void a(void){DW b;}// generate opcode function
#define OPN(a,b)a,                 // extract name
OP(OPF)void(*tab[])()={OP(OPN)};   // generate functions, declare and populate fp table with names

V clean(C*s){I i;       // replace unprintable characters in 80-byte buffer with spaces
    for(i=0;i<80;i++)
        if(!isprint(s[i]))
            s[i]=' ';
}
V video(){I i;          // dump the (cleaned) video memory to the console
    C buf[81]="";
    if(!trace)P("\e[0;0;f");
    for(i=0;i<28;i++)
        memcpy(buf, mem+0x8000+i*80, 80),
        clean(buf),
        P("\n%s",buf);
    P("\n");
}

static I ct;        // timer memory for period video dump
V run(){while(!halt){if(trace>1)dump();
    //if(!ct--){ct=10; video();}
    tab[o=fetchb()]();}}
V dbg(){
    while(!halt){
        C c;
        if(!ct--){ct=10; video();}
        if(trace)dump();
        //scanf("%c", &c);
        fgetc(stdin);
        //switch(c){
        //case '\n':
        //case 's':
            tab[o=fetchb()]();
            //break;
        //}
    }
}

I load(C*f){struct stat s; FILE*fp;     // load a file into memory at address zero
    R (fp=fopen(f,"rb"))
        && fstat(fileno(fp),&s) || fread(mem,s.st_size,1,fp); }

#include "forth3.h"
#include "bios.h"
#include "dos.h"


I main(I c,C**v){
    init();
    if(c>1){            // if there's an argument
        load(v[1]);     //     load named file
    }else{
        load_bios( mem ); //0000-03FF F000-F0FF
	puts("bios");
	load_dos( mem );  //0400-05FF
	puts("dos");
        forth( mem, mem + (*ip=0x600) );
	puts("forth");
    }
    *sp=0xF000;          // initialize stack pointer
    if(debug) dbg();    // if debugging, debug
    else run();         // otherwise, just run
    //video();            // dump final video
    return 0;}
