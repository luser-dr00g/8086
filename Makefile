#CFLAGS+= -Os
SRC=a8086.c asm8086.h fdict.h forth3.h undefs.h

a8086:$(SRC)
	$(CC) -o $@ $(CFLAGS) $< $(LDLIBS)

count:
	wc $(SRC)
