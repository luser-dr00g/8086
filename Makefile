#CFLAGS+= -Os
CFLAGS=-Wno-incompatible-pointer-types $(cflags)
SRC=a8086.c asm8086.h fdict.h forth3.h undefs.h cp437.h bios.h dos.h

a8086:$(SRC)
	$(CC) $(CFLAGS) -o $@ $< $(LDLIBS)

count:
	wc $(SRC)
	@echo `grep HEADLESS forth3.h | wc -l` HEADLESS words in forth3.h
	@echo `grep CODE forth3.h | wc -l` CODE words in forth3.h
	@echo `grep WORD forth3.h | wc -l` high level WORDs in forth3.h

a8086.pp:$(SRC)
	cpp -P $< >$@
	indent $@ 2>/dev/null
