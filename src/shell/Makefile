CFLAGS = -Wall -g

OBJS = mysh.o

all: mysh

mysh: $(OBJS)
	$(CC) $(CFLAGS) $^ builtin.c tokenizer.c -o $@ $(LDFLAGS)

clean:
	rm -f *.o *~ mysh

.PHONY: all clean

