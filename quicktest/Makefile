
INCDIR=.
CC=gcc
CFLAGS=-I$(INCDIR) -O2 -lm -lrt
LIBS=

DEPS = rnglib.h ranlib.h
OBJ = rnglib.o ranlib.o

BIN = quicktest

all: $(BIN)

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(BIN): %: $(OBJ) %.o
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f *.o *~ core $(INCDIR)/*~ $(BIN)




