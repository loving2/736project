
INCDIR=.
CC=gcc
CFLAGS=-I$(INCDIR) -O2 -std=c99 -lrt
LIBS=

DEPS = frame.h readwrite.h cobs.h rs485uart.h
OBJ = frame.o readwrite.o cobs.o rs485uart.o

BIN = frame_test cobs_test readwrite_test serialtest echo chatter

all: $(BIN)

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(BIN): %: $(OBJ) %.o
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f *.o *~ core $(INCDIR)/*~ $(BIN)




