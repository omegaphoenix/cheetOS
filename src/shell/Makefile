CFLAGS = -Wall -g

OBJS = mysh.o
SOURCES = builtin.c tokenizer.c commandStruct.c tokenStruct.c

all: mysh

mysh: $(OBJS)
	$(CC) $(CFLAGS) $^ $(SOURCES) -o $@ $(LDFLAGS)

clean:
	rm -f *.o *~ mysh

.PHONY: all clean

