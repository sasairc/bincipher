CC        = cc
RM        = rm
CFLAGS    = -O2 -pipe
DEFCFLAGS = -lm -lncipher

all: bin2cipher cipher2bin

bin2cipher: bin2cipher.c
	$(CC) $(CFLAGS) $^ -o $@ $(DEFCFLAGS)

cipher2bin: cipher2bin.c misc.c
	$(CC) $(CFLAGS) $^  -o $@ $(DEFCFLAGS)

clean:
	-$(RM) -f cipher2bin
	-$(RM) -f bin2cipher

.PHONY: all		\
	bin2cipher	\
	cipher2bin	\
	clean
