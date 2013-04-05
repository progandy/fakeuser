CC = gcc

CFLAGS +=  -D_GNU_SOURCE -std=c99 -Wall

all: libfakeuser.so fakeadd

libfakeuser.so: fakeuser.c util.c
	$(CC)  $(CFLAGS) -shared -fPIC -o libfakeuser.so fakeuser.c util.c -ldl $(LDFLAGS)

fakeadd: fakeadd.c util.c
	$(CC) $(CFLAGS) -o fakeadd fakeadd.c util.c -ldl $(LDFLAGS)
