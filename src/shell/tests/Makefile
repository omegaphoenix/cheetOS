BINARIES = outerr sleepycat cattysleep tokenizer-test builtin-test command-struct-test

CFLAGS = -Wall


all: $(BINARIES)


outerr: outerr.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

sleepycat: sleepycat.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

cattysleep: cattysleep.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

tokenizer-test: tokenizer-test.o
	$(CC) $(CFLAGS) $^ ../tokenizer.c -o $@ $(LDFLAGS)

builtin-test: builtin-test.o
	$(CC) $(CFLAGS) $^ ../builtin.c -o $@ $(LDFLAGS)

command-struct-test: command-struct-test.o
	$(CC) $(CFLAGS) $^ ../commandStruct.c -o $@ $(LDFLAGS)

clean:
	rm -f *.o $(BINARIES) *~


.PHONY: all clean

