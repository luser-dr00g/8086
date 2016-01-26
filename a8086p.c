typedef intptr_t I;
typedef uintptr_t U;
typedef short S;
typedef unsigned short US;
typedef signed char C;
typedef unsigned char UC;
typedef void V;
U o, w, d, f;
U x, y, z;
void *p;
UC halt, debug = 0, trace = 2, reg[28], null[2], mem[0xffff] = {
  1, (3 << 6),
  1, (3 << 6) + (4 << 3),
  3, (3 << 6) + (4 << 3),
  0xf4
};

UC *al;
UC *ah;
UC *cl;
UC *ch;
UC *dl;
UC *dh;
UC *bl;
UC *bh;
US *ax;
US *cx;
US *dx;
US *bx;
US *sp;
US *bp;
US *si;
US *di;
US *ip;
US *fl;
US *cs;
US *ds;
US *ss;
US *es;
V
init ()
{
  I i = 0;
  al = (UC *) (reg + i++);
  ah = (UC *) (reg + i++);
  cl = (UC *) (reg + i++);
  ch = (UC *) (reg + i++);
  dl = (UC *) (reg + i++);
  dh = (UC *) (reg + i++);
  bl = (UC *) (reg + i++);
  bh = (UC *) (reg + i++);
  i = 0;
  ax = (US *) (reg + i);
  i += 2;
  cx = (US *) (reg + i);
  i += 2;
  dx = (US *) (reg + i);
  i += 2;
  bx = (US *) (reg + i);
  i += 2;
  sp = (US *) (reg + i);
  i += 2;
  bp = (US *) (reg + i);
  i += 2;
  si = (US *) (reg + i);
  i += 2;
  di = (US *) (reg + i);
  i += 2;
  ip = (US *) (reg + i);
  i += 2;
  fl = (US *) (reg + i);
  i += 2;
  cs = (US *) (reg + i);
  i += 2;
  ds = (US *) (reg + i);
  i += 2;
  ss = (US *) (reg + i);
  i += 2;
  es = (US *) (reg + i);
  i += 2;
}

enum
{ CF = 1, AF = 1 << 4, ZF = 1 << 6, SF = 1 << 7, OF = 1 << 11 };
V
dump ()
{
  printf ("\n");
  printf ("ax" ":%04x ", *ax);
  printf ("cx" ":%04x ", *cx);
  printf ("dx" ":%04x ", *dx);
  printf ("bx" ":%04x ", *bx);
  printf ("sp" ":%04x ", *sp);
  printf ("bp" ":%04x ", *bp);
  printf ("si" ":%04x ", *si);
  printf ("di" ":%04x ", *di);
  printf ("ip" ":%04x ", *ip);
  printf ("fl" ":%04x ", *fl);
  if (trace)
    printf ("%s %s %s %s ", *fl & CF ? "CA" : "NC", *fl & OF ? "OV" : "NO",
	    *fl & SF ? "SN" : "NS", *fl & ZF ? "ZR" : "NZ");
  printf ("\n");
}

I
get_ (void *p, U w)
{
  return w ? *(UC *) p + (((UC *) p)[1] << 8) : *(UC *) p;
}

V
put_ (void *p, U x, U w)
{
  if (w)
    {
      *(UC *) p = x;
      ((UC *) p)[1] = x >> 8;
    }
  else
    *(UC *) p = x;
}

UC
fetchb ()
{
  U x = get_ (mem + (*ip)++, 0);
  if (trace)
    printf ("%02x(%03o) ", x, x);
  return x;
}

US
fetchw ()
{
  I w = fetchb ();
  return w | (fetchb () << 8);
}

typedef struct rm
{
  U mod, reg, r_m;
} rm;
rm
mrm (U m)
{
  return (rm)
  {
  (m >> 6) & 3, (m >> 3) & 7, m & 7};
}

U
decreg (U reg, U w)
{
  if (w)
    return (U) ((US *[])
		{
		ax, cx, dx, bx, sp, bp, si, di}
		[reg]);
  else
  return (U) ((UC *[])
	      {
	      al, cl, dl, bl, ah, ch, dh, bh}
	      [reg]);
}

U
decrm (rm r, U w)
{
  U x = (U[]){ *bx + *si, *bx + *di, *bp + *si, *bp + *di, *si, *di, *bp, *bx }
  [r.r_m];
  switch (r.mod)
    {
    case 0:
      if (r.r_m == 6)
	return (U) (mem + fetchw ());
      break;
    case 1:
      x += fetchb ();
      break;
    case 2:
      x += fetchw ();
      break;
    case 3:
      return decreg (r.r_m, w);
    }
  return (U) (mem + x);
}

void
addbf ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  rm r = mrm (fetchb ());
  x = decreg (r.reg, w);
  y = decrm (r, w);
  if (trace > 1)
    {
      printf ("x:%d\n", x);
      printf ("y:%d\n", y);
    }
  p = d ? (void *) x : (void *) y;
  x = get_ ((void *) x, w);
  y = get_ ((void *) y, w);
  if (trace)
    {
      printf ("x:%d\n", x);
      printf ("y:%d\n", y);
    }
  z = x + y;
  *fl = 0;
  *fl |=
    ((z & (w ? 0x8000 : 0x80)) ? SF : 0) | ((z & (w ? 0xffff : 0xff)) ==
					    0 ? ZF : 0);
  *fl |=
    ((z & (w ? 0xffff0000 : 0xff00)) ? CF : 0) |
    (((z ^ x) & (z ^ y) & (w ? 0x8000 : 0x80)) ? OF : 0) |
    (((x ^ y ^ z) & 0x10) ? AF : 0);
  if (trace)
    printf (w ? "->%04x " : "->%02x ", z);
  put_ (p, z, w);;
}

void
addwf ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  rm r = mrm (fetchb ());
  x = decreg (r.reg, w);
  y = decrm (r, w);
  if (trace > 1)
    {
      printf ("x:%d\n", x);
      printf ("y:%d\n", y);
    }
  p = d ? (void *) x : (void *) y;
  x = get_ ((void *) x, w);
  y = get_ ((void *) y, w);
  if (trace)
    {
      printf ("x:%d\n", x);
      printf ("y:%d\n", y);
    }
  z = x + y;
  *fl = 0;
  *fl |=
    ((z & (w ? 0x8000 : 0x80)) ? SF : 0) | ((z & (w ? 0xffff : 0xff)) ==
					    0 ? ZF : 0);
  *fl |=
    ((z & (w ? 0xffff0000 : 0xff00)) ? CF : 0) |
    (((z ^ x) & (z ^ y) & (w ? 0x8000 : 0x80)) ? OF : 0) |
    (((x ^ y ^ z) & 0x10) ? AF : 0);
  if (trace)
    printf (w ? "->%04x " : "->%02x ", z);
  put_ (p, z, w);;
}

void
addbt ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  rm r = mrm (fetchb ());
  x = decreg (r.reg, w);
  y = decrm (r, w);
  if (trace > 1)
    {
      printf ("x:%d\n", x);
      printf ("y:%d\n", y);
    }
  p = d ? (void *) x : (void *) y;
  x = get_ ((void *) x, w);
  y = get_ ((void *) y, w);
  if (trace)
    {
      printf ("x:%d\n", x);
      printf ("y:%d\n", y);
    }
  z = x + y;
  *fl = 0;
  *fl |=
    ((z & (w ? 0x8000 : 0x80)) ? SF : 0) | ((z & (w ? 0xffff : 0xff)) ==
					    0 ? ZF : 0);
  *fl |=
    ((z & (w ? 0xffff0000 : 0xff00)) ? CF : 0) |
    (((z ^ x) & (z ^ y) & (w ? 0x8000 : 0x80)) ? OF : 0) |
    (((x ^ y ^ z) & 0x10) ? AF : 0);
  if (trace)
    printf (w ? "->%04x " : "->%02x ", z);
  put_ (p, z, w);;
}

void
addwt ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  rm r = mrm (fetchb ());
  x = decreg (r.reg, w);
  y = decrm (r, w);
  if (trace > 1)
    {
      printf ("x:%d\n", x);
      printf ("y:%d\n", y);
    }
  p = d ? (void *) x : (void *) y;
  x = get_ ((void *) x, w);
  y = get_ ((void *) y, w);
  if (trace)
    {
      printf ("x:%d\n", x);
      printf ("y:%d\n", y);
    }
  z = x + y;
  *fl = 0;
  *fl |=
    ((z & (w ? 0x8000 : 0x80)) ? SF : 0) | ((z & (w ? 0xffff : 0xff)) ==
					    0 ? ZF : 0);
  *fl |=
    ((z & (w ? 0xffff0000 : 0xff00)) ? CF : 0) |
    (((z ^ x) & (z ^ y) & (w ? 0x8000 : 0x80)) ? OF : 0) |
    (((x ^ y ^ z) & 0x10) ? AF : 0);
  if (trace)
    printf (w ? "->%04x " : "->%02x ", z);
  put_ (p, z, w);;
}

void
addbi ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  x = (U) (p = w ? (UC *) ax : al);
  x = get_ ((void *) x, w);
  y = w ? fetchw () : fetchb ();
  z = x + y;
  *fl = 0;
  *fl |=
    ((z & (w ? 0x8000 : 0x80)) ? SF : 0) | ((z & (w ? 0xffff : 0xff)) ==
					    0 ? ZF : 0);
  *fl |=
    ((z & (w ? 0xffff0000 : 0xff00)) ? CF : 0) |
    (((z ^ x) & (z ^ y) & (w ? 0x8000 : 0x80)) ? OF : 0) |
    (((x ^ y ^ z) & 0x10) ? AF : 0);
  if (trace)
    printf (w ? "->%04x " : "->%02x ", z);
  put_ (p, z, w);;
}

void
addwi ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  x = (U) (p = w ? (UC *) ax : al);
  x = get_ ((void *) x, w);
  y = w ? fetchw () : fetchb ();
  z = x + y;
  *fl = 0;
  *fl |=
    ((z & (w ? 0x8000 : 0x80)) ? SF : 0) | ((z & (w ? 0xffff : 0xff)) ==
					    0 ? ZF : 0);
  *fl |=
    ((z & (w ? 0xffff0000 : 0xff00)) ? CF : 0) |
    (((z ^ x) & (z ^ y) & (w ? 0x8000 : 0x80)) ? OF : 0) |
    (((x ^ y ^ z) & 0x10) ? AF : 0);
  if (trace)
    printf (w ? "->%04x " : "->%02x ", z);
  put_ (p, z, w);;
}

void
pushes ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  put_ (mem + (*sp -= 2), *(es), 1);
}

void
popes ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  *(es) = get_ (mem + (*sp += 2) - 2, 1);
}

void
orbf ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  rm r = mrm (fetchb ());
  x = decreg (r.reg, w);
  y = decrm (r, w);
  if (trace > 1)
    {
      printf ("x:%d\n", x);
      printf ("y:%d\n", y);
    }
  p = d ? (void *) x : (void *) y;
  x = get_ ((void *) x, w);
  y = get_ ((void *) y, w);
  if (trace)
    {
      printf ("x:%d\n", x);
      printf ("y:%d\n", y);
    }
  z = x | y;
  *fl = 0;
  *fl |=
    ((z & (w ? 0x8000 : 0x80)) ? SF : 0) | ((z & (w ? 0xffff : 0xff)) ==
					    0 ? ZF : 0);
  if (trace)
    printf (w ? "->%04x " : "->%02x ", z);
  put_ (p, z, w);;
}

void
orwf ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  rm r = mrm (fetchb ());
  x = decreg (r.reg, w);
  y = decrm (r, w);
  if (trace > 1)
    {
      printf ("x:%d\n", x);
      printf ("y:%d\n", y);
    }
  p = d ? (void *) x : (void *) y;
  x = get_ ((void *) x, w);
  y = get_ ((void *) y, w);
  if (trace)
    {
      printf ("x:%d\n", x);
      printf ("y:%d\n", y);
    }
  z = x | y;
  *fl = 0;
  *fl |=
    ((z & (w ? 0x8000 : 0x80)) ? SF : 0) | ((z & (w ? 0xffff : 0xff)) ==
					    0 ? ZF : 0);
  if (trace)
    printf (w ? "->%04x " : "->%02x ", z);
  put_ (p, z, w);;
}

void
orbt ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  rm r = mrm (fetchb ());
  x = decreg (r.reg, w);
  y = decrm (r, w);
  if (trace > 1)
    {
      printf ("x:%d\n", x);
      printf ("y:%d\n", y);
    }
  p = d ? (void *) x : (void *) y;
  x = get_ ((void *) x, w);
  y = get_ ((void *) y, w);
  if (trace)
    {
      printf ("x:%d\n", x);
      printf ("y:%d\n", y);
    }
  z = x | y;
  *fl = 0;
  *fl |=
    ((z & (w ? 0x8000 : 0x80)) ? SF : 0) | ((z & (w ? 0xffff : 0xff)) ==
					    0 ? ZF : 0);
  if (trace)
    printf (w ? "->%04x " : "->%02x ", z);
  put_ (p, z, w);;
}

void
orwt ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  rm r = mrm (fetchb ());
  x = decreg (r.reg, w);
  y = decrm (r, w);
  if (trace > 1)
    {
      printf ("x:%d\n", x);
      printf ("y:%d\n", y);
    }
  p = d ? (void *) x : (void *) y;
  x = get_ ((void *) x, w);
  y = get_ ((void *) y, w);
  if (trace)
    {
      printf ("x:%d\n", x);
      printf ("y:%d\n", y);
    }
  z = x | y;
  *fl = 0;
  *fl |=
    ((z & (w ? 0x8000 : 0x80)) ? SF : 0) | ((z & (w ? 0xffff : 0xff)) ==
					    0 ? ZF : 0);
  if (trace)
    printf (w ? "->%04x " : "->%02x ", z);
  put_ (p, z, w);;
}

void
orbi ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  x = (U) (p = w ? (UC *) ax : al);
  x = get_ ((void *) x, w);
  y = w ? fetchw () : fetchb ();
  z = x | y;
  *fl = 0;
  *fl |=
    ((z & (w ? 0x8000 : 0x80)) ? SF : 0) | ((z & (w ? 0xffff : 0xff)) ==
					    0 ? ZF : 0);
  if (trace)
    printf (w ? "->%04x " : "->%02x ", z);
  put_ (p, z, w);;
}

void
orwi ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  x = (U) (p = w ? (UC *) ax : al);
  x = get_ ((void *) x, w);
  y = w ? fetchw () : fetchb ();
  z = x | y;
  *fl = 0;
  *fl |=
    ((z & (w ? 0x8000 : 0x80)) ? SF : 0) | ((z & (w ? 0xffff : 0xff)) ==
					    0 ? ZF : 0);
  if (trace)
    printf (w ? "->%04x " : "->%02x ", z);
  put_ (p, z, w);;
}

void
pushcs ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  put_ (mem + (*sp -= 2), *(cs), 1);
}

void
nop0 ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;;
}

void
adcbf ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  rm r = mrm (fetchb ());
  x = decreg (r.reg, w);
  y = decrm (r, w);
  if (trace > 1)
    {
      printf ("x:%d\n", x);
      printf ("y:%d\n", y);
    }
  p = d ? (void *) x : (void *) y;
  x = get_ ((void *) x, w);
  y = get_ ((void *) y, w);
  if (trace)
    {
      printf ("x:%d\n", x);
      printf ("y:%d\n", y);
    }
  x += (*fl & CF);
  z = x + y;
  *fl = 0;
  *fl |=
    ((z & (w ? 0x8000 : 0x80)) ? SF : 0) | ((z & (w ? 0xffff : 0xff)) ==
					    0 ? ZF : 0);
  *fl |=
    ((z & (w ? 0xffff0000 : 0xff00)) ? CF : 0) |
    (((z ^ x) & (z ^ y) & (w ? 0x8000 : 0x80)) ? OF : 0) |
    (((x ^ y ^ z) & 0x10) ? AF : 0);
  if (trace)
    printf (w ? "->%04x " : "->%02x ", z);
  put_ (p, z, w);;
}

void
adcwf ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  rm r = mrm (fetchb ());
  x = decreg (r.reg, w);
  y = decrm (r, w);
  if (trace > 1)
    {
      printf ("x:%d\n", x);
      printf ("y:%d\n", y);
    }
  p = d ? (void *) x : (void *) y;
  x = get_ ((void *) x, w);
  y = get_ ((void *) y, w);
  if (trace)
    {
      printf ("x:%d\n", x);
      printf ("y:%d\n", y);
    }
  x += (*fl & CF);
  z = x + y;
  *fl = 0;
  *fl |=
    ((z & (w ? 0x8000 : 0x80)) ? SF : 0) | ((z & (w ? 0xffff : 0xff)) ==
					    0 ? ZF : 0);
  *fl |=
    ((z & (w ? 0xffff0000 : 0xff00)) ? CF : 0) |
    (((z ^ x) & (z ^ y) & (w ? 0x8000 : 0x80)) ? OF : 0) |
    (((x ^ y ^ z) & 0x10) ? AF : 0);
  if (trace)
    printf (w ? "->%04x " : "->%02x ", z);
  put_ (p, z, w);;
}

void
adcbt ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  rm r = mrm (fetchb ());
  x = decreg (r.reg, w);
  y = decrm (r, w);
  if (trace > 1)
    {
      printf ("x:%d\n", x);
      printf ("y:%d\n", y);
    }
  p = d ? (void *) x : (void *) y;
  x = get_ ((void *) x, w);
  y = get_ ((void *) y, w);
  if (trace)
    {
      printf ("x:%d\n", x);
      printf ("y:%d\n", y);
    }
  x += (*fl & CF);
  z = x + y;
  *fl = 0;
  *fl |=
    ((z & (w ? 0x8000 : 0x80)) ? SF : 0) | ((z & (w ? 0xffff : 0xff)) ==
					    0 ? ZF : 0);
  *fl |=
    ((z & (w ? 0xffff0000 : 0xff00)) ? CF : 0) |
    (((z ^ x) & (z ^ y) & (w ? 0x8000 : 0x80)) ? OF : 0) |
    (((x ^ y ^ z) & 0x10) ? AF : 0);
  if (trace)
    printf (w ? "->%04x " : "->%02x ", z);
  put_ (p, z, w);;
}

void
adcwt ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  rm r = mrm (fetchb ());
  x = decreg (r.reg, w);
  y = decrm (r, w);
  if (trace > 1)
    {
      printf ("x:%d\n", x);
      printf ("y:%d\n", y);
    }
  p = d ? (void *) x : (void *) y;
  x = get_ ((void *) x, w);
  y = get_ ((void *) y, w);
  if (trace)
    {
      printf ("x:%d\n", x);
      printf ("y:%d\n", y);
    }
  x += (*fl & CF);
  z = x + y;
  *fl = 0;
  *fl |=
    ((z & (w ? 0x8000 : 0x80)) ? SF : 0) | ((z & (w ? 0xffff : 0xff)) ==
					    0 ? ZF : 0);
  *fl |=
    ((z & (w ? 0xffff0000 : 0xff00)) ? CF : 0) |
    (((z ^ x) & (z ^ y) & (w ? 0x8000 : 0x80)) ? OF : 0) |
    (((x ^ y ^ z) & 0x10) ? AF : 0);
  if (trace)
    printf (w ? "->%04x " : "->%02x ", z);
  put_ (p, z, w);;
}

void
adcbi ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  x = (U) (p = w ? (UC *) ax : al);
  x = get_ ((void *) x, w);
  y = w ? fetchw () : fetchb ();
  x += (*fl & CF);
  z = x + y;
  *fl = 0;
  *fl |=
    ((z & (w ? 0x8000 : 0x80)) ? SF : 0) | ((z & (w ? 0xffff : 0xff)) ==
					    0 ? ZF : 0);
  *fl |=
    ((z & (w ? 0xffff0000 : 0xff00)) ? CF : 0) |
    (((z ^ x) & (z ^ y) & (w ? 0x8000 : 0x80)) ? OF : 0) |
    (((x ^ y ^ z) & 0x10) ? AF : 0);
  if (trace)
    printf (w ? "->%04x " : "->%02x ", z);
  put_ (p, z, w);;
}

void
adcwi ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  x = (U) (p = w ? (UC *) ax : al);
  x = get_ ((void *) x, w);
  y = w ? fetchw () : fetchb ();
  x += (*fl & CF);
  z = x + y;
  *fl = 0;
  *fl |=
    ((z & (w ? 0x8000 : 0x80)) ? SF : 0) | ((z & (w ? 0xffff : 0xff)) ==
					    0 ? ZF : 0);
  *fl |=
    ((z & (w ? 0xffff0000 : 0xff00)) ? CF : 0) |
    (((z ^ x) & (z ^ y) & (w ? 0x8000 : 0x80)) ? OF : 0) |
    (((x ^ y ^ z) & 0x10) ? AF : 0);
  if (trace)
    printf (w ? "->%04x " : "->%02x ", z);
  put_ (p, z, w);;
}

void
pushss ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  put_ (mem + (*sp -= 2), *(ss), 1);
}

void
popss ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  *(ss) = get_ (mem + (*sp += 2) - 2, 1);
}

void
sbbbf ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  rm r = mrm (fetchb ());
  x = decreg (r.reg, w);
  y = decrm (r, w);
  if (trace > 1)
    {
      printf ("x:%d\n", x);
      printf ("y:%d\n", y);
    }
  p = d ? (void *) x : (void *) y;
  x = get_ ((void *) x, w);
  y = get_ ((void *) y, w);
  if (trace)
    {
      printf ("x:%d\n", x);
      printf ("y:%d\n", y);
    }
  d ? y += *fl & CF : (x += *fl & CF);
  z = d ? x - y : y - x;
  *fl = 0;
  *fl |=
    ((z & (w ? 0x8000 : 0x80)) ? SF : 0) | ((z & (w ? 0xffff : 0xff)) ==
					    0 ? ZF : 0);
  *fl |=
    ((z & (w ? 0xffff0000 : 0xff00)) ? CF : 0) |
    (((z ^ x) & (z ^ y) & (w ? 0x8000 : 0x80)) ? OF : 0) |
    (((x ^ y ^ z) & 0x10) ? AF : 0);
  if (trace)
    printf (w ? "->%04x " : "->%02x ", z);
  put_ (p, z, w);;
}

void
sbbwf ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  rm r = mrm (fetchb ());
  x = decreg (r.reg, w);
  y = decrm (r, w);
  if (trace > 1)
    {
      printf ("x:%d\n", x);
      printf ("y:%d\n", y);
    }
  p = d ? (void *) x : (void *) y;
  x = get_ ((void *) x, w);
  y = get_ ((void *) y, w);
  if (trace)
    {
      printf ("x:%d\n", x);
      printf ("y:%d\n", y);
    }
  d ? y += *fl & CF : (x += *fl & CF);
  z = d ? x - y : y - x;
  *fl = 0;
  *fl |=
    ((z & (w ? 0x8000 : 0x80)) ? SF : 0) | ((z & (w ? 0xffff : 0xff)) ==
					    0 ? ZF : 0);
  *fl |=
    ((z & (w ? 0xffff0000 : 0xff00)) ? CF : 0) |
    (((z ^ x) & (z ^ y) & (w ? 0x8000 : 0x80)) ? OF : 0) |
    (((x ^ y ^ z) & 0x10) ? AF : 0);
  if (trace)
    printf (w ? "->%04x " : "->%02x ", z);
  put_ (p, z, w);;
}

void
sbbbt ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  rm r = mrm (fetchb ());
  x = decreg (r.reg, w);
  y = decrm (r, w);
  if (trace > 1)
    {
      printf ("x:%d\n", x);
      printf ("y:%d\n", y);
    }
  p = d ? (void *) x : (void *) y;
  x = get_ ((void *) x, w);
  y = get_ ((void *) y, w);
  if (trace)
    {
      printf ("x:%d\n", x);
      printf ("y:%d\n", y);
    }
  d ? y += *fl & CF : (x += *fl & CF);
  z = d ? x - y : y - x;
  *fl = 0;
  *fl |=
    ((z & (w ? 0x8000 : 0x80)) ? SF : 0) | ((z & (w ? 0xffff : 0xff)) ==
					    0 ? ZF : 0);
  *fl |=
    ((z & (w ? 0xffff0000 : 0xff00)) ? CF : 0) |
    (((z ^ x) & (z ^ y) & (w ? 0x8000 : 0x80)) ? OF : 0) |
    (((x ^ y ^ z) & 0x10) ? AF : 0);
  if (trace)
    printf (w ? "->%04x " : "->%02x ", z);
  put_ (p, z, w);;
}

void
sbbwt ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  rm r = mrm (fetchb ());
  x = decreg (r.reg, w);
  y = decrm (r, w);
  if (trace > 1)
    {
      printf ("x:%d\n", x);
      printf ("y:%d\n", y);
    }
  p = d ? (void *) x : (void *) y;
  x = get_ ((void *) x, w);
  y = get_ ((void *) y, w);
  if (trace)
    {
      printf ("x:%d\n", x);
      printf ("y:%d\n", y);
    }
  d ? y += *fl & CF : (x += *fl & CF);
  z = d ? x - y : y - x;
  *fl = 0;
  *fl |=
    ((z & (w ? 0x8000 : 0x80)) ? SF : 0) | ((z & (w ? 0xffff : 0xff)) ==
					    0 ? ZF : 0);
  *fl |=
    ((z & (w ? 0xffff0000 : 0xff00)) ? CF : 0) |
    (((z ^ x) & (z ^ y) & (w ? 0x8000 : 0x80)) ? OF : 0) |
    (((x ^ y ^ z) & 0x10) ? AF : 0);
  if (trace)
    printf (w ? "->%04x " : "->%02x ", z);
  put_ (p, z, w);;
}

void
sbbbi ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  x = (U) (p = w ? (UC *) ax : al);
  x = get_ ((void *) x, w);
  y = w ? fetchw () : fetchb ();
  d ? y += *fl & CF : (x += *fl & CF);
  z = d ? x - y : y - x;
  *fl = 0;
  *fl |=
    ((z & (w ? 0x8000 : 0x80)) ? SF : 0) | ((z & (w ? 0xffff : 0xff)) ==
					    0 ? ZF : 0);
  *fl |=
    ((z & (w ? 0xffff0000 : 0xff00)) ? CF : 0) |
    (((z ^ x) & (z ^ y) & (w ? 0x8000 : 0x80)) ? OF : 0) |
    (((x ^ y ^ z) & 0x10) ? AF : 0);
  if (trace)
    printf (w ? "->%04x " : "->%02x ", z);
  put_ (p, z, w);;
}

void
sbbwi ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  x = (U) (p = w ? (UC *) ax : al);
  x = get_ ((void *) x, w);
  y = w ? fetchw () : fetchb ();
  d ? y += *fl & CF : (x += *fl & CF);
  z = d ? x - y : y - x;
  *fl = 0;
  *fl |=
    ((z & (w ? 0x8000 : 0x80)) ? SF : 0) | ((z & (w ? 0xffff : 0xff)) ==
					    0 ? ZF : 0);
  *fl |=
    ((z & (w ? 0xffff0000 : 0xff00)) ? CF : 0) |
    (((z ^ x) & (z ^ y) & (w ? 0x8000 : 0x80)) ? OF : 0) |
    (((x ^ y ^ z) & 0x10) ? AF : 0);
  if (trace)
    printf (w ? "->%04x " : "->%02x ", z);
  put_ (p, z, w);;
}

void
pushds ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  put_ (mem + (*sp -= 2), *(ds), 1);
}

void
popds ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  *(ds) = get_ (mem + (*sp += 2) - 2, 1);
}

void
andbf ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  rm r = mrm (fetchb ());
  x = decreg (r.reg, w);
  y = decrm (r, w);
  if (trace > 1)
    {
      printf ("x:%d\n", x);
      printf ("y:%d\n", y);
    }
  p = d ? (void *) x : (void *) y;
  x = get_ ((void *) x, w);
  y = get_ ((void *) y, w);
  if (trace)
    {
      printf ("x:%d\n", x);
      printf ("y:%d\n", y);
    }
  z = x & y;
  *fl = 0;
  *fl |=
    ((z & (w ? 0x8000 : 0x80)) ? SF : 0) | ((z & (w ? 0xffff : 0xff)) ==
					    0 ? ZF : 0);
  if (trace)
    printf (w ? "->%04x " : "->%02x ", z);
  put_ (p, z, w);;
}

void
andwf ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  rm r = mrm (fetchb ());
  x = decreg (r.reg, w);
  y = decrm (r, w);
  if (trace > 1)
    {
      printf ("x:%d\n", x);
      printf ("y:%d\n", y);
    }
  p = d ? (void *) x : (void *) y;
  x = get_ ((void *) x, w);
  y = get_ ((void *) y, w);
  if (trace)
    {
      printf ("x:%d\n", x);
      printf ("y:%d\n", y);
    }
  z = x & y;
  *fl = 0;
  *fl |=
    ((z & (w ? 0x8000 : 0x80)) ? SF : 0) | ((z & (w ? 0xffff : 0xff)) ==
					    0 ? ZF : 0);
  if (trace)
    printf (w ? "->%04x " : "->%02x ", z);
  put_ (p, z, w);;
}

void
andbt ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  rm r = mrm (fetchb ());
  x = decreg (r.reg, w);
  y = decrm (r, w);
  if (trace > 1)
    {
      printf ("x:%d\n", x);
      printf ("y:%d\n", y);
    }
  p = d ? (void *) x : (void *) y;
  x = get_ ((void *) x, w);
  y = get_ ((void *) y, w);
  if (trace)
    {
      printf ("x:%d\n", x);
      printf ("y:%d\n", y);
    }
  z = x & y;
  *fl = 0;
  *fl |=
    ((z & (w ? 0x8000 : 0x80)) ? SF : 0) | ((z & (w ? 0xffff : 0xff)) ==
					    0 ? ZF : 0);
  if (trace)
    printf (w ? "->%04x " : "->%02x ", z);
  put_ (p, z, w);;
}

void
andwt ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  rm r = mrm (fetchb ());
  x = decreg (r.reg, w);
  y = decrm (r, w);
  if (trace > 1)
    {
      printf ("x:%d\n", x);
      printf ("y:%d\n", y);
    }
  p = d ? (void *) x : (void *) y;
  x = get_ ((void *) x, w);
  y = get_ ((void *) y, w);
  if (trace)
    {
      printf ("x:%d\n", x);
      printf ("y:%d\n", y);
    }
  z = x & y;
  *fl = 0;
  *fl |=
    ((z & (w ? 0x8000 : 0x80)) ? SF : 0) | ((z & (w ? 0xffff : 0xff)) ==
					    0 ? ZF : 0);
  if (trace)
    printf (w ? "->%04x " : "->%02x ", z);
  put_ (p, z, w);;
}

void
andbi ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  x = (U) (p = w ? (UC *) ax : al);
  x = get_ ((void *) x, w);
  y = w ? fetchw () : fetchb ();
  z = x & y;
  *fl = 0;
  *fl |=
    ((z & (w ? 0x8000 : 0x80)) ? SF : 0) | ((z & (w ? 0xffff : 0xff)) ==
					    0 ? ZF : 0);
  if (trace)
    printf (w ? "->%04x " : "->%02x ", z);
  put_ (p, z, w);;
}

void
andwi ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  x = (U) (p = w ? (UC *) ax : al);
  x = get_ ((void *) x, w);
  y = w ? fetchw () : fetchb ();
  z = x & y;
  *fl = 0;
  *fl |=
    ((z & (w ? 0x8000 : 0x80)) ? SF : 0) | ((z & (w ? 0xffff : 0xff)) ==
					    0 ? ZF : 0);
  if (trace)
    printf (w ? "->%04x " : "->%02x ", z);
  put_ (p, z, w);;
}

void
esseg ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;;
}

void
daa ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;;
}

void
subbf ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  rm r = mrm (fetchb ());
  x = decreg (r.reg, w);
  y = decrm (r, w);
  if (trace > 1)
    {
      printf ("x:%d\n", x);
      printf ("y:%d\n", y);
    }
  p = d ? (void *) x : (void *) y;
  x = get_ ((void *) x, w);
  y = get_ ((void *) y, w);
  if (trace)
    {
      printf ("x:%d\n", x);
      printf ("y:%d\n", y);
    }
  z = d ? x - y : y - x;
  *fl = 0;
  *fl |=
    ((z & (w ? 0x8000 : 0x80)) ? SF : 0) | ((z & (w ? 0xffff : 0xff)) ==
					    0 ? ZF : 0);
  *fl |=
    ((z & (w ? 0xffff0000 : 0xff00)) ? CF : 0) |
    (((z ^ x) & (z ^ y) & (w ? 0x8000 : 0x80)) ? OF : 0) |
    (((x ^ y ^ z) & 0x10) ? AF : 0);
  if (trace)
    printf (w ? "->%04x " : "->%02x ", z);
  put_ (p, z, w);;
}

void
subwf ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  rm r = mrm (fetchb ());
  x = decreg (r.reg, w);
  y = decrm (r, w);
  if (trace > 1)
    {
      printf ("x:%d\n", x);
      printf ("y:%d\n", y);
    }
  p = d ? (void *) x : (void *) y;
  x = get_ ((void *) x, w);
  y = get_ ((void *) y, w);
  if (trace)
    {
      printf ("x:%d\n", x);
      printf ("y:%d\n", y);
    }
  z = d ? x - y : y - x;
  *fl = 0;
  *fl |=
    ((z & (w ? 0x8000 : 0x80)) ? SF : 0) | ((z & (w ? 0xffff : 0xff)) ==
					    0 ? ZF : 0);
  *fl |=
    ((z & (w ? 0xffff0000 : 0xff00)) ? CF : 0) |
    (((z ^ x) & (z ^ y) & (w ? 0x8000 : 0x80)) ? OF : 0) |
    (((x ^ y ^ z) & 0x10) ? AF : 0);
  if (trace)
    printf (w ? "->%04x " : "->%02x ", z);
  put_ (p, z, w);;
}

void
subbt ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  rm r = mrm (fetchb ());
  x = decreg (r.reg, w);
  y = decrm (r, w);
  if (trace > 1)
    {
      printf ("x:%d\n", x);
      printf ("y:%d\n", y);
    }
  p = d ? (void *) x : (void *) y;
  x = get_ ((void *) x, w);
  y = get_ ((void *) y, w);
  if (trace)
    {
      printf ("x:%d\n", x);
      printf ("y:%d\n", y);
    }
  z = d ? x - y : y - x;
  *fl = 0;
  *fl |=
    ((z & (w ? 0x8000 : 0x80)) ? SF : 0) | ((z & (w ? 0xffff : 0xff)) ==
					    0 ? ZF : 0);
  *fl |=
    ((z & (w ? 0xffff0000 : 0xff00)) ? CF : 0) |
    (((z ^ x) & (z ^ y) & (w ? 0x8000 : 0x80)) ? OF : 0) |
    (((x ^ y ^ z) & 0x10) ? AF : 0);
  if (trace)
    printf (w ? "->%04x " : "->%02x ", z);
  put_ (p, z, w);;
}

void
subwt ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  rm r = mrm (fetchb ());
  x = decreg (r.reg, w);
  y = decrm (r, w);
  if (trace > 1)
    {
      printf ("x:%d\n", x);
      printf ("y:%d\n", y);
    }
  p = d ? (void *) x : (void *) y;
  x = get_ ((void *) x, w);
  y = get_ ((void *) y, w);
  if (trace)
    {
      printf ("x:%d\n", x);
      printf ("y:%d\n", y);
    }
  z = d ? x - y : y - x;
  *fl = 0;
  *fl |=
    ((z & (w ? 0x8000 : 0x80)) ? SF : 0) | ((z & (w ? 0xffff : 0xff)) ==
					    0 ? ZF : 0);
  *fl |=
    ((z & (w ? 0xffff0000 : 0xff00)) ? CF : 0) |
    (((z ^ x) & (z ^ y) & (w ? 0x8000 : 0x80)) ? OF : 0) |
    (((x ^ y ^ z) & 0x10) ? AF : 0);
  if (trace)
    printf (w ? "->%04x " : "->%02x ", z);
  put_ (p, z, w);;
}

void
subbi ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  x = (U) (p = w ? (UC *) ax : al);
  x = get_ ((void *) x, w);
  y = w ? fetchw () : fetchb ();
  z = d ? x - y : y - x;
  *fl = 0;
  *fl |=
    ((z & (w ? 0x8000 : 0x80)) ? SF : 0) | ((z & (w ? 0xffff : 0xff)) ==
					    0 ? ZF : 0);
  *fl |=
    ((z & (w ? 0xffff0000 : 0xff00)) ? CF : 0) |
    (((z ^ x) & (z ^ y) & (w ? 0x8000 : 0x80)) ? OF : 0) |
    (((x ^ y ^ z) & 0x10) ? AF : 0);
  if (trace)
    printf (w ? "->%04x " : "->%02x ", z);
  put_ (p, z, w);;
}

void
subwi ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  x = (U) (p = w ? (UC *) ax : al);
  x = get_ ((void *) x, w);
  y = w ? fetchw () : fetchb ();
  z = d ? x - y : y - x;
  *fl = 0;
  *fl |=
    ((z & (w ? 0x8000 : 0x80)) ? SF : 0) | ((z & (w ? 0xffff : 0xff)) ==
					    0 ? ZF : 0);
  *fl |=
    ((z & (w ? 0xffff0000 : 0xff00)) ? CF : 0) |
    (((z ^ x) & (z ^ y) & (w ? 0x8000 : 0x80)) ? OF : 0) |
    (((x ^ y ^ z) & 0x10) ? AF : 0);
  if (trace)
    printf (w ? "->%04x " : "->%02x ", z);
  put_ (p, z, w);;
}

void
csseg ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;;
}

void
das ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;;
}

void
xorbf ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  rm r = mrm (fetchb ());
  x = decreg (r.reg, w);
  y = decrm (r, w);
  if (trace > 1)
    {
      printf ("x:%d\n", x);
      printf ("y:%d\n", y);
    }
  p = d ? (void *) x : (void *) y;
  x = get_ ((void *) x, w);
  y = get_ ((void *) y, w);
  if (trace)
    {
      printf ("x:%d\n", x);
      printf ("y:%d\n", y);
    }
  z = x ^ y;
  *fl = 0;
  *fl |=
    ((z & (w ? 0x8000 : 0x80)) ? SF : 0) | ((z & (w ? 0xffff : 0xff)) ==
					    0 ? ZF : 0);
  if (trace)
    printf (w ? "->%04x " : "->%02x ", z);
  put_ (p, z, w);;
}

void
xorwf ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  rm r = mrm (fetchb ());
  x = decreg (r.reg, w);
  y = decrm (r, w);
  if (trace > 1)
    {
      printf ("x:%d\n", x);
      printf ("y:%d\n", y);
    }
  p = d ? (void *) x : (void *) y;
  x = get_ ((void *) x, w);
  y = get_ ((void *) y, w);
  if (trace)
    {
      printf ("x:%d\n", x);
      printf ("y:%d\n", y);
    }
  z = x ^ y;
  *fl = 0;
  *fl |=
    ((z & (w ? 0x8000 : 0x80)) ? SF : 0) | ((z & (w ? 0xffff : 0xff)) ==
					    0 ? ZF : 0);
  if (trace)
    printf (w ? "->%04x " : "->%02x ", z);
  put_ (p, z, w);;
}

void
xorbt ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  rm r = mrm (fetchb ());
  x = decreg (r.reg, w);
  y = decrm (r, w);
  if (trace > 1)
    {
      printf ("x:%d\n", x);
      printf ("y:%d\n", y);
    }
  p = d ? (void *) x : (void *) y;
  x = get_ ((void *) x, w);
  y = get_ ((void *) y, w);
  if (trace)
    {
      printf ("x:%d\n", x);
      printf ("y:%d\n", y);
    }
  z = x ^ y;
  *fl = 0;
  *fl |=
    ((z & (w ? 0x8000 : 0x80)) ? SF : 0) | ((z & (w ? 0xffff : 0xff)) ==
					    0 ? ZF : 0);
  if (trace)
    printf (w ? "->%04x " : "->%02x ", z);
  put_ (p, z, w);;
}

void
xorwt ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  rm r = mrm (fetchb ());
  x = decreg (r.reg, w);
  y = decrm (r, w);
  if (trace > 1)
    {
      printf ("x:%d\n", x);
      printf ("y:%d\n", y);
    }
  p = d ? (void *) x : (void *) y;
  x = get_ ((void *) x, w);
  y = get_ ((void *) y, w);
  if (trace)
    {
      printf ("x:%d\n", x);
      printf ("y:%d\n", y);
    }
  z = x ^ y;
  *fl = 0;
  *fl |=
    ((z & (w ? 0x8000 : 0x80)) ? SF : 0) | ((z & (w ? 0xffff : 0xff)) ==
					    0 ? ZF : 0);
  if (trace)
    printf (w ? "->%04x " : "->%02x ", z);
  put_ (p, z, w);;
}

void
xorbi ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  x = (U) (p = w ? (UC *) ax : al);
  x = get_ ((void *) x, w);
  y = w ? fetchw () : fetchb ();
  z = x ^ y;
  *fl = 0;
  *fl |=
    ((z & (w ? 0x8000 : 0x80)) ? SF : 0) | ((z & (w ? 0xffff : 0xff)) ==
					    0 ? ZF : 0);
  if (trace)
    printf (w ? "->%04x " : "->%02x ", z);
  put_ (p, z, w);;
}

void
xorwi ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  x = (U) (p = w ? (UC *) ax : al);
  x = get_ ((void *) x, w);
  y = w ? fetchw () : fetchb ();
  z = x ^ y;
  *fl = 0;
  *fl |=
    ((z & (w ? 0x8000 : 0x80)) ? SF : 0) | ((z & (w ? 0xffff : 0xff)) ==
					    0 ? ZF : 0);
  if (trace)
    printf (w ? "->%04x " : "->%02x ", z);
  put_ (p, z, w);;
}

void
ssseg ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;;
}

void
aaa ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;;
}

void
cmpbf ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  rm r = mrm (fetchb ());
  x = decreg (r.reg, w);
  y = decrm (r, w);
  if (trace > 1)
    {
      printf ("x:%d\n", x);
      printf ("y:%d\n", y);
    }
  p = d ? (void *) x : (void *) y;
  x = get_ ((void *) x, w);
  y = get_ ((void *) y, w);
  if (trace)
    {
      printf ("x:%d\n", x);
      printf ("y:%d\n", y);
    }
  p = null;
  z = d ? x - y : y - x;
  *fl = 0;
  *fl |=
    ((z & (w ? 0x8000 : 0x80)) ? SF : 0) | ((z & (w ? 0xffff : 0xff)) ==
					    0 ? ZF : 0);
  *fl |=
    ((z & (w ? 0xffff0000 : 0xff00)) ? CF : 0) |
    (((z ^ x) & (z ^ y) & (w ? 0x8000 : 0x80)) ? OF : 0) |
    (((x ^ y ^ z) & 0x10) ? AF : 0);
  if (trace)
    printf (w ? "->%04x " : "->%02x ", z);
  put_ (p, z, w);;
}

void
cmpwf ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  rm r = mrm (fetchb ());
  x = decreg (r.reg, w);
  y = decrm (r, w);
  if (trace > 1)
    {
      printf ("x:%d\n", x);
      printf ("y:%d\n", y);
    }
  p = d ? (void *) x : (void *) y;
  x = get_ ((void *) x, w);
  y = get_ ((void *) y, w);
  if (trace)
    {
      printf ("x:%d\n", x);
      printf ("y:%d\n", y);
    }
  p = null;
  z = d ? x - y : y - x;
  *fl = 0;
  *fl |=
    ((z & (w ? 0x8000 : 0x80)) ? SF : 0) | ((z & (w ? 0xffff : 0xff)) ==
					    0 ? ZF : 0);
  *fl |=
    ((z & (w ? 0xffff0000 : 0xff00)) ? CF : 0) |
    (((z ^ x) & (z ^ y) & (w ? 0x8000 : 0x80)) ? OF : 0) |
    (((x ^ y ^ z) & 0x10) ? AF : 0);
  if (trace)
    printf (w ? "->%04x " : "->%02x ", z);
  put_ (p, z, w);;
}

void
cmpbt ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  rm r = mrm (fetchb ());
  x = decreg (r.reg, w);
  y = decrm (r, w);
  if (trace > 1)
    {
      printf ("x:%d\n", x);
      printf ("y:%d\n", y);
    }
  p = d ? (void *) x : (void *) y;
  x = get_ ((void *) x, w);
  y = get_ ((void *) y, w);
  if (trace)
    {
      printf ("x:%d\n", x);
      printf ("y:%d\n", y);
    }
  p = null;
  z = d ? x - y : y - x;
  *fl = 0;
  *fl |=
    ((z & (w ? 0x8000 : 0x80)) ? SF : 0) | ((z & (w ? 0xffff : 0xff)) ==
					    0 ? ZF : 0);
  *fl |=
    ((z & (w ? 0xffff0000 : 0xff00)) ? CF : 0) |
    (((z ^ x) & (z ^ y) & (w ? 0x8000 : 0x80)) ? OF : 0) |
    (((x ^ y ^ z) & 0x10) ? AF : 0);
  if (trace)
    printf (w ? "->%04x " : "->%02x ", z);
  put_ (p, z, w);;
}

void
cmpwt ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  rm r = mrm (fetchb ());
  x = decreg (r.reg, w);
  y = decrm (r, w);
  if (trace > 1)
    {
      printf ("x:%d\n", x);
      printf ("y:%d\n", y);
    }
  p = d ? (void *) x : (void *) y;
  x = get_ ((void *) x, w);
  y = get_ ((void *) y, w);
  if (trace)
    {
      printf ("x:%d\n", x);
      printf ("y:%d\n", y);
    }
  p = null;
  z = d ? x - y : y - x;
  *fl = 0;
  *fl |=
    ((z & (w ? 0x8000 : 0x80)) ? SF : 0) | ((z & (w ? 0xffff : 0xff)) ==
					    0 ? ZF : 0);
  *fl |=
    ((z & (w ? 0xffff0000 : 0xff00)) ? CF : 0) |
    (((z ^ x) & (z ^ y) & (w ? 0x8000 : 0x80)) ? OF : 0) |
    (((x ^ y ^ z) & 0x10) ? AF : 0);
  if (trace)
    printf (w ? "->%04x " : "->%02x ", z);
  put_ (p, z, w);;
}

void
cmpbi ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  x = (U) (p = w ? (UC *) ax : al);
  x = get_ ((void *) x, w);
  y = w ? fetchw () : fetchb ();
  p = null;
  z = d ? x - y : y - x;
  *fl = 0;
  *fl |=
    ((z & (w ? 0x8000 : 0x80)) ? SF : 0) | ((z & (w ? 0xffff : 0xff)) ==
					    0 ? ZF : 0);
  *fl |=
    ((z & (w ? 0xffff0000 : 0xff00)) ? CF : 0) |
    (((z ^ x) & (z ^ y) & (w ? 0x8000 : 0x80)) ? OF : 0) |
    (((x ^ y ^ z) & 0x10) ? AF : 0);
  if (trace)
    printf (w ? "->%04x " : "->%02x ", z);
  put_ (p, z, w);;
}

void
cmpwi ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  x = (U) (p = w ? (UC *) ax : al);
  x = get_ ((void *) x, w);
  y = w ? fetchw () : fetchb ();
  p = null;
  z = d ? x - y : y - x;
  *fl = 0;
  *fl |=
    ((z & (w ? 0x8000 : 0x80)) ? SF : 0) | ((z & (w ? 0xffff : 0xff)) ==
					    0 ? ZF : 0);
  *fl |=
    ((z & (w ? 0xffff0000 : 0xff00)) ? CF : 0) |
    (((z ^ x) & (z ^ y) & (w ? 0x8000 : 0x80)) ? OF : 0) |
    (((x ^ y ^ z) & 0x10) ? AF : 0);
  if (trace)
    printf (w ? "->%04x " : "->%02x ", z);
  put_ (p, z, w);;
}

void
dsseg ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;;
}

void
aas ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;;
}

void
incax ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  w = 1;
  d = 1;
  p = (V *) ax;
  x = (S) * ax;
  y = 1;
  f = *fl & CF;
  z = x + y;
  *fl = 0;
  *fl |=
    ((z & (w ? 0x8000 : 0x80)) ? SF : 0) | ((z & (w ? 0xffff : 0xff)) ==
					    0 ? ZF : 0);
  *fl |=
    ((z & (w ? 0xffff0000 : 0xff00)) ? CF : 0) |
    (((z ^ x) & (z ^ y) & (w ? 0x8000 : 0x80)) ? OF : 0) |
    (((x ^ y ^ z) & 0x10) ? AF : 0);
  if (trace)
    printf (w ? "->%04x " : "->%02x ", z);
  put_ (p, z, w);
  *fl = (*fl & ~CF) | f;;
}

void
inccx ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  w = 1;
  d = 1;
  p = (V *) cx;
  x = (S) * cx;
  y = 1;
  f = *fl & CF;
  z = x + y;
  *fl = 0;
  *fl |=
    ((z & (w ? 0x8000 : 0x80)) ? SF : 0) | ((z & (w ? 0xffff : 0xff)) ==
					    0 ? ZF : 0);
  *fl |=
    ((z & (w ? 0xffff0000 : 0xff00)) ? CF : 0) |
    (((z ^ x) & (z ^ y) & (w ? 0x8000 : 0x80)) ? OF : 0) |
    (((x ^ y ^ z) & 0x10) ? AF : 0);
  if (trace)
    printf (w ? "->%04x " : "->%02x ", z);
  put_ (p, z, w);
  *fl = (*fl & ~CF) | f;;
}

void
incdx ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  w = 1;
  d = 1;
  p = (V *) dx;
  x = (S) * dx;
  y = 1;
  f = *fl & CF;
  z = x + y;
  *fl = 0;
  *fl |=
    ((z & (w ? 0x8000 : 0x80)) ? SF : 0) | ((z & (w ? 0xffff : 0xff)) ==
					    0 ? ZF : 0);
  *fl |=
    ((z & (w ? 0xffff0000 : 0xff00)) ? CF : 0) |
    (((z ^ x) & (z ^ y) & (w ? 0x8000 : 0x80)) ? OF : 0) |
    (((x ^ y ^ z) & 0x10) ? AF : 0);
  if (trace)
    printf (w ? "->%04x " : "->%02x ", z);
  put_ (p, z, w);
  *fl = (*fl & ~CF) | f;;
}

void
incbx ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  w = 1;
  d = 1;
  p = (V *) bx;
  x = (S) * bx;
  y = 1;
  f = *fl & CF;
  z = x + y;
  *fl = 0;
  *fl |=
    ((z & (w ? 0x8000 : 0x80)) ? SF : 0) | ((z & (w ? 0xffff : 0xff)) ==
					    0 ? ZF : 0);
  *fl |=
    ((z & (w ? 0xffff0000 : 0xff00)) ? CF : 0) |
    (((z ^ x) & (z ^ y) & (w ? 0x8000 : 0x80)) ? OF : 0) |
    (((x ^ y ^ z) & 0x10) ? AF : 0);
  if (trace)
    printf (w ? "->%04x " : "->%02x ", z);
  put_ (p, z, w);
  *fl = (*fl & ~CF) | f;;
}

void
incsp ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  w = 1;
  d = 1;
  p = (V *) sp;
  x = (S) * sp;
  y = 1;
  f = *fl & CF;
  z = x + y;
  *fl = 0;
  *fl |=
    ((z & (w ? 0x8000 : 0x80)) ? SF : 0) | ((z & (w ? 0xffff : 0xff)) ==
					    0 ? ZF : 0);
  *fl |=
    ((z & (w ? 0xffff0000 : 0xff00)) ? CF : 0) |
    (((z ^ x) & (z ^ y) & (w ? 0x8000 : 0x80)) ? OF : 0) |
    (((x ^ y ^ z) & 0x10) ? AF : 0);
  if (trace)
    printf (w ? "->%04x " : "->%02x ", z);
  put_ (p, z, w);
  *fl = (*fl & ~CF) | f;;
}

void
incbp ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  w = 1;
  d = 1;
  p = (V *) bp;
  x = (S) * bp;
  y = 1;
  f = *fl & CF;
  z = x + y;
  *fl = 0;
  *fl |=
    ((z & (w ? 0x8000 : 0x80)) ? SF : 0) | ((z & (w ? 0xffff : 0xff)) ==
					    0 ? ZF : 0);
  *fl |=
    ((z & (w ? 0xffff0000 : 0xff00)) ? CF : 0) |
    (((z ^ x) & (z ^ y) & (w ? 0x8000 : 0x80)) ? OF : 0) |
    (((x ^ y ^ z) & 0x10) ? AF : 0);
  if (trace)
    printf (w ? "->%04x " : "->%02x ", z);
  put_ (p, z, w);
  *fl = (*fl & ~CF) | f;;
}

void
incsi ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  w = 1;
  d = 1;
  p = (V *) si;
  x = (S) * si;
  y = 1;
  f = *fl & CF;
  z = x + y;
  *fl = 0;
  *fl |=
    ((z & (w ? 0x8000 : 0x80)) ? SF : 0) | ((z & (w ? 0xffff : 0xff)) ==
					    0 ? ZF : 0);
  *fl |=
    ((z & (w ? 0xffff0000 : 0xff00)) ? CF : 0) |
    (((z ^ x) & (z ^ y) & (w ? 0x8000 : 0x80)) ? OF : 0) |
    (((x ^ y ^ z) & 0x10) ? AF : 0);
  if (trace)
    printf (w ? "->%04x " : "->%02x ", z);
  put_ (p, z, w);
  *fl = (*fl & ~CF) | f;;
}

void
incdi ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  w = 1;
  d = 1;
  p = (V *) di;
  x = (S) * di;
  y = 1;
  f = *fl & CF;
  z = x + y;
  *fl = 0;
  *fl |=
    ((z & (w ? 0x8000 : 0x80)) ? SF : 0) | ((z & (w ? 0xffff : 0xff)) ==
					    0 ? ZF : 0);
  *fl |=
    ((z & (w ? 0xffff0000 : 0xff00)) ? CF : 0) |
    (((z ^ x) & (z ^ y) & (w ? 0x8000 : 0x80)) ? OF : 0) |
    (((x ^ y ^ z) & 0x10) ? AF : 0);
  if (trace)
    printf (w ? "->%04x " : "->%02x ", z);
  put_ (p, z, w);
  *fl = (*fl & ~CF) | f;;
}

void
decax ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  w = 1;
  d = 1;
  p = (V *) ax;
  x = (S) * ax;
  y = 1;
  f = *fl & CF;
  z = d ? x - y : y - x;
  *fl = 0;
  *fl |=
    ((z & (w ? 0x8000 : 0x80)) ? SF : 0) | ((z & (w ? 0xffff : 0xff)) ==
					    0 ? ZF : 0);
  *fl |=
    ((z & (w ? 0xffff0000 : 0xff00)) ? CF : 0) |
    (((z ^ x) & (z ^ y) & (w ? 0x8000 : 0x80)) ? OF : 0) |
    (((x ^ y ^ z) & 0x10) ? AF : 0);
  if (trace)
    printf (w ? "->%04x " : "->%02x ", z);
  put_ (p, z, w);
  *fl = (*fl & ~CF) | f;;
}

void
deccx ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  w = 1;
  d = 1;
  p = (V *) cx;
  x = (S) * cx;
  y = 1;
  f = *fl & CF;
  z = d ? x - y : y - x;
  *fl = 0;
  *fl |=
    ((z & (w ? 0x8000 : 0x80)) ? SF : 0) | ((z & (w ? 0xffff : 0xff)) ==
					    0 ? ZF : 0);
  *fl |=
    ((z & (w ? 0xffff0000 : 0xff00)) ? CF : 0) |
    (((z ^ x) & (z ^ y) & (w ? 0x8000 : 0x80)) ? OF : 0) |
    (((x ^ y ^ z) & 0x10) ? AF : 0);
  if (trace)
    printf (w ? "->%04x " : "->%02x ", z);
  put_ (p, z, w);
  *fl = (*fl & ~CF) | f;;
}

void
decdx ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  w = 1;
  d = 1;
  p = (V *) dx;
  x = (S) * dx;
  y = 1;
  f = *fl & CF;
  z = d ? x - y : y - x;
  *fl = 0;
  *fl |=
    ((z & (w ? 0x8000 : 0x80)) ? SF : 0) | ((z & (w ? 0xffff : 0xff)) ==
					    0 ? ZF : 0);
  *fl |=
    ((z & (w ? 0xffff0000 : 0xff00)) ? CF : 0) |
    (((z ^ x) & (z ^ y) & (w ? 0x8000 : 0x80)) ? OF : 0) |
    (((x ^ y ^ z) & 0x10) ? AF : 0);
  if (trace)
    printf (w ? "->%04x " : "->%02x ", z);
  put_ (p, z, w);
  *fl = (*fl & ~CF) | f;;
}

void
decbx ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  w = 1;
  d = 1;
  p = (V *) bx;
  x = (S) * bx;
  y = 1;
  f = *fl & CF;
  z = d ? x - y : y - x;
  *fl = 0;
  *fl |=
    ((z & (w ? 0x8000 : 0x80)) ? SF : 0) | ((z & (w ? 0xffff : 0xff)) ==
					    0 ? ZF : 0);
  *fl |=
    ((z & (w ? 0xffff0000 : 0xff00)) ? CF : 0) |
    (((z ^ x) & (z ^ y) & (w ? 0x8000 : 0x80)) ? OF : 0) |
    (((x ^ y ^ z) & 0x10) ? AF : 0);
  if (trace)
    printf (w ? "->%04x " : "->%02x ", z);
  put_ (p, z, w);
  *fl = (*fl & ~CF) | f;;
}

void
decsp ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  w = 1;
  d = 1;
  p = (V *) sp;
  x = (S) * sp;
  y = 1;
  f = *fl & CF;
  z = d ? x - y : y - x;
  *fl = 0;
  *fl |=
    ((z & (w ? 0x8000 : 0x80)) ? SF : 0) | ((z & (w ? 0xffff : 0xff)) ==
					    0 ? ZF : 0);
  *fl |=
    ((z & (w ? 0xffff0000 : 0xff00)) ? CF : 0) |
    (((z ^ x) & (z ^ y) & (w ? 0x8000 : 0x80)) ? OF : 0) |
    (((x ^ y ^ z) & 0x10) ? AF : 0);
  if (trace)
    printf (w ? "->%04x " : "->%02x ", z);
  put_ (p, z, w);
  *fl = (*fl & ~CF) | f;;
}

void
decbp ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  w = 1;
  d = 1;
  p = (V *) bp;
  x = (S) * bp;
  y = 1;
  f = *fl & CF;
  z = d ? x - y : y - x;
  *fl = 0;
  *fl |=
    ((z & (w ? 0x8000 : 0x80)) ? SF : 0) | ((z & (w ? 0xffff : 0xff)) ==
					    0 ? ZF : 0);
  *fl |=
    ((z & (w ? 0xffff0000 : 0xff00)) ? CF : 0) |
    (((z ^ x) & (z ^ y) & (w ? 0x8000 : 0x80)) ? OF : 0) |
    (((x ^ y ^ z) & 0x10) ? AF : 0);
  if (trace)
    printf (w ? "->%04x " : "->%02x ", z);
  put_ (p, z, w);
  *fl = (*fl & ~CF) | f;;
}

void
decsi ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  w = 1;
  d = 1;
  p = (V *) si;
  x = (S) * si;
  y = 1;
  f = *fl & CF;
  z = d ? x - y : y - x;
  *fl = 0;
  *fl |=
    ((z & (w ? 0x8000 : 0x80)) ? SF : 0) | ((z & (w ? 0xffff : 0xff)) ==
					    0 ? ZF : 0);
  *fl |=
    ((z & (w ? 0xffff0000 : 0xff00)) ? CF : 0) |
    (((z ^ x) & (z ^ y) & (w ? 0x8000 : 0x80)) ? OF : 0) |
    (((x ^ y ^ z) & 0x10) ? AF : 0);
  if (trace)
    printf (w ? "->%04x " : "->%02x ", z);
  put_ (p, z, w);
  *fl = (*fl & ~CF) | f;;
}

void
decdi ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  w = 1;
  d = 1;
  p = (V *) di;
  x = (S) * di;
  y = 1;
  f = *fl & CF;
  z = d ? x - y : y - x;
  *fl = 0;
  *fl |=
    ((z & (w ? 0x8000 : 0x80)) ? SF : 0) | ((z & (w ? 0xffff : 0xff)) ==
					    0 ? ZF : 0);
  *fl |=
    ((z & (w ? 0xffff0000 : 0xff00)) ? CF : 0) |
    (((z ^ x) & (z ^ y) & (w ? 0x8000 : 0x80)) ? OF : 0) |
    (((x ^ y ^ z) & 0x10) ? AF : 0);
  if (trace)
    printf (w ? "->%04x " : "->%02x ", z);
  put_ (p, z, w);
  *fl = (*fl & ~CF) | f;;
}

void
pushax ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  put_ (mem + (*sp -= 2), *(ax), 1);
}

void
pushcx ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  put_ (mem + (*sp -= 2), *(cx), 1);
}

void
pushdx ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  put_ (mem + (*sp -= 2), *(dx), 1);
}

void
pushbx ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  put_ (mem + (*sp -= 2), *(bx), 1);
}

void
pushsp ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  put_ (mem + (*sp -= 2), *(sp), 1);
}

void
pushbp ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  put_ (mem + (*sp -= 2), *(bp), 1);
}

void
pushsi ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  put_ (mem + (*sp -= 2), *(si), 1);
}

void
pushdi ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  put_ (mem + (*sp -= 2), *(di), 1);
}

void
popax ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  *(ax) = get_ (mem + (*sp += 2) - 2, 1);
}

void
popcx ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  *(cx) = get_ (mem + (*sp += 2) - 2, 1);
}

void
popdx ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  *(dx) = get_ (mem + (*sp += 2) - 2, 1);
}

void
popbx ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  *(bx) = get_ (mem + (*sp += 2) - 2, 1);
}

void
popsp ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  *(sp) = get_ (mem + (*sp += 2) - 2, 1);
}

void
popbp ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  *(bp) = get_ (mem + (*sp += 2) - 2, 1);
}

void
popsi ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  *(si) = get_ (mem + (*sp += 2) - 2, 1);
}

void
popdi ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  *(di) = get_ (mem + (*sp += 2) - 2, 1);
}

void
nop1 ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;;
}

void
nop2 ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;;
}

void
nop3 ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;;
}

void
nop4 ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;;
}

void
nop5 ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;;
}

void
nop6 ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;;
}

void
nop7 ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;;
}

void
nop8 ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;;
}

void
nop9 ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;;
}

void
nopA ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;;
}

void
nopB ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;;
}

void
nopC ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;;
}

void
nopD ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;;
}

void
nopE ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;;
}

void
nopF ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;;
}

void
nopG ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;;
}

void
jo ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  U cf = ! !(*fl & CF), of = ! !(*fl & OF), sf = ! !(*fl & SF), zf =
    ! !(*fl & ZF);
  y = (S) (C) fetchb ();
  if (trace)
    printf ("<%d> ", of);
  if (of)
    *ip += (S) y;;
}

void
jno ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  U cf = ! !(*fl & CF), of = ! !(*fl & OF), sf = ! !(*fl & SF), zf =
    ! !(*fl & ZF);
  y = (S) (C) fetchb ();
  if (trace)
    printf ("<%d> ", !(of));
  if (!(of))
    *ip += (S) y;;
}

void
jb ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  U cf = ! !(*fl & CF), of = ! !(*fl & OF), sf = ! !(*fl & SF), zf =
    ! !(*fl & ZF);
  y = (S) (C) fetchb ();
  if (trace)
    printf ("<%d> ", cf);
  if (cf)
    *ip += (S) y;;
}

void
jnb ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  U cf = ! !(*fl & CF), of = ! !(*fl & OF), sf = ! !(*fl & SF), zf =
    ! !(*fl & ZF);
  y = (S) (C) fetchb ();
  if (trace)
    printf ("<%d> ", !(cf));
  if (!(cf))
    *ip += (S) y;;
}

void
jz ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  U cf = ! !(*fl & CF), of = ! !(*fl & OF), sf = ! !(*fl & SF), zf =
    ! !(*fl & ZF);
  y = (S) (C) fetchb ();
  if (trace)
    printf ("<%d> ", zf);
  if (zf)
    *ip += (S) y;;
}

void
jnz ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  U cf = ! !(*fl & CF), of = ! !(*fl & OF), sf = ! !(*fl & SF), zf =
    ! !(*fl & ZF);
  y = (S) (C) fetchb ();
  if (trace)
    printf ("<%d> ", !(zf));
  if (!(zf))
    *ip += (S) y;;
}

void
jbe ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  U cf = ! !(*fl & CF), of = ! !(*fl & OF), sf = ! !(*fl & SF), zf =
    ! !(*fl & ZF);
  y = (S) (C) fetchb ();
  if (trace)
    printf ("<%d> ", cf | zf);
  if (cf | zf)
    *ip += (S) y;;
}

void
jnbe ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  U cf = ! !(*fl & CF), of = ! !(*fl & OF), sf = ! !(*fl & SF), zf =
    ! !(*fl & ZF);
  y = (S) (C) fetchb ();
  if (trace)
    printf ("<%d> ", !(cf | zf));
  if (!(cf | zf))
    *ip += (S) y;;
}

void
js ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  U cf = ! !(*fl & CF), of = ! !(*fl & OF), sf = ! !(*fl & SF), zf =
    ! !(*fl & ZF);
  y = (S) (C) fetchb ();
  if (trace)
    printf ("<%d> ", sf);
  if (sf)
    *ip += (S) y;;
}

void
jns ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  U cf = ! !(*fl & CF), of = ! !(*fl & OF), sf = ! !(*fl & SF), zf =
    ! !(*fl & ZF);
  y = (S) (C) fetchb ();
  if (trace)
    printf ("<%d> ", !(sf));
  if (!(sf))
    *ip += (S) y;;
}

void
jp ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;;
}

void
jnp ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;;
}

void
jl ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  U cf = ! !(*fl & CF), of = ! !(*fl & OF), sf = ! !(*fl & SF), zf =
    ! !(*fl & ZF);
  y = (S) (C) fetchb ();
  if (trace)
    printf ("<%d> ", sf ^ of);
  if (sf ^ of)
    *ip += (S) y;;
}

void
jnl_ ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  U cf = ! !(*fl & CF), of = ! !(*fl & OF), sf = ! !(*fl & SF), zf =
    ! !(*fl & ZF);
  y = (S) (C) fetchb ();
  if (trace)
    printf ("<%d> ", !(sf ^ of));
  if (!(sf ^ of))
    *ip += (S) y;;
}

void
jle ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  U cf = ! !(*fl & CF), of = ! !(*fl & OF), sf = ! !(*fl & SF), zf =
    ! !(*fl & ZF);
  y = (S) (C) fetchb ();
  if (trace)
    printf ("<%d> ", (sf ^ of) | zf);
  if ((sf ^ of) | zf)
    *ip += (S) y;;
}

void
jnle ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  U cf = ! !(*fl & CF), of = ! !(*fl & OF), sf = ! !(*fl & SF), zf =
    ! !(*fl & ZF);
  y = (S) (C) fetchb ();
  if (trace)
    printf ("<%d> ", !((sf ^ of) | zf));
  if (!((sf ^ of) | zf))
    *ip += (S) y;;
}

void
immb ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  rm r = mrm (fetchb ());
  p = (void *) (y = decrm (r, w));
  x = w ? fetchw () : fetchb ();
  d = 0;
  y = get_ ((void *) y, w);
  if (trace)
    {
      printf ("x:%d\n", x);
      printf ("y:%d\n", y);
    }
  if (trace)
    {
      printf ("%s ", (C *[])
	      {
	      "ADD", "OR", "ADC", "SBB", "AND", "SUB", "XOR", "CMP"}
	      [r.reg]);
    }
  switch (r.reg)
    {
    case 0:
      z = x + y;
      *fl = 0;
      *fl |=
	((z & (w ? 0x8000 : 0x80)) ? SF : 0) | ((z & (w ? 0xffff : 0xff)) ==
						0 ? ZF : 0);
      *fl |=
	((z & (w ? 0xffff0000 : 0xff00)) ? CF : 0) |
	(((z ^ x) & (z ^ y) & (w ? 0x8000 : 0x80)) ? OF : 0) |
	(((x ^ y ^ z) & 0x10) ? AF : 0);
      if (trace)
	printf (w ? "->%04x " : "->%02x ", z);
      put_ (p, z, w);
      break;
    case 1:
      z = x | y;
      *fl = 0;
      *fl |=
	((z & (w ? 0x8000 : 0x80)) ? SF : 0) | ((z & (w ? 0xffff : 0xff)) ==
						0 ? ZF : 0);
      if (trace)
	printf (w ? "->%04x " : "->%02x ", z);
      put_ (p, z, w);
      break;
    case 2:
      x += (*fl & CF);
      z = x + y;
      *fl = 0;
      *fl |=
	((z & (w ? 0x8000 : 0x80)) ? SF : 0) | ((z & (w ? 0xffff : 0xff)) ==
						0 ? ZF : 0);
      *fl |=
	((z & (w ? 0xffff0000 : 0xff00)) ? CF : 0) |
	(((z ^ x) & (z ^ y) & (w ? 0x8000 : 0x80)) ? OF : 0) |
	(((x ^ y ^ z) & 0x10) ? AF : 0);
      if (trace)
	printf (w ? "->%04x " : "->%02x ", z);
      put_ (p, z, w);
      break;
    case 3:
      d ? y += *fl & CF : (x += *fl & CF);
      z = d ? x - y : y - x;
      *fl = 0;
      *fl |=
	((z & (w ? 0x8000 : 0x80)) ? SF : 0) | ((z & (w ? 0xffff : 0xff)) ==
						0 ? ZF : 0);
      *fl |=
	((z & (w ? 0xffff0000 : 0xff00)) ? CF : 0) |
	(((z ^ x) & (z ^ y) & (w ? 0x8000 : 0x80)) ? OF : 0) |
	(((x ^ y ^ z) & 0x10) ? AF : 0);
      if (trace)
	printf (w ? "->%04x " : "->%02x ", z);
      put_ (p, z, w);
      break;
    case 4:
      z = x & y;
      *fl = 0;
      *fl |=
	((z & (w ? 0x8000 : 0x80)) ? SF : 0) | ((z & (w ? 0xffff : 0xff)) ==
						0 ? ZF : 0);
      if (trace)
	printf (w ? "->%04x " : "->%02x ", z);
      put_ (p, z, w);
      break;
    case 5:
      z = d ? x - y : y - x;
      *fl = 0;
      *fl |=
	((z & (w ? 0x8000 : 0x80)) ? SF : 0) | ((z & (w ? 0xffff : 0xff)) ==
						0 ? ZF : 0);
      *fl |=
	((z & (w ? 0xffff0000 : 0xff00)) ? CF : 0) |
	(((z ^ x) & (z ^ y) & (w ? 0x8000 : 0x80)) ? OF : 0) |
	(((x ^ y ^ z) & 0x10) ? AF : 0);
      if (trace)
	printf (w ? "->%04x " : "->%02x ", z);
      put_ (p, z, w);
      break;
    case 6:
      z = x ^ y;
      *fl = 0;
      *fl |=
	((z & (w ? 0x8000 : 0x80)) ? SF : 0) | ((z & (w ? 0xffff : 0xff)) ==
						0 ? ZF : 0);
      if (trace)
	printf (w ? "->%04x " : "->%02x ", z);
      put_ (p, z, w);
      break;
    case 7:
      p = null;
      z = d ? x - y : y - x;
      *fl = 0;
      *fl |=
	((z & (w ? 0x8000 : 0x80)) ? SF : 0) | ((z & (w ? 0xffff : 0xff)) ==
						0 ? ZF : 0);
      *fl |=
	((z & (w ? 0xffff0000 : 0xff00)) ? CF : 0) |
	(((z ^ x) & (z ^ y) & (w ? 0x8000 : 0x80)) ? OF : 0) |
	(((x ^ y ^ z) & 0x10) ? AF : 0);
      if (trace)
	printf (w ? "->%04x " : "->%02x ", z);
      put_ (p, z, w);
      break;
    };
}

void
immw ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  rm r = mrm (fetchb ());
  p = (void *) (y = decrm (r, w));
  x = w ? fetchw () : fetchb ();
  d = 0;
  y = get_ ((void *) y, w);
  if (trace)
    {
      printf ("x:%d\n", x);
      printf ("y:%d\n", y);
    }
  if (trace)
    {
      printf ("%s ", (C *[])
	      {
	      "ADD", "OR", "ADC", "SBB", "AND", "SUB", "XOR", "CMP"}
	      [r.reg]);
    }
  switch (r.reg)
    {
    case 0:
      z = x + y;
      *fl = 0;
      *fl |=
	((z & (w ? 0x8000 : 0x80)) ? SF : 0) | ((z & (w ? 0xffff : 0xff)) ==
						0 ? ZF : 0);
      *fl |=
	((z & (w ? 0xffff0000 : 0xff00)) ? CF : 0) |
	(((z ^ x) & (z ^ y) & (w ? 0x8000 : 0x80)) ? OF : 0) |
	(((x ^ y ^ z) & 0x10) ? AF : 0);
      if (trace)
	printf (w ? "->%04x " : "->%02x ", z);
      put_ (p, z, w);
      break;
    case 1:
      z = x | y;
      *fl = 0;
      *fl |=
	((z & (w ? 0x8000 : 0x80)) ? SF : 0) | ((z & (w ? 0xffff : 0xff)) ==
						0 ? ZF : 0);
      if (trace)
	printf (w ? "->%04x " : "->%02x ", z);
      put_ (p, z, w);
      break;
    case 2:
      x += (*fl & CF);
      z = x + y;
      *fl = 0;
      *fl |=
	((z & (w ? 0x8000 : 0x80)) ? SF : 0) | ((z & (w ? 0xffff : 0xff)) ==
						0 ? ZF : 0);
      *fl |=
	((z & (w ? 0xffff0000 : 0xff00)) ? CF : 0) |
	(((z ^ x) & (z ^ y) & (w ? 0x8000 : 0x80)) ? OF : 0) |
	(((x ^ y ^ z) & 0x10) ? AF : 0);
      if (trace)
	printf (w ? "->%04x " : "->%02x ", z);
      put_ (p, z, w);
      break;
    case 3:
      d ? y += *fl & CF : (x += *fl & CF);
      z = d ? x - y : y - x;
      *fl = 0;
      *fl |=
	((z & (w ? 0x8000 : 0x80)) ? SF : 0) | ((z & (w ? 0xffff : 0xff)) ==
						0 ? ZF : 0);
      *fl |=
	((z & (w ? 0xffff0000 : 0xff00)) ? CF : 0) |
	(((z ^ x) & (z ^ y) & (w ? 0x8000 : 0x80)) ? OF : 0) |
	(((x ^ y ^ z) & 0x10) ? AF : 0);
      if (trace)
	printf (w ? "->%04x " : "->%02x ", z);
      put_ (p, z, w);
      break;
    case 4:
      z = x & y;
      *fl = 0;
      *fl |=
	((z & (w ? 0x8000 : 0x80)) ? SF : 0) | ((z & (w ? 0xffff : 0xff)) ==
						0 ? ZF : 0);
      if (trace)
	printf (w ? "->%04x " : "->%02x ", z);
      put_ (p, z, w);
      break;
    case 5:
      z = d ? x - y : y - x;
      *fl = 0;
      *fl |=
	((z & (w ? 0x8000 : 0x80)) ? SF : 0) | ((z & (w ? 0xffff : 0xff)) ==
						0 ? ZF : 0);
      *fl |=
	((z & (w ? 0xffff0000 : 0xff00)) ? CF : 0) |
	(((z ^ x) & (z ^ y) & (w ? 0x8000 : 0x80)) ? OF : 0) |
	(((x ^ y ^ z) & 0x10) ? AF : 0);
      if (trace)
	printf (w ? "->%04x " : "->%02x ", z);
      put_ (p, z, w);
      break;
    case 6:
      z = x ^ y;
      *fl = 0;
      *fl |=
	((z & (w ? 0x8000 : 0x80)) ? SF : 0) | ((z & (w ? 0xffff : 0xff)) ==
						0 ? ZF : 0);
      if (trace)
	printf (w ? "->%04x " : "->%02x ", z);
      put_ (p, z, w);
      break;
    case 7:
      p = null;
      z = d ? x - y : y - x;
      *fl = 0;
      *fl |=
	((z & (w ? 0x8000 : 0x80)) ? SF : 0) | ((z & (w ? 0xffff : 0xff)) ==
						0 ? ZF : 0);
      *fl |=
	((z & (w ? 0xffff0000 : 0xff00)) ? CF : 0) |
	(((z ^ x) & (z ^ y) & (w ? 0x8000 : 0x80)) ? OF : 0) |
	(((x ^ y ^ z) & 0x10) ? AF : 0);
      if (trace)
	printf (w ? "->%04x " : "->%02x ", z);
      put_ (p, z, w);
      break;
    };
}

void
immb1 ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  rm r = mrm (fetchb ());
  p = (void *) (y = decrm (r, w));
  x = w ? fetchw () : fetchb ();
  d = 0;
  y = get_ ((void *) y, w);
  if (trace)
    {
      printf ("x:%d\n", x);
      printf ("y:%d\n", y);
    }
  if (trace)
    {
      printf ("%s ", (C *[])
	      {
	      "ADD", "OR", "ADC", "SBB", "AND", "SUB", "XOR", "CMP"}
	      [r.reg]);
    }
  switch (r.reg)
    {
    case 0:
      z = x + y;
      *fl = 0;
      *fl |=
	((z & (w ? 0x8000 : 0x80)) ? SF : 0) | ((z & (w ? 0xffff : 0xff)) ==
						0 ? ZF : 0);
      *fl |=
	((z & (w ? 0xffff0000 : 0xff00)) ? CF : 0) |
	(((z ^ x) & (z ^ y) & (w ? 0x8000 : 0x80)) ? OF : 0) |
	(((x ^ y ^ z) & 0x10) ? AF : 0);
      if (trace)
	printf (w ? "->%04x " : "->%02x ", z);
      put_ (p, z, w);
      break;
    case 1:
      z = x | y;
      *fl = 0;
      *fl |=
	((z & (w ? 0x8000 : 0x80)) ? SF : 0) | ((z & (w ? 0xffff : 0xff)) ==
						0 ? ZF : 0);
      if (trace)
	printf (w ? "->%04x " : "->%02x ", z);
      put_ (p, z, w);
      break;
    case 2:
      x += (*fl & CF);
      z = x + y;
      *fl = 0;
      *fl |=
	((z & (w ? 0x8000 : 0x80)) ? SF : 0) | ((z & (w ? 0xffff : 0xff)) ==
						0 ? ZF : 0);
      *fl |=
	((z & (w ? 0xffff0000 : 0xff00)) ? CF : 0) |
	(((z ^ x) & (z ^ y) & (w ? 0x8000 : 0x80)) ? OF : 0) |
	(((x ^ y ^ z) & 0x10) ? AF : 0);
      if (trace)
	printf (w ? "->%04x " : "->%02x ", z);
      put_ (p, z, w);
      break;
    case 3:
      d ? y += *fl & CF : (x += *fl & CF);
      z = d ? x - y : y - x;
      *fl = 0;
      *fl |=
	((z & (w ? 0x8000 : 0x80)) ? SF : 0) | ((z & (w ? 0xffff : 0xff)) ==
						0 ? ZF : 0);
      *fl |=
	((z & (w ? 0xffff0000 : 0xff00)) ? CF : 0) |
	(((z ^ x) & (z ^ y) & (w ? 0x8000 : 0x80)) ? OF : 0) |
	(((x ^ y ^ z) & 0x10) ? AF : 0);
      if (trace)
	printf (w ? "->%04x " : "->%02x ", z);
      put_ (p, z, w);
      break;
    case 4:
      z = x & y;
      *fl = 0;
      *fl |=
	((z & (w ? 0x8000 : 0x80)) ? SF : 0) | ((z & (w ? 0xffff : 0xff)) ==
						0 ? ZF : 0);
      if (trace)
	printf (w ? "->%04x " : "->%02x ", z);
      put_ (p, z, w);
      break;
    case 5:
      z = d ? x - y : y - x;
      *fl = 0;
      *fl |=
	((z & (w ? 0x8000 : 0x80)) ? SF : 0) | ((z & (w ? 0xffff : 0xff)) ==
						0 ? ZF : 0);
      *fl |=
	((z & (w ? 0xffff0000 : 0xff00)) ? CF : 0) |
	(((z ^ x) & (z ^ y) & (w ? 0x8000 : 0x80)) ? OF : 0) |
	(((x ^ y ^ z) & 0x10) ? AF : 0);
      if (trace)
	printf (w ? "->%04x " : "->%02x ", z);
      put_ (p, z, w);
      break;
    case 6:
      z = x ^ y;
      *fl = 0;
      *fl |=
	((z & (w ? 0x8000 : 0x80)) ? SF : 0) | ((z & (w ? 0xffff : 0xff)) ==
						0 ? ZF : 0);
      if (trace)
	printf (w ? "->%04x " : "->%02x ", z);
      put_ (p, z, w);
      break;
    case 7:
      p = null;
      z = d ? x - y : y - x;
      *fl = 0;
      *fl |=
	((z & (w ? 0x8000 : 0x80)) ? SF : 0) | ((z & (w ? 0xffff : 0xff)) ==
						0 ? ZF : 0);
      *fl |=
	((z & (w ? 0xffff0000 : 0xff00)) ? CF : 0) |
	(((z ^ x) & (z ^ y) & (w ? 0x8000 : 0x80)) ? OF : 0) |
	(((x ^ y ^ z) & 0x10) ? AF : 0);
      if (trace)
	printf (w ? "->%04x " : "->%02x ", z);
      put_ (p, z, w);
      break;
    };
}

void
immis ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  rm r = mrm (fetchb ());
  p = (void *) (y = decrm (r, w));
  w = 0;
  x = w ? fetchw () : fetchb ();
  w = 1;
  x = (S) (C) x;
  d = 0;
  y = get_ ((void *) y, w);
  if (trace)
    {
      printf ("x:%d\n", x);
      printf ("y:%d\n", y);
    }
  if (trace)
    {
      printf ("%s ", (C *[])
	      {
	      "ADD", "OR", "ADC", "SBB", "AND", "SUB", "XOR", "CMP"}
	      [r.reg]);
    }
  switch (r.reg)
    {
    case 0:
      z = x + y;
      *fl = 0;
      *fl |=
	((z & (w ? 0x8000 : 0x80)) ? SF : 0) | ((z & (w ? 0xffff : 0xff)) ==
						0 ? ZF : 0);
      *fl |=
	((z & (w ? 0xffff0000 : 0xff00)) ? CF : 0) |
	(((z ^ x) & (z ^ y) & (w ? 0x8000 : 0x80)) ? OF : 0) |
	(((x ^ y ^ z) & 0x10) ? AF : 0);
      if (trace)
	printf (w ? "->%04x " : "->%02x ", z);
      put_ (p, z, w);
      break;
    case 1:
      z = x | y;
      *fl = 0;
      *fl |=
	((z & (w ? 0x8000 : 0x80)) ? SF : 0) | ((z & (w ? 0xffff : 0xff)) ==
						0 ? ZF : 0);
      if (trace)
	printf (w ? "->%04x " : "->%02x ", z);
      put_ (p, z, w);
      break;
    case 2:
      x += (*fl & CF);
      z = x + y;
      *fl = 0;
      *fl |=
	((z & (w ? 0x8000 : 0x80)) ? SF : 0) | ((z & (w ? 0xffff : 0xff)) ==
						0 ? ZF : 0);
      *fl |=
	((z & (w ? 0xffff0000 : 0xff00)) ? CF : 0) |
	(((z ^ x) & (z ^ y) & (w ? 0x8000 : 0x80)) ? OF : 0) |
	(((x ^ y ^ z) & 0x10) ? AF : 0);
      if (trace)
	printf (w ? "->%04x " : "->%02x ", z);
      put_ (p, z, w);
      break;
    case 3:
      d ? y += *fl & CF : (x += *fl & CF);
      z = d ? x - y : y - x;
      *fl = 0;
      *fl |=
	((z & (w ? 0x8000 : 0x80)) ? SF : 0) | ((z & (w ? 0xffff : 0xff)) ==
						0 ? ZF : 0);
      *fl |=
	((z & (w ? 0xffff0000 : 0xff00)) ? CF : 0) |
	(((z ^ x) & (z ^ y) & (w ? 0x8000 : 0x80)) ? OF : 0) |
	(((x ^ y ^ z) & 0x10) ? AF : 0);
      if (trace)
	printf (w ? "->%04x " : "->%02x ", z);
      put_ (p, z, w);
      break;
    case 4:
      z = x & y;
      *fl = 0;
      *fl |=
	((z & (w ? 0x8000 : 0x80)) ? SF : 0) | ((z & (w ? 0xffff : 0xff)) ==
						0 ? ZF : 0);
      if (trace)
	printf (w ? "->%04x " : "->%02x ", z);
      put_ (p, z, w);
      break;
    case 5:
      z = d ? x - y : y - x;
      *fl = 0;
      *fl |=
	((z & (w ? 0x8000 : 0x80)) ? SF : 0) | ((z & (w ? 0xffff : 0xff)) ==
						0 ? ZF : 0);
      *fl |=
	((z & (w ? 0xffff0000 : 0xff00)) ? CF : 0) |
	(((z ^ x) & (z ^ y) & (w ? 0x8000 : 0x80)) ? OF : 0) |
	(((x ^ y ^ z) & 0x10) ? AF : 0);
      if (trace)
	printf (w ? "->%04x " : "->%02x ", z);
      put_ (p, z, w);
      break;
    case 6:
      z = x ^ y;
      *fl = 0;
      *fl |=
	((z & (w ? 0x8000 : 0x80)) ? SF : 0) | ((z & (w ? 0xffff : 0xff)) ==
						0 ? ZF : 0);
      if (trace)
	printf (w ? "->%04x " : "->%02x ", z);
      put_ (p, z, w);
      break;
    case 7:
      p = null;
      z = d ? x - y : y - x;
      *fl = 0;
      *fl |=
	((z & (w ? 0x8000 : 0x80)) ? SF : 0) | ((z & (w ? 0xffff : 0xff)) ==
						0 ? ZF : 0);
      *fl |=
	((z & (w ? 0xffff0000 : 0xff00)) ? CF : 0) |
	(((z ^ x) & (z ^ y) & (w ? 0x8000 : 0x80)) ? OF : 0) |
	(((x ^ y ^ z) & 0x10) ? AF : 0);
      if (trace)
	printf (w ? "->%04x " : "->%02x ", z);
      put_ (p, z, w);
      break;
    };
}

void
testb ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  rm r = mrm (fetchb ());
  x = decreg (r.reg, w);
  y = decrm (r, w);
  if (trace > 1)
    {
      printf ("x:%d\n", x);
      printf ("y:%d\n", y);
    }
  p = d ? (void *) x : (void *) y;
  x = get_ ((void *) x, w);
  y = get_ ((void *) y, w);
  if (trace)
    {
      printf ("x:%d\n", x);
      printf ("y:%d\n", y);
    }
  z = x & y;
  *fl = 0;
  *fl |=
    ((z & (w ? 0x8000 : 0x80)) ? SF : 0) | ((z & (w ? 0xffff : 0xff)) ==
					    0 ? ZF : 0);
  *fl |=
    ((z & (w ? 0xffff0000 : 0xff00)) ? CF : 0) |
    (((z ^ x) & (z ^ y) & (w ? 0x8000 : 0x80)) ? OF : 0) |
    (((x ^ y ^ z) & 0x10) ? AF : 0);;
}

void
testw ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  rm r = mrm (fetchb ());
  x = decreg (r.reg, w);
  y = decrm (r, w);
  if (trace > 1)
    {
      printf ("x:%d\n", x);
      printf ("y:%d\n", y);
    }
  p = d ? (void *) x : (void *) y;
  x = get_ ((void *) x, w);
  y = get_ ((void *) y, w);
  if (trace)
    {
      printf ("x:%d\n", x);
      printf ("y:%d\n", y);
    }
  z = x & y;
  *fl = 0;
  *fl |=
    ((z & (w ? 0x8000 : 0x80)) ? SF : 0) | ((z & (w ? 0xffff : 0xff)) ==
					    0 ? ZF : 0);
  *fl |=
    ((z & (w ? 0xffff0000 : 0xff00)) ? CF : 0) |
    (((z ^ x) & (z ^ y) & (w ? 0x8000 : 0x80)) ? OF : 0) |
    (((x ^ y ^ z) & 0x10) ? AF : 0);;
}

void
xchgb ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  rm r = mrm (fetchb ());
  x = decreg (r.reg, w);
  y = decrm (r, w);
  if (trace > 1)
    {
      printf ("x:%d\n", x);
      printf ("y:%d\n", y);
    }
  p = d ? (void *) x : (void *) y;
  f = x;
  z = y;
  x = get_ ((void *) x, w);
  y = get_ ((void *) y, w);
  if (trace)
    {
      printf ("x:%d\n", x);
      printf ("y:%d\n", y);
    }
  if (w)
    {
      *(US *) f = y;
      *(US *) z = x;
    }
  else
    {
      *(UC *) f = y;
      *(UC *) z = x;
    };
}

void
xchgw ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  rm r = mrm (fetchb ());
  x = decreg (r.reg, w);
  y = decrm (r, w);
  if (trace > 1)
    {
      printf ("x:%d\n", x);
      printf ("y:%d\n", y);
    }
  p = d ? (void *) x : (void *) y;
  f = x;
  z = y;
  x = get_ ((void *) x, w);
  y = get_ ((void *) y, w);
  if (trace)
    {
      printf ("x:%d\n", x);
      printf ("y:%d\n", y);
    }
  if (w)
    {
      *(US *) f = y;
      *(US *) z = x;
    }
  else
    {
      *(UC *) f = y;
      *(UC *) z = x;
    };
}

void
movbf ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  rm r = mrm (fetchb ());
  x = decreg (r.reg, w);
  y = decrm (r, w);
  if (trace > 1)
    {
      printf ("x:%d\n", x);
      printf ("y:%d\n", y);
    }
  p = d ? (void *) x : (void *) y;
  x = get_ ((void *) x, w);
  y = get_ ((void *) y, w);
  if (trace)
    {
      printf ("x:%d\n", x);
      printf ("y:%d\n", y);
    }
  z = d ? y : x;
  if (trace)
    printf (w ? "->%04x " : "->%02x ", z);
  put_ (p, z, w);;
}

void
movwf ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  rm r = mrm (fetchb ());
  x = decreg (r.reg, w);
  y = decrm (r, w);
  if (trace > 1)
    {
      printf ("x:%d\n", x);
      printf ("y:%d\n", y);
    }
  p = d ? (void *) x : (void *) y;
  x = get_ ((void *) x, w);
  y = get_ ((void *) y, w);
  if (trace)
    {
      printf ("x:%d\n", x);
      printf ("y:%d\n", y);
    }
  z = d ? y : x;
  if (trace)
    printf (w ? "->%04x " : "->%02x ", z);
  put_ (p, z, w);;
}

void
movbt ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  rm r = mrm (fetchb ());
  x = decreg (r.reg, w);
  y = decrm (r, w);
  if (trace > 1)
    {
      printf ("x:%d\n", x);
      printf ("y:%d\n", y);
    }
  p = d ? (void *) x : (void *) y;
  x = get_ ((void *) x, w);
  y = get_ ((void *) y, w);
  if (trace)
    {
      printf ("x:%d\n", x);
      printf ("y:%d\n", y);
    }
  z = d ? y : x;
  if (trace)
    printf (w ? "->%04x " : "->%02x ", z);
  put_ (p, z, w);;
}

void
movwt ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  rm r = mrm (fetchb ());
  x = decreg (r.reg, w);
  y = decrm (r, w);
  if (trace > 1)
    {
      printf ("x:%d\n", x);
      printf ("y:%d\n", y);
    }
  p = d ? (void *) x : (void *) y;
  x = get_ ((void *) x, w);
  y = get_ ((void *) y, w);
  if (trace)
    {
      printf ("x:%d\n", x);
      printf ("y:%d\n", y);
    }
  z = d ? y : x;
  if (trace)
    printf (w ? "->%04x " : "->%02x ", z);
  put_ (p, z, w);;
}

void
movsegf ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  rm r = mrm (fetchb ());
  x = decreg (r.reg, w);
  y = decrm (r, w);
  if (trace > 1)
    {
      printf ("x:%d\n", x);
      printf ("y:%d\n", y);
    }
  p = d ? (void *) x : (void *) y;
  x = get_ ((void *) x, w);
  y = get_ ((void *) y, w);
  if (trace)
    {
      printf ("x:%d\n", x);
      printf ("y:%d\n", y);
    };
}

void
lea ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  rm r = mrm (fetchb ());
  x = decreg (r.reg, w);
  y = decrm (r, w);
  if (trace > 1)
    {
      printf ("x:%d\n", x);
      printf ("y:%d\n", y);
    }
  p = d ? (void *) x : (void *) y;
  z = ((UC *) y) - mem;
  if (trace)
    printf (w ? "->%04x " : "->%02x ", z);
  put_ (p, z, w);;
}

void
movsegt ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  rm r = mrm (fetchb ());
  x = decreg (r.reg, w);
  y = decrm (r, w);
  if (trace > 1)
    {
      printf ("x:%d\n", x);
      printf ("y:%d\n", y);
    }
  p = d ? (void *) x : (void *) y;
  x = get_ ((void *) x, w);
  y = get_ ((void *) y, w);
  if (trace)
    {
      printf ("x:%d\n", x);
      printf ("y:%d\n", y);
    };
}

void
poprm ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  rm r = mrm (fetchb ());
  x = decreg (r.reg, w);
  y = decrm (r, w);
  if (trace > 1)
    {
      printf ("x:%d\n", x);
      printf ("y:%d\n", y);
    }
  p = d ? (void *) x : (void *) y;
  x = get_ ((void *) x, w);
  y = get_ ((void *) y, w);
  if (trace)
    {
      printf ("x:%d\n", x);
      printf ("y:%d\n", y);
    }
  *((US *) p) = get_ (mem + (*sp += 2) - 2, 1);
}

void
nopH ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;;
}

void
xchgac ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  x = (U) ax;
  y = (U) (cx);
  w = 1;
  f = x;
  z = y;
  x = get_ ((void *) x, w);
  y = get_ ((void *) y, w);
  if (trace)
    {
      printf ("x:%d\n", x);
      printf ("y:%d\n", y);
    }
  if (w)
    {
      *(US *) f = y;
      *(US *) z = x;
    }
  else
    {
      *(UC *) f = y;
      *(UC *) z = x;
    };
}

void
xchgad ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  x = (U) ax;
  y = (U) (dx);
  w = 1;
  f = x;
  z = y;
  x = get_ ((void *) x, w);
  y = get_ ((void *) y, w);
  if (trace)
    {
      printf ("x:%d\n", x);
      printf ("y:%d\n", y);
    }
  if (w)
    {
      *(US *) f = y;
      *(US *) z = x;
    }
  else
    {
      *(UC *) f = y;
      *(UC *) z = x;
    };
}

void
xchgab ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  x = (U) ax;
  y = (U) (bx);
  w = 1;
  f = x;
  z = y;
  x = get_ ((void *) x, w);
  y = get_ ((void *) y, w);
  if (trace)
    {
      printf ("x:%d\n", x);
      printf ("y:%d\n", y);
    }
  if (w)
    {
      *(US *) f = y;
      *(US *) z = x;
    }
  else
    {
      *(UC *) f = y;
      *(UC *) z = x;
    };
}

void
xchgasp ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  x = (U) ax;
  y = (U) (sp);
  w = 1;
  f = x;
  z = y;
  x = get_ ((void *) x, w);
  y = get_ ((void *) y, w);
  if (trace)
    {
      printf ("x:%d\n", x);
      printf ("y:%d\n", y);
    }
  if (w)
    {
      *(US *) f = y;
      *(US *) z = x;
    }
  else
    {
      *(UC *) f = y;
      *(UC *) z = x;
    };
}

void
xchabp ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  x = (U) ax;
  y = (U) (bp);
  w = 1;
  f = x;
  z = y;
  x = get_ ((void *) x, w);
  y = get_ ((void *) y, w);
  if (trace)
    {
      printf ("x:%d\n", x);
      printf ("y:%d\n", y);
    }
  if (w)
    {
      *(US *) f = y;
      *(US *) z = x;
    }
  else
    {
      *(UC *) f = y;
      *(UC *) z = x;
    };
}

void
xchgasi ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  x = (U) ax;
  y = (U) (si);
  w = 1;
  f = x;
  z = y;
  x = get_ ((void *) x, w);
  y = get_ ((void *) y, w);
  if (trace)
    {
      printf ("x:%d\n", x);
      printf ("y:%d\n", y);
    }
  if (w)
    {
      *(US *) f = y;
      *(US *) z = x;
    }
  else
    {
      *(UC *) f = y;
      *(UC *) z = x;
    };
}

void
xchadi ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  x = (U) ax;
  y = (U) (di);
  w = 1;
  f = x;
  z = y;
  x = get_ ((void *) x, w);
  y = get_ ((void *) y, w);
  if (trace)
    {
      printf ("x:%d\n", x);
      printf ("y:%d\n", y);
    }
  if (w)
    {
      *(US *) f = y;
      *(US *) z = x;
    }
  else
    {
      *(UC *) f = y;
      *(UC *) z = x;
    };
}

void
cbw ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  *ax = (S) (C) * al;;
}

void
cwd ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  z = (I) (S) * ax;
  *dx = z >> 16;;
}

void
farcall ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;;
}

void
wait ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;;
}

void
pushf ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  put_ (mem + (*sp -= 2), *(fl), 1);
}

void
popf ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  *(fl) = get_ (mem + (*sp += 2) - 2, 1);
}

void
sahf ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;;
}

void
lahf ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;;
}

void
movalb ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  if (d)
    {
      x = get_ (mem + fetchw (), w);
      if (w)
	*ax = x;
      else
	*al = x;
    }
  else
    {
      put_ (mem + fetchw (), w ? *ax : *al, w);
    };
}

void
movaxw ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  if (d)
    {
      x = get_ (mem + fetchw (), w);
      if (w)
	*ax = x;
      else
	*al = x;
    }
  else
    {
      put_ (mem + fetchw (), w ? *ax : *al, w);
    };
}

void
movbal ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  if (d)
    {
      x = get_ (mem + fetchw (), w);
      if (w)
	*ax = x;
      else
	*al = x;
    }
  else
    {
      put_ (mem + fetchw (), w ? *ax : *al, w);
    };
}

void
movwax ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  if (d)
    {
      x = get_ (mem + fetchw (), w);
      if (w)
	*ax = x;
      else
	*al = x;
    }
  else
    {
      put_ (mem + fetchw (), w ? *ax : *al, w);
    };
}

void
movsb ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;;
}

void
movsw ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;;
}

void
cmpsb ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;;
}

void
cmpsw ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;;
}

void
testaib ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  x = (U) (p = w ? (UC *) ax : al);
  x = get_ ((void *) x, w);
  y = w ? fetchw () : fetchb ();
  z = x & y;
  *fl = 0;
  *fl |=
    ((z & (w ? 0x8000 : 0x80)) ? SF : 0) | ((z & (w ? 0xffff : 0xff)) ==
					    0 ? ZF : 0);
  *fl |=
    ((z & (w ? 0xffff0000 : 0xff00)) ? CF : 0) |
    (((z ^ x) & (z ^ y) & (w ? 0x8000 : 0x80)) ? OF : 0) |
    (((x ^ y ^ z) & 0x10) ? AF : 0);;
} void

testaiw ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  x = (U) (p = w ? (UC *) ax : al);
  x = get_ ((void *) x, w);
  y = w ? fetchw () : fetchb ();
  z = x & y;
  *fl = 0;
  *fl |=
    ((z & (w ? 0x8000 : 0x80)) ? SF : 0) | ((z & (w ? 0xffff : 0xff)) ==
					    0 ? ZF : 0);
  *fl |=
    ((z & (w ? 0xffff0000 : 0xff00)) ? CF : 0) |
    (((z ^ x) & (z ^ y) & (w ? 0x8000 : 0x80)) ? OF : 0) |
    (((x ^ y ^ z) & 0x10) ? AF : 0);;
} void

stosb ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;;
}

void
stosw ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;;
}

void
lodsb ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;;
}

void
lodsw ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;;
}

void
scasb ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;;
}

void
scasw ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;;
}

void
movali ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  (*al) = fetchb ();;
}

void
movcli ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  (*cl) = fetchb ();;
}

void
movdli ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  (*dl) = fetchb ();;
}

void
movbli ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  (*bl) = fetchb ();;
}

void
movahi ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  (*ah) = fetchb ();;
}

void
movchi ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  (*ch) = fetchb ();;
}

void
movdhi ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  (*dh) = fetchb ();;
}

void
movbhi ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  (*bh) = fetchb ();;
}

void
movaxi ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  (*ax) = fetchw ();;
}

void
movcxi ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  (*cx) = fetchw ();;
}

void
movdxi ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  (*dx) = fetchw ();;
}

void
movbxi ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  (*bx) = fetchw ();;
}

void
movspi ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  (*sp) = fetchw ();;
}

void
movbpi ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  (*bp) = fetchw ();;
}

void
movsii ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  (*si) = fetchw ();;
}

void
movdii ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  (*di) = fetchw ();;
}

void
nopI ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;;
}

void
nopJ ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;;
}

void
reti ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  *(ip) = get_ (mem + (*sp += 2) - 2, 1);
  if (fetchw ())
    *sp += fetchw () * 2;;
}

void
retz ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  *(ip) = get_ (mem + (*sp += 2) - 2, 1);
  if (0)
    *sp += 0 * 2;;
}

void
les ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;;
}

void
lds ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;;
}

void
movimb ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  rm r = mrm (fetchb ());
  x = decreg (r.reg, w);
  y = decrm (r, w);
  if (trace > 1)
    {
      printf ("x:%d\n", x);
      printf ("y:%d\n", y);
    }
  p = d ? (void *) x : (void *) y;
  if (w)
    {
      (*(US *) y) = fetchw ();
    }
  else
    {
      (*(UC *) y) = fetchb ();
    };
}

void
movimw ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  rm r = mrm (fetchb ());
  x = decreg (r.reg, w);
  y = decrm (r, w);
  if (trace > 1)
    {
      printf ("x:%d\n", x);
      printf ("y:%d\n", y);
    }
  p = d ? (void *) x : (void *) y;
  if (w)
    {
      (*(US *) y) = fetchw ();
    }
  else
    {
      (*(UC *) y) = fetchb ();
    };
}

void
nopK ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;;
}

void
nopL ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;;
}

void
freti ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  *(cs) = get_ (mem + (*sp += 2) - 2, 1);
  *(ip) = get_ (mem + (*sp += 2) - 2, 1);
  if (fetchw ())
    *sp += fetchw () * 2;;
}

void
fretz ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  *(cs) = get_ (mem + (*sp += 2) - 2, 1);
  *(ip) = get_ (mem + (*sp += 2) - 2, 1);
  if (0)
    *sp += 0 * 2;;
}

void
int3 ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;;
}

void
inti ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;;
}

void
int0 ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;;
}

void
iret ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;;
}

void
shiftb ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;;
}

void
shiftw ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;;
}

void
shiftbv ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;;
}

void
shiftwv ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;;
}

void
aam ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;;
}

void
aad ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;;
}

void
nopM ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;;
}

void
xlat ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;;
}

void
esc0 ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;;
}

void
esc1 ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;;
}

void
esc2 ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;;
}

void
esc3 ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;;
}

void
esc4 ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;;
}

void
esc5 ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;;
}

void
esc6 ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;;
}

void
esc7 ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;;
}

void
loopnz ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;;
}

void
loopz ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;;
}

void
loop ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;;
}

void
jcxz ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;;
}

void
inb ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;;
}

void
inw ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;;
}

void
outb ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;;
}

void
outw ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;;
}

void
call ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  w = 1;
  x = w ? fetchw () : (S) (C) fetchb ();
  put_ (mem + (*sp -= 2), *(ip), 1);
  (*ip) += (S) x;;
}

void
jmp ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  x = fetchw ();
  *ip += (S) x;;
}

void
farjmp ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;;
}

void
sjmp ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  x = (S) (C) fetchb ();
  *ip += (S) x;;
}

void
invb ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;;
}

void
invw ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;;
}

void
outvb ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;;
}

void
outvw ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;;
}

void
lock ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;;
}

void
nopN ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;;
}

void
rep ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;;
}

void
repz ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;;
}

void
hlt ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  halt = 1;
}

void
cmc ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  *fl = (*fl & ~CF) | ((*fl & CF) ^ 1);;
}

void
grp1b ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;;
}

void
grp1w ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;;
}

void
clc ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  *fl = *fl & ~CF;;
}

void
stc ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  *fl = *fl | CF;;
}

void
cli ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;;
}

void
sti ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;;
}

void
cld ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;;
}

void
std ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;;
}

void
grp2b ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  rm r = mrm (fetchb ());
  y = decrm (r, w);
  if (trace)
    printf ("%s ", (C *[])
	    {
	    "INC", "DEC", "CALL", "CALL", "JMP", "JMP", "PUSH"}
	    [r.reg]);
  switch (r.reg)
    {
    case 0:
      w = 1;
      d = 1;
      p = (V *) (S *) y;
      x = (S) * (S *) y;
      y = 1;
      f = *fl & CF;
      z = x + y;
      *fl = 0;
      *fl |=
	((z & (w ? 0x8000 : 0x80)) ? SF : 0) | ((z & (w ? 0xffff : 0xff)) ==
						0 ? ZF : 0);
      *fl |=
	((z & (w ? 0xffff0000 : 0xff00)) ? CF : 0) |
	(((z ^ x) & (z ^ y) & (w ? 0x8000 : 0x80)) ? OF : 0) |
	(((x ^ y ^ z) & 0x10) ? AF : 0);
      if (trace)
	printf (w ? "->%04x " : "->%02x ", z);
      put_ (p, z, w);
      *fl = (*fl & ~CF) | f;;
      break;
    case 1:
      w = 1;
      d = 1;
      p = (V *) (S *) y;
      x = (S) * (S *) y;
      y = 1;
      f = *fl & CF;
      z = d ? x - y : y - x;
      *fl = 0;
      *fl |=
	((z & (w ? 0x8000 : 0x80)) ? SF : 0) | ((z & (w ? 0xffff : 0xff)) ==
						0 ? ZF : 0);
      *fl |=
	((z & (w ? 0xffff0000 : 0xff00)) ? CF : 0) |
	(((z ^ x) & (z ^ y) & (w ? 0x8000 : 0x80)) ? OF : 0) |
	(((x ^ y ^ z) & 0x10) ? AF : 0);
      if (trace)
	printf (w ? "->%04x " : "->%02x ", z);
      put_ (p, z, w);
      *fl = (*fl & ~CF) | f;;
      break;
    case 2:
      x = w ? fetchw () : (S) (C) fetchb ();
      put_ (mem + (*sp -= 2), *(ip), 1);
      (*ip) += (S) x;;
      break;
    case 3:
      x = w ? fetchw () : (S) (C) fetchb ();
      put_ (mem + (*sp -= 2), *(ip), 1);
      (*ip) += (S) x;;
      break;
    case 4:
      *ip += (S) y;
      break;
    case 5:
      x = fetchw ();
      *ip += (S) x;;
      break;
    case 6:
      put_ (mem + (*sp -= 2), *((S *) y), 1);
      break;
    };
}

void
grp2w ()
{
  if (trace)
    {
      printf ("%s:\n", __func__);
    }
  d = ! !(o & 2);
  w = o & 1;
  rm r = mrm (fetchb ());
  y = decrm (r, w);
  if (trace)
    printf ("%s ", (C *[])
	    {
	    "INC", "DEC", "CALL", "CALL", "JMP", "JMP", "PUSH"}
	    [r.reg]);
  switch (r.reg)
    {
    case 0:
      w = 1;
      d = 1;
      p = (V *) (S *) y;
      x = (S) * (S *) y;
      y = 1;
      f = *fl & CF;
      z = x + y;
      *fl = 0;
      *fl |=
	((z & (w ? 0x8000 : 0x80)) ? SF : 0) | ((z & (w ? 0xffff : 0xff)) ==
						0 ? ZF : 0);
      *fl |=
	((z & (w ? 0xffff0000 : 0xff00)) ? CF : 0) |
	(((z ^ x) & (z ^ y) & (w ? 0x8000 : 0x80)) ? OF : 0) |
	(((x ^ y ^ z) & 0x10) ? AF : 0);
      if (trace)
	printf (w ? "->%04x " : "->%02x ", z);
      put_ (p, z, w);
      *fl = (*fl & ~CF) | f;;
      break;
    case 1:
      w = 1;
      d = 1;
      p = (V *) (S *) y;
      x = (S) * (S *) y;
      y = 1;
      f = *fl & CF;
      z = d ? x - y : y - x;
      *fl = 0;
      *fl |=
	((z & (w ? 0x8000 : 0x80)) ? SF : 0) | ((z & (w ? 0xffff : 0xff)) ==
						0 ? ZF : 0);
      *fl |=
	((z & (w ? 0xffff0000 : 0xff00)) ? CF : 0) |
	(((z ^ x) & (z ^ y) & (w ? 0x8000 : 0x80)) ? OF : 0) |
	(((x ^ y ^ z) & 0x10) ? AF : 0);
      if (trace)
	printf (w ? "->%04x " : "->%02x ", z);
      put_ (p, z, w);
      *fl = (*fl & ~CF) | f;;
      break;
    case 2:
      x = w ? fetchw () : (S) (C) fetchb ();
      put_ (mem + (*sp -= 2), *(ip), 1);
      (*ip) += (S) x;;
      break;
    case 3:
      x = w ? fetchw () : (S) (C) fetchb ();
      put_ (mem + (*sp -= 2), *(ip), 1);
      (*ip) += (S) x;;
      break;
    case 4:
      *ip += (S) y;
      break;
    case 5:
      x = fetchw ();
      *ip += (S) x;;
      break;
    case 6:
      put_ (mem + (*sp -= 2), *((S *) y), 1);
      break;
    };
}

void (*tab[]) () =
{
addbf, addwf, addbt, addwt, addbi, addwi, pushes, popes, orbf, orwf, orbt,
    orwt, orbi, orwi, pushcs, nop0, adcbf, adcwf, adcbt, adcwt, adcbi,
    adcwi, pushss, popss, sbbbf, sbbwf, sbbbt, sbbwt, sbbbi, sbbwi, pushds,
    popds, andbf, andwf, andbt, andwt, andbi, andwi, esseg, daa, subbf,
    subwf, subbt, subwt, subbi, subwi, csseg, das, xorbf, xorwf, xorbt,
    xorwt, xorbi, xorwi, ssseg, aaa, cmpbf, cmpwf, cmpbt, cmpwt, cmpbi,
    cmpwi, dsseg, aas, incax, inccx, incdx, incbx, incsp, incbp, incsi,
    incdi, decax, deccx, decdx, decbx, decsp, decbp, decsi, decdi, pushax,
    pushcx, pushdx, pushbx, pushsp, pushbp, pushsi, pushdi, popax, popcx,
    popdx, popbx, popsp, popbp, popsi, popdi, nop1, nop2, nop3, nop4, nop5,
    nop6, nop7, nop8, nop9, nopA, nopB, nopC, nopD, nopE, nopF, nopG, jo,
    jno, jb, jnb, jz, jnz, jbe, jnbe, js, jns, jp, jnp, jl, jnl_, jle, jnle,
    immb, immw, immb1, immis, testb, testw, xchgb, xchgw, movbf, movwf,
    movbt, movwt, movsegf, lea, movsegt, poprm, nopH, xchgac, xchgad,
    xchgab, xchgasp, xchabp, xchgasi, xchadi, cbw, cwd, farcall, wait,
    pushf, popf, sahf, lahf, movalb, movaxw, movbal, movwax, movsb, movsw,
    cmpsb, cmpsw, testaib, testaiw, stosb, stosw, lodsb, lodsw, scasb,
    scasw, movali, movcli, movdli, movbli, movahi, movchi, movdhi, movbhi,
    movaxi, movcxi, movdxi, movbxi, movspi, movbpi, movsii, movdii, nopI,
    nopJ, reti, retz, les, lds, movimb, movimw, nopK, nopL, freti, fretz,
    int3, inti, int0, iret, shiftb, shiftw, shiftbv, shiftwv, aam, aad,
    nopM, xlat, esc0, esc1, esc2, esc3, esc4, esc5, esc6, esc7, loopnz,
    loopz, loop, jcxz, inb, inw, outb, outw, call, jmp, farjmp, sjmp, invb,
    invw, outvb, outvw, lock, nopN, rep, repz, hlt, cmc, grp1b, grp1w, clc,
    stc, cli, sti, cld, std, grp2b, grp2w,};
V
clean (C * s)
{
  I i;
  for (i = 0; i < 80; i++)
    if (!isprint (s[i]))
      s[i] = ' ';
}

V
video ()
{
  I i;
  C buf[81] = "";
  for (i = 0; i < 28; i++)
    memcpy (buf, mem + 0x8000 + i * 80, 80),
      clean (buf), printf ("\n%s", buf);
  printf ("\n");
}

static I ct;
V
run ()
{
  while (!halt)
    {
      if (trace)
	dump ();
      if (!ct--)
	{
	  ct = 10;
	  video ();
	}
      tab[o = fetchb ()]();
    }
}

V
dbg ()
{
  while (!halt)
    {
      C c;
      if (!ct--)
	{
	  ct = 10;
	  video ();
	}
      if (trace)
	dump ();
      fgetc (stdin);
      tab[o = fetchb ()]();
    }
}

I
load (C * f)
{
  struct stat s;
  FILE *fp;
  return (fp = fopen (f, "rb"))
    && fstat (fileno (fp), &s) || fread (mem, s.st_size, 1, fp);
}

I
main (I c, C ** v)
{
  init ();
  if (c > 1)
    {
      load (v[1]);
    }
  *sp = 0x100;
  if (debug)
    dbg ();
  else
    run ();
  video ();
  return 0;
}
