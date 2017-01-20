BINARIES = video-test

CFLAGS = -Wall


all: $(BINARIES)


video-test: video-test.o
	$(CC) $(CFLAGS) $^ ../video.c -o $@ $(LDFLAGS)


clean:
	rm -f *.o $(BINARIES) *~


.PHONY: all clean

