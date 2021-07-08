#CFLAGS+= -Os
CFLAGS=-Wno-incompatible-pointer-types
SRC=a8086.c asm8086.h fdict.h forth3.h undefs.h cp437.h bios.h dos.h

a8086:$(SRC)
	$(CC) $(CFLAGS) -o $@ $< $(LDLIBS)

count:
	wc $(SRC)

a8086.pp:$(SRC)
	cpp -P $< >$@
	indent $@ 2>/dev/null
